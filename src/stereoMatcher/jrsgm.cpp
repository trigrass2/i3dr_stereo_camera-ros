#include <stereoMatcher/jrsgm.h>

//Initialise matcher
void jrsgm::init(std::string &sConfigFile, cv::Size _image_size)
{
  this->image_size = _image_size;
  std::cout << sConfigFile << std::endl;
  JR::Phobos::ReadIniFile(params, sConfigFile);
  createMatcher();
}

int jrsgm::getErrorDisparity(void)
{
  return -10000;
}

int jrsgm::round_up_to_32(int val)
{
  return round(val / 32) * 32;
}

size_t jrsgm::checkMemoryAvailable()
{
  //re-calculate current CUDA GPU memory
  cudaMemory.calcMem();
  size_t memFree = cudaMemory.getMemFree();
  //size_t memUsed = cudaMemory.getMemUsed();
  //size_t memTotal = cudaMemory.getMemTotal();
  /*
  std::cout << "GPU MEMORY FREE: " << float(memFree) / 1073741824. << std::endl;
  std::cout << "GPU MEMORY USED: " << float(memUsed) / 1073741824. << std::endl;
  std::cout << "GPU MEMORY TOTAL: " << float(memTotal) / 1073741824. << std::endl;
  */
  return (memFree);
}

int jrsgm::checkMemoryCensus(int image_width, int image_height)
{
  int num_of_gpus = params.oGPUs.size();
  int num_of_pyramids = params.nNumberOfPyramids;
  int window_size_x = params.oPyramidParams[0].oMetricParams.nWindowSizeX;
  int window_size_y = params.oPyramidParams[0].oMetricParams.nWindowSizeY;
  int num_of_slanted_window_scales = params.oPyramidParams[0].oMetricParams.nNumberOfScales;
  int num_of_slanted_window_shifts = params.oPyramidParams[0].oMetricParams.nNumberOfSlants;
  int have_no_data = params.bHasNodata;

  bool enMeanCensus = true;
  int NumberOfNeededCensusBytes;
  if (enMeanCensus)
  {
    NumberOfNeededCensusBytes = round_up_to_32(window_size_x * window_size_y * (1 + have_no_data)) / 4 * (num_of_slanted_window_scales + num_of_slanted_window_shifts * 2);
  }
  else
  {
    NumberOfNeededCensusBytes = round_up_to_32((window_size_x * window_size_y - 1) * (1 + have_no_data)) / 4 * (num_of_slanted_window_scales + num_of_slanted_window_shifts * 2);
  }
  int MemoryCensus_PerGPU = (num_of_pyramids * (2 * image_width * image_height * NumberOfNeededCensusBytes)) / num_of_gpus;
  return MemoryCensus_PerGPU;
}

int jrsgm::checkMemoryDSI(int image_width, int image_height)
{
  int num_of_pyramids = params.nNumberOfPyramids;
  int num_of_disparities = params.oPyramidParams[0].nMaximumNumberOfDisparities;
  bool enTextureDSI = params.oPyramidParams[0].oSGMParams.bUseDSITexture;
  int texture_dsi;
  if (enTextureDSI)
  {
    texture_dsi = 2;
  }
  else
  {
    texture_dsi = 1;
  }
  int MemoryDSI_PerGPU = num_of_pyramids * (2 * image_width * image_height * num_of_disparities);
  return MemoryDSI_PerGPU;
}

int jrsgm::checkMemoryExtra(int image_width, int image_height)
{
  int num_of_pyramids = params.nNumberOfPyramids;
  return (num_of_pyramids * (image_width * image_height * 4 * 10));
}

bool jrsgm::checkMemoryValid(int image_width, int image_height)
{
  int memoryDSI = checkMemoryDSI(image_width, image_height);
  int memoryCensus = checkMemoryCensus(image_width, image_height);
  int memoryExtra = checkMemoryExtra(image_width, image_height);
  int memoryUsed = memoryDSI + memoryCensus + memoryExtra;
  size_t memoryAvaiable = checkMemoryAvailable();
  std::cout << "Image size: (" << image_width << "," << image_height << ")" << std::endl;
  std::cout << "GPU MEMORY USED: " << float(memoryUsed) / 1073741824. << std::endl;
  std::cout << "GPU MEMORY AVAILABLE: " << float(memoryAvaiable) / 1073741824. << std::endl;
  if ((memoryUsed) < memoryAvaiable)
  {
    isMemoryValid = true;
    return true;
  }
  else
  {
    isMemoryValid = false;
    return false;
  }
}

void jrsgm::setImages(cv::Mat left_image, cv::Mat right_image)
{
  left_image.copyTo(this->image_left);
  right_image.copyTo(this->image_right);
  image_size = left_image.size();
}

//compute disparity
int jrsgm::compute(cv::Mat &disp)
{
  std::cout << "[jrsgm] Starting match..." << std::endl;
  if (isMemoryValid)
  {
    if (matcher_handle != nullptr)
    {
      std::string sgm_log = "./sgm_log.txt";
      try
      {
        JR::Phobos::MatchStereo(matcher_handle,
                                image_left,
                                image_right,
                                cv::Mat(),
                                cv::Mat(),
                                disp,
                                sgm_log,
                                JR::Phobos::e_logError);
      }
      catch (...)
      {
        std::cerr << "FAILED TO COMPUTE" << std::endl;
        return -1;
      }
    }
    else
    {
      std::cerr << "Matcher handle not found" << std::endl;
      createMatcher();
      return -3;
    }
  }
  else
  {
    return -2;
    std::cerr << "Invalid GPU memory for stereo match" << std::endl;
  }
  std::cout << "[jrsgm] Match complete." << std::endl;
  return 0;
}

//backward match disparity
int jrsgm::backwardMatch(cv::Mat &disp)
{
  isMemoryValid = true;
  if (isMemoryValid)
  {
    std::string sgm_log = "./sgm_log.txt";
    try
    {
      JR::Phobos::MatchStereo(matcher_handle,
                              image_left,
                              image_right,
                              cv::Mat(),
                              cv::Mat(),
                              disp,
                              sgm_log,
                              JR::Phobos::e_logError);
    }
    catch (...)
    {
      std::cerr << "FAILED TO COMPUTE" << std::endl;
      return -1;
    }
  }
  else
  {
    return -2;
    std::cerr << "Invalid GPU memory for stereo match" << std::endl;
  }
  return 0;
}

void jrsgm::setSpeckleDifference(float diff){
  std::cout << "Speckle difference: " << diff << std::endl;
  diff = diff / 10;
  for (auto &pyramid : params.oPyramidParams)
  {
    pyramid.fSpeckleMaxDiff = diff;
  }
  createMatcher();
}

void jrsgm::setSpeckleSize(int size){
  std::cout << "Speckle size: " << size << std::endl;
  size = size / 10;
  for (auto &pyramid : params.oPyramidParams)
  {
    pyramid.iSpeckleMaxSize = size;
  }
  createMatcher();
}

void jrsgm::setP1(float P1)
{
  float P1_scaled = P1 / 1000;
  for (auto &pyramid : params.oPyramidParams)
  {
    pyramid.oSGMParams.fP1_E_W = P1_scaled;
    pyramid.oSGMParams.fP1_SE_NW = P1_scaled;
    pyramid.oSGMParams.fP1_SW_NE = P1_scaled;
    pyramid.oSGMParams.fP1_S_N = P1_scaled;
  }
  createMatcher();
}

void jrsgm::setP2(float P2)
{
  float P2_scaled = P2 / 1000;
  for (auto &pyramid : params.oPyramidParams)
  {
    pyramid.oSGMParams.fP2_E_W = P2_scaled;
    pyramid.oSGMParams.fP2_SE_NW = P2_scaled;
    pyramid.oSGMParams.fP2_SW_NE = P2_scaled;
    pyramid.oSGMParams.fP2_S_N = P2_scaled;
  }
  createMatcher();
}

void jrsgm::setWindowSize(int census_size)
{
  if (census_size % 2 == 0)
  {
    census_size++;
  }
  
  for (auto &pyramid : params.oPyramidParams)
  {
    pyramid.oMetricParams.nWindowSizeX = census_size;
    pyramid.oMetricParams.nWindowSizeY = census_size;
  }
  
  //params.oPyramidParams[0].oMetricParams.nWindowSizeX = census_size;
  //params.oPyramidParams[0].oMetricParams.nWindowSizeY = census_size;

  createMatcher();
}

void jrsgm::setDisparityShift(int shift)
{
  //params.fTopPredictionShift = shift / pow(2, params.nNumberOfPyramids - 1);
  //params.fTopPredictionShift = shift/10;
  /*
  std::cout << "disparity: " << shift << std::endl;
  double x2 = -0.0000463226511471872;
  double x = 0.2008471924;
  double c = -22.52815177235;
  double shift_d = (double)shift;

  double shift_t = (x2*(shift_d*shift_d)) + (x*shift_d) + c;
  int shift_p = -(int)shift_t;
  std::cout << "shift: " << shift_p << std::endl;
  */
  std::cout << "shift: " << shift << std::endl;
  double shift_p = (double)shift/100;
  std::cout << "shift_p: " << shift_p << std::endl;
  params.fTopPredictionShift = shift_p;
  createMatcher();
}

void jrsgm::enableSubpixel(bool enable)
{
  params.oFinalSubPixelParameters.bCompute = enable;

  createMatcher();
}

void jrsgm::setDisparityRange(int n)
{
  double x2 = -0.0004562532;
  double x = 1.9971518547;
  double c = -221.5576254807;

  //disparity_range = (x2*(n*n)) + (x*n) + c;
  disparity_range = n / 10;
  //double sub_pix_disparity_range = disparity_range;
  //force odd number
  if (disparity_range % 2 == 0)
  {
    disparity_range++;
  }
  /* Set disparity range for all pyramids */
  params.oPyramidParams[0].nMaximumNumberOfDisparities = disparity_range;
  params.oPyramidParams[1].nMaximumNumberOfDisparities = disparity_range;
  params.oPyramidParams[2].nMaximumNumberOfDisparities = disparity_range;
  params.oPyramidParams[3].nMaximumNumberOfDisparities = disparity_range;
  params.oPyramidParams[4].nMaximumNumberOfDisparities = disparity_range;
  params.oFinalSubPixelParameters.nMaximumNumberOfDisparities = disparity_range/10;
  createMatcher();
}

void jrsgm::enableTextureDSI(bool enable)
{
  for (auto &pyramid : params.oPyramidParams)
  {
    pyramid.oSGMParams.bUseDSITexture = enable;
  }
  createMatcher();
}

void jrsgm::enableInterpolation(bool enable)
{
  /* Toggle interpolation */
  for (auto &pyramid : params.oPyramidParams)
  {
    pyramid.bInterpol = enable;
  }

  params.oFinalSubPixelParameters.bInterpol = enable;
  createMatcher();
}

void jrsgm::enableOcclusionDetection(bool enable)
{
  /* Toggle occlusion detection */
  for (auto &pyramid : params.oPyramidParams)
  {
    pyramid.bOcclusionDetection = enable;
  }

  params.oFinalSubPixelParameters.bOcclusionDetection = enable;

  createMatcher();
}

void jrsgm::enableOccInterpol(bool enable)
{
  /* Toggle occlusion interpolation */
  for (auto &pyramid : params.oPyramidParams)
  {
    pyramid.bOccInterpol = enable;
  }

  params.oFinalSubPixelParameters.bOccInterpol = enable;

  createMatcher();
}

void jrsgm::createMatcher()
{
  if (matcher_handle != nullptr)
  {
    JR::Phobos::DestroyMatchStereoHandle(matcher_handle);
  }
  try
  {
    //checkMemoryValid(image_size.width, image_size.height);
    isMemoryValid = true;
    if (isMemoryValid)
    {
      std::cout << "Re-creating matcher with new paramters..." << std::endl;
      matcher_handle = JR::Phobos::CreateMatchStereoHandle(params);
      std::cout << "Re-created matcher with new paramters." << std::endl;
    }
    else
    {
      std::cerr << "NOT ENOUGH GPU MEMORY AVAILABLE" << std::endl;
    }
  }
  catch (...)
  {
    std::cerr << "Failed to create I3DR matcher with chosen parameters" << std::endl;
    //std::cerr << ex.what() << std::endl;
  }
}