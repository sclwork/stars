//
// Created by scliang on 1/19/21.
//

#ifndef STARS_LOOP_H
#define STARS_LOOP_H

#define USE_CONCURRENT_QUEUE

namespace media {

/*
 * post runnable to media main thread loop
 */
void loop_post_main(void (*runnable)(void *ctx, void (*callback)(void *ctx)),
                    void *ctx = nullptr,
                    void (*callback)(void *ctx) = nullptr);

} //namespace media

#endif //STARS_LOOP_H
