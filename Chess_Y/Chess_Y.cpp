// Chess.cpp : �������̨Ӧ�ó������ڵ㡣
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
#define num_p 350//���ֵ�ǵ��Գ����ģ��Լ����ע�����ı��д�������Ƕ��ٸ��ڵ�
struct MyPoint
{
	int x;
	int y;
	char *s=new char[10];
};
int _tmain(int argc, _TCHAR* argv[])
{
	printf("����˵����\nĿǰ���е����Ӷ��ܹ������������ҷ��ص���Ӧ�����̵�����ϵ��\n���λ����ȷ�����㷨��Ŀǰֻ������˧��λ�ã�������λ��û�����롣�ж��㷨��һ���ġ�\n");
	printf("����˵����2�����Ƽ����룺��һ�Ρ�˧-1�����ڶ��Ρ�˧-1��\n");
	Mat img,img_show;
	img_show = imread("old.jpg", IMREAD_COLOR);
	if (img_show.empty())
	{
		printf("open file failed!\n");
		exit(0);
	}
	namedWindow("ԭͼ��");
	imshow("ԭͼ��", img_show);
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
	//�����洢�����ϸ����Ӧ��ͼ�ϵ�λ�á�
	MyPoint Img_Chess[10 * 9] ;
	MyPoint *pImg_Chess = Img_Chess;
	//�����洢�����ϸ����Ӧ�����������ϵ�λ�á�
	MyPoint Original_Chess[10 * 9] ;
	MyPoint *pOri_Chess = Original_Chess;
	//������ͼ�����̵Ľǵ�
	int Chess_points(Mat M, MyPoint*, MyPoint*);
	Chess_points(img, p1, p2);
	//������ͼ�����̵����ԣ���dx��dy������������ͼ������
	int Chess_Img(MyPoint*, MyPoint *p1, MyPoint *p2);
	Chess_Img(Img_Chess, p1, p2);
	//����Ҫƥ��ĸ������ӣ���Ӧ��ͼ�ϵ������Ƕ���
	int temp_num_chess;
	int templateMatching(Mat,MyPoint*);
	temp_num_chess=templateMatching(img,p_zu);
	if (temp_num_chess==0)
	{
		printf("û���ҵ�����������ӣ�ƥ��ʧ�ܣ����������\n");
		exit(0);
	}
	//��ƥ��ĸ������Ӷ�Ӧ������������������λ�á�
	int Chess_Ori(MyPoint*p_zu, MyPoint *pOri_Chess,MyPoint *p1, MyPoint *p2, int temp_num_chess);	
	Chess_Ori(p_zu, pOri_Chess, p1, p2, temp_num_chess);

	//�ж���ڵ�λ����û�зŴ�,������ԭʼ�����̸���λ�����飬�Լ�������ѯ��λ�õ���������,������Ҫ��˧��
	char*str = new char[10];
	printf("����������ҵ����ӵ����ƣ�����'��-1''˧-2'�ĸ�ʽ����\n");
	scanf("%s", str);
	int judge_p_chess(MyPoint* pOri_Chess, char*str);
	judge_p_chess(pOri_Chess, str);
	return 0;
}
int templateMatching(Mat img, MyPoint *my_point)//��������ģ��ƥ��
{
	int width = img.cols;
	int height = img.rows;
	Mat img2;
	img2.create(height, width, CV_8UC1);
	uchar *pImg = (uchar *)img.data;
	char *strName=new char [10];
	char *strName2 = new char[10];
	printf("\n������ҪѰ�ҵ��������ƣ���Ϊ1����Ϊ2������'��-1'or'��-2'�ĸ�ʽ���룺\n");
	scanf("%s", strName);
	strcpy(strName2, strName);
	/*�����ƥ��ͼ��ģ��*/
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

	/*��ͼ���Ե��ģ���Ե���зֱ����Ƚϲ�ֵ�ľ���ֵ֮�����趨��ֵ��С����ֵ��ƥ��ɹ����������ƥ�� */
	uchar *lpsrc;
	uchar *lptempsrc;
	double pixel;
	double temppixel;
	int T = 100000;//��ֵ
	int min = INT_MAX;
	int left, top, bottom, right;
	int p = 0;
#if yy_sobel
	/*����sobel���ӽ��б�Ե���*/
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
	/*��ģ����б�Ե���*/
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

			for (int n = 0; n < height0 - 1; ++n)//ģ����
			{
				for (int m = 0; m < width0 - 1; ++m)//ģ����
				{
					lpsrc = pImg + width*(j + n) + (i + m);// ָ��Դͼ������j+n�У���i+m������	
					lptempsrc = pImg_muban + width0*n + m;// ָ��ģ�嵹����n�У���i+m������	     
					pixel = (double)(*lpsrc);
					temppixel = (double)(*lptempsrc);
					d += abs(pixel - temppixel);// ���ľ���ֵ�ĺ�Ϊ�Ƚϲ���
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
	//img0��ģ��ģ�2��Դͼ�����ݣ�3��ģ������
	int p_true = 0;
	for (int q = 0; q < p; q++)
	{
		double temp1, temp2;
		
		temp1 = (rect[q].left + rect[q].right) / 2;
		temp2 = (rect[q].top + rect[q].bottom) / 2;
		if ((int(temp1 + 0.5) - (my_point + q - 1)->x) != 0||q==0)
		{
			printf("ƥ������ ��:%d  ��:%d  ��%d   �ң�%d\n", rect[q].top, rect[q].bottom, rect[q].left, rect[q].right);
			p_true++;
			(my_point + q)->x = int(temp1 + 0.5);
			(my_point + q)->y = int(temp2 + 0.5);
			(my_point + q)->s = strName;
		}
		else
			continue;
		//������
		
		for (int j = rect[q].top; j < rect[q].bottom; j++)
		{
			for (int i = rect[q].left; i < rect[q].right; i++)
			{
				*(img2.data + j*width + i) = (double)(*( pImg+ j*width + i));
			}
		}
	}
//	imshow("1", img_muban);//Դͼ��
	imshow("0", img2);//�����
	waitKey(0);
	return p_true;
}

int Chess_points(Mat img_sta, MyPoint *p1, MyPoint *p2)
{

	if (img_sta.empty())
	{
		fprintf(stderr, "���ܴ򿪸��ļ�");
		waitKey(6000);
		return -1;
	}
	int channel = img_sta.channels();
	int height = img_sta.rows;
	int width = img_sta.cols;
	int hist[256] = { 0 };
	uchar *pImg = img_sta.data;
	int nThreshold = 0, nNewThreshold = 0;//������ֵ�ĳ�ʼ��
	int greymax = 0, greymin = 255;  //��Ҷȵ������Сֵ��
	int n_times;//��������
	int IS1, IS2;//������1��2����������
	double IP1, IP2;//������1��2��������
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
		//���ü�Ȩƽ��ֵ�ķ��������ֵ������ֵ��С����ֵ���ȫȨ�����ں�һ���ֵĴ�Ȩ��
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
//###############################################################################1111111��Y�����ŵģ�
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
	//###############################################################################222222��X�����ŵģ�
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
	//###############################################################################33333��X�����ŵģ�
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
	//###############################################################################444444��Y�����ŵģ�
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
	printf("�ǵ����꣺(%d,%d)  (%d,%d)\n", p1->x, p1->y, p2->x, p2->y);

	
	// ��ʾͼƬ
//	namedWindow("image", CV_WINDOW_AUTOSIZE);
//	imshow("image", img_sta);
	// �ȴ������󴰿��Զ��ر� 
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
			//������ͼ���ڵ�����
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
		//ֱ���ñ��ʽ����temp�������������Ĵ���.
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
		printf("\n�����ҵ�  %s  ��Ӧ�������ڵ�����Ϊ��(%d,%d)\n", (p_zu + i)->s, ori_x, ori_y );
	}
	return 1;
}

int judge_p_chess(MyPoint* pOri_Chess,char*str)
{	
	
	//���ѡ������Ӷ�Ӧ�������������λ��������
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
	//�ҵ������Ҫ���ҵ����������̵�����
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
		printf("δ�ҵ���ҪѰ�ҵ����ӣ�ԭ�����£�\n1����������������\n2������������û�ж��룬��ģ��ƥ���ȶ�����������\n3�����Ե��ˣ�����û������Ӷ��ˣ�\n");
		exit(0);
	}



	

	for (int i = 0; p_temp1 != "˧-1" || p_temp2 != "˧-2"&&i <= 90; i++)//�ж������ϻ��Ǻ�����
	{
		if (flag == 1)break;
		if (strcmp(pOri_Chess[i].s, "˧-1")==0)
		{
			strcpy(p_temp1, "˧-1");
			if (pOri_Chess[i].y <= 4)
			{
				strcpy(p_chesses_red, "��");
				strcpy(p_chesses_black, "��");
			}
			else
			{
				strcpy(p_chesses_red, "��");
				strcpy(p_chesses_black, "��");
			}
			num_your_chess = i;
			flag = 1;
		}
		if (strcmp(pOri_Chess[i].s, "˧-2")==0)
		{
			strcpy(p_temp1, "˧-2");
			if (pOri_Chess[i].y <= 4)
			{
				strcpy(p_chesses_red, "��");
				strcpy(p_chesses_black, "��");
			}
			else
			{
				strcpy(p_chesses_red, "��");
				strcpy(p_chesses_black, "��");
			}
			num_your_chess = i;
			flag = 1;
		}
	}
//	int check_p_chess();
	int temp_judge;
	//�˴�����һ������λ�����鼴�ɣ���ʾ�����������ӵ�����֮����
	MyPoint p_leader_red[9];
	MyPoint p_leader_black[9];
//	MyPoint p_elephant_red[7];
//	MyPoint p_elephant_black[7];

	if (strcmp(p_chesses_red, "��")==0)
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
		//�˴�����һ��ѭ������ֵ��������㼴�ɡ�
		//����7���㣬2.0  0.2  4.2  6.0  8.2  2.4  6.4  y2=9-y1
	}
	else if (strcmp(p_chesses_red, "��") == 0)
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
		//�˴�����һ��ѭ������ֵ��������㼴�ɡ�
		//����7���㣬2.0  0.2  4.2  6.0  8.2  2.4  6.4  y2=9-y1



	int judge_error = 1;
	for (int i = 0; i < 9; i++)
	{
		if (p_leader_red[i].x == pOri_Chess[num_your_chess].x&&p_leader_red[i].y == pOri_Chess[num_your_chess].y)
			judge_error = 0;
	}
	if (judge_error)
	{
		printf("���������%sλ�ðڷŴ���\n", pOri_Chess[num_your_chess].s);
	}
	else
	{
		printf("���������%sλ�ðڷ���ȷ��\n", pOri_Chess[num_your_chess].s);
	}
	return 1;
}