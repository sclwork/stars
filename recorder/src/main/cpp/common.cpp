//
// Created by scliang on 1/6/21.
//

#include "common.h"

#define d(...)  LOG_D("Recorder-Native:common", __VA_ARGS__)
#define e(...)  LOG_E("Recorder-Native:common", __VA_ARGS__)

namespace recorder {
} //namespace recorder

recorder::common::common(std::string &cascade, std::string &mnn)
:cascade(cascade),mnn(mnn) {
    d("created. cascade:%s, mnn:%s", cascade.c_str(), mnn.c_str());
}

recorder::common::~common() {
    d("release.");
}
