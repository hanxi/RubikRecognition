#include "func.h"

RNG rng(12345);


//求出魔方的轮廓
void findCubeContours(const Mat& image, vector<vector<Point> >& o_contours)
{
	/// 转成灰度图并进行模糊降噪
	Mat grayImage;
	cvtColor(image, grayImage, CV_BGR2GRAY);
	blur(grayImage, grayImage, Size(3,3));

	Mat image_copy = image.clone();
	Mat threshold_output;
	vector<Vec4i> hierarchy;
	int thresh = 100;
//	int max_thresh = 255;

	/// 对图像进行二值化
	threshold(grayImage, threshold_output, thresh, 255, THRESH_BINARY);

	/// 寻找轮廓
	findContours(threshold_output, o_contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// 对每个轮廓计算其凸包
//	o_hull.resize(contours.size());
//	for (int i = 0; i < contours.size(); i++)
//	{  
//		convexHull(Mat(contours[i]), o_hull[i], false); 
//	}
	/// 绘出轮廓
	Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
	for( unsigned int i = 0; i< o_contours.size(); i++ )
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
		drawContours(drawing, o_contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());
	}

	/// 把结果显示在窗体
//	namedWindow("contours demo", CV_WINDOW_AUTOSIZE);
//	imshow("contours demo", drawing);
}

//去除最外界边框
void cutOutsideContour(vector<vector<Point> >& io_contours)
{
	RNG rng(12345);
	//去除最外界边框
	vector<vector<Point> > contours2(0);
	int ki = -1;//标记该去除的边框的序号
	for (unsigned int i=0; i<io_contours.size(); i++)
	{
		for (unsigned int j=0; j<io_contours[i].size(); j++)
		{
			if (io_contours[i][j].x < 5 )//近窗体边框
			{
				ki = i;
				break;
			}
		}
		if (ki == -1)
		{
			contours2.push_back(io_contours[i]);
		}
		else
		{
			ki = -1;
		}
	}

	for (unsigned int i = 0; i < contours2.size(); i++)
	{  
//		cout << contours2[i] << endl;
	}

	io_contours = contours2;
}

int bgrTohex(uchar i_b, uchar i_g, uchar i_r)
{
	return i_b << 16 | i_g << 8 | i_r;
}
void hexTobgr(int i_hex, uchar& o_b, uchar& o_g, uchar& o_r)
{
	//(1111 0000 0000 & xxxx xxxx xxxx) >> 16  即取出b分量
	o_b = ( 0xff <<16 & i_hex ) >> 16;
	o_g = ( 0xff <<8 & i_hex ) >> 8;
	o_r = 0xff & i_hex;
}
