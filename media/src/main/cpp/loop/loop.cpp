//
// Created by scliang on 1/5/21.
//

#include <thread>
#include "loop.h"
#include "common.h"
#include "safe_queue.hpp"

#define log_d(...)  LOG_D("Media-Native:loop", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:loop", __VA_ARGS__)

namespace media {

class ln {
public:
    ln():t(0), ctx(nullptr), runnable(nullptr), callback(nullptr) {}
    ln(void (*runnable)(void*, void (*)(void*)),void *ctx,void (*callback)(void*))
        :t(0), ctx(ctx), runnable(runnable), callback(callback) {}
    ~ln() { ctx = nullptr; };

public:
    static ln create_exit_ln() {
        ln n; n.t = -1;
        return n;
    }

    bool is_exit() const {
        return t == -1;
    }

    void run() {
        if (runnable) {
            runnable(ctx, callback);
        }
    }

private:
    int32_t t;
    //////////////////////
    void *ctx;
    void (*runnable)(void *ctx, void (*callback)(void *));
    void (*callback)(void *ctx);
};

/*
 * global thread object
 */
static std::unique_ptr<common> com_ptr;
static std::atomic_bool        loop_main_running(false);
static safe_queue<ln>          queue_main;
static std::atomic_bool        loop_collect_a_running(false);
static std::atomic_bool        loop_collect_b_running(false);
static std::atomic_bool        loop_collect_c_running(false);
static safe_queue<ln>          queue_collect;
static std::atomic_bool        loop_encode_a_running(false);
static std::atomic_bool        loop_encode_b_running(false);
static std::atomic_bool        loop_encode_c_running(false);
static safe_queue<ln>          queue_encode;

void renderer_init() {
    com_ptr->renderer_init();
    log_d("renderer init.");
}

void renderer_release() {
    com_ptr->renderer_release();
    log_d("renderer release.");
}

int32_t renderer_surface_created() {
    log_d("renderer surface created.");
    return com_ptr->renderer_surface_created();
}

void renderer_surface_destroyed() {
    com_ptr->renderer_surface_destroyed();
    log_d("renderer surface destroyed.");
}

void renderer_surface_changed(int32_t w, int32_t h) {
    com_ptr->renderer_surface_changed(w, h);
    log_d("renderer surface changed: %d,%d.", w, h);
}

void renderer_draw_frame() {
    com_ptr->renderer_draw_frame();
//    d("renderer draw frame.");
}

void renderer_select_camera(int camera) {
    com_ptr->renderer_select_camera(camera);
    log_d("renderer select camera: %d.", camera);
}

void renderer_record_start(const char *name) {
    com_ptr->renderer_record_start(std::string(name));
//    log_d("renderer record start: %s.", name);
}

void renderer_record_stop() {
    com_ptr->renderer_record_stop();
    log_d("renderer record stop.");
}

bool renderer_record_running() {
    return com_ptr == nullptr ? false : com_ptr->renderer_record_running();
}

static void loop_main_run() {
    loop_main_running = true;
    log_d("hardware concurrency: %d", std::thread::hardware_concurrency());
    log_d("main loop running...");
    while (true) {
        auto n = queue_main.wait_and_pop();
        if (n->is_exit()) { break; }
        n->run();
    }

    if (!loop_collect_a_running && !loop_collect_b_running && !loop_collect_c_running &&
        !loop_encode_a_running && !loop_encode_b_running && !loop_encode_c_running) {
        common *com = com_ptr.release();
        delete com;
    }
    loop_main_running = false;
    log_d("main loop exited...");
}

static void loop_collect_a_run() {
    loop_collect_a_running = true;
    log_d("collect_a loop running...");
    while (true) {
        auto n = queue_collect.wait_and_pop();
        if (n->is_exit()) { break; }
        n->run();
//        log_d("loop_collect_a_run has count: %d.", queue_collect.size());
    }

    if (!loop_collect_b_running && !loop_collect_c_running &&
        !loop_main_running && !loop_encode_a_running && !loop_encode_b_running && !loop_encode_c_running) {
        common *com = com_ptr.release();
        delete com;
    }
    loop_collect_a_running = false;
    log_d("collect_a loop exited...");
    if (loop_collect_a_running || loop_collect_b_running || loop_collect_c_running) {
        queue_collect.push(ln::create_exit_ln());
    }
}

static void loop_collect_b_run() {
    loop_collect_b_running = true;
    log_d("collect_b loop running...");
    while (true) {
        auto n = queue_collect.wait_and_pop();
        if (n->is_exit()) { break; }
        n->run();
//        log_d("loop_collect_b_run has count: %d.", queue_collect.size());
    }

    if (!loop_collect_a_running && !loop_collect_c_running &&
        !loop_main_running && !loop_encode_a_running && !loop_encode_b_running && !loop_encode_c_running) {
        common *com = com_ptr.release();
        delete com;
    }
    loop_collect_b_running = false;
    log_d("collect_b loop exited...");
    if (loop_collect_a_running || loop_collect_b_running || loop_collect_c_running) {
        queue_collect.push(ln::create_exit_ln());
    }
}

static void loop_collect_c_run() {
    loop_collect_c_running = true;
    log_d("collect_c loop running...");
    while (true) {
        auto n = queue_collect.wait_and_pop();
        if (n->is_exit()) { break; }
        n->run();
//        log_d("loop_collect_c_run has count: %d.", queue_collect.size());
    }

    if (!loop_collect_a_running && !loop_collect_b_running &&
        !loop_main_running && !loop_encode_a_running && !loop_encode_b_running && !loop_encode_c_running) {
        common *com = com_ptr.release();
        delete com;
    }
    loop_collect_c_running = false;
    log_d("collect_c loop exited...");
    if (loop_collect_a_running || loop_collect_b_running || loop_collect_c_running) {
        queue_collect.push(ln::create_exit_ln());
    }
}

static void loop_encode_a_run() {
    loop_encode_a_running = true;
    log_d("encode_a loop running...");
    while (true) {
        auto n = queue_encode.wait_and_pop();
        if (n->is_exit()) { break; }
        n->run();
    }

    if (!loop_main_running &&
        !loop_encode_b_running && !loop_encode_c_running &&
        !loop_collect_a_running && !loop_collect_b_running && !loop_collect_c_running) {
        common *com = com_ptr.release();
        delete com;
    }
    loop_encode_a_running = false;
    log_d("encode_a loop exited...");
    if (loop_encode_a_running || loop_encode_b_running || loop_encode_c_running) {
        queue_encode.push(ln::create_exit_ln());
    }
}

static void loop_encode_b_run() {
    loop_encode_b_running = true;
    log_d("encode_b loop running...");
    while (true) {
        auto n = queue_encode.wait_and_pop();
        if (n->is_exit()) { break; }
        n->run();
    }

    if (!loop_main_running &&
        !loop_encode_a_running && !loop_encode_c_running &&
        !loop_collect_a_running && !loop_collect_b_running && !loop_collect_c_running) {
        common *com = com_ptr.release();
        delete com;
    }
    loop_encode_b_running = false;
    log_d("encode_b loop exited...");
    if (loop_encode_a_running || loop_encode_b_running || loop_encode_c_running) {
        queue_encode.push(ln::create_exit_ln());
    }
}

static void loop_encode_c_run() {
    loop_encode_c_running = true;
    log_d("encode_c loop running...");
    while (true) {
        auto n = queue_encode.wait_and_pop();
        if (n->is_exit()) { break; }
        n->run();
    }

    if (!loop_main_running &&
        !loop_encode_a_running && !loop_encode_b_running &&
        !loop_collect_a_running && !loop_collect_b_running && !loop_collect_c_running) {
        common *com = com_ptr.release();
        delete com;
    }
    loop_encode_c_running = false;
    log_d("encode_c loop exited...");
    if (loop_encode_a_running || loop_encode_b_running || loop_encode_c_running) {
        queue_encode.push(ln::create_exit_ln());
    }
}

} //namespace media

void media::loop_start(const char *cascade, const char *mnn) {
    if (loop_main_running) {
        return;
    }

    std::string c(cascade); std::string m(mnn);
    com_ptr.reset(new common(c, m));

    std::thread main_t(loop_main_run);
    main_t.detach();
    std::thread collect_a_t(loop_collect_a_run);
    collect_a_t.detach();
    std::thread collect_b_t(loop_collect_b_run);
    collect_b_t.detach();
    std::thread collect_c_t(loop_collect_c_run);
    collect_c_t.detach();
    std::thread encode_a_t(loop_encode_a_run);
    encode_a_t.detach();
    std::thread encode_b_t(loop_encode_b_run);
    encode_b_t.detach();
    std::thread encode_c_t(loop_encode_c_run);
    encode_c_t.detach();
}

void media::loop_exit() {
    if (loop_encode_a_running || loop_encode_b_running || loop_encode_c_running) {
        queue_encode.push(ln::create_exit_ln());
    }

    if (loop_collect_a_running || loop_collect_b_running || loop_collect_c_running) {
        queue_collect.push(ln::create_exit_ln());
    }

    if (loop_main_running) {
        queue_main.push(ln::create_exit_ln());
    }

    if (!loop_main_running &&
        !loop_encode_a_running && !loop_encode_b_running && !loop_encode_c_running  &&
        !loop_collect_a_running && !loop_collect_b_running && !loop_collect_c_running) {
        common *com = com_ptr.release();
        delete com;
    }
}

void media::loop_post_main(void (*runnable)(void*, void (*)(void*)),
                           void *ctx,
                           void (*callback)(void*)) {
    if (!loop_main_running) {
        return;
    }

    if (runnable == nullptr) {
        return;
    }

    queue_main.push(ln(runnable, ctx, callback));
}

void media::loop_post_collect(void (*runnable)(void*, void (*)(void*)),
                              void *ctx,
                              void (*callback)(void*)) {
    if (!loop_collect_a_running && !loop_collect_b_running && !loop_collect_c_running) {
        return;
    }

    if (runnable == nullptr) {
        return;
    }

    queue_collect.push(ln(runnable, ctx, callback));
}

int32_t media::loop_collect_count() {
    return queue_collect.size();
}

void media::loop_post_encode(void (*runnable)(void*, void (*)(void*)),
                             void *ctx,
                             void (*callback)(void*)) {
    if (!loop_encode_a_running && !loop_encode_b_running && !loop_encode_c_running) {
        return;
    }

    if (runnable == nullptr) {
        return;
    }

    queue_encode.push(ln(runnable, ctx, callback));
}
