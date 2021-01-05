//
// Created by scliang on 1/5/21.
//

#ifndef STARS_COMMON_H
#define STARS_COMMON_H

#include "log.h"

/*
 * recorder main thread loop start/exit
 */
void recorder_loop_start(const char *cascade, const char *mnn);
void recorder_loop_exit();

#endif //STARS_COMMON_H
