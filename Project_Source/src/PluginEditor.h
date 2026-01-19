/*
  ==============================================================================
    VT-2B Black - EMU AUDIO
    Plugin Editor (UI)

    デバッグモード: ⌘+ドラッグでノブ位置変更、⌥+ドラッグでサイズ変更
  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"

// デバッグモード
#define VT2B_DEBUG_MODE 0

//==============================================================================
/**
 * 画像ベースのノブ - 回転するゴールドノブ
 */
class VT2BImageKnob : public juce::Component {
public:
  VT2BImageKnob();
  ~VT2BImageKnob() override;

  void paint(juce::Graphics &g) override;
  void resized() override;

  void setImage(const juce::Image &knobImage);
  void setRange(double min, double max, double interval = 0.0);
  void
  setValue(double newValue,
           juce::NotificationType notification = juce::sendNotificationAsync);
  double getValue() const;

  void setLabel(const juce::String &labelText);
  void setRotationRange(float startAngleRadians, float endAngleRadians);

  std::function<void()> onValueChange;

private:
  void mouseDown(const juce::MouseEvent &event) override;
  void mouseDrag(const juce::MouseEvent &event) override;
  void mouseUp(const juce::MouseEvent &event) override;
  void mouseDoubleClick(const juce::MouseEvent &event) override;
  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;

  juce::Image knobImage;

  double value = 0.0;
  double minValue = 0.0;
  double maxValue = 10.0;
  double defaultValue = 0.0;
  double dragStartValue = 0.0;
  int dragStartY = 0;

  float startAngle = -2.35619f; // -135 degrees
  float endAngle = 2.35619f;    // 135 degrees

  juce::String label;

#if VT2B_DEBUG_MODE
  bool debugMode = false;
  int debugDragStartX = 0;
  int debugDragStartY = 0;
#endif

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VT2BImageKnob)
};

//==============================================================================
/**
 * メインエディター - 背景画像とノブ画像を使用
 */
class VT2BBlackEditor : public juce::AudioProcessorEditor {
public:
  explicit VT2BBlackEditor(VT2BBlackProcessor &);
  ~VT2BBlackEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  VT2BBlackProcessor &audioProcessor;

  // 画像
  juce::Image backgroundImage;
  juce::Image knobImage;

  // ノブ
  VT2BImageKnob driveKnob;
  VT2BImageKnob mixKnob;

  // 内部スライダー（アタッチメント用）
  juce::Slider driveSlider;
  juce::Slider mixSlider;

  // パラメータアタッチメント
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      driveAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      mixAttachment;

  // 画像ロード
  void loadImages();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VT2BBlackEditor)
};
