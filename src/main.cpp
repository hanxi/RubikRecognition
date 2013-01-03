//============================================================================
// Name        : manin.cpp
// Author      : huangliangming
// Version     :
// Copyright   : jxust
// Description : Hello World in C++, Ansi-style
//============================================================================


#include "main.h"

int main(int argc, char** argv)
{
	string imagename = "./res/image/cube.jpg";
	Mat img = imread(imagename);//读入图像
	if(img.empty())  //如果读入图像失败
	{
		cout<<"read failed!";
		return -1;
	}
	namedWindow("image",1);  //创建窗口
	imshow("image", img);  //显示图像

	//测试3面魔方识别
	ImageProcess imageProcess(&img);
	imageProcess.recognition();

	waitKey();  //等待按键，按键盘任意键返回
	return 0;
}
