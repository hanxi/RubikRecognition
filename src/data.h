#ifndef DATA_H_
#define DATA_H_
/**
 *    魔方展开结构示意图
 *        |----------|
 *        |    UP    |
 * |------|----------|---------|--------|
 * |LEFT  |   FRONT  |   IGHT  |  BACK  |
 * |------|----------|---------|--------|
 *        |   DOWN   |
 *        |----------|
 * 
 */
enum SIDE{UP=0, LEFT=1, FRONT=2, RIGHT=3, BACK=4, DOWN=5};
//颜色比例结构体
class Bgrp
{
public:
	Bgrp(int i_bgr, float i_p):bgr(i_bgr),p(i_p) {}
	int bgr;
	float p;
	bool operator<(const Bgrp& right) const//按p值从大到小排序
	{
		return (*this).p > right.p;
	}
};

#endif
