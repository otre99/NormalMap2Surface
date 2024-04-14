#ifndef UTIL_FUNCTIONS_H
#define UTIL_FUNCTIONS_H
#include <chrono>
#include <opencv2/opencv.hpp>

using TimePoint = std::chrono::high_resolution_clock::time_point;

TimePoint GetTimePoint();
double GetElapsedTime(const TimePoint &tp);

void SaveMatToBinary(cv::Mat &data, const std::string &fname);

void SaveMatToDepthMap(cv::Mat &data, const std::string &fname);
std::string GetOutputBaseName(const std::string &output, int lrtb_n, int opt_n);
#endif // UTIL_FUNCTIONS_H
