#ifndef FUNC_H_
#define FUNC_H_

#include <opencv2/opencv.hpp>
using namespace cv;

extern RNG rng;

//查找魔方图片的轮廓
void findCubeContours(const Mat& image, vector<vector<Point> >& o_hull);

//去除最外界边框
void cutOutsideContour(vector<vector<Point> >& io_contours);

//将bgr格式的颜色转换为0xffffff格式的颜色值
int bgrTohex(uchar i_b, uchar i_g, uchar i_r);

//将0xffffff格式的颜色转换为bgr格式的颜色值
void hexTobgr(int i_hex, uchar& o_b, uchar& o_g, uchar& o_r);

#endif