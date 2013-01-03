#include "ImageProcess.h"
#include "func.h"

#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;
using std::swap;

/**
 * 魔方的3个面的识别
 *    /-----------/|
 *   /     UP    / |
 *  /           / T|
 * /-----------/ GH|
 * |           |RI/
 * |   LEFT    | /
 * |-----------|/
 */
//识别，将结果存入face数组
void ImageProcess::recognition()
{
	vector<vector<Point> > contours;
//	findCubeContours(*m_pImage, contours);
//	cutOutsideContour(contours);
//	vector<Point2f> pts;//魔方的6个顶点
//	cubePosition(contours, pts);
	vector<Vec4i> lines;
	findLines(*m_pImage, lines);//寻找直线
	vector<Parallels> linesClassifyByParallel;
	separateParallels(lines,linesClassifyByParallel);//分离平行直线
	int parallesSize = linesClassifyByParallel.size();
	cout << "linesClassifyByParallel.size="<< parallesSize << endl;
	int sum = 0;
	for (int i=0; i<parallesSize; i++)
	{
		int isize = linesClassifyByParallel[i].lines.size();
		sum += isize;
		cout << i <<":count=" << isize << endl;
	}
	cout << "总直线数：" << sum << endl;

	getMax3Parallels(linesClassifyByParallel);
	parallesSize = linesClassifyByParallel.size();
	cout << "linesClassifyByParallel.size="<< parallesSize << endl;
	sum = 0;
	for (int i=0; i<parallesSize; i++)
	{
		int isize = linesClassifyByParallel[i].lines.size();
		sum += isize;
		cout << i <<":count=" << isize
			<<",k=" << linesClassifyByParallel[i].alpha << endl;		
	}
	cout << "总直线数：" << sum << endl;

	//显示直线图结果
	Mat lineImage(m_pImage->size(), m_pImage->type());
	lineImage = Scalar::all(0);
	sum=0;
	for (int i=0; i<parallesSize; i++)
	{
		//去除很短的线段
		deleteShortLine(linesClassifyByParallel[i],15.0);
		//合并相关线段
		mergeMicroDistanceLine(linesClassifyByParallel[i]);
		//去除很短的线段
		deleteShortLine(linesClassifyByParallel[i],20.0);
		int isize = linesClassifyByParallel[i].lines.size();
		for (int j=0; j<isize; j++)
		{
			Vec4i ilines(linesClassifyByParallel[i].lines[j]);
			line(lineImage,Point(ilines[0],ilines[1]),
				Point(ilines[2],ilines[3]),
			    Scalar( 255, 255, 255),
				0,
				0);
		}
		sum += isize;
		cout << i <<":count=" << isize << endl;
	}
	imshow("lineImage2",lineImage);

	//求6条直线
	vector<Vec4i> lines6;
	find6Lines(linesClassifyByParallel, 
		Vec2i(m_pImage->size().width,m_pImage->size().height), 
		lines6);
	//显示直线图结果
	showLineImage(*m_pImage, lines6, '0');
	cout << "图片大小："<<m_pImage->size().width<<","<<m_pImage->size().height<<endl;
	//求6个顶点
	vector<Vec2i> points6;
	find6Points(lines6, points6);
	showPointImage(*m_pImage, points6, '1');
	Mat LEFTImage;//确定该面位置的顶点(1,2,3)
	Mat RIGHTImage;//确定该面位置的顶点(3,4,5)
	Mat UPImage;//确定该面位置的顶点(5,0,1)
	recognition3Side(*m_pImage, points6,
					LEFTImage, RIGHTImage,
					UPImage);
	imshow("LEFT",LEFTImage);
	imshow("RIGHT",RIGHTImage);
	imshow("UP",UPImage);
	m_oneSideProcess.init(&LEFTImage,m_dimension);	//暂时使用只有一面魔方的图片
	m_oneSideProcess.recognition(RIGHT,m_Face);
	m_oneSideProcess.init(&RIGHTImage,m_dimension);	//暂时使用只有一面魔方的图片
	m_oneSideProcess.recognition(RIGHT,m_Face);
	m_oneSideProcess.init(&UPImage,m_dimension);	//暂时使用只有一面魔方的图片
	m_oneSideProcess.recognition(UP,m_Face);

}

//显示直线图结果
void showLineImage(const Mat& i_image, const vector<Vec4i>& i_lines, char ch)
{
	Mat lineImage(i_image.size(), i_image.type());
	lineImage = Scalar::all(0);
	for (int i=0; i<i_lines.size(); i++)
	{
		line(lineImage,Point(i_lines[i][0],i_lines[i][1]),
			Point(i_lines[i][2],i_lines[i][3]),
		    Scalar( 255, 255, 255),
			0,
			0);
	}
	string s(1,ch);
	imshow(s.c_str(),lineImage);	
}
//显示给出点的图像
void showPointImage(const Mat& i_image, const vector<Vec2i>& i_points, char ch)
{
	Mat pointImage(i_image.size(), i_image.type());
	pointImage = Scalar::all(0);
	for (int i=0; i<i_points.size(); i++)
	{
		cout << "(" << i_points[i][0]
			<< "," << i_points[i][1] <<")"<< endl;
		line(pointImage,Point(i_points[i][0],i_points[i][1]),
			Point(i_points[i][0]+1,i_points[i][1]+1),
		    Scalar( 255, 255, 255),
			3,
			8);
	}
	string s(1,ch);
	imshow(s.c_str(),pointImage);	
}

void findLines(const Mat& i_image, vector<Vec4i>& o_lines, double rho, double theta, int threshold,double minLineLength, double maxLineGap)
{
	// 声明 原始图片，灰度图片，和 canny边缘图片
	Mat cedge;
	Mat gray, edge;
	// 生成灰度图片，因为只有灰度图片才能生成边缘图片
	cedge.create(i_image.size(), i_image.type());
	//全部设为0
	cedge = Scalar::all(0);
	cvtColor(i_image,gray, CV_BGR2GRAY);
//	imshow("grayImage", gray);
	blur(gray, edge, Size(3,3));
	int areaImage = gray.size().area();
	cout << "image area: " << areaImage << endl;
	Canny(gray, edge, 30, 80, 3);
	//寻找直线
	HoughLinesP(edge, o_lines, rho, theta, threshold, minLineLength, maxLineGap);
	//拷贝边缘的象素点
	i_image.copyTo(cedge, edge);
//	imshow("Edge map", edge);

	Mat lineImage(edge.size(), edge.type());
//	edge.copyTo(lineImage);
	lineImage = Scalar::all(0);
	cout << "lines.size : " << o_lines.size() << endl;
	for (int i=0; i<o_lines.size(); i++)
	{
		line(lineImage,Point(o_lines[i][0],o_lines[i][1]),
			Point(o_lines[i][2],o_lines[i][3]),
	        Scalar( 255, 255, 255),
			0,
			0);
	}
	static char ch = '0';
	string s(1,ch++);
	imshow(s.c_str(),lineImage);
}

//寻找所有的直线
void findLines(const Mat& i_image, vector<Vec4i>& o_lines)
{
	// 声明 原始图片，灰度图片，和 canny边缘图片
	Mat cedge;
	Mat gray, edge;
	// 生成灰度图片，因为只有灰度图片才能生成边缘图片
	cedge.create(i_image.size(), i_image.type());
	//全部设为0
	cedge = Scalar::all(0);
	cvtColor(i_image,gray, CV_BGR2GRAY);
//	imshow("grayImage", gray);
	blur(gray, edge, Size(3,3));
	int areaImage = gray.size().area();
	cout << "image area: " << areaImage << endl;
	Canny(gray, edge, 30, 80, 3);
	//寻找直线
	double rho = 0.6;
	double theta = 0.4;
	int threshold = 1;
	double minLineLength = 12;
	double maxLineGap = 6;
	HoughLinesP(edge, o_lines, rho, theta, threshold, minLineLength, maxLineGap);
	//拷贝边缘的象素点
//	i_image.copyTo(cedge, edge);
//	imshow("Edge map", edge);

	Mat lineImage(edge.size(), edge.type());
	lineImage = Scalar::all(0);
	cout << "lines.size : " << o_lines.size() << endl;
	for (int i=0; i<o_lines.size(); i++)
	{
		line(lineImage,Point(o_lines[i][0],o_lines[i][1]),
			Point(o_lines[i][2],o_lines[i][3]),
	        Scalar( 255, 255, 255),
			0,
			0);
	}
	imshow("lineImage",lineImage);
}


//
//取得魔方的六个顶点
//使用凸包法求出来的效果不好，暂时未使用。
void cubePosition(const vector<vector<Point> >& i_contours, vector<Point2f>& o_pts)
{
	double areaMax=0;//轮廓面积
	int k=0;
	//寻找最大的轮廓
	for (int i=0; i<i_contours.size(); i++)
	{
		double area = contourArea(i_contours[i]);
		if (areaMax<area)
		{
			areaMax = area;
			k = i;
		}
	}
	cout << k << ":" << areaMax << endl;
/// 对每个轮廓计算其凸包
   vector<vector<Point> >hull( i_contours.size() );
   for( int i = 0; i < i_contours.size(); i++ )
      {  convexHull( Mat(i_contours[i]), hull[i], false ); }

   /// 绘出轮廓及其凸包
   Mat drawing = Mat::zeros( 400,400, CV_8UC3 );
   for( int i = 0; i< i_contours.size(); i++ )
      {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, i_contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
      }

   /// 把结果显示在窗体
   namedWindow( "Hull demo", CV_WINDOW_AUTOSIZE );
   imshow( "Hull demo", drawing );
}

//分离平行直线
void separateParallels(const vector<Vec4i>& i_lines, vector<Parallels>& o_parallelsLines)
{
	for (int i=0; i<i_lines.size(); i++)
	{
		bool isInsertOK = false;
		for (int j=0; j<o_parallelsLines.size(); j++)
		{
			if (o_parallelsLines[j].insertToParallels(i_lines[i]))
			{
				isInsertOK = true;
				break;
			}
		}
		if (!isInsertOK)//如果没能插入到已有的平行直线中，则新建一组
		{
			Parallels paralles;
			paralles.insertToParallels(i_lines[i]);
			//cout << "paralles.lines.size=" << paralles.lines.size() << endl;
			o_parallelsLines.push_back(paralles);
		}
	}
}

//取平行线中最多的3组
void getMax3Parallels(vector<Parallels>& io_parallelsLines)
{
	//选择法
	for (int i=0; i<3; i++)
	{
		int flag = i;
		for (int j=i+1; j<io_parallelsLines.size(); j++)
		{
			if (io_parallelsLines[flag].lines.size()
				< io_parallelsLines[j].lines.size())
			{
				flag = j;
			}
		}
		if (flag != i)
		{
			swap(io_parallelsLines[i],io_parallelsLines[flag]);
		}
	}
	io_parallelsLines.erase (io_parallelsLines.begin()+3,io_parallelsLines.end());
}
//去除很短的线段
void deleteShortLine(Parallels& io_parallels, const float minLineLength)
{
	for (int i=0; i<io_parallels.lines.size(); i++)
	{
		float d = disP2P(io_parallels.lines[i][0],
			io_parallels.lines[i][1],io_parallels.lines[i][2],
			io_parallels.lines[i][3]);
		if (d < minLineLength)
		{
			io_parallels.lines.erase(io_parallels.lines.begin()+i);
			i--;
		}
	}
}

//合并距离小的线段
void mergeMicroDistanceLine(Parallels& io_parallels, const float minDisLine)
{
	for (int i=0; i<io_parallels.lines.size(); i++)
	{
		for (int j=i+1; j<io_parallels.lines.size(); j++)
		{
			if (disLine2Line(io_parallels.lines[i],io_parallels.lines[j])
				< minDisLine)
			{
				//求中线，中线用最长线段表示
				Vec4i lineD;
				solveMidLine(io_parallels.lines[i],io_parallels.lines[j],lineD);
//				cout << "(" << lineD[0] << "," << lineD[1] 
//					<< ")(" << lineD[2] << "," << lineD[3] << ")" << endl;
				io_parallels.lines[i] = lineD;
				io_parallels.lines.erase(io_parallels.lines.begin()+j);
				j--;
			}
		}
	}
}

//点p1(x1,y1)到点p2(x2,y2)的距离
float disP2P(float x1, float y1, float x2, float y2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}
//点到直线的距离
float disP2Line(const Vec2i& i_P, const Vec4i i_lineB, Vec2i& o_Pe, const float minus)
{
	float ax = i_P[0];
	float ay = i_P[1];
	float cx = i_lineB[0];
	float cy = i_lineB[1];
	//   B
	//a  cd
	//过点a作cd的垂线ae，垂点为e，距离为ae的长度
	if (abs(i_lineB[2]-i_lineB[0])>minus)//不垂直于x轴
	{
		float kcd = static_cast<float>(i_lineB[3]-i_lineB[1])/(i_lineB[2]-i_lineB[0]);
		float kae = -1.0/kcd;
		//求垂点（ae和cd的交点）
		//kcd = (cy-ey)/(cx-ex)  =>  ex = cx - (cy-ey)/kcd = cx-cy/kcd + ey/kcd
		//kae = (ay-ey)/(ax-ex)  =>  ey = ay - kae*(ax-ex) 
		//=> ey = ay - kae*ax + kae*(cx-cy/kcd + ey/kcd)
		//=> ey = ay - kae*ax + kae*(cx-cy/kcd) + kae*ey/kcd
		//=> ey-kae*ey/kcd = ay - kae*ax + kae*(cx-cy/kcd)
		//=> ey(1-kae/kcd) = ay - kae*(ax - cx + cy/kcd)
		//=> ey = (ay - kae*(ax - cx + cy/kcd))/(1-kae/kcd)
		float ey = (ay - kae*(ax - cx + cy/kcd))/(1-kae/kcd);
		float ex = cx-cy/kcd + ey/kcd;
		o_Pe[0] = ex;
		o_Pe[1] = ey;
		return disP2P(ax,ay,ex,ey);
	}
	else//垂直于x轴
	{
		o_Pe[0] = cx;
		o_Pe[1] = ay;
		return abs(ax-cx);
	}
}

//求平均线段之间的距离，如果线段不平行则求A的1个端点到B的距离
float disLine2Line(const Vec4i& i_lineA, const Vec4i i_lineB)
{
	//A   B
	//ab  cd
	//过点a作cd的垂线ae，垂点为e，距离为ae的长度
	Vec2i Pa(i_lineA[0],i_lineA[1]);
	Vec2i Pe;
	return disP2Line(Pa,i_lineB,Pe);
}

//求中线
void solveMidLine(const Vec4i& i_lineA, const Vec4i i_lineB, Vec4i& o_lineD, const float minus)
{
	//A   B
	//ab  cd
	//过点a作cd的垂线ae，垂点为e
	if (abs(i_lineA[2]-i_lineA[0])>minus)//不垂直于x轴
	{
		float kab = static_cast<float>(i_lineA[3]-i_lineA[1])/(i_lineA[2]-i_lineA[0]);
		float kae = -1.0/kab;
		//求垂点（ae和cd的交点）
		//kcd = (cy-ey)/(cx-ex)  =>  ex = cx - (cy-ey)/kcd = cx-cy/kcd + ey/kcd
		//kae = (ay-ey)/(ax-ex)  =>  ey = ay - kae*(ax-ex) 
		//=> ey = ay - kae*ax + kae*(cx-cy/kcd + ey/kcd)
		//=> ey = ay - kae*ax + kae*(cx-cy/kcd) + kae*ey/kcd
		//=> ey-kae*ey/kcd = ay - kae*ax + kae*(cx-cy/kcd)
		//=> ey(1-kae/kcd) = ay - kae*(ax - cx + cy/kcd)
		//=> ey = (ay - kae*(ax - cx + cy/kcd))/(1-kae/kcd)
		float ax = i_lineA[0];
		float ay = i_lineA[1];
		float cx = i_lineB[0];
		float cy = i_lineB[1];
		float kcd = kab;
		float ey = (ay - kae*(ax - cx + cy/kcd))/(1-kae/kcd);
		float ex = cx-cy/kcd + ey/kcd;
		float fx = (ax+ex)/2;
		float fy = (ay+ey)/2;
		o_lineD[0] = fx;
		o_lineD[1] = fy;
		float f1x = fx+10;
		float f1y = kab*f1x+(fy-kab*fx);
		o_lineD[2] = f1x;
		o_lineD[3] = f1y;
	}
	else//垂直于x轴
	{
		float ax = i_lineA[0];
		float ay = i_lineA[1];
		float cx = i_lineB[0];
		float fx = (ax+cx)/2;
		float fy = ay;
		o_lineD[0] = fx;
		o_lineD[1] = fy;
		float f1x = fx;
		float f1y = fy+10;
		o_lineD[2] = f1x;
		o_lineD[3] = f1y;
	}
	Vec2i a(i_lineA[0],i_lineA[1]),
		b(i_lineA[2],i_lineA[3]),
		c(i_lineB[0],i_lineB[1]),
		d(i_lineB[2],i_lineB[3]);
	Vec2i a1,b1,c1,d1;//垂点
	disP2Line(a,o_lineD,a1);
	disP2Line(b,o_lineD,b1);
	disP2Line(c,o_lineD,c1);
	disP2Line(d,o_lineD,d1);
	Vec2i Points[4] = {a1,b1,c1,d1};
	float maxDis = 0;
	int ii=0,jj=1;
	for (int i=0; i<3; i++)
	{
		for (int j=i+1; j<4; j++)
		{
			float d = disP2P(Points[i][0],Points[i][1],Points[j][0],Points[j][1]);
			if (d > maxDis)
			{
				maxDis = d;
				ii=i;
				jj=j;
			}
		}
	}
//	cout << ii << "," 
//		<< jj << endl;
	o_lineD = Vec4i(Points[ii][0],Points[ii][1],Points[jj][0],Points[jj][1]);
//	cout << "(" << o_lineD[0] << "," << o_lineD[1]
//		<< ")(" << o_lineD[2] << "," << o_lineD[3] << ")" << endl;
}


//求6条直线
void find6Lines(const vector<Parallels>& i_parallelsLines, const Vec2i& XY, vector<Vec4i>& o_lines6)
{
	o_lines6.resize(6);
	float maxAlpha = 0;
	int beFlag = 0;//求直线be的组号
	int adFlag = 0;//求直线ad的组号
	int cfFlag = 0;//求直线cf的组号
	for (int i=0;  i<i_parallelsLines.size(); i++)
	{
		if (abs(i_parallelsLines[i].alpha)>maxAlpha)
		{
			maxAlpha = abs(i_parallelsLines[i].alpha);
			beFlag = i;
		}
	}
	for (int i=0;  i<i_parallelsLines.size(); i++)
	{
		if (i_parallelsLines[i].alpha<0 && i!=beFlag)
		{
			adFlag = i;
		}
		else if (i_parallelsLines[i].alpha>0 && i!=beFlag)
		{
			cfFlag = i;
		}
	}
	cout << "ad=" << adFlag << endl;
	cout << "be=" << beFlag << endl;
	cout << "cf=" << cfFlag << endl;
	//求距离最短的直线
	minDisLine(i_parallelsLines[adFlag],Vec2i(0,0),o_lines6[0]);//a
	minDisLine(i_parallelsLines[adFlag],XY,o_lines6[3]);		//d
	minDisLine(i_parallelsLines[beFlag],Vec2i(0,static_cast<float>(XY[1])/2),o_lines6[1]);		//b
	minDisLine(i_parallelsLines[beFlag],Vec2i(XY[0],static_cast<float>(XY[1])/2),o_lines6[4]);	//e
	minDisLine(i_parallelsLines[cfFlag],Vec2i(0,XY[1]),o_lines6[2]);		//c
	minDisLine(i_parallelsLines[cfFlag],Vec2i(XY[0],0),o_lines6[5]);		//f
}
//求距离最短的直线
void minDisLine(const Parallels& i_parallels, const Vec2i i_p, Vec4i& o_lines)
{
	float minDis = 1e10;//取一个相当大的数
	for (int i=0; i<i_parallels.lines.size(); i++)
	{
		Vec2i pe;
		float d = disP2Line(i_p,i_parallels.lines[i],pe);
		if (minDis > d)
		{
			minDis = d;
			o_lines = i_parallels.lines[i];
		}
	}
}

//求6个顶点
void find6Points(const vector<Vec4i>& i_lines6, vector<Vec2i>& o_points6)
{
	o_points6.resize(6);
	//第0个顶点
	findIntersectionPoint(i_lines6[0],i_lines6[5],o_points6[0]);
	for (int i=0; i<5; i++)
	{
		findIntersectionPoint(i_lines6[i],i_lines6[i+1],o_points6[i+1]);
	}
}

//求直线的一般式
void solveABC(const Vec4i& i_P, float& A, float& B, float& C)
{
	if (i_P[0] == i_P[2])
	{
		B = 0;
		A = 1;
		C = -(i_P[0]);
	}
	else
	{
		B = 1;
		A = -(i_P[3] - i_P[1]) / (i_P[2] - i_P[0]);
		C = i_P[0] * (i_P[3] - i_P[1]) / (i_P[2] - i_P[0]) - i_P[1];
	}
}
//求两直线的交点
void findIntersectionPoint(const Vec4i& i_Pa, const Vec4i& i_Pb, Vec2i& o_point)
{
	//A1x+B1y+C1=0
	//A2x+B2y+C2=0
	//0   1   2    3
	//x1  y1  x2   y2
	// 直线 a 上的两点：(ax1, ay1), (ax2, ay2)
	// 直线 b 上的两点：(bx1, by1), (bx2, by2)
	float ax1 = i_Pa[0];
	float ay1 = i_Pa[1];
	float ax2 = i_Pa[2];
	float ay2 = i_Pa[3];
	float bx1 = i_Pb[0];
	float by1 = i_Pb[1];
	float bx2 = i_Pb[2];
	float by2 = i_Pb[3];
	double d = (ay2-ay1) * (bx2-bx1) - (by2-by1) * (ax2-ax1);
	if (d==0)	assert("两直线平行");
	float x = (
	(ax2-ax1)*(bx2-bx1)*(by1-ay1)+(ay2-ay1)*(bx2-bx1)*ax1-(by2-by1)*(ax2-ax1)*bx1
	) / d;
	float y = (
	(ay2-ay1)*(by2-by1)*(bx1-ax1)+(ax2-ax1)*(by2-by1)*ay1-(bx2-bx1)*(ay2-ay1)*by1
	) / -d;
	o_point = Vec2i(x,y);
}

void extractOneSide(const Mat& i_image, const vector<Vec2i>& i_srcTri, const vector<Vec2i>& i_dstTri, Mat& o_image)
{
	Point2f srcTri[3];
	Point2f dstTri[3];
	Mat rot_mat( 2, 3, CV_32FC1 );
	Mat warp_mat( 2, 3, CV_32FC1 );
	/// 设置目标图像的大小和类型与源图像一致
	o_image = Mat::zeros(i_image.rows, i_image.cols, i_image.type());
	/// 设置源图像和目标图像上的三组点以计算仿射变换
	srcTri[0] = Point2f(i_srcTri[0][0], i_srcTri[0][1]);
	srcTri[1] = Point2f(i_srcTri[1][0], i_srcTri[1][1]);
	srcTri[2] = Point2f(i_srcTri[2][0], i_srcTri[2][1]);
	dstTri[0] = Point2f(i_dstTri[0][0], i_dstTri[0][1]);
	dstTri[1] = Point2f(i_dstTri[1][0], i_dstTri[1][1]);
	dstTri[2] = Point2f(i_dstTri[2][0], i_dstTri[2][1]);
	/// 求得仿射变换
	warp_mat = getAffineTransform(srcTri, dstTri);
	/// 对源图像应用上面求得的仿射变换
	warpAffine(i_image, o_image, warp_mat, o_image.size());
}

void recognition3Side(const Mat& i_image, const vector<Vec2i>& i_points6,
						   Mat& o_LEFTImage, Mat& o_RIGHTImage,
						   Mat& o_UPImage)
{
	//确定该面位置的顶点(1,2,3)
	vector<Vec2i> srcPointLeft;
	vector<Vec2i> dstPointLeft;
	srcPointLeft.push_back(i_points6[1]);
	srcPointLeft.push_back(i_points6[2]);
	srcPointLeft.push_back(i_points6[3]);
	dstPointLeft.push_back(Vec2i(-15,-15));
	dstPointLeft.push_back(Vec2i(0,i_image.rows-1));
	dstPointLeft.push_back(Vec2i(i_image.cols-1,i_image.rows-1));
	//确定该面位置的顶点(3,4,5)
	vector<Vec2i> srcPointRight;
	vector<Vec2i> dstPointRight;
	srcPointRight.push_back(i_points6[3]);
	srcPointRight.push_back(i_points6[4]);
	srcPointRight.push_back(i_points6[5]);
	dstPointRight.push_back(Vec2i(0,i_image.rows-1));
	dstPointRight.push_back(Vec2i(i_image.cols-1,i_image.rows-1));
	dstPointRight.push_back(Vec2i(i_image.cols-1,0));
	//确定该面位置的顶点(5,0,1)
	vector<Vec2i> srcPointUp;
	vector<Vec2i> dstPointUp;
	srcPointUp.push_back(i_points6[5]);
	srcPointUp.push_back(i_points6[0]);
	srcPointUp.push_back(i_points6[1]);
	dstPointUp.push_back(Vec2i(i_image.cols-1,0));
	dstPointUp.push_back(Vec2i(-15,-15));
	dstPointUp.push_back(Vec2i(0,i_image.rows-1));
	extractOneSide(i_image,srcPointLeft,dstPointLeft,o_LEFTImage);//提取左面
	extractOneSide(i_image,srcPointRight,dstPointRight,o_RIGHTImage);//提取右面
	extractOneSide(i_image,srcPointUp,dstPointUp,o_UPImage);//提取上面
}