#include "stereoMatcher/matcherI3DRSGM.h"

void MatcherI3DRSGM::init(void)
{
    matcher = new I3DRSGM(param_file_, image_size_);

    setupDefaultMatcher();
    cv::Mat(image_size, CV_16S).copyTo(disparity_lr);
    cv::Mat(image_size, CV_16S).copyTo(disparity_rl);
}

void MatcherI3DRSGM::setupDefaultMatcher(void)
{
    //setOcclusionDetection(false);
    //matcher->enableOccInterpol(false);
    //matcher->enableTextureDSI(false);
    //matcher->enableSubpixel(true);
    //matcher->maxPyramid(4);
}

int MatcherI3DRSGM::forwardMatch()
{
    matcher->setImages(*left, *right);
    int exitCode = matcher->compute(disparity_lr);
    if (exitCode == 0)
    {
        disparity_lr.convertTo(disparity_lr, CV_32FC1,-16);
    }
    return exitCode;
}

int MatcherI3DRSGM::backwardMatch()
{
    int exitCode = matcher->backwardMatch(disparity_rl);
    if (exitCode == 0)
    {
        disparity_rl.convertTo(disparity_rl, CV_32FC1, -16);
    }
    return exitCode;
}

void MatcherI3DRSGM::setWindowSize(int window_size)
{
    this->window_size = window_size;
    matcher->setWindowSize(window_size);
}

void MatcherI3DRSGM::setDisparityRange(int disparity_range)
{
    this->disparity_range = disparity_range;
    matcher->setDisparityRange(disparity_range);
}

void MatcherI3DRSGM::setMinDisparity(int min_disparity)
{
    this->min_disparity = min_disparity;
    matcher->setDisparityShift(min_disparity);
}

void MatcherI3DRSGM::setInterpolation(bool enable)
{
    matcher->enableInterpolation(enable);
    this->interpolate = enable;
}

void MatcherI3DRSGM::setP1(float P1)
{
    matcher->setP1(P1);
}

void MatcherI3DRSGM::setP2(float P2)
{
    matcher->setP2(P2);
}

void MatcherI3DRSGM::setOcclusionDetection(bool enable)
{
    matcher->enableOcclusionDetection(enable);
}

void MatcherI3DRSGM::setSpeckleFilterWindow(int window)
{
    matcher->setSpeckleSize(window);
}
void MatcherI3DRSGM::setSpeckleFilterRange(int range)
{
    matcher->setSpeckleDifference(range);
}
void MatcherI3DRSGM::setPreFilterCap(int cap){
    matcher->maxPyramid(cap);
}