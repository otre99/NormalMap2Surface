#include <iostream>
#include <opencv2/opencv.hpp>

#include "math_functions.h"
#include "util_functions.h"
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>

using namespace std;
using namespace cv;

string keys = "{@nm_image  |  <none>  | normal map image path}"
              "{mask_image |          | mask for valid pixels}"
              "{output_file| out      | output file prefix name}"
              "{opt_iters  | 5000     | smoothing passes}"
              "{lrtb_iters | 180      | number of iterations}"
              "{scale      | 1.0      | spatial resolution}"
              "{help       |          | show help}";

void MethodLRTB(Mat &nm, Mat &invalid, Mat &surf, int niters) {
  printf("  LRBT method %9d iterations\n", niters);
  if (niters <= 0)
    return;
  SetCircularMask(invalid);
  nm.setTo(Vec2f{0.0, 0.0}, invalid);

  surf = IntegrateLRTB(nm, invalid);
  const double delta_angle = 90.0 / niters;
  const int dp_n = niters / 10;
  const Point2f center{0.5f * nm.cols, 0.5f * nm.rows};
  auto t1 = GetTimePoint();
  for (int i = 1; i < niters; ++i) {
    Mat rot_nm;
    Mat rot_invalid;
    const double angle = i * delta_angle;
    GetRotateProblem(nm, invalid, angle, rot_nm, rot_invalid);
    auto tmp = IntegrateLRTB(rot_nm, rot_invalid);
    const Mat R = getRotationMatrix2D(center, -angle, 1.0);
    warpAffine(tmp, tmp, R, tmp.size(), INTER_LINEAR, BORDER_CONSTANT,
               Scalar(0.0, 0.0));
    surf += tmp;
    if (i % dp_n == 0) {
      printf("    Progress: %5.2f%%  Iter/Sec: %7.2f     \r",
             (100.0 * i) / niters, i / GetElapsedTime(t1));
      fflush(stdout);
    }
  }
  surf /= niters;
}

void MethodOPTCUDA(Mat &nm, Mat &invalid, Mat &src, int niters) {
  printf("   OPT method %9d iterations             \n", niters);
  if (niters <= 0)
    return;

  auto t1 = GetTimePoint();
  const int dp_n = niters / 10;
  nm.setTo(cv::Scalar{0.0, 0.0}, invalid);
  static const float kdata[] = {0.0,  0.25, 0.0,  0.25, 0.0,
                                0.25, 0.0,  0.25, 0.0};
  const Mat kernel(3, 3, CV_32F, (void *)kdata);
  cuda::GpuMat deltaZ_cuda(calculateDeltaZ(nm));
  cuda::GpuMat invalid_cuda(invalid);
  cuda::GpuMat nm_cuda(nm);
  cuda::GpuMat src_cuda(src);
  cuda::GpuMat dst_cuda = src_cuda.clone();
  Ptr<cv::cuda::Filter> filter =
      cuda::createLinearFilter(CV_32F, CV_32F, kernel);

  for (int i = 0; i < niters; ++i) {
    filter->apply(src_cuda, dst_cuda);
    cv::cuda::add(dst_cuda, deltaZ_cuda, dst_cuda);
    dst_cuda.swap(src_cuda);
    src_cuda.setTo(0.0, invalid_cuda);

    if (i % dp_n == 0) {
      printf("    Progress: %5.2f%%  Iter/Sec: %7.2f     \r",
             (100.0 * i) / niters, i / GetElapsedTime(t1));
      fflush(stdout);
    }
  }
  src_cuda.download(src);
}

int main(int argc, char *argv[]) {
  CommandLineParser parser(argc, argv, keys);
  if (parser.has("help")) {
    parser.printMessage();
    return 0;
  }

  const string input_image_path = parser.get<string>(0);
  const string input_mask_path = parser.get<string>("mask_image");
  const string output_file = parser.get<string>("output_file");

  const float scale = parser.get<float>("scale");
  const int lrtb_iters = parser.get<int>("lrtb_iters");
  const int opt_iters = parser.get<int>("opt_iters");

  Mat nm = imread(input_image_path, IMREAD_UNCHANGED);
  nm = NormalMapImageToFxFyImage(nm, scale, scale);

  Mat invalid_pts = GenMask(nm.size(), input_mask_path);

  printf("Parameters: \n");
  printf("  Input image       : '%s' [%d x %d]\n", input_image_path.data(),
         nm.cols, nm.rows);
  printf("  LRTB interations  : %8d\n", lrtb_iters);
  printf("  OPT interations   : %8d\n", opt_iters);
  printf("  Spatial scale     : %f\n", scale);

  printf("Inflating...\n");
  auto t0 = GetTimePoint();
  Mat surf(nm.size(), CV_32F, {0.0});
  MethodLRTB(nm, invalid_pts, surf, lrtb_iters);
  MethodOPTCUDA(nm, invalid_pts, surf, opt_iters);
  printf("Done. Elapse time: %.2f seconds           \n", GetElapsedTime(t0));

  const string base_name =
      GetOutputBaseName(output_file, lrtb_iters, opt_iters);
  const string surf_file = base_name + ".r32";
  const string mask_file = base_name + ".png";

  printf("Saving surface to: '%s'\n", surf_file.data());
  SaveMatToBinary(surf, base_name + ".surf");

  printf("Saving mask to   : '%s'\n", mask_file.data());
  invalid_pts.setTo({255}, invalid_pts == 0);
  invalid_pts.setTo({0}, invalid_pts == 1);
  cv::imwrite(mask_file, invalid_pts);
  return 0;
}
