//
// Created by scliang on 1/13/21.
//

#ifndef STARS_MNN_H
#define STARS_MNN_H

#include <memory>
#include <vector>
#include <opencv2/opencv.hpp>
#include <MNN/Interpreter.hpp>
#include <video/collect/image_cache.h>

namespace media {

// face_detect iou tmp args
struct face_args {
    float xmin0, ymin0, xmax0, ymax0;
    float xmin1, ymin1, xmax1, ymax1;
    float aw, ah, ai, au;
    float xcenter, ycenter, bw, bh, ss;
    float ymin, xmin, ymax, xmax, nonface_prob, face_prob;
};

class mnn {
public:
    mnn(std::string &blazeface_file, uint32_t threads_num);
    ~mnn();

public:
    void face_detect(const std::shared_ptr<image_cache> &frame,
            std::vector<cv::Rect> &faces, const int32_t min_face = 64);

public:
    static void flag_faces(const std::shared_ptr<image_cache> &frame,
            std::vector<cv::Rect> &faces);

private:
    mnn(mnn&&) = delete;
    mnn(const mnn&) = delete;
    mnn& operator=(mnn&&) = delete;
    mnn& operator=(const mnn&) = delete;

private:
    struct face_args face_args;
    std::shared_ptr<MNN::Interpreter> b_net;
    MNN::Session *b_session;
    MNN::Tensor *b_input;
    MNN::Tensor *b_out_scores;
    MNN::Tensor *b_out_boxes;
    MNN::Tensor *b_out_anchors;
};

} //namespace media

#endif //STARS_MNN_H
