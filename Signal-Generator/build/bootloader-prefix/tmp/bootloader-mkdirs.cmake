# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v4.4.2/components/bootloader/subproject"
  "C:/Espressif/Projects/Git/Signal-Generator/build/bootloader"
  "C:/Espressif/Projects/Git/Signal-Generator/build/bootloader-prefix"
  "C:/Espressif/Projects/Git/Signal-Generator/build/bootloader-prefix/tmp"
  "C:/Espressif/Projects/Git/Signal-Generator/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Espressif/Projects/Git/Signal-Generator/build/bootloader-prefix/src"
  "C:/Espressif/Projects/Git/Signal-Generator/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Espressif/Projects/Git/Signal-Generator/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()