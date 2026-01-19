/*
  ==============================================================================
    VT-2B Black - EMU AUDIO
    Console Bus Glue Processor

    "音を一つにする回路"

    設計思想:
    - 非線形は控えめ、サチュレーションより「密度増加」
    - 音をまとめる方向に作用
    - バス・マスターで常時挿しておけるプロ仕様
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
// 定数 - 効果を分かりやすくするために強化
namespace VT2BConstants {
// サチュレーション - 効果を強く（ユーザー要望により大幅強化）
constexpr float kSaturationCoeffMin = 0.0f;
constexpr float kSaturationCoeffMax = 3.0f; // 0.8 -> 3.0
constexpr float kSaturationCurve = 2.5f; // 1.5 -> 2.5 硬めのカーブで質感を出す

// 倍音生成 - より明確に
constexpr float kHarmonic2ndAmount = 0.40f; // 0.15 -> 0.40 (暖かさ)
constexpr float kHarmonic3rdAmount = 0.25f; // 0.08 -> 0.25 (エッジ)

// トランジェント - 音のアタック感を強調
constexpr float kTransientThreshold = 0.2f;
constexpr float kTransientKnee = 0.15f;
constexpr float kTransientAmountMin = 0.08f;
constexpr float kTransientAmountMax = 0.50f; // 0.25 -> 0.50
constexpr float kEnvelopeAttack = 0.001f;
constexpr float kEnvelopeRelease = 0.050f;

// オールパス（位相安定化）
constexpr float kAllpassFrequency = 80.0f; // Hz

// パラメータ範囲
constexpr float kDriveMin = 0.0f;
constexpr float kDriveMax = 10.0f;
constexpr float kDriveDefault = 0.0f;
constexpr float kMixMin = 0.0f;
constexpr float kMixMax = 100.0f;
constexpr float kMixDefault = 100.0f;
} // namespace VT2BConstants

//==============================================================================
VT2BBlackProcessor::VT2BBlackProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("VT2BBlack"),
                 createParameterLayout()) {
  driveParameter = parameters.getRawParameterValue("drive");
  mixParameter = parameters.getRawParameterValue("mix");
}

VT2BBlackProcessor::~VT2BBlackProcessor() {}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
VT2BBlackProcessor::createParameterLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  // Drive パラメータ
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"drive", 1}, "Drive",
      juce::NormalisableRange<float>(VT2BConstants::kDriveMin,
                                     VT2BConstants::kDriveMax, 0.1f),
      VT2BConstants::kDriveDefault,
      juce::AudioParameterFloatAttributes().withLabel("Drive")));

  // Mix パラメータ
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      juce::ParameterID{"mix", 1}, "Mix",
      juce::NormalisableRange<float>(VT2BConstants::kMixMin,
                                     VT2BConstants::kMixMax, 1.0f),
      VT2BConstants::kMixDefault,
      juce::AudioParameterFloatAttributes().withLabel("%")));

  return {params.begin(), params.end()};
}

//==============================================================================
const juce::String VT2BBlackProcessor::getName() const {
  return JucePlugin_Name;
}

bool VT2BBlackProcessor::acceptsMidi() const { return false; }
bool VT2BBlackProcessor::producesMidi() const { return false; }
bool VT2BBlackProcessor::isMidiEffect() const { return false; }
double VT2BBlackProcessor::getTailLengthSeconds() const { return 0.0; }

int VT2BBlackProcessor::getNumPrograms() { return 1; }
int VT2BBlackProcessor::getCurrentProgram() { return 0; }
void VT2BBlackProcessor::setCurrentProgram(int) {}
const juce::String VT2BBlackProcessor::getProgramName(int) { return {}; }
void VT2BBlackProcessor::changeProgramName(int, const juce::String &) {}

//==============================================================================
void VT2BBlackProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  currentSampleRate = sampleRate;

  // スムージング設定（ジッパーノイズ防止）
  smoothedDrive.reset(sampleRate, 0.02); // 20ms
  smoothedMix.reset(sampleRate, 0.02);

  // 状態リセット
  envelopeL = 0.0f;
  envelopeR = 0.0f;
  allpassStateL = 0.0f;
  allpassStateR = 0.0f;
}

void VT2BBlackProcessor::releaseResources() {}

bool VT2BBlackProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;

  return true;
}

//==============================================================================
void VT2BBlackProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                      juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  juce::ignoreUnused(midiMessages);

  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // 未使用チャンネルをクリア
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // パラメータ取得
  float drive = *driveParameter;
  float mix = *mixParameter / 100.0f; // 0-1に正規化

  smoothedDrive.setTargetValue(drive);
  smoothedMix.setTargetValue(mix);

  // サンプル処理
  auto *channelDataL = buffer.getWritePointer(0);
  auto *channelDataR =
      totalNumInputChannels > 1 ? buffer.getWritePointer(1) : nullptr;

  for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
    float currentDrive = smoothedDrive.getNextValue();
    float currentMix = smoothedMix.getNextValue();

    // ドライ信号保存
    float dryL = channelDataL[sample];
    float dryR = channelDataR ? channelDataR[sample] : dryL;

    // 入力ブースト (Pre-Drive Gain)
    // Drive値に応じて入力を持ち上げ、サチュレーション回路を積極的にドライブさせる
    // Mixノブでのブレンド時に効果がはっきりわかるようにする
    float preDriveGain = 1.0f + (currentDrive / VT2BConstants::kDriveMax) *
                                    1.5f; // 最大+9dB程度までブースト

    // === 左チャンネル処理 ===
    float wetL = dryL * preDriveGain;

    // 1. サチュレーション（密度増加）
    wetL = processSaturation(wetL, currentDrive);

    // 2. 倍音生成
    wetL += processHarmonics(dryL * preDriveGain, currentDrive);

    // 3. トランジェント整形
    wetL = processTransient(wetL, envelopeL, currentDrive);

    // 4. 位相安定化 (Allpass) -> 廃止
    // 原音の位相・キャラクターを維持するため、位相シフトを行わない
    // wetL = processAllpass(wetL, allpassStateL);

    // 5. ゲイン補償
    wetL *= calculateMakeupGain(currentDrive);

    // === 右チャンネル処理 ===
    float wetR = dryR;

    if (channelDataR != nullptr) {
      wetR = dryR * preDriveGain;

      wetR = processSaturation(wetR, currentDrive);
      wetR += processHarmonics(dryR * preDriveGain, currentDrive);
      wetR = processTransient(wetR, envelopeR, currentDrive);
      // wetR = processAllpass(wetR, allpassStateR);
      wetR *= calculateMakeupGain(currentDrive);
    } else {
      wetR = wetL;
    }

    // Dry/Wet ミックス
    // 原音(Dry)に対してDrive効果(Wet)を加えるイメージ
    // 位相ズレがないため、綺麗にブレンドされる
    channelDataL[sample] = dryL * (1.0f - currentMix) + wetL * currentMix;

    if (channelDataR != nullptr)
      channelDataR[sample] = dryR * (1.0f - currentMix) + wetR * currentMix;
  }
}

//==============================================================================
// DSP処理関数実装

float VT2BBlackProcessor::processSaturation(float input, float drive) {
  // 密度増加型飽和特性: f(x) = x / (1 + k * |x|^n)
  // テープ系の柔らかい非線形

  float normalizedDrive = drive / VT2BConstants::kDriveMax;
  float k = VT2BConstants::kSaturationCoeffMin +
            normalizedDrive * (VT2BConstants::kSaturationCoeffMax -
                               VT2BConstants::kSaturationCoeffMin);

  float absInput = std::abs(input);
  float saturation = std::pow(absInput, VT2BConstants::kSaturationCurve);

  return input / (1.0f + k * saturation);
}

float VT2BBlackProcessor::processHarmonics(float input, float drive) {
  // 低次倍音の微量付加
  // 2次倍音（偶数）= 暖かさ、3次倍音（奇数）= 存在感

  float normalizedDrive = drive / VT2BConstants::kDriveMax;

  // 倍音量はDriveに比例（ただし控えめ）
  float harmonic2 =
      input * input * VT2BConstants::kHarmonic2ndAmount * normalizedDrive;
  float harmonic3 = input * input * input * VT2BConstants::kHarmonic3rdAmount *
                    normalizedDrive;

  // 偶数倍音は常に正、奇数倍音は符号を保持
  harmonic2 = (input >= 0.0f) ? harmonic2 : -harmonic2;

  return harmonic2 + harmonic3;
}

float VT2BBlackProcessor::processTransient(float input, float &envelope,
                                           float drive) {
  // エンベロープフォロワー
  float absInput = std::abs(input);

  float attackCoeff = 1.0f - std::exp(-1.0f / (float(currentSampleRate) *
                                               VT2BConstants::kEnvelopeAttack));
  float releaseCoeff =
      1.0f - std::exp(-1.0f / (float(currentSampleRate) *
                               VT2BConstants::kEnvelopeRelease));

  if (absInput > envelope)
    envelope = envelope + attackCoeff * (absInput - envelope);
  else
    envelope = envelope + releaseCoeff * (absInput - envelope);

  // トランジェント抑制量計算
  float normalizedDrive = drive / VT2BConstants::kDriveMax;
  float amount = VT2BConstants::kTransientAmountMin +
                 normalizedDrive * (VT2BConstants::kTransientAmountMax -
                                    VT2BConstants::kTransientAmountMin);

  // ソフトニー適用
  float threshold = VT2BConstants::kTransientThreshold;
  float knee = VT2BConstants::kTransientKnee;

  float reduction = 0.0f;
  if (envelope > threshold) {
    float excess = (envelope - threshold) / knee;
    reduction = std::min(1.0f, excess) * amount;
  }

  return input * (1.0f - reduction);
}

// 保持（ヘッダーで宣言されているため定義必須、ただし未使用）
float VT2BBlackProcessor::processAllpass(float input, float &state) {
  // 1次オールパスフィルタ（位相安定化）
  // 低域の位相を安定させ、ステレオ像を維持

  float omega = 2.0f * juce::MathConstants<float>::pi *
                VT2BConstants::kAllpassFrequency / float(currentSampleRate);
  float coeff =
      (1.0f - std::tan(omega / 2.0f)) / (1.0f + std::tan(omega / 2.0f));

  float output = coeff * (input - state) + state;
  state = coeff * (output - input) + input;

  // 元信号との50%ブレンド
  return input * 0.5f + output * 0.5f;
}

float VT2BBlackProcessor::calculateMakeupGain(float drive) {
  // Driveによる音量変化（入力ブースト + サチュレーション）を相殺
  float normalizedDrive = drive / VT2BConstants::kDriveMax;

  // ブースト分を抑えつつ、サチュレーションでの圧縮感を残すバランス
  return 1.0f / (1.0f + normalizedDrive * 0.8f);
}

//==============================================================================
bool VT2BBlackProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor *VT2BBlackProcessor::createEditor() {
  return new VT2BBlackEditor(*this);
}

//==============================================================================
void VT2BBlackProcessor::getStateInformation(juce::MemoryBlock &destData) {
  auto state = parameters.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void VT2BBlackProcessor::setStateInformation(const void *data,
                                             int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(parameters.state.getType()))
      parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new VT2BBlackProcessor();
}
