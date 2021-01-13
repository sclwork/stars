//
// Created by scliang on 1/13/21.
//

#include <regex>
#include <MNN/MNNDefine.h>
#include <MNN/Tensor.hpp>
#include <MNN/ImageProcess.hpp>
#include "log.h"
#include "mnn.h"

#define log_d(...)  LOG_D("Media-Native:mnn", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:mnn", __VA_ARGS__)

#define INPUT_SIZE      128
#define OUTPUT_NUM      960
#define X_SCALE         10.0f
#define Y_SCALE         10.0f
#define H_SCALE         5.0f
#define W_SCALE         5.0f
#define score_threshold 0.5f
#define nms_threshold   0.45f

namespace media {

static float iou(struct face_args &face_args, const cv::Rect &box0, const cv::Rect &box1) {
    face_args.xmin0 = box0.x;
    face_args.ymin0 = box0.y;
    face_args.xmax0 = (float)box0.x + box0.width;
    face_args.ymax0 = (float)box0.y + box0.height;

    face_args.xmin1 = box1.x;
    face_args.ymin1 = box1.y;
    face_args.xmax1 = (float)box1.x + box1.width;
    face_args.ymax1 = (float)box1.y + box1.height;

    face_args.aw = fmax(0.0f, fmin(face_args.xmax0, face_args.xmax1) - fmax(face_args.xmin0, face_args.xmin1));
    face_args.ah = fmax(0.0f, fmin(face_args.ymax0, face_args.ymax1) - fmax(face_args.ymin0, face_args.ymin1));

    face_args.ai = face_args.aw * face_args.ah;
    face_args.au = (face_args.xmax0 - face_args.xmin0) * (face_args.ymax0 - face_args.ymin0) +
            (face_args.xmax1 - face_args.xmin1) * (face_args.ymax1 - face_args.ymin1) - face_args.ai;

    if (face_args.au <= 0.0) return 0.0f;
    else                     return face_args.ai / face_args.au;
}

} //namespace media

media::mnn::mnn(std::string &blazeface_file, uint32_t threads_num):face_args() {
    log_d("created.");
    std::regex re{ ";" };
    std::vector<std::string> names {
            std::sregex_token_iterator(blazeface_file.begin(), blazeface_file.end(), re, -1),
            std::sregex_token_iterator()
    };
    if (!names.empty()) {
        log_d("use blazeface file: %s, threads num: %d.", names[0].c_str(), threads_num);
        b_net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(names[0].c_str()));
        if (b_net == nullptr) {
            log_e("b_net create fail.");
        } else {
            MNN::ScheduleConfig config;
            config.numThread = threads_num;
            config.type = MNNForwardType::MNN_FORWARD_CPU;
            config.backupType = MNNForwardType::MNN_FORWARD_OPENCL;

            MNN::BackendConfig backendConfig;
            backendConfig.precision = MNN::BackendConfig::Precision_Low;
            backendConfig.power = MNN::BackendConfig::Power_High;
            config.backendConfig = &backendConfig;

            b_session = b_net->createSession(config);
            b_input = b_net->getSessionInput(b_session, nullptr);
            b_out_scores = b_net->getSessionOutput(b_session, "convert_scores");
            b_out_boxes = b_net->getSessionOutput(b_session, "Squeeze");
            b_out_anchors = b_net->getSessionOutput(b_session, "anchors");
            log_d("b_net create success.");
        }
    } else {
        b_net = nullptr;
        log_e("b_net create fail.");
    }
}

media::mnn::~mnn() {
    b_net->releaseModel();
    log_d("release.");
}

void media::mnn::face_detect(const std::shared_ptr<image_cache> &frame,
        std::vector<cv::Rect> &faces, const int32_t min_face) {
    int32_t width, height; uint32_t *data;
    frame->get(&width, &height, &data);

    cv::Mat img;
    cv::Mat origin(height, width, CV_8UC4, (unsigned char *) data);
    cvtColor(origin, img, cv::COLOR_RGBA2BGR);

    int32_t raw_image_width  = img.cols;
    int32_t raw_image_height = img.rows;

    cv::Mat image;
    cv::resize(img, image, cv::Size(INPUT_SIZE, INPUT_SIZE));
    image.convertTo(image, CV_32FC3);
    image = (image * 2 / 255.0f) - 1;

    std::vector<int32_t> dims{1, INPUT_SIZE, INPUT_SIZE, 3};
    auto nhwc_tensor = MNN::Tensor::create<float>(dims, nullptr, MNN::Tensor::TENSORFLOW);
    auto nhwc_data = nhwc_tensor->host<float>();
    auto nhwc_size = nhwc_tensor->size();
    ::memcpy(nhwc_data, image.data, nhwc_size);

    b_input->copyFromHostTensor(nhwc_tensor);
    b_net->runSession(b_session);

    MNN::Tensor tensor_scores_host(b_out_scores, b_out_scores->getDimensionType());
    MNN::Tensor tensor_boxes_host(b_out_boxes, b_out_boxes->getDimensionType());
    MNN::Tensor tensor_anchors_host(b_out_anchors, b_out_anchors->getDimensionType());

    b_out_scores->copyToHostTensor(&tensor_scores_host);
    b_out_boxes->copyToHostTensor(&tensor_boxes_host);
    b_out_anchors->copyToHostTensor(&tensor_anchors_host);

    auto scores_dataPtr  = tensor_scores_host.host<float>();
    auto boxes_dataPtr   = tensor_boxes_host.host<float>();
    auto anchors_dataPtr = tensor_anchors_host.host<float>();

    std::vector<cv::Rect> tmp_faces;
    for(int32_t i = 0; i < OUTPUT_NUM; ++i) {
        face_args.ycenter = boxes_dataPtr[i*4 + 0] / Y_SCALE  * anchors_dataPtr[i*4 + 2] + anchors_dataPtr[i*4 + 0];
        face_args.xcenter = boxes_dataPtr[i*4 + 1] / X_SCALE  * anchors_dataPtr[i*4 + 3] + anchors_dataPtr[i*4 + 1];
        face_args.bh       = exp(boxes_dataPtr[i*4 + 2] / H_SCALE) * anchors_dataPtr[i*4 + 2];
        face_args.bw       = exp(boxes_dataPtr[i*4 + 3] / W_SCALE) * anchors_dataPtr[i*4 + 3];

        face_args.ymin    = (float)(face_args.ycenter - face_args.bh * 0.5) * (float)raw_image_height;
        face_args.xmin    = (float)(face_args.xcenter - face_args.bw * 0.5) * (float)raw_image_width;
        face_args.ymax    = (float)(face_args.ycenter + face_args.bh * 0.5) * (float)raw_image_height;
        face_args.xmax    = (float)(face_args.xcenter + face_args.bw * 0.5) * (float)raw_image_width;

        face_args.nonface_prob = exp(scores_dataPtr[i*2 + 0]);
        face_args.face_prob    = exp(scores_dataPtr[i*2 + 1]);

        face_args.ss           = face_args.nonface_prob + face_args.face_prob;
        face_args.nonface_prob /= face_args.ss;
        face_args.face_prob    /= face_args.ss;

        if (face_args.face_prob > score_threshold &&
            face_args.xmax - face_args.xmin >= (float)min_face &&
            face_args.ymax - face_args.ymin >= (float)min_face) {
            cv::Rect tmp_face;
            tmp_face.x = face_args.xmin;
            tmp_face.y = face_args.ymin;
            tmp_face.width  = face_args.xmax - face_args.xmin;
            tmp_face.height = face_args.ymax - face_args.ymin;
            tmp_faces.push_back(tmp_face);
        }
    }

    int32_t N = tmp_faces.size();
    std::vector<int32_t> labels(N, -1);
    for(int32_t i = 0; i < N-1; ++i) {
        for (int32_t j = i+1; j < N; ++j) {
            cv::Rect pre_box = tmp_faces[i];
            cv::Rect cur_box = tmp_faces[j];
            float iou_ = iou(face_args, pre_box, cur_box);
            if (iou_ > nms_threshold) {
                labels[j] = 0;
            }
        }
    }

    faces.clear();
    for (int32_t i = 0; i < N; ++i) {
        if (labels[i] == -1) {
            faces.push_back(tmp_faces[i]);
        }
    }
}

void media::mnn::flag_faces(const std::shared_ptr<image_cache> &frame,
        std::vector<cv::Rect> &faces) {
    if (faces.empty()) {
        return;
    }

    int32_t width, height; uint32_t *data;
    frame->get(&width, &height, &data);
    cv::Mat img(height, width, CV_8UC4, (unsigned char *) data);
    for (const auto &face : faces) {
        cv::rectangle(img, face, cv::Scalar(255, 0, 0, 255), 4);
    }
}
