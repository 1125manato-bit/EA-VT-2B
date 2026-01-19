# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/manato/Documents/EA プラグインデータ/EA VT-2B/Project_Source/build/_deps/juce-src"
  "/Users/manato/Documents/EA プラグインデータ/EA VT-2B/Project_Source/build/_deps/juce-build"
  "/Users/manato/Documents/EA プラグインデータ/EA VT-2B/Project_Source/build/_deps/juce-subbuild/juce-populate-prefix"
  "/Users/manato/Documents/EA プラグインデータ/EA VT-2B/Project_Source/build/_deps/juce-subbuild/juce-populate-prefix/tmp"
  "/Users/manato/Documents/EA プラグインデータ/EA VT-2B/Project_Source/build/_deps/juce-subbuild/juce-populate-prefix/src/juce-populate-stamp"
  "/Users/manato/Documents/EA プラグインデータ/EA VT-2B/Project_Source/build/_deps/juce-subbuild/juce-populate-prefix/src"
  "/Users/manato/Documents/EA プラグインデータ/EA VT-2B/Project_Source/build/_deps/juce-subbuild/juce-populate-prefix/src/juce-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/manato/Documents/EA プラグインデータ/EA VT-2B/Project_Source/build/_deps/juce-subbuild/juce-populate-prefix/src/juce-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/manato/Documents/EA プラグインデータ/EA VT-2B/Project_Source/build/_deps/juce-subbuild/juce-populate-prefix/src/juce-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
