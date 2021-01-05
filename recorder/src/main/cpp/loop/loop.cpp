//
// Created by scliang on 1/5/21.
//

#include <thread>
#include "common.h"
#include "safe_queue.hpp"

#define d(...)  LOG_D("Recorder-Native:loop", __VA_ARGS__)
#define e(...)  LOG_E("Recorder-Native:loop", __VA_ARGS__)

class ln {
public:
    ln():t(0){}
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
    }

private:
    int32_t t;
};

/*
 * global thread object
 */
static std::atomic_bool loop_running(false);
static safe_queue<ln> queue;

static void loop_run(const char *cascade, const char *mnn) {
    loop_running = true;

    d("=================================================");
    d("hardware concurrency: %d", std::thread::hardware_concurrency());
    d("cascade:%s, mnn:%s", cascade, mnn);
    d("recorder loop running...");
    while (true) {
        auto n = queue.wait_and_pop();
        if (n->is_exit()) {
            break;
        }

        n->run();
    }

    loop_running = false;
    d("recorder loop exited...");
    d("=================================================");
}

void recorder_loop_start(const char *cascade, const char *mnn) {
    if (loop_running) {
        return;
    }

    std::thread t(loop_run, cascade, mnn);
    t.detach();
}

void recorder_loop_exit() {
    if (!loop_running) {
        return;
    }

    queue.push(ln::create_exit_ln());
}
