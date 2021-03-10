//
// Created by Scliang on 3/10/21.
//

#ifndef STARS_PROC_H
#define STARS_PROC_H

#include <string>

namespace media {

static bool startWith(const std::string &str, const std::string &head) {
    return str.compare(0, head.size(), head) == 0;
}

static bool endWith(const std::string &str, const std::string &tail) {
    return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
}

} //namespace media

#endif //STARS_PROC_H
