//
// Created by scliang on 1/5/21.
//

#include <thread>
#include "loop.h"
#include "common.h"
#include "jni_bridge.h"
#include "concurrent_queue.h"

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
    static ln create_exit_ln() { ln n; n.t = -1; return n; }
    bool is_exit() const { return t == -1; }
    void run() { if (runnable) { runnable(ctx, callback); }}

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
static moodycamel::ConcurrentQueue<ln> mainQ;

void renderer_init() {
    com_ptr->renderer_init();
    log_d("renderer init.");
}

void renderer_release() {
    com_ptr->renderer_release();
    log_d("renderer release.");
}

void renderer_surface_created() {
    log_d("renderer surface created.");
    com_ptr->renderer_surface_created();
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

void renderer_updt_paint(const std::string &name) {
    com_ptr->renderer_updt_paint(name);
}

void video_record_start(const std::string &name) {
    com_ptr->video_record_start(std::string(name));
}

void video_record_stop() {
    com_ptr->video_record_stop();
}

bool video_recording() {
    return com_ptr->video_recording();
}

void camera_select(int32_t cam) {
    com_ptr->camera_select(cam);
}

static void loop_main_run() {
    loop_main_running = true;
    log_d("hardware concurrency: %d", std::thread::hardware_concurrency());
    log_d("main loop running...");

    while (true) {
        ln n;
        bool h = mainQ.try_dequeue(n);
        if (!h) { std::this_thread::sleep_for(std::chrono::microseconds(10)); continue; }
        if (n.is_exit()) { break; }
        n.run();
    }

    common *com = com_ptr.release();
    delete com;
    loop_main_running = false;
    log_d("main loop exited...");
}

} //namespace media

void media::loop_start(const std::string &file_root,
                       const std::string &cascade,
                       const std::string &mnn,
                       void (*on_request_render)(int32_t)) {
    if (loop_main_running) {
        return;
    }

    log_d("==================================================");
    com_ptr.reset(new common(file_root, cascade, mnn, on_request_render));

    std::thread main_t(loop_main_run);
    main_t.detach();
}

void media::loop_exit() {
    if (loop_main_running) {
        mainQ.enqueue(ln::create_exit_ln());
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

    mainQ.enqueue(ln(runnable, ctx, callback));
}
