cmake_minimum_required(VERSION 3.22)

# EA VT-2B - EMU AUDIO
# Console Bus Glue Processor Plugin
project(EA_VT_2B VERSION 1.1.0)

# C++17を使用
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCEをFetchContentで取得
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

# プラグインターゲット
juce_add_plugin(EA_VT_2B
    # プラグイン情報
    COMPANY_NAME "EMU AUDIO"
    COMPANY_WEBSITE "https://emuaudio.com"
    COMPANY_EMAIL "contact@emuaudio.com"
    
    PLUGIN_MANUFACTURER_CODE EmuA
    PLUGIN_CODE Evt2                   # Changed from Vt2b to Evt2 to force re-scan
    
    # フォーマット
    FORMATS VST3 AU Standalone
    
    # プラグイン名
    PRODUCT_NAME "EA VT-2B"
    
    # プラグインタイプ
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    
    # アイコン（後で設定可能）
    # ICON_BIG ""
    # ICON_SMALL ""
    
    # バンドル識別子
    BUNDLE_ID "com.emuaudio.ea-vt2b"
    
    # AU固有設定
    AU_MAIN_TYPE kAudioUnitType_Effect
    
    # VST3カテゴリ
    VST3_CATEGORIES Fx Distortion
    
    # コピープロテクト無効
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# ソースファイル
target_sources(EA_VT_2B
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginProcessor.h
        src/PluginEditor.cpp
        src/PluginEditor.h
)

# プリプロセッサ定義
target_compile_definitions(EA_VT_2B
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        
        # プラグイン情報
        JucePlugin_Name="EA VT-2B"
        JucePlugin_Manufacturer="EMU AUDIO"
        JucePlugin_ManufacturerCode=0x456d7541  # "EmuA"
        JucePlugin_PluginCode=0x45767432        # "Evt2" (Updated)
        JucePlugin_Version="1.1.0"
        JucePlugin_VersionString="1.1.0"
)

# リンクするJUCEモジュール
target_link_libraries(EA_VT_2B
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# インクルードパス
target_include_directories(EA_VT_2B
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
)

# リソース（GUI画像など）
juce_add_binary_data(EA_VT_2B_Data
    SOURCES
        resources/background.png
        resources/knob.png
)
target_link_libraries(EA_VT_2B PRIVATE EA_VT_2B_Data)
