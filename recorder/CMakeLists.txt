cmake_minimum_required(VERSION 3.6.0)

SET(CMAKE_BUILD_TYPE "Release")
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-unused-variable -pthread")
elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread")
endif()

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -pthread")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -pthread")

# lamemp3
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/jniLibs/${ANDROID_ABI})
include_directories(src/main/cpp/lamemp3 src/main/cpp/lamemp3/lame)
aux_source_directory(src/main/cpp/lamemp3 LAME_SRC_LIST)
add_library(
        mp3lame
        SHARED
        ${LAME_SRC_LIST})
find_library(
        log-lib
        log)
target_link_libraries(
        mp3lame
        ${log-lib})

# media
include_directories(
        jniLibs/include
        src/main/cpp
        src/main/cpp/glm )

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavcodec.so)
add_library(avdevice SHARED IMPORTED)
set_target_properties(avdevice PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavdevice.so)
add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavfilter.so)
add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavformat.so)
add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavutil.so)
add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libswresample.so)
add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libswscale.so)
add_library(postproc SHARED IMPORTED)
set_target_properties(postproc PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libpostproc.so)

add_library(opencv4 SHARED IMPORTED)
set_target_properties(opencv4 PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libopencv_java4.so)

add_library(tflite SHARED IMPORTED)
set_target_properties(tflite PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libtensorflowlite_jni.so)

add_library(mnn SHARED IMPORTED)
set_target_properties(mnn PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libMNN.so)
add_library(mnn_expr SHARED IMPORTED)
set_target_properties(mnn_expr PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libMNN_Express.so)
add_library(mnn_cl SHARED IMPORTED)
set_target_properties(mnn_cl PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libMNN_CL.so)
add_library(mnn_gl SHARED IMPORTED)
set_target_properties(mnn_gl PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libMNN_GL.so)
add_library(mnn_vulkan SHARED IMPORTED)
set_target_properties(mnn_vulkan PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libMNN_Vulkan.so)

aux_source_directory(src/main/cpp/loop  LOOP_SRC_LIST)
aux_source_directory(src/main/cpp/video VIDEO_SRC_LIST)

add_library( # Sets the name of the library.
        recorder

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${LOOP_SRC_LIST}
        ${VIDEO_SRC_LIST}
        src/main/cpp/common.cpp
        src/main/cpp/jni.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )
find_library(
        OpenSLES-lib
        OpenSLES )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        recorder
        # Links the target library to the ffmpeg library
        postproc
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        opencv4
        tflite
        mnn
        mnn_expr
        mnn_cl
        mnn_gl
        mnn_vulkan
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${OpenSLES-lib}
        android
        camera2ndk
        mediandk
        GLESv3 )
