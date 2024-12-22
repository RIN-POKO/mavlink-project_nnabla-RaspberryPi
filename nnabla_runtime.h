// nnabla_runtime
#ifndef NNABLA_RUNTIME_H
#define NNABLA_RUNTIME_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>
#include <nbla/singleton_manager.hpp>
#include <nbla_utils/nnp.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <csignal> // SIGINT用
#include <pthread.h>
#include "LCCV/include/lccv.hpp"
#include "LCCV/include/libcamera_app.hpp"

using namespace cv;
using namespace std;

#define CAM_WIDTH (320)
#define CAM_HEIGHT (240)
#define OFFSET_X (104)
#define OFFSET_Y (0)
#define CLIP_WIDTH (112)
#define CLIP_HEIGHT (224)
#define DNN_WIDTH (28)
#define DNN_HEIGHT (28)

class CameraInference
{
public:
    CameraInference();
    ~CameraInference();

    static void *cameraThreadFuncWrapper(void *arg);
    static void *inferenceThreadFuncWrapper(void *arg);

    void initializeNnabla(const std::string &nnpFile, const std::string &executorName);
    void initializeCamera(int width, int height, int framerate);
    void *cameraThreadFunc();
    void *inferenceThreadFunc();
    void start();
    void stop();

private:
    std::unique_ptr<lccv::PiCamera> camera;
    std::unique_ptr<nbla::utils::nnp::Nnp> nnp;
    std::shared_ptr<nbla::utils::nnp::Executor> executor;
    pthread_t cameraThread, inferenceThread;
    pthread_mutex_t frameMutex;
    volatile sig_atomic_t stopFlag;
    cv::Mat globalFrame;
    volatile bool frameAvailable;

    static const char *handSigns[4];

    cv::Mat processFrame(const cv::Mat &frame);
    void convertToMnistFormat(const cv::Mat &image, float *data);
};

#endif // CAMERA_INFERENCE_H
