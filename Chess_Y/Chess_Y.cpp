// Chess.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>   
#include "math.h"
#include <opencv2/opencv.hpp>  
#include "opencv2/opencv_modules.hpp"
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  
#include <string.h> 
#pragma comment(lib,"ws2_32.lib")
using namespace cv;
using namespace std;
#define pi 3.14159
#define width_chess 9
#define height_chess 10
#define yuzhi 127
#define yy_sobel 0
#define num_p 350//这个值是调试出来的，自己大概注意最多的边行大概像素是多少个黑点
struct MyPoint
{
	int x;
	int y;
	char *s=new char[10];
};
int _tmain(int argc, _TCHAR* argv[])
{
	printf("程序说明：\n目前所有的棋子都能够检测出来，并且返回到对应象棋盘的坐标系。\n检测位置正确与否的算法，目前只输入了帅的位置，其他的位置没有输入。判断算法是一样的。\n");
	printf("程序说明（2）：推荐输入：第一次‘帅-1’，第二次‘帅-1’\n");
	Mat img,img_show;
	img_show = imread("old.jpg", IMREAD_COLOR);
	if (img_show.empty())
	{
		printf("open file failed!\n");
		exit(0);
	}
	namedWindow("原图像");
	imshow("原图像", img_show);
	waitKey(0);
	
	img = imread("old.jpg", 0);
	if (img.empty())
	{
		printf("open file failed!\n");
		exit(0);
	}

	struct MyPoint m1, m2,m3[5];
	struct MyPoint *p1, *p2,*p_zu;
	p1 = &m1;
	p2 = &m2;
	p_zu = m3;
	p1->x = 0;
	p1->y = 0;
	p2->x = 0;
	p2->y = 0;
	for (int i = 0; i < 5; i++)
	{
		(p_zu+i)->x = 0;
		(p_zu+i)->y = 0;
	}
	//用来存储棋盘上各点对应在图上的位置。
	MyPoint Img_Chess[10 * 9] ;
	MyPoint *pImg_Chess = Img_Chess;
	//用来存储棋盘上各点对应在象棋数组上的位置。
	MyPoint Original_Chess[10 * 9] ;
	MyPoint *pOri_Chess = Original_Chess;
	//用来求图上棋盘的角点
	int Chess_points(Mat M, MyPoint*, MyPoint*);
	Chess_points(img, p1, p2);
	//用来求图上棋盘的属性，如dx、dy，各个网格点的图上坐标
	int Chess_Img(MyPoint*, MyPoint *p1, MyPoint *p2);
	Chess_Img(Img_Chess, p1, p2);
	//求需要匹配的各个棋子，对应在图上的坐标是多少
	int temp_num_chess;
	int templateMatching(Mat,MyPoint*);
	temp_num_chess=templateMatching(img,p_zu);
	if (temp_num_chess==0)
	{
		printf("没有找到您输入的棋子，匹配失败，程序结束。\n");
		exit(0);
	}
	//求匹配的各个棋子对应在象棋数组里的坐标和位置。
	int Chess_Ori(MyPoint*p_zu, MyPoint *pOri_Chess,MyPoint *p1, MyPoint *p2, int temp_num_chess);	
	Chess_Ori(p_zu, pOri_Chess, p1, p2, temp_num_chess);

	//判定红黑的位置有没有放错,传进来原始的棋盘各点位置数组，以及玩家想查询的位置的棋子名称,起码需要读帅。
	char*str = new char[10];
	printf("输入你想查找的棋子的名称，按照'炮-1''帅-2'的格式输入\n");
	scanf("%s", str);
	int judge_p_chess(MyPoint* pOri_Chess, char*str);
	judge_p_chess(pOri_Chess, str);
	return 0;
}
int templateMatching(Mat img, MyPoint *my_point)//王佳琪的模板匹配
{
	int width = img.cols;
	int height = img.rows;
	Mat img2;
	img2.create(height, width, CV_8UC1);
	uchar *pImg = (uchar *)img.data;
	char *strName=new char [10];
	char *strName2 = new char[10];
	printf("\n请输入要寻找的棋子名称，红为1，黑为2，按照'车-1'or'炮-2'的格式输入：\n");
	scanf("%s", strName);
	strcpy(strName2, strName);
	/*载入待匹配图像模板*/
	Mat img_muban;
	strcat(strName2, ".png");
	img_muban = imread(strName2, 0);

	if (img_muban.empty())
	{
		printf("open file of muban failed!\n");
		exit(0);
	}
	int width0 = img_muban.cols;
	int height0 = img_muban.rows;
	uchar *pImg_muban = img_muban.data;

	/*for (int j = 0; j < height0; j++)
	{
		for (int i = 0; i < width0; i++)
		{
			pix[j*width0 + i] = (double)(*(pImg_muban + j*width0 + i));
		}
	}*/

	struct{
		int left;
		int top;
		int right;
		int bottom;
		char name[10];
	}rect[32];

	/*对图像边缘与模板边缘进行分别求差，比较差值的绝对值之和与设定阈值，小于阈值则匹配成功，否则继续匹配 */
	uchar *lpsrc;
	uchar *lptempsrc;
	double pixel;
	double temppixel;
	int T = 100000;//阈值
	int min = INT_MAX;
	int left, top, bottom, right;
	int p = 0;
#if yy_sobel
	/*利用sobel算子进行边缘检测*/
	int sobel[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
	for (int i = 1; i < height - 1; i++)
	{
		for (int j = 1; j < width - 1; j++)
		{
			double sum = 0; int t = 0;
			for (int m = 0; m < 3; m++)
			{
				for (int n = 0; n < 3; n++)
				{
					sum += *(pImg + width*(i - 1 + m) + (j - 1 + n))*sobel[t++];
				}
			}
			if (sum < 0)           pImg[i*width + j] = 0;
			else if (sum>255)    pImg[i*width + j] = 255;
			else                pImg[i*width + j] = sum;
		}
	}
	/*对模板进行边缘检测*/
	for (int i = 1; i < height0 - 1; i++)
	{
		for (int j = 1; j < width0 - 1; j++)
		{
			double sum = 0; int t = 0;
			for (int m = 0; m < 3; m++)
			{
				for (int n = 0; n < 3; n++)
				{
					sum += *(pImg_muban + width0*(i - 1 + m) + (j - 1 + n))*sobel[t++];
				}
			}
			if (sum < 0)           pImg_muban[i*width0 + j] = 0;
			else if (sum>255)    pImg_muban[i*width0 + j] = 255;
			else                pImg_muban[i*width0 + j] = sum;
		}
	}
#endif
	int judge = 0;
	for (int j = 0; j < height - height0 + 1; ++j)
	{
		if (judge==1)
		{
			break;
		}
		for (int i = 0; i < width - width0 - 1; ++i)
		{
			if (judge==1)
			{
				break;
			}
			int d = 0;
			int flag = -1;

			for (int n = 0; n < height0 - 1; ++n)//模板行
			{
				for (int m = 0; m < width0 - 1; ++m)//模板列
				{
					lpsrc = pImg + width*(j + n) + (i + m);// 指向源图像倒数第j+n行，第i+m个像素	
					lptempsrc = pImg_muban + width0*n + m;// 指向模板倒数第n行，第i+m个像素	     
					pixel = (double)(*lpsrc);
					temppixel = (double)(*lptempsrc);
					d += abs(pixel - temppixel);// 求差的绝对值的和为比较参数
				}//end of m
				if (d > T)
				{
					flag = 1;
					break;
				}
			}//end of n
			if (flag == 1) continue;
			if (min > d)
				min = d;
			if (min < T)
			{
				rect[p].left = i;
				rect[p].top = j;
				rect[p].right = i + width0 - 1;
				rect[p].bottom = j + height0 - 1;
				p++;
			}
			
			rectangle(img, cvPoint(rect[p - 1].left, rect[p - 1].top), cvPoint(rect[p - 1].right, rect[p - 1].bottom), Scalar(255, 255, 255), 2, 8, 0);
		}//end of i
	}//end of j
	printf("%d\n", p);
	//img0是模板的，2是源图像数据，3是模板数据
	int p_true = 0;
	for (int q = 0; q < p; q++)
	{
		double temp1, temp2;
		
		temp1 = (rect[q].left + rect[q].right) / 2;
		temp2 = (rect[q].top + rect[q].bottom) / 2;
		if ((int(temp1 + 0.5) - (my_point + q - 1)->x) != 0||q==0)
		{
			printf("匹配区域： 上:%d  下:%d  左：%d   右：%d\n", rect[q].top, rect[q].bottom, rect[q].left, rect[q].right);
			p_true++;
			(my_point + q)->x = int(temp1 + 0.5);
			(my_point + q)->y = int(temp2 + 0.5);
			(my_point + q)->s = strName;
		}
		else
			continue;
		//画出来
		
		for (int j = rect[q].top; j < rect[q].bottom; j++)
		{
			for (int i = rect[q].left; i < rect[q].right; i++)
			{
				*(img2.data + j*width + i) = (double)(*( pImg+ j*width + i));
			}
		}
	}
//	imshow("1", img_muban);//源图像
	imshow("0", img2);//处理后
	waitKey(0);
	return p_true;
}

int Chess_points(Mat img_sta, MyPoint *p1, MyPoint *p2)
{

	if (img_sta.empty())
	{
		fprintf(stderr, "不能打开该文件");
		waitKey(6000);
		return -1;
	}
	int channel = img_sta.channels();
	int height = img_sta.rows;
	int width = img_sta.cols;
	int hist[256] = { 0 };
	uchar *pImg = img_sta.data;
	int nThreshold = 0, nNewThreshold = 0;//定义阈值的初始化
	int greymax = 0, greymin = 255;  //求灰度的最大最小值。
	int n_times;//迭代次数
	int IS1, IS2;//代表类1、2的像素总数
	double IP1, IP2;//代表类1、2的质量矩
	for (int m = 0; m < channel; m++)
		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++)
			{
				int temp = (int)pImg[(i*width + j)*channel + m];
				//if(channel==1)
				hist[temp]++;
				//else
				//hist[m][temp]++;
				if (temp > greymax)greymax = temp;
				if (temp < greymin)greymin = temp;
			}
	nNewThreshold = int((greymax + greymin) / 2);
	for (n_times = 0; nThreshold != nNewThreshold&&n_times < 100; n_times++)
	{
		nThreshold = nNewThreshold;
		IP1 = 0.0;
		IP2 = 0.0;
		IS1 = 0;
		IS2 = 0;
		//利用加权平均值的方法，求二值化的阈值。小于阈值点的全权积等于后一部分的带权积
		for (int k = greymin; k <= nThreshold; k++)
		{
			IP1 += (double)k*hist[k];
			IS1 += hist[k];
		}
		for (int k = nThreshold + 1; k <= greymax; k++)
		{
			IP2 += (double)k*hist[k];
			IS2 += hist[k];
		}
		double value_ave1, value_ave2;
		value_ave1 = IP1 / IS1;
		value_ave2 = IP2 / IS2;
		nNewThreshold = int((value_ave1 + value_ave2) / 2);
	}
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			if ((int)pImg[i*width + j] <= nThreshold)
				pImg[i*width + j] = 0;
			else
				pImg[i*width + j] = 255;
		}
	//imwrite("two.bmp",img_sta);


			
	int k1, k2, k3, k4;
	int temp = 0, judge = 0;
//###############################################################################1111111上Y（横着的）
	for (int i1 = 0; i1 < height; i1++)
	{
		if (judge == 1)break;
		temp = 0;
		for (int j1 = 0; j1 < width; j1++)
		{
			if (judge == 1)break;
			if (img_sta.at<uchar>(i1,j1)==0)
			{
				temp++;
			}
			if (temp>num_p)
			{
				p1->y = i1;
				judge = 1;
			}
		}
	}
	temp = 0;
	judge = 0;
	//###############################################################################222222上X（竖着的）
	for (int j1 = 0; j1 < width; j1++)
	{
		if (judge == 1)break;
		temp = 0;
		for (int i1 = 0; i1 < height; i1++)
		{
			if (judge == 1)break;
			if (img_sta.at<uchar>(i1, j1) == 0)
			{
				temp++;
			}
			if (temp > num_p)
			{
				p1->x = j1;
				judge = 1;
			}
		}
	}
	temp = 0;
	judge = 0;
	//###############################################################################33333下X（竖着的）
	for (int j1 = width-1; j1 > 0; j1--)
	{
		if (judge == 1)break;
		temp = 0;
		for (int i1 = height-1; i1 > 0; i1--)
		{
			if (judge == 1)break;
			if (img_sta.at<uchar>(i1, j1) == 0)
			{
				temp++;
			}
			if (temp > num_p)
			{
				p2->x = j1;
				judge = 1;
			}
		}
	}
	temp = 0;
	judge = 0;
	//###############################################################################444444下Y（横着的）
	for (int i1 = height - 1; i1 > 0; i1--)
	{
		if (judge == 1)break;
		temp = 0;
		for (int j1 = width - 1; j1 > 0; j1--)
		{
			if (judge == 1)break;
			if (img_sta.at<uchar>(i1, j1) == 0)
			{
				temp++;
			}
			if (temp > num_p)
			{
				p2->y = i1;
				judge = 1;
			}
		}
	}
	printf("角点坐标：(%d,%d)  (%d,%d)\n", p1->x, p1->y, p2->x, p2->y);

	
	// 显示图片
//	namedWindow("image", CV_WINDOW_AUTOSIZE);
//	imshow("image", img_sta);
	// 等待按键后窗口自动关闭 
//	waitKey();
 	return 0;
}

int Chess_Img(MyPoint* pImg_Chess, MyPoint *p1, MyPoint *p2)
{
	int c_img_width = 0;
	int c_img_height = 0;
	c_img_height = abs(p2->y - p1->y);
	c_img_width = abs(p1->x - p2->x);

	for (int i = 0; i < height_chess; i++)
	{
		if (i != 0)
//			printf("\n");
		for (int j = 0; j < width_chess; j++)
		{
			int num = i*width_chess + j;
			int dy = 0, dx = 0;
			dy = c_img_height / (height_chess-1);
			dx = c_img_width / (width_chess-1);
			//棋盘在图像内的坐标
			pImg_Chess[i*width_chess + j].x = j*dx + p1->x;
			pImg_Chess[num].y = i*dy + p1->y;
//			printf("(%d,%d)  ", pImg_Chess[num].x, pImg_Chess[num].y);
		}
	}
	return 0;
}

int Chess_Ori(MyPoint*p_zu, MyPoint *pOri_Chess, MyPoint *p1, MyPoint *p2, int temp_num_chess)
{
	double dx, dy;
	int c_img_width = 0;
	int c_img_height = 0;
	c_img_height = abs(p2->y - p1->y);
	c_img_width = abs(p1->x - p2->x);
	dy = c_img_height / (height_chess - 1);
	dx = c_img_width / (width_chess - 1);
	int ori_x, ori_y;
	for (int i = 0; i < temp_num_chess; i++)
	{
		//直接用表达式不用temp会造成四舍五入的错误.
		double temp3 = 0.0, temp4 = 0.0;
		double temp1 = 0.0, temp2 = 0.0;
		temp1 = ((p_zu + i)->x - p1->x);
		temp2 = ((p_zu + i)->y - p1->y);
		temp3 = temp1 / dx;
		temp4 = temp2 / dy;
		ori_x = int(temp3 + 0.5);
		ori_y = int(temp4 + 0.5);
		pOri_Chess[ori_y*width_chess + ori_x].s = (p_zu + i)->s;
		pOri_Chess[ori_y*width_chess + ori_x].x = ori_x;
		pOri_Chess[ori_y*width_chess + ori_x].y = ori_y;
		printf("\n您查找的  %s  对应在棋盘内的坐标为：(%d,%d)\n", (p_zu + i)->s, ori_x, ori_y );
	}
	return 1;
}

int judge_p_chess(MyPoint* pOri_Chess,char*str)
{	
	
	//玩家选择的棋子对应在象棋数组里的位置是哪里
	int num_your_chess = -1;
	char *str1=new char[10];
	char *str2=new char [10];
	char p_chesses_red[10];
	char p_chesses_black[10];
	char p_temp1[10], p_temp2[10];
	strcpy(str1, str);
	strcpy(str2, str);
	strcat(str1, "-1");
	strcat(str2, "-2");
	//找到输入的要查找的棋子在棋盘的哪里
	int flag = 0;
	for (int i = 0; pOri_Chess[i].s != str&&i<=90; i++)
	{
		if (flag == 1)break;
		if (strcmp(pOri_Chess[i].s, str)==0)
		{
			num_your_chess = i;
			flag = 1;

		}
	}
	flag = 0;
	if (num_your_chess==-1)
	{
		printf("未找到您要寻找的棋子，原因有下：\n1、可能是输入有误\n2、可能是数据没有读入，请模板匹配先读入您的数据\n3、被吃掉了，本就没有这个子儿了！\n");
		exit(0);
	}



	

	for (int i = 0; p_temp1 != "帅-1" || p_temp2 != "帅-2"&&i <= 90; i++)//判定红在上还是黑在上
	{
		if (flag == 1)break;
		if (strcmp(pOri_Chess[i].s, "帅-1")==0)
		{
			strcpy(p_temp1, "帅-1");
			if (pOri_Chess[i].y <= 4)
			{
				strcpy(p_chesses_red, "上");
				strcpy(p_chesses_black, "下");
			}
			else
			{
				strcpy(p_chesses_red, "下");
				strcpy(p_chesses_black, "上");
			}
			num_your_chess = i;
			flag = 1;
		}
		if (strcmp(pOri_Chess[i].s, "帅-2")==0)
		{
			strcpy(p_temp1, "帅-2");
			if (pOri_Chess[i].y <= 4)
			{
				strcpy(p_chesses_red, "下");
				strcpy(p_chesses_black, "上");
			}
			else
			{
				strcpy(p_chesses_red, "上");
				strcpy(p_chesses_black, "下");
			}
			num_your_chess = i;
			flag = 1;
		}
	}
//	int check_p_chess();
	int temp_judge;
	//此处定义一堆特殊位置数组即可，表示各个特殊棋子的所到之处。
	MyPoint p_leader_red[9];
	MyPoint p_leader_black[9];
//	MyPoint p_elephant_red[7];
//	MyPoint p_elephant_black[7];

	if (strcmp(p_chesses_red, "上")==0)
	{
		temp_judge = 7;
		int temp = 0;
		for (int i = 3; i <= 5; i++)
			for (int j = 0; j <= 2; j++)
			{
				p_leader_red[temp].x = i;
				p_leader_red[temp].y = j;
				p_leader_black[temp].x = i;
				p_leader_black[temp].y = 9-j;
				temp++;
			}
		//此处加上一堆循环，赋值各个特殊点即可。
		//象是7个点，2.0  0.2  4.2  6.0  8.2  2.4  6.4  y2=9-y1
	}
	else if (strcmp(p_chesses_red, "下") == 0)
	{
		temp_judge = 7;
		int temp = 0;
		for (int i = 3; i <= 5; i++)
			for (int j = 0; j <= 2; j++)
			{
				p_leader_red[temp].x = i;
				p_leader_red[temp].y = 9 - j;
				p_leader_black[temp].x = i;
				p_leader_black[temp].y = j;
				temp++;
			}
	}
		//此处加上一堆循环，赋值各个特殊点即可。
		//象是7个点，2.0  0.2  4.2  6.0  8.2  2.4  6.4  y2=9-y1



	int judge_error = 1;
	for (int i = 0; i < 9; i++)
	{
		if (p_leader_red[i].x == pOri_Chess[num_your_chess].x&&p_leader_red[i].y == pOri_Chess[num_your_chess].y)
			judge_error = 0;
	}
	if (judge_error)
	{
		printf("你检查的棋子%s位置摆放错误！\n", pOri_Chess[num_your_chess].s);
	}
	else
	{
		printf("你检查的棋子%s位置摆放正确！\n", pOri_Chess[num_your_chess].s);
	}
	return 1;
}