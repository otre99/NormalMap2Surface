#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H
#include <opencv2/opencv.hpp>
void dp(cv::Mat m, std::string title);

cv::Mat NormalMapImageToFxFyImage(const cv::Mat &nm_img, const float dx = 1.0,
                                  const float dy = 1.0);

cv::Mat IntegrateLRTB(const cv::Mat &fxfy, const cv::Mat &valid_pts);

#ifdef HAVE_CUDA 
void OPTIteration(const cv::cuda::Mat &dZ,
                  const cv::gpu::Mat &invalid_pixels,
                  const cv::gpu::Mat &src,
                  const cv::gpu::Mat &dst);
#else
void OPTIteration(const cv::Mat &dZ,
                  const cv::Mat &invalid_pixels,
                  const cv::Mat &src,
                  cv::Mat &dst);
#endif

void GetRotateProblem(const cv::Mat &nm, 
                      const cv::Mat &invalid, 
                      const double angle,
                      cv::Mat &nm_rotated, 
                      cv::Mat &invalid_rotated);

void SetBordersToInvalid(cv::Mat &valid_pts);
void SetCircularMask(cv::Mat &valid_pixels);
cv::Mat GenMask(const cv::Size &ss, const std::string &mask_file);
cv::Mat calculateDeltaZ(const cv::Mat &fxfy);

#endif // MATH_FUNCTIONS_H
