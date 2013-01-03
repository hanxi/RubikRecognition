#ifndef ONE_SIDE_PROCESS_H_
#define ONE_SIDE_PROCESS_H_

#include "data.h"
#include <opencv2/opencv.hpp>
#include <deque>
#include <vector>
#include <list>
using std::deque;
using std::list;
using std::vector;

using namespace cv;

/**
 * 处理魔方的一个面
 * 
 * 采用垂直扫描线的方法辨别魔方
 */

class OneSideProcess
{
public:
	OneSideProcess() {}
	void init(Mat* a_pSideImage, int a_dimension)
	{
		m_pSideImage = a_pSideImage;
		m_dimension = a_dimension;
		m_colors.clear();
	}
	void recognition(SIDE a_side, std::vector<int>& o_face);	//识别，将结果存入face数组对应位置
private:
	Mat* m_pSideImage;			//处理后的一面的图片
	int m_dimension;			//魔方的维数
	vector<Scalar> m_colors;	//该面魔方的颜色存储
};

//计算魔方一个面的位置
void cubeOneSidePosition(const vector<vector<Point> >& i_contours, RotatedRect& o_rect);

//找出左上角的坐标点
void getLeftTopPoint(const RotatedRect& i_rect, Point2f& o_pt);

//识别所选区域的颜色
Scalar getColor(const Mat& image, int x, int y, int w, int h);

//插入数据到bgrc表中
void insertToTable(const Mat& i_color, list<Vec2i>& io_bgrcTable);

//将bgrc表转换为bgrp表
void bgrcToBgrp(const list<Vec2i>& i_bgrcTable, int i_pixCount, vector<Bgrp>& o_bgrpTable);

//统计bgrp表的颜色的平均值
Scalar averColor(const vector<Bgrp>& i_bgrpTable);

#endif
