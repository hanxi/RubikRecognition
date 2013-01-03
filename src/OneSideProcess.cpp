#include "OneSideProcess.h"
#include "func.h"
#include "data.h"
#include <iostream>
using std::cout;
using std::endl;

const int QUE_LEN = 5;//用于存储扫描线颜色的队列长度
const float DETA = 30.0;//误差
const uchar COLOR_ERR = 10;//人眼识别颜色误差值
const float ALPHA=0;//这个值很关键，关系到取色的准确问题,为0时取像素比例最大时的颜色值

//识别，将结果存入face数组对应位置
void OneSideProcess::recognition(SIDE a_side, std::vector<int>& o_face)
{
	vector<vector<Point> > contours;
	findCubeContours(*m_pSideImage, contours);
	cutOutsideContour(contours);
//	RotatedRect rect;
//	cubeOneSidePosition(contours, rect);//计算魔方一个面的位置
//	Size2f size = rect.size;
	Point2f pt;
//	getLeftTopPoint(rect, pt);
	pt = Point2f(0,0);
//	int w = size.width/m_dimension;//单元格子的宽度
//	int h = size.height/m_dimension;//单元格子的高度
	int w = m_pSideImage->cols/m_dimension;//单元格子的宽度
	int h = m_pSideImage->rows/m_dimension;//单元格子的高度
	cout << "魔方维数：" << m_dimension << endl;
	Mat recognitionImage(m_pSideImage->size(),m_pSideImage->type());
	recognitionImage = Scalar::all(0);
	for (int i=0; i<m_dimension; i++)
//	for (int i=1; i<2; i++)
	{
		int y = pt.y + h*i;//第i行对应的y坐标
		for (int j=0; j<m_dimension; j++)
//		for (int j=2; j<3; j++)
		{//第i行第j列
			int x = pt.x + w*j;
			Scalar color = getColor(*m_pSideImage,x,y,w,h);
			m_colors.push_back(color);
			cout <<"m_color[b,g,r]=[" << m_colors.back()[0] << "," 
				<< m_colors.back()[1] << "," << m_colors.back()[2] << "]" << endl;
			rectangle(recognitionImage,
					   Point( x, y),
					   Point( x+w, y+h),
					   color,
					   -1,
					   8 );
		}
	}
	static char i='0';
	++i;
	string caption="recognitionImage"+string(1,i);	
	imshow(caption.c_str(), recognitionImage);
}

//获取矩形的颜色
//取色模型（概率模型）
Scalar getColor(const Mat& image, int x, int y, int w, int h)
{
	Scalar color=Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
	Range rowRange(y,y+h);//行
	Range colRange(x,x+w);//列
//	cout << "rowRange:" << rowRange.start <<"," << rowRange.end << endl;
//	cout << "colRange:" << colRange.start <<"," << colRange.end << endl;
	Mat subCube(image,rowRange,colRange);

	list<Vec2i> bgrcTable;//{[bgr,c],...},需要频繁插入
//	cout << subCube << endl;
	for (int i=0; i<subCube.rows; i++)
	{
		for (int j=0; j<subCube.cols; j++)
		{
			Mat color = subCube.row(i).col(j);
//			cout << "color:[" << (int)color.at<uchar>(0,0)
//				<<  "," << (int)color.at<uchar>(0,1)
//				<<  "," << (int)color.at<uchar>(0,2)
//				<< "]:" << color << endl;
			insertToTable(color,bgrcTable);
		}
	}
	cout << "像素点个数:" << subCube.rows*subCube.cols << "\t";
	cout << "bgrcTable元素个数:" << bgrcTable.size() << endl;
	//将bgrc表改为bgrp表（比例表：c项归一化为p）
	vector<Bgrp> bgrpTable;
	int pixCount = subCube.rows*subCube.cols;
	bgrcToBgrp(bgrcTable,pixCount,bgrpTable);
	std::sort(bgrpTable.begin(),bgrpTable.end());
	color = averColor(bgrpTable);

//	static char i='0';
//	++i;
//	string caption=string(1,i);
	//显示结果
//	namedWindow(caption.c_str(), 1);
//    imshow(caption.c_str(), subCube);

//	cout <<"color:[" << color[0] << "," 
//		<< color[1] << "," << color[2] << "]" << endl;
//	Mat testMat(100,100,CV_8UC3,color);
//	caption=string(2,i);
//	namedWindow(caption.c_str(), 1);
//    imshow(caption.c_str(), testMat);
	return color;
}

//获取像素的平均颜色值
//前置条件：i_bgrpTable按p值从到小排序
Scalar averColor(const vector<Bgrp>& i_bgrpTable)
{
	Scalar color;
	float sum=0.0;
	float sumb=0.0;
	float sumg=0.0;
	float sumr=0.0;
	int n=0;
	for (vector<Bgrp>::const_iterator it=i_bgrpTable.begin();
		it != i_bgrpTable.end(); it++)
	{
		n++;
		cout << "概率p" << n << "=" << it->p << endl;
		sum += it->p;
		uchar b,g,r;
		hexTobgr(it->bgr,b,g,r);
		sumb += b*it->p;
		sumg += g*it->p;
		sumr += r*it->p;
		if (sum>ALPHA)	break;
	}
	cout << "概率Pn="<< sum << endl;
	int ab,ag,ar;
	ab = sumb/(n*sum);
	ag = sumg/(n*sum);
	ar = sumr/(n*sum);
	color = Scalar(ab,ag,ar);
	return color;
}

//将bgrc表改为bgrp表（比例表：c项归一化为p）
void bgrcToBgrp(const list<Vec2i>& i_bgrcTable, int i_pixCount, vector<Bgrp>& o_bgrpTable)
{
	o_bgrpTable.clear();
	for (list<Vec2i>::const_iterator it=i_bgrcTable.begin(); it!=i_bgrcTable.end(); it++)
	{
		uchar bb,gg,rr;
		hexTobgr((*it)[0],bb,gg,rr);
//		cout << "[" << (int)bb << "," << (int)gg <<"," << (int)rr << "]"
//			<< ":" << (*it)[1] << "," <<(float)(*it)[1]/i_pixCount << endl;
		Bgrp bgrp((*it)[0],(float)(*it)[1]/i_pixCount);
		o_bgrpTable.push_back(bgrp);
	}
}


//将数据插入到表中用于统计像素点的个数
void insertToTable(const Mat& i_color, list<Vec2i>& io_bgrcTable)
{
	uchar b = i_color.at<uchar>(0,0);
	uchar g = i_color.at<uchar>(0,1);
	uchar r = i_color.at<uchar>(0,2);
	int c = bgrTohex(b,g,r);//转换为0xFFFFFF格式
	Vec2i bgrc(c,1);
	if (io_bgrcTable.empty())
	{
		io_bgrcTable.push_back(bgrc);
		return;
	}
	list<Vec2i>::iterator it;
	for (it=io_bgrcTable.begin(); it!=io_bgrcTable.end(); it++)
	{
		//比较bgr大小，排序
		uchar bb,gg,rr;
		hexTobgr((*it)[0],bb,gg,rr);
		if (abs(bb-b)<COLOR_ERR
			&& abs(gg-g)<COLOR_ERR
			&& abs(rr-r)<COLOR_ERR)//误差内相等颜色
		{
//			cout << "bgr1:" << b << "," << g << "," << r << endl;
//			cout << "bgr2:" << bb << "," << gg << "," << rr << endl;
			(*it)[1]++;
			return;
		}
		if ((*it)[0] > c)
		{
			io_bgrcTable.insert(it,bgrc);
			return;
		}
	}
	if (it == io_bgrcTable.end())//需要讨论这种情况：待插入值最大时
	{
		io_bgrcTable.push_back(bgrc);
	}
}


//找出左上角的点
void getLeftTopPoint(const RotatedRect& i_rect, Point2f& o_pt)
{
	Point2f center = i_rect.center;
	Point2f pts[4];
	i_rect.points(pts);
	for (int i=0; i<4; i++)
	{
		if (pts[i].x < center.x && pts[i].y < center.y)
		{
			o_pt = pts[i];
			break;
		}
	}
}

void cubeOneSidePosition(const vector<vector<Point> >& i_contours, RotatedRect& o_rect)//计算魔方一个面的位置
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
	o_rect = minAreaRect(i_contours[k]);
	Point2f pts[4];//4个端点
	o_rect.points(pts);

	/// 绘出轮廓及
	Mat drawing = Mat::zeros(Size(600,300), CV_8UC3);
	for( int i = 0; i< 3; i++ )
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
		line(drawing,pts[i],pts[i+1],color);
//		cout << i <<":" << pts[i] << endl;
	}

	/// 把结果显示在窗体
//	namedWindow("side demo", CV_WINDOW_AUTOSIZE);
//	imshow("side demo", drawing);
}



//扫描线相关
/*
template<class Con>
float average(const Con& a_deq)
{
	float sum = 0;
	for (Con::const_iterator it=a_deq.begin();
		it != a_deq.end(); it++)
	{
		sum +=*it;
	}
	return sum/a_deq.size();
}

{
	Mat grayImage,borderImage,drawImage;
	Mat HLSImage;

	drawImage = m_pSideImage->clone();

	drawImage.convertTo(drawImage,CV_32F);
	HLSImage.create(drawImage.size(),CV_32F);
	cvtColor(drawImage, HLSImage, CV_BGR2HSV);


	int M = drawImage.cols;
	int N = drawImage.rows;
	cout << "N=" << N << endl;
	cout << "M=" << M << endl;
	deque<float> sequence(QUE_LEN,0);//临时存储扫描线颜色
	for (int i=0; i<M; i++)//扫描线x=i
	{
		Mat imRows = HLSImage.row(i);//一列的颜色
		//如果到达魔方区域，计算颜色分布
		vector<float> vec;//临时存储扫描线上连续的颜色
		vector<float> lineColor;
		for (int j=0; j<N; j++)
		{
			float H = imRows.col(j).at<float>(0);
			sequence.pop_back();
			sequence.push_front(H);
			float aver = average(sequence);
			if (std::fabs(aver-sequence.front())>DETA)
			{
				vec.push_back(H);
				drawImage.row(i).col(j).at<float>(0) = 0;
				drawImage.row(i).col(j).at<float>(1) = 1;
				drawImage.row(i).col(j).at<float>(2) = 1;
//				cout << sequence.back() << "\t\t" << aver << "\t\t" << H << endl;
				//cout <<"(" << i << "," << j << ")"<< endl;
			}
			else
			{
				if (vec.size())
				{
					float averH = average(vec);
					char* sstr[12] = {"红","红-黄","黄","黄-绿","绿","绿青","青","青-蓝","蓝","蓝-紫","紫","紫-红"};
//					cout << sstr[(int)aver%30] << endl;
//					cout << "size:" << vec.size() << endl;
//					cout <<"(" << i << "," << j << ")"<< endl;
//					cout << "averH:" << averH << endl;
					lineColor.push_back(averH);
				}
				vec.clear();
			}
		}
//		cout << "颜色分成:" <<lineColor.size()<<endl;
	}
	//显示结果
	namedWindow("border Image", 1);
    imshow("border Image", borderImage);
	namedWindow("gray Image", 1);
    imshow("gray Image", grayImage);
	namedWindow("draw Image", 1);
    imshow("draw Image", drawImage);
 	namedWindow("HLS Image", 1);
    imshow("HLS Image", HLSImage);
 	namedWindow("src Image", 1);
    imshow("src Image", *m_pSideImage);
    waitKey(0);
}*/
