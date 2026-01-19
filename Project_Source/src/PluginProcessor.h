/*
  ==============================================================================
    VT-2B Black - EMU AUDIO
    Console Bus Glue Processor

    "音を一つにする回路"
  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

//==============================================================================
/**
 * VT-2B Black Processor
 *
 * コンソールサミング/バス回路を意識した密度増加型サチュレーション。
 * 派手さを抑え、音をまとめる方向に作用する。
 */
class VT2BBlackProcessor : public juce::AudioProcessor {
public:
  //==============================================================================
  VT2BBlackProcessor();
  ~VT2BBlackProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  //==============================================================================
  // パラメータアクセス
  juce::AudioProcessorValueTreeState &getParameters() { return parameters; }

private:
  //==============================================================================
  // パラメータ
  juce::AudioProcessorValueTreeState parameters;

  std::atomic<float> *driveParameter = nullptr;
  std::atomic<float> *mixParameter = nullptr;

  //==============================================================================
  // DSP状態
  double currentSampleRate = 44100.0;

  // エンベロープフォロワー（トランジェント検出用）
  float envelopeL = 0.0f;
  float envelopeR = 0.0f;

  // オールパスフィルタ状態
  float allpassStateL = 0.0f;
  float allpassStateR = 0.0f;

  // スムージング
  juce::SmoothedValue<float> smoothedDrive;
  juce::SmoothedValue<float> smoothedMix;

  //==============================================================================
  // DSP処理関数

  /**
   * 密度増加型サチュレーション
   * テープ系の柔らかい飽和特性をモデリング
   */
  float processSaturation(float input, float drive);

  /**
   * 低次倍音生成（2次/3次）
   * 暖かさと存在感を微量付加
   */
  float processHarmonics(float input, float drive);

  /**
   * トランジェント整形
   * ピークの暴れを抑えつつパンチは残す
   */
  float processTransient(float input, float &envelope, float drive);

  /**
   * 位相安定化オールパス
   * 低域の位相を安定させステレオ像を維持
   */
  float processAllpass(float input, float &state);

  /**
   * 自動ゲイン補償
   * Drive増加による音量変化を相殺
   */
  float calculateMakeupGain(float drive);

  //==============================================================================
  // パラメータレイアウト作成
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VT2BBlackProcessor)
};
