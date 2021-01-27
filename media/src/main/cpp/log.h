//
// Created by scliang on 1/5/21.
//

#ifndef STARS_LOG_H
#define STARS_LOG_H

#include <android/log.h>

/*
 * log able: 0/1
 */
#define LOG_ABLE      1
#define LOG_DRAW_TIME 1

/*
 * log debug info
 */
#if LOG_ABLE
#define LOG_D(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)
#else
#define LOG_D(tag, ...)
#endif

/*
 * log error info
 */
#if LOG_ABLE
#define LOG_E(tag, ...) __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)
#else
#define LOG_E(tag, ...)
#endif

#endif //STARS_LOG_H
