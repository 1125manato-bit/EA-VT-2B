/*
  ==============================================================================
    VT-2B Black - EMU AUDIO
    Plugin Editor (UI) Implementation

    デバッグモード: ⌘+ドラッグでノブ位置変更、⌥+ドラッグでサイズ変更
  ==============================================================================
*/

#include "PluginEditor.h"
#include "BinaryData.h"
#include "PluginProcessor.h"

// デバッグモード: 1=位置調整モード、0=通常モード
#define VT2B_DEBUG_MODE 0

// デバッグ用静的変数（両ノブ間で共有）
#if VT2B_DEBUG_MODE
static int g_debugDriveX = 195;
static int g_debugDriveY = 550;
static int g_debugMixX = 829;
static int g_debugMixY = 550;
static int g_debugKnobSize = 160;
#endif

//==============================================================================
// VT2BImageKnob Implementation
//==============================================================================

VT2BImageKnob::VT2BImageKnob() { setRepaintsOnMouseActivity(true); }

VT2BImageKnob::~VT2BImageKnob() {}

void VT2BImageKnob::setImage(const juce::Image &image) {
  knobImage = image;
  repaint();
}

void VT2BImageKnob::paint(juce::Graphics &g) {
  auto bounds = getLocalBounds().toFloat();
  auto centre = bounds.getCentre();

  if (knobImage.isValid()) {
    // ノブの回転角度を計算
    float normalizedValue =
        static_cast<float>((value - minValue) / (maxValue - minValue));
    float angle = startAngle + normalizedValue * (endAngle - startAngle);

    // 画像サイズを取得
    float knobSize = juce::jmin(bounds.getWidth(), bounds.getHeight());
    float scale = knobSize / static_cast<float>(knobImage.getWidth());

    // 回転変換を適用
    juce::AffineTransform transform =
        juce::AffineTransform::rotation(
            angle, static_cast<float>(knobImage.getWidth()) / 2.0f,
            static_cast<float>(knobImage.getHeight()) / 2.0f)
            .scaled(scale)
            .translated(centre.x - (knobImage.getWidth() * scale) / 2.0f,
                        centre.y - (knobImage.getHeight() * scale) / 2.0f);

    g.drawImageTransformed(knobImage, transform, false);
  }

#if VT2B_DEBUG_MODE
  // デバッグ: ノブの境界を表示
  g.setColour(juce::Colours::red.withAlpha(0.5f));
  g.drawRect(getLocalBounds(), 2);

  // 中心マーク
  g.setColour(juce::Colours::yellow);
  g.fillEllipse(centre.x - 3, centre.y - 3, 6, 6);
#endif
}

void VT2BImageKnob::resized() {}

void VT2BImageKnob::setRange(double min, double max, double interval) {
  minValue = min;
  maxValue = max;
  defaultValue = (min + max) / 2.0;
  juce::ignoreUnused(interval);
}

void VT2BImageKnob::setValue(double newValue,
                             juce::NotificationType notification) {
  value = juce::jlimit(minValue, maxValue, newValue);
  repaint();

  if (notification != juce::dontSendNotification && onValueChange)
    onValueChange();
}

double VT2BImageKnob::getValue() const { return value; }

void VT2BImageKnob::setLabel(const juce::String &labelText) {
  label = labelText;
  repaint();
}

void VT2BImageKnob::setRotationRange(float startAngleRadians,
                                     float endAngleRadians) {
  startAngle = startAngleRadians;
  endAngle = endAngleRadians;
}

void VT2BImageKnob::mouseDown(const juce::MouseEvent &event) {
#if VT2B_DEBUG_MODE
  // デバッグモード: ⌘キーで位置調整、⌥キーでサイズ調整
  if (event.mods.isCommandDown() || event.mods.isAltDown()) {
    debugMode = true;
    debugDragStartX = event.getScreenX();
    debugDragStartY = event.getScreenY();
    return;
  }
#endif
  dragStartValue = value;
  dragStartY = event.y;
}

void VT2BImageKnob::mouseDrag(const juce::MouseEvent &event) {
#if VT2B_DEBUG_MODE
  if (debugMode) {
    int dx = event.getScreenX() - debugDragStartX;
    int dy = event.getScreenY() - debugDragStartY;

    if (event.mods.isCommandDown()) {
      // 位置移動
      if (label == "DRIVE") {
        g_debugDriveX += dx;
        g_debugDriveY += dy;
      } else {
        g_debugMixX += dx;
        g_debugMixY += dy;
      }
    } else if (event.mods.isAltDown()) {
      // サイズ変更
      g_debugKnobSize += dx;
      g_debugKnobSize = juce::jlimit(50, 300, g_debugKnobSize);
    }

    debugDragStartX = event.getScreenX();
    debugDragStartY = event.getScreenY();

    // 親に通知してレイアウト更新
    if (auto *parent = getParentComponent()) {
      parent->resized();
      parent->repaint();
    }
    return;
  }
#endif

  float sensitivity = event.mods.isShiftDown() ? 0.002f : 0.01f;
  double delta = static_cast<double>(dragStartY - event.y) * sensitivity *
                 (maxValue - minValue);
  setValue(dragStartValue + delta);
}

void VT2BImageKnob::mouseUp(const juce::MouseEvent &) {
#if VT2B_DEBUG_MODE
  if (debugMode) {
    debugMode = false;
    // 最終値を出力
    DBG("// ===== ノブ位置設定 =====");
    DBG("// DRIVE: x=" << g_debugDriveX << ", y=" << g_debugDriveY);
    DBG("// MIX: x=" << g_debugMixX << ", y=" << g_debugMixY);
    DBG("// Size: " << g_debugKnobSize);
    DBG("driveKnob.setBounds(" << (g_debugDriveX - g_debugKnobSize / 2) << ", "
                               << g_debugDriveY << ", " << g_debugKnobSize
                               << ", " << g_debugKnobSize << ");");
    DBG("mixKnob.setBounds(" << (g_debugMixX - g_debugKnobSize / 2) << ", "
                             << g_debugMixY << ", " << g_debugKnobSize << ", "
                             << g_debugKnobSize << ");");
  }
#endif
}

void VT2BImageKnob::mouseDoubleClick(const juce::MouseEvent &) {
  setValue(defaultValue);
}

void VT2BImageKnob::mouseWheelMove(const juce::MouseEvent &,
                                   const juce::MouseWheelDetails &wheel) {
  double delta = wheel.deltaY * (maxValue - minValue) * 0.05;
  setValue(value + delta);
}

//==============================================================================
// VT2BBlackEditor Implementation
//==============================================================================

VT2BBlackEditor::VT2BBlackEditor(VT2BBlackProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  // 画像をロード
  loadImages();

  // ウィンドウサイズ（背景画像に合わせる）
  if (backgroundImage.isValid())
    setSize(backgroundImage.getWidth(), backgroundImage.getHeight());
  else
    setSize(800, 600);

  // Driveノブ設定
  driveKnob.setImage(knobImage);
  driveKnob.setLabel("DRIVE");
  driveKnob.setRange(0.0, 100.0, 1.0);
  driveKnob.setValue(0.0);
  driveKnob.setRotationRange(-2.35619f, 2.35619f); // -135° to 135°
  addAndMakeVisible(driveKnob);

  // Mixノブ設定
  mixKnob.setImage(knobImage);
  mixKnob.setLabel("MIX");
  mixKnob.setRange(0.0, 100.0, 1.0);
  mixKnob.setValue(100.0);
  mixKnob.setRotationRange(-2.35619f, 2.35619f);
  addAndMakeVisible(mixKnob);

  // 内部スライダー（アタッチメント用）
  driveSlider.setRange(0.0, 10.0);
  mixSlider.setRange(0.0, 100.0);

  // 値同期（Drive: 0-10 を 0-100 にスケール）
  // アタッチメント作成前に定義することで、初期値の同期を確実にする
  driveKnob.onValueChange = [this]() {
    driveSlider.setValue(driveKnob.getValue() / 10.0,
                         juce::sendNotificationSync);
  };
  mixKnob.onValueChange = [this]() {
    mixSlider.setValue(mixKnob.getValue(), juce::sendNotificationSync);
  };

  driveSlider.onValueChange = [this]() {
    driveKnob.setValue(driveSlider.getValue() * 10.0,
                       juce::dontSendNotification);
  };
  mixSlider.onValueChange = [this]() {
    mixKnob.setValue(mixSlider.getValue(), juce::dontSendNotification);
  };

  // パラメータ接続
  driveAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.getParameters(), "drive", driveSlider);
  mixAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.getParameters(), "mix", mixSlider);
}

VT2BBlackEditor::~VT2BBlackEditor() {
  // アタッチメントを先に解放してクラッシュを防止
  driveAttachment.reset();
  mixAttachment.reset();
}

void VT2BBlackEditor::loadImages() {
  // BinaryDataから画像をロード
  backgroundImage = juce::ImageCache::getFromMemory(
      BinaryData::background_png, BinaryData::background_pngSize);

  knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png,
                                              BinaryData::knob_pngSize);
}

void VT2BBlackEditor::paint(juce::Graphics &g) {
  // 背景画像を描画
  if (backgroundImage.isValid()) {
    g.drawImage(backgroundImage, getLocalBounds().toFloat());
  } else {
    // フォールバック背景
    g.fillAll(juce::Colour(0xff1a1a1a));
  }

#if VT2B_DEBUG_MODE
  // デバッグ: 操作説明
  g.setColour(juce::Colours::yellow);
  g.setFont(14.0f);

  juce::String info = "DEBUG MODE | Cmd+Drag: Move | Option+Drag: Resize";
  g.drawText(info, 10, 10, getWidth() - 20, 20, juce::Justification::left);

  // 現在の設定値を表示
  g.setFont(12.0f);
  juce::String values = "DRIVE: (" + juce::String(g_debugDriveX) + "," +
                        juce::String(g_debugDriveY) + ") | MIX: (" +
                        juce::String(g_debugMixX) + "," +
                        juce::String(g_debugMixY) +
                        ") | Size: " + juce::String(g_debugKnobSize);
  g.drawText(values, 10, 30, getWidth() - 20, 20, juce::Justification::left);
#endif
}

void VT2BBlackEditor::resized() {
#if VT2B_DEBUG_MODE
  // デバッグモード: グローバル変数から位置を設定
  driveKnob.setBounds(g_debugDriveX - g_debugKnobSize / 2, g_debugDriveY,
                      g_debugKnobSize, g_debugKnobSize);
  mixKnob.setBounds(g_debugMixX - g_debugKnobSize / 2, g_debugMixY,
                    g_debugKnobSize, g_debugKnobSize);
#else
  // 通常モード: 固定位置（ユーザー指定値）
  // DRIVE: x=216, y=523 | MIX: x=809, y=523 | Size=206
  int knobSize = 206;
  driveKnob.setBounds(216 - knobSize / 2, 523, knobSize, knobSize);
  mixKnob.setBounds(809 - knobSize / 2, 523, knobSize, knobSize);
#endif
}
