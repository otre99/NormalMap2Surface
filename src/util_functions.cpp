#include "util_functions.h"
#include <fstream>

TimePoint GetTimePoint() { return std::chrono::high_resolution_clock::now(); }

double GetElapsedTime(const TimePoint &tp) {
  const auto end = std::chrono::high_resolution_clock::now();
  auto count =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - tp).count();
  return static_cast<double>(count) / 1.0e9;
}

void SaveMatToBinary(cv::Mat &data, const std::string &fname) {
  std::ofstream ofile(fname, std::ios::binary);
  const int n = data.rows * data.cols;
  float img_size[2] = {float(data.rows), float(data.cols)};

  ofile.write(reinterpret_cast<char *>(img_size), 2 * sizeof(float));
  ofile.write(data.ptr<char>(0), n * sizeof(float));
  ofile.close();
}

void SaveMatToDepthMap(cv::Mat &data, const std::string &fname) {

  std::ofstream ofile(fname, std::ios::binary);

  double maxVal, minVal;
  cv::minMaxLoc(data, &minVal, &maxVal);
  const double dd = maxVal - minVal;
  const double pix16max = (1 << 16) - 1;

  cv::Mat out;
  data.convertTo(out, CV_16U, pix16max / dd, -(pix16max * minVal) / dd);
  cv::minMaxLoc(out, &minVal, &maxVal);
  cv::imwrite(fname, out);
}

std::string GetOutputBaseName(const std::string &output, int lrtb_n,
                              int opt_n) {
  char buffer[256];
  sprintf(buffer, "%s_lrtb%09d_opt%09d", output.data(), lrtb_n, opt_n);
  return std::string(buffer);
}
