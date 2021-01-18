//
// Created by scliang on 1/5/21.
//

#include <thread>
#include "common.h"
#include "safe_queue.hpp"

#define log_d(...)  LOG_D("Media-Native:loop", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:loop", __VA_ARGS__)

namespace media {

class ln {
public:
    ln():t(0),ctx(nullptr),runnable(nullptr),callback(nullptr){}
    ln(void (*runnable)(void*, void (*)(void*)),void *ctx,void (*callback)(void*))
        :t(0),ctx(ctx),runnable(runnable),callback(callback){}
    ~ln()= default;

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
static std::atomic_bool loop_running(false);
static std::unique_ptr<common> com_ptr;
static safe_queue<ln> queue;

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

static void loop_run() {
    loop_running = true;
    log_d("hardware concurrency: %d", std::thread::hardware_concurrency());
    log_d("media loop running...");
    while (true) {
        auto n = queue.wait_and_pop();
        if (n->is_exit()) {
            break;
        }

        n->run();
    }

    common *com = com_ptr.release();
    delete com;
    loop_running = false;
    log_d("media loop exited...");
    log_d("=================================================");
}

} //namespace media

void media::loop_start(const char *cascade, const char *mnn) {
    if (loop_running) {
        return;
    }

    log_d("=================================================");
    std::string c(cascade); std::string m(mnn);
    com_ptr.reset(new common(c, m));
    std::thread t(loop_run);
    t.detach();
}

void media::loop_exit() {
    if (!loop_running) {
        common *com = com_ptr.release();
        delete com;
        return;
    }

    queue.push(ln::create_exit_ln());
}

void media::loop_post(void (*runnable)(void*, void (*)(void*)),
                         void *ctx,
                         void (*callback)(void*)) {
    if (!loop_running) {
        return;
    }

    if (runnable == nullptr) {
        return;
    }

    queue.push(ln(runnable, ctx, callback));
}
