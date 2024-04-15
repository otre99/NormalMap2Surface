#include "math_functions.h"

#include <algorithm>
#include <execution>
#include <numeric>
#include <vector>

using namespace std;
using namespace cv;

//void dp(Mat m, string title) {
//  namedWindow(title, WINDOW_NORMAL);
//  imshow(title, m);
//}

Mat NormalMapImageToFxFyImage(const Mat &nm_img, const float dx,
                              const float dy) {
  Mat tmpf32;
  assert(nm_img.channels() == 3 &&
         (nm_img.depth() == CV_8U || nm_img.depth() == CV_16U));
  const double alpha = (nm_img.depth() == CV_8U) ? 2.0 / 255.0 : 2.0 / 65535.0;
  nm_img.convertTo(tmpf32, CV_32F, alpha, -1.0);

  Mat bgr[3];
  split(tmpf32, bgr);

  bgr[1] /= bgr[0];
  bgr[1] *= -dx;
  bgr[2] /= bgr[0];
  bgr[2] *= -dy;

  Mat result;
  merge(vector<Mat>{bgr[2], bgr[1]}, result);
  return result;
}

Mat IntegrateLRTB(const Mat &nm, const Mat &invalid) {
  //TODO(otre99): optimize this code
  const int rows = nm.rows;
  const int cols = nm.cols;

  Mat al = Mat(nm.size(), nm.depth(), Scalar(0.0));
  Mat ar = Mat(nm.size(), nm.depth(), Scalar(0.0));
  vector<int> hidxs(rows);
  iota(hidxs.begin(), hidxs.end(), 0);
  for_each(execution::par_unseq, hidxs.begin(), hidxs.end(), [&](const int i) {
    for (int j = 0; j < cols; ++j) {
      if (invalid.at<uchar>(i, j))
        continue;
      const float fx = nm.at<Vec2f>(i, j)[0];
      const float lfx = (fx + ((j > 0) ? nm.at<Vec2f>(i, j - 1)[0] : 0.0)) / 2;
      const float lz = (j > 0) ? al.at<float>(i, j - 1) : 0.0;
      al.at<float>(i, j) = lz + lfx;
    }
    for (int j = cols - 1; j > -1; --j) {
      if (invalid.at<uchar>(i, j))
        continue;
      const float fx = nm.at<Vec2f>(i, j)[0];
      const float rfx =
          (fx + ((j < cols - 1) ? nm.at<Vec2f>(i, j + 1)[0] : 0.0)) / 2;
      const float rz = (j < cols - 1) ? ar.at<float>(i, j + 1) : 0.0;
      ar.at<float>(i, j) = rz - rfx;
    }
  });

  Mat at = Mat(nm.size(), nm.depth(), Scalar(0.0));
  Mat ab = Mat(nm.size(), nm.depth(), Scalar(0.0));
  hidxs.resize(cols);
  iota(hidxs.begin(), hidxs.end(), 0);
  for_each(execution::par_unseq, hidxs.begin(), hidxs.end(), [&](const int j) {
    for (int i = 0; i < rows; ++i) {
      if (invalid.at<uchar>(i, j))
        continue;
      const float fy = nm.at<Vec2f>(i, j)[1];
      const float tfy = (fy + ((i > 0) ? nm.at<Vec2f>(i - 1, j)[1] : 0.0)) / 2;
      const float tz = (i > 0) ? at.at<float>(i - 1, j) : 0.0;
      at.at<float>(i, j) = tz - tfy;
    }
    for (int i = rows - 1; i > -1; --i) {
      if (invalid.at<uchar>(i, j))
        continue;
      const float fy = nm.at<Vec2f>(i, j)[1];
      const float bfy =
          (fy + ((i < rows - 1) ? nm.at<Vec2f>(i + 1, j)[1] : 0.0)) / 2;
      const float bz = (i < rows - 1) ? ab.at<float>(i + 1, j) : 0.0;
      ab.at<float>(i, j) = bz + bfy;
    }
  });
  return 0.25 * (al + ar + at + ab);
}

void OPTIteration(const Mat &dZ, const Mat &invalid_pixels, const Mat &src,
                  Mat &dst) {
  static const float kdata[] = {0.0,  0.25, 0.0,  0.25, 0.0,
                                0.25, 0.0,  0.25, 0.0};
  const Mat kernel(3, 3, CV_32FC1, (void *)kdata);
  filter2D(src, dst, src.depth(), kernel, Point{-1, -1}, 0.0, BORDER_DEFAULT);
  dst += dZ;
  dst.setTo({0.0}, invalid_pixels);
}

void GetRotateProblem(const Mat &nm, const Mat &invalid, const double angle,
                      Mat &nm_rotated, Mat &invalid_rotated) {
  const int rows = nm.rows;
  const int cols = nm.cols;
  const Point2f center{0.5f * nm.cols, 0.5f * nm.rows};
  const Mat R = getRotationMatrix2D(center, angle, 1.0);

  const double r11 = R.at<double>(0, 0);
  const double r12 = -R.at<double>(0, 1);
  const double r21 = -R.at<double>(1, 0);
  const double r22 = R.at<double>(1, 1);

  // // r11 r12
  // // r21 r22
  // cout << "ROT-Y (0,1) " << (r11*0.0 + r12*1.0) << " " << (r21*0.0 +
  // r22*1.0) << endl; cout << "ROT-X (1,0) " << (r11*1.0 + r12*0.0)
  // << " " << (r21*1.0 + r22*0.0) << endl; cout << "ROT-X (1,1) " <<
  // (r11*1.0 + r12*1.0) << " " << (r21*1.0 + r22*1.0) << endl;

  nm.copyTo(nm_rotated);
  vector<int> hidxs(rows);
  iota(hidxs.begin(), hidxs.end(), 0);
  for_each(execution::par, hidxs.begin(), hidxs.end(), [&](const int i) {
    for (int j = 0; j < cols; ++j) {
      const Vec2f &src = nm.at<Vec2f>(i, j);
      Vec2f &dst = nm_rotated.at<Vec2f>(i, j);
      dst[0] = r11 * src[0] + r12 * src[1];
      dst[1] = r21 * src[0] + r22 * src[1];
    }
  });
  warpAffine(nm_rotated, nm_rotated, R, nm_rotated.size(), INTER_LINEAR,
             BORDER_CONSTANT, Scalar(0.0, 0.0));
  warpAffine(invalid, invalid_rotated, R, nm_rotated.size(), INTER_LINEAR,
             BORDER_CONSTANT, Scalar(0));

  nm_rotated.setTo(Scalar{0.0, 0.0}, invalid_rotated);
}

void SetBordersToInvalid(Mat &valid_pts) {
  const int rows = valid_pts.rows;
  const int cols = valid_pts.cols;
  for (int i = 0; i < rows; ++i) {
    valid_pts.at<uchar>(i, 0) = 1;
    valid_pts.at<uchar>(i, cols - 1) = 1;
  }
  for (int j = 0; j < cols; ++j) {
    valid_pts.at<uchar>(0, j) = 1;
    valid_pts.at<uchar>(rows - 1, j) = 1;
  }
}

void SetCircularMask(Mat &invalid_pts) {
  const int rows = invalid_pts.rows;
  const int cols = invalid_pts.cols;
  const float cx = 0.5 * cols;
  const float cy = 0.5 * rows;
  const float r2 = pow(min(cx, cy), 2);
  for (int i = 0; i < rows; ++i) {
    const float y2 = pow(0.5 + i - cy, 2);
    for (int j = 0; j < cols; ++j) {
      const float x2 = pow(0.5 + j - cx, 2);
      if (x2 + y2 >= r2) {
        invalid_pts.at<uchar>(i, j) = 1;
      }
    }
  }
}

Mat GenMask(const Size &ss, const string &mask_file) {
  Mat invalid(ss, CV_8U, Scalar{0});
  SetBordersToInvalid(invalid);
  if (!mask_file.empty()) {
    auto mask = imread(mask_file, IMREAD_GRAYSCALE);
    invalid.setTo(Scalar{1}, mask == 0);
  }
  return invalid;
}

Mat calculateDeltaZ(const Mat &nm) {
  const int rows = nm.rows;
  const int cols = nm.cols;
  Mat dz = Mat(nm.size(), CV_32F);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      const float lfx = (j > 0) ? nm.at<Vec2f>(i, j - 1)[0] : 0.0;
      const float rfx = (j < cols - 1) ? nm.at<Vec2f>(i, j + 1)[0] : 0.0;
      const float tfy = (i > 0) ? nm.at<Vec2f>(i - 1, j)[1] : 0.0;
      const float bfy = (i < rows - 1) ? nm.at<Vec2f>(i + 1, j)[1] : 0.0;
      dz.at<float>(i, j) = 0.25 * (lfx - rfx - tfy + bfy);
    }
  }
  return dz;
}
