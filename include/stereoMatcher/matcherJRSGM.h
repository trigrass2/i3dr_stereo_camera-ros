#ifndef MATCHERJRSGM_H
#define MATCHERJRSGM_H

#include "stereoMatcher/abstractStereoMatcher.h"
#include "stereoMatcher/jrsgm.h"

class MatcherJRSGM : public AbstractStereoMatcher
{
public:
    explicit MatcherJRSGM(std::string &param_file, cv::Size image_size)
        : AbstractStereoMatcher(param_file,image_size), param_file_(param_file), image_size_(image_size)
    {
        init();
    }

    int forwardMatch(void);
    int backwardMatch(void);

    void setWindowSize(int window_size);
    void setDisparityRange(int disparity_range);
    void setMinDisparity(int min_disparity);
    void setInterpolation(bool enable);
    void setP1(float p1);
    void setP2(float p2);
    void setOcclusionDetection(bool enable);
    void setSpeckleFilterWindow(int window);
    void setSpeckleFilterRange(int range);

    void setPreFilterCap(int cap); // used for max pyramids instead

    //Not used in JR SGM
    void setUniquenessRatio(int ratio){};
    void setTextureThreshold(int threshold){};
    void setDisp12MaxDiff(int diff){};
    void setPreFilterSize(int size){};

private:

    void init(void);
    void setupDefaultMatcher(void);

    std::string param_file_;
    cv::Size image_size_;

    jrsgm *JR_matcher;
};

#endif // MATCHERJRSGM_H