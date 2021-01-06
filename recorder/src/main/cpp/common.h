//
// Created by scliang on 1/5/21.
//

#ifndef STARS_COMMON_H
#define STARS_COMMON_H

#include <string>
#include "log.h"

#define MATH_PI 3.1415926535897932384626433832802

namespace recorder {

/*
 * recorder main thread loop start/exit
 */
void loop_start(const char *cascade, const char *mnn);
void loop_exit();

/*
 * post runnable to recorder main thread loop
 */
void loop_post(void (*runnable)(void *ctx, void (*callback)(void *ctx)),
               void *ctx = nullptr,
               void (*callback)(void *ctx) = nullptr);

/**
 * recorder common object/res ...
 */
class common {
public:
    common(std::string &cascade, std::string &mnn);
    ~common();

private:
    common(common&&) = delete;
    common(const common&) = delete;
    common& operator=(common&&) = delete;
    common& operator=(const common&) = delete;

private:
    std::string cascade;
    std::string mnn;
};

} //namespace recorder

#endif //STARS_COMMON_H
