//
// Created by scliang on 1/5/21.
//

#include <thread>
#include "common.h"
#include "safe_queue.hpp"

#define d(...)  LOG_D("Recorder-Native:loop", __VA_ARGS__)
#define e(...)  LOG_E("Recorder-Native:loop", __VA_ARGS__)

namespace recorder {

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

static void loop_run(std::string &&cascade, std::string &&mnn) {
    loop_running = true;

    d("=================================================");
    com_ptr.reset(new common(cascade, mnn));
    d("hardware concurrency: %d", std::thread::hardware_concurrency());
//    d("cascade:%s, mnn:%s", cascade.c_str(), mnn.c_str());
    d("recorder loop running...");
    while (true) {
        auto n = queue.wait_and_pop();
        if (n->is_exit()) {
            break;
        }

        n->run();
    }

    d("recorder loop exited...");
    common *com = com_ptr.release(); delete com;
    loop_running = false;
    d("=================================================");
}

} //namespace recorder

void recorder::loop_start(const char *cascade, const char *mnn) {
    if (loop_running) {
        return;
    }

    std::thread t(loop_run, std::string(cascade), std::string(mnn));
    t.detach();
}

void recorder::loop_exit() {
    if (!loop_running) {
        return;
    }

    queue.push(ln::create_exit_ln());
}

void recorder::loop_post(void (*runnable)(void*, void (*)(void*)),
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
