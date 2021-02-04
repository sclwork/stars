//
// Created by scliang on 1/19/21.
//

#ifndef STARS_LOOP_H
#define STARS_LOOP_H

#define USE_CONCURRENT_QUEUE
#define USE_SINGLE_THREAD

namespace media {

/*
 * post runnable to media main thread loop
 */
void loop_post_main(void (*runnable)(void *ctx, void (*callback)(void *ctx)),
                    void *ctx = nullptr,
                    void (*callback)(void *ctx) = nullptr);

/*
 * post runnable to media collect thread loop
 */
void loop_post_collect(void (*runnable)(void *ctx, void (*callback)(void *ctx)),
                       void *ctx = nullptr,
                       void (*callback)(void *ctx) = nullptr);

int32_t loop_collect_count();

/*
 * post runnable to media encode thread loop
 */
void loop_post_encode(void (*runnable)(void *ctx, void (*callback)(void *ctx)),
                      void *ctx = nullptr,
                      void (*callback)(void *ctx) = nullptr);

} //namespace media

#endif //STARS_LOOP_H
