#include "stereoMatcher/JRSGM/matcherJRSGM.h"

void MatcherJRSGM::init(void)
{
    //TODO convert to non QT
    /*
    std::string matcher_parameters = QStandardPaths::AppConfigLocation+"/stereo_bm_params.xml";
    if(QFile(matcher_parameters).exists()){
        try {
            matcher = cv::StereoBM::load<cv::StereoBM>(matcher_parameters.toStdString());
                  } catch (cv::Exception& e) {
            qDebug() << "Error loading block matching parameters" << e.msg.c_str();
            setupDefaultMatcher();
        }
    }else{
        setupDefaultMatcher();
    }
    */
    std::cout << param_file_ << std::endl;
    //readConfig(sConfigFile);
    JR::Phobos::ReadIniFile(params, param_file_);

    //TODO setup with ROS params

    setupDefaultMatcher();

    // Setup for 16-bit disparity
    cv::Mat(image_size, CV_16S).copyTo(disparity_lr);
    cv::Mat(image_size, CV_16S).copyTo(disparity_rl);
}

void MatcherJRSGM::setupDefaultMatcher(void)
{
    //set default values
    //enableInterpolation(false);
    //enableOcclusionDetection(false);
    //setWindowSize(9);
    //enableSubpixel(false);

    min_disparity = 0;
    disparity_range = 65;

    //setDisparityRange(21);
    //setMatchCosts(0.5, 1.5);
    matcher_handle = JR::Phobos::CreateMatchStereoHandle(params);
}

void MatcherJRSGM::forwardMatch()
{
    std::string sgm_log = "";
    try
    {
        JR::Phobos::MatchStereo(matcher_handle,
                                *left,
                                *right,
                                cv::Mat(),
                                cv::Mat(),
                                disparity_lr,
                                sgm_log,
                                JR::Phobos::e_logInfo);
    }
    catch (const std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }
    disparity_lr.convertTo(disparity_lr, CV_32F);
}

void MatcherJRSGM::backwardMatch()
{
    //TODO create backward matcher for JRSGM

    //auto right_matcher = cv::ximgproc::createRightMatcher(matcher);
    //right_matcher->compute(*right, *left, disparity_rl);
}