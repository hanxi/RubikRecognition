#ifndef IMAGE_PROCESS_H_
#define IMAGE_PROCESS_H_

#include "OneSideProcess.h"
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;

//平行直线
#include <cstdlib>
#include <cmath>
const float PI = 3.1415926;
const float D_ALPHA = 1.0/18.0;//大概相差10度
struct Parallels
{
	vector<Vec4i> lines;	//近似平行的直线
	float alpha;			//近似平行直线的平均倾斜角[-pi/2,pi/2]
	bool insertToParallels(const Vec4f& i_line)//返回是否插入成功
	{
		float k = static_cast<float>(i_line[3]-i_line[1])/(i_line[2]-i_line[0]);
		float ialpha = atan(k);
		if (lines.empty())
		{
			alpha = ialpha;
			lines.push_back(i_line);
			return true;
		}
		if (abs(alpha-ialpha) < D_ALPHA)
		{
			unsigned long size = lines.size();
			alpha = (alpha*size+ialpha)/(size+1);//取平均值
			lines.push_back(i_line);
			return true;
		}
		return false;
	}
};

/**
 *  将图片处理为魔方的数字结构
 */
class ImageProcess
{
public:
	ImageProcess(Mat* a_pImage):m_pImage(a_pImage)
	{
		m_Face.resize(54,0);				//6 X 3^2
		m_dimension = 3;					//维数默认为3
	}

	void recognition();	//识别，将结果存入face数组

private:
	Mat* m_pImage;					//待处理的图片
	int m_dimension;				//魔方的维数
	OneSideProcess m_oneSideProcess;//处理魔方的一个面
	std::vector<int> m_Face;		//数组，存储魔方的颜色分布
};

//取得魔方的六个顶点
void cubePosition(const vector<vector<Point> >& i_contours, vector<Point2f>& o_pts);

//寻找所有的条直线
void findLines(const Mat& i_image, vector<Vec4i>& o_lines);
void findLines(const Mat& i_image, vector<Vec4i>& o_lines, double rho, double theta, int threshold,double minLineLength, double maxLineGap);

//分离平行直线
void separateParallels(const vector<Vec4i>& i_lines, vector<Parallels>& o_parallelsLines);

//取平行线中最多的3组
void getMax3Parallels(vector<Parallels>& io_parallelsLines);
//去除很短的线段
void deleteShortLine(Parallels& io_parallels, const float minLineLength = 15.0);

//合并距离小的线段
void mergeMicroDistanceLine(Parallels& io_parallels, const float minDisLine=10.0);
//点p1(x1,y1)到点p2(x2,y2)的距离
float disP2P(float x1, float y1, float x2, float y2);
//点到直线的距离
float disP2Line(const Vec2i& i_P, const Vec4i i_lineB, Vec2i& o_Pe, const float minus=1.0);
//求平均线段之间的距离，如果线段不平行则求A的1个端点到B的距离
float disLine2Line(const Vec4i& i_lineA, const Vec4i i_lineB);
void solveMidLine(const Vec4i& i_lineA, const Vec4i i_lineB, Vec4i& o_lineD, const float minus=1.0);

//显示直线图结果
void showLineImage(const Mat& i_image, const vector<Vec4i>& i_lines, char ch);
//显示多给出点的图像
void showPointImage(const Mat& i_image, const vector<Vec2i>& i_points, char ch);

//求6条直线
void find6Lines(const vector<Parallels>& i_parallelsLines, const Vec2i& XY, vector<Vec4i>& o_lines6);
//求距离最短的直线
void minDisLine(const Parallels& i_parallels, const Vec2i i_p, Vec4i& o_lines);
//求6个顶点
void find6Points(const vector<Vec4i>& i_lines6, vector<Vec2i>& o_points6);
//求两直线的交点
void findIntersectionPoint(const Vec4i& i_Pa, const Vec4i& i_Pb, Vec2i& o_point);

void extractOneSide(const Mat& i_image, const vector<Vec2i>& i_srcTri, const vector<Vec2i>& i_dstTri, Mat& o_image);

//识别3个面。
void recognition3Side(const Mat& i_image, const vector<Vec2i>& i_points6,
						   Mat& o_LEFTImage, Mat& o_RIGHTImage,
						   Mat& o_UPImage);

#endif