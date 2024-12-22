#include "nnabla_runtime.h"

using namespace cv;

CameraInference::CameraInference()
    : stopFlag(false), frameAvailable(false)
{
    pthread_mutex_init(&frameMutex, nullptr);
}

CameraInference::~CameraInference()
{
    stop();
    pthread_mutex_destroy(&frameMutex);
}

void CameraInference::initializeNnabla(const std::string &nnpFile, const std::string &executorName)
{
    // Load NNP and executor
    nbla::Context cpu_ctx{{"cpu:float"}, "CpuCachedArray", "0"};
    nnp = std::make_unique<nbla::utils::nnp::Nnp>(cpu_ctx);
    nnp->add(nnpFile);
    executor = nnp->get_executor(executorName);
    executor->set_batch_size(1);
}

void CameraInference::initializeCamera(int width, int height, int framerate)
{
    camera = std::make_unique<lccv::PiCamera>();
    camera->options->video_height = height;
    camera->options->video_width = width;
    camera->options->framerate = framerate;
    camera->startVideo();
}

void CameraInference::start()
{
    pthread_create(&cameraThread, nullptr, cameraThreadFuncWrapper, this);
    pthread_create(&inferenceThread, nullptr, inferenceThreadFuncWrapper, this);
}

void CameraInference::stop()
{
    pthread_join(cameraThread, nullptr);
    pthread_join(inferenceThread, nullptr);
    stopFlag = true;
    std::cout << "Inference and camera stopped." << std::endl;
}

const char *CameraInference::handSigns[4] = {"rock", "paper", "scissors", "none"};

void *CameraInference::cameraThreadFunc()
{
    Mat frame;

    while (!stopFlag)
    {
        if (!camera->getVideoFrame(frame, 1000))
        {
            std::cerr << "Camera error" << std::endl;
            continue;
        }

        pthread_mutex_lock(&frameMutex);
        globalFrame = frame.clone();
        frameAvailable = true;
        pthread_mutex_unlock(&frameMutex);

        usleep(100000);
    }
    camera->stopVideo();
    camera.reset();
    return nullptr;
}

void *CameraInference::inferenceThreadFunc()
{
    nbla::Context cpu_ctx{{"cpu:float"}, "CpuCachedArray", "0"};
    auto x = executor->get_data_variables().at(0).variable;
    float *data = x->variable()->cast_data_and_get_pointer<float>(cpu_ctx);
    int frameCount = 0;

    while (!stopFlag)
    {
        pthread_mutex_lock(&frameMutex);
        if (!frameAvailable)
        {
            pthread_mutex_unlock(&frameMutex);
            usleep(50000);
            continue;
        }

        Mat frame = globalFrame.clone();
        frameAvailable = false;
        pthread_mutex_unlock(&frameMutex);

        Mat processedFrame = processFrame(frame);

        try
        {
            convertToMnistFormat(processedFrame, data);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error processing image: " << e.what() << std::endl;
            continue;
        }

        executor->execute();

        auto y = executor->get_output_variables().at(0).variable;
        const float *y_data = y->variable()->get_data_pointer<float>(cpu_ctx);
        int prediction = 0;
        float max_score = -1e10;

        std::cout << "Frame " << frameCount++ << std::endl;
        std::cout << "Prediction scores:" << std::endl;
        for (int i = 0; i < 4; ++i)
        {
            if (y_data[i] > max_score)
            {
                prediction = i;
                max_score = y_data[i];
            }
            std::cout << " " << i << ": " << y_data[i] << std::endl;
        }
        if (max_score > 0.7f)
        {
            std::cout << "Predicted label: " << handSigns[prediction] << " (score: " << max_score << ")" << std::endl;
        }
        else
        {
            std::cout << "Predicted label: None" << std::endl;
        }
    }
    nbla::SingletonManager::clear();

    return nullptr;
}

Mat CameraInference::processFrame(const Mat &frame)
{
    Rect roi(OFFSET_X, OFFSET_Y, CLIP_WIDTH, CLIP_HEIGHT);
    Mat croppedFrame = frame(roi);

    Mat resizedFrame;
    resize(croppedFrame, resizedFrame, Size(DNN_WIDTH, DNN_HEIGHT));

    Mat grayFrame;
    cvtColor(resizedFrame, grayFrame, COLOR_BGR2GRAY);

    return grayFrame;
}

void CameraInference::convertToMnistFormat(const Mat &image, float *data)
{
    if (image.cols != 28 || image.rows != 28)
        throw std::runtime_error("Image size must be 28x28.");

    if (image.type() != CV_8U)
        throw std::runtime_error("Image must be single-channel 8-bit.");

    for (int i = 0; i < 28 * 28; i++)
    {
        data[i] = static_cast<float>(image.data[i]) / 255.0f;
    }
}

void *CameraInference::cameraThreadFuncWrapper(void *arg)
{
    return static_cast<CameraInference *>(arg)->cameraThreadFunc();
}

void *CameraInference::inferenceThreadFuncWrapper(void *arg)
{
    return static_cast<CameraInference *>(arg)->inferenceThreadFunc();
}