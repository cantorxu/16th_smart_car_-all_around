#include "image.h"
#include <iostream>
#include"math.h"

#define gap 4
#define PROSPECT 30

//白条的属性
typedef struct range
{
	uint8_t number;//编号
	uint8_t fpoint;//父节点
	uint8_t left;//左边界
	uint8_t right;//右边界
	uint8_t width;//宽度
}white_range;

//每行白条子属性
typedef struct all_range
{
	uint8_t num;//白条总数
	white_range range[white_num_MAX];
}allrange;

//寻找到的赛道白条的属性
typedef struct road
{
	uint8_t left;//左边界
	uint8_t right;//右边界
	uint8_t width;//宽度
}road_t;

float parameterA, parameterB;

uint8_t IMG[CAMERA_H][CAMERA_W];//二值化图像数组
allrange all_range[CAMERA_H];
allrange road_m[CAMERA_H];//可能的赛道白条
road_t my_road[CAMERA_H];
uint8_t my_road_num[CAMERA_H];//进入的道路的序号
uint8_t all_num = 0;//编号从0开始
uint8_t road_end = FAR_LINE;//可视道路尽头
uint8_t threshold = MISS;//阈值
uint8_t x_left[CAMERA_H];
uint8_t x_right[CAMERA_H];
uint8_t mid_line[CAMERA_H];

uint8_t left_line[CAMERA_H];
uint8_t right_line[CAMERA_H];
uint8_t left_side[CAMERA_H] = {0};
uint8_t right_side[CAMERA_H] = { CAMERA_W - 1 };

uint8_t fork_point;
uint8_t my_straight;
uint8_t x_left_straight,x_right_straight;

uint8_t road_flag = 0;

//十字
uint8_t my_cross = 0;//识别十字标志量
uint8_t cross_outL = MISS;
uint8_t cross_outR = MISS;
uint8_t cross_enterL = MISS;
uint8_t cross_enterR = MISS;

uint8_t white_through = MISS;//定义最近的贯穿白条位置

//岔路
uint8_t fork_breakL = 0;//left broken point
uint8_t fork_breakR = 0;//right broken point

//斑马线和车库
uint8_t zebra_start_line = MISS;//定义识别斑马线标志量
uint8_t zebra_end_line = MISS;//定义斑马线的出口
uint8_t my_garage = 1;//识别出入库的标志量
uint8_t garage_dir = MISS;//车库方向(0为左侧，1为右侧)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//辛老板环岛定义量

uint8_t left_break_down[2], right_break_down[2];
uint8_t left_break_up[2], right_break_up[2];

uint8_t road_case_1 = 1; //左边直线，右下方是直线段，无上断点，有C型弧线
uint8_t road_case_2 = 2;//左边直线，右下方无线段，无上端点，有C型弧线
uint8_t road_case_3 = 3;//左边直线，右下方无线段，有上端点，有C型弧线
uint8_t road_case_4 = 4;//左边直线，右下方C型弧线段，有上端点
uint8_t road_case_5 = 5;//左边直线，右下方C型弧线段，无上端点

uint8_t break_point_on_left[6];
uint8_t break_point_on_right[6];
uint8_t how_many_break_on_right;
uint8_t how_many_break_on_left;

uint8_t akward_area;
uint8_t tell_all_white_flag;

////////////////////////////////////////////
//功能：初始化
//输入：
//输出：
//备注：将全局变量和数组初始化
///////////////////////////////////////////
void element_init(void)
{
	all_num = 0;
	fork_point = MISS;
	road_end = FAR_LINE;
	memset(right_side, CAMERA_W - 1, CAMERA_H);
	for (uint8_t i = NEAR_LINE; i >= FAR_LINE; i--)
	{
		my_road[i].left = 0;
		my_road[i].right = 0;
		my_road[i].width = 0;
		all_range[i].num = 0;
		road_m[i].num = 0;
	}
}

////////////////////////////////////////////
//功能：在指定行数绘制一条线
//输入：
//输出：
//备注：purple
///////////////////////////////////////////
void draw_line(uint8_t line)
{
	for (uint8_t i = 30; i <= 90; i++)
	{
		IMG[line][i] = purple;
	}
}

float HistGram[256];
void get_histgram(void);
void GetOSTUThreshold(float* HistGram);
////////////////////////////////////////////
//功能：二值化
//输入：灰度图片
//输出：二值化图片
//备注：
///////////////////////////////////////////
void THRE()
{
	get_histgram();
	GetOSTUThreshold(HistGram);
	//threshold = 113;
	//printf("阈值为：%d\n", threshold);

	uint8_t* map;
	uint8_t temp = 0;
	map = fullBuffer;
	for (uint8_t i = 0; i < CAMERA_H; i++)
	{
		for (uint8_t j = 0; j <= CAMERA_W; j++)
		{

			if (j == 0)
			{
				if ((*map) > threshold)
					temp = 1;
				else
					temp = 0;
				map++;
			}
			else if (j == CAMERA_W)
			{
				*(map - 1) = temp;
			}
			else
			{
				*(map - 1) = temp;
				if ((*map) > threshold)
					temp = 1;
				else
					temp = 0;
				map++;
			}
			/*
			if ((*map) > threshold)
				(*my_map) = 1;
			else (*my_map) = 0;
			map++;
			my_map++;
			*/
		}
	}
}
void get_histgram()
{
	uint8_t* map;
	float parameter = 11200.0;
	map = fullBuffer;
	uint8_t temp = 0;
	uint8_t i = 0;
	float* hist_head = &HistGram[0];
	//while (*map)
	while (map < fullBuffer + 14000)
	{
		*(hist_head + *map) = *(hist_head + *map) + 1.0;
		map++;
	}
	for (int i = 0; i <= 255; i++)
	{
		if (*(hist_head + i))
		{
			if (i == 0)
				temp = ((float)*(hist_head + i) / (22253.0)) * 10000.0;
			else if (i == 255)
			{
				*(hist_head + i - 1) = temp;
				*(hist_head + i) = ((float)*(hist_head + i) / (22253.0)) * 10000.0;
			}
			else
			{
				*(hist_head + i - 1) = temp;
				temp = ((float)*(hist_head + i) / (22253.0)) * 10000.0;
			}
		}
	}
}
void  GetOSTUThreshold(float* HistGram)
{
	int X, Y, Amount = 0;
	int PixelBack = 0, PixelFore = 0, PixelIntegralBack = 0, PixelIntegralFore = 0, PixelIntegral = 0;
	double OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma;              // 잚쇌렘뀌;
	int MinValue, MaxValue;
	uint8_t Threshold = 0;

	for (MinValue = 0; MinValue < 256 && HistGram[MinValue] == 0; MinValue++);
	for (MaxValue = 255; MaxValue > MinValue && HistGram[MinValue] == 0; MaxValue--);

	for (Y = MinValue; Y <= MaxValue; Y++) Amount += HistGram[Y];        //  獗羹悧鑒

	PixelIntegral = 0;
	for (Y = MinValue; Y <= MaxValue; Y++) PixelIntegral += HistGram[Y] * Y;
	SigmaB = -1;
	for (Y = MinValue; Y < MaxValue; Y++)
	{
		PixelBack = PixelBack + HistGram[Y];
		PixelFore = Amount - PixelBack;
		OmegaBack = (double)PixelBack / Amount;
		OmegaFore = (double)PixelFore / Amount;
		PixelIntegralBack += HistGram[Y] * Y;
		PixelIntegralFore = PixelIntegral - PixelIntegralBack;
		MicroBack = (double)PixelIntegralBack / PixelBack;
		MicroFore = (double)PixelIntegralFore / PixelFore;
		Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);
		if (Sigma > SigmaB)
		{
			SigmaB = Sigma;
			Threshold = (uint8_t)Y;
		}
	}
	threshold = Threshold;
}

void head_clear(void)
{
	uint8_t* my_map;
	for (int i = 99; i >= 83; i--)
	{
		my_map = &IMG[i][0];
		for (int j =36; j <= 106; j++)
		{
			*(my_map + j) = white;
		}
	}
}

void search_white_range(void)
{
	uint8_t num = 0;//每行的白条数量
	uint8_t* map = NULL;
	for (int i = NEAR_LINE; i >= FAR_LINE; i--)
	{
		map = &IMG[i][LEFT_SIDE];//指针行走加快访问速度
		num = 0;
		*map = 0;
		*(map + RIGHT_SIDE) = 0;
		for (int j = LEFT_SIDE; j < RIGHT_SIDE; j++)
		{
			white_range* now_white;
			//左边界
			if (!(*(map + j)) && (*(map + j + 1)))
			{
				now_white = &all_range[i].range[num];
				now_white->left = j + 1;
			}
			//右边界
			if ((*(map + j)) && !(*(map + j + 1)))
			{
				now_white = &all_range[i].range[num];
				now_white->right = j;
				now_white->width = j - now_white->left;
				if (i > CAMERA_H - 10)
				{
					if (now_white->width > 10)//长度过短，舍弃
					{
						num++;
						if (num >= white_num_MAX)
							break;
						now_white->number = all_num;//白条数加一，给这个白条编号
						now_white->fpoint = all_num++;//父节点与编号相同
					}
				}
				else
				{
					num++;
					if (num >= white_num_MAX)break;
					now_white->number = all_num;//白条数加一，给这个白条编号
					now_white->fpoint = all_num++;//父节点与编号相同
				}
			}
		}
		all_range[i].num = num;//一行有几个白条
	}
	/*for (uint8_t i = FAR_LINE; i <= NEAR_LINE;i++)
	{
		printf("第%d行的白条子数:%d,", i, all_range[i].num);
		for (uint8_t j = 0; j < all_range[i].num; j++)
		{
			printf(" 父节点为：%d,宽度为：%d", all_range[i].range[j].fpoint, all_range[i].range[j].width);
		}
		printf("\n");
	}*/
}

void find_all_connect()
{
	//u为up d为down 即为当前处理的这两行中的上面那行和下面那行
	//u_num：上面行白条数
	//u_left：上面行当前白条左边界
	//u_right：上面行当前白条右边界
	//i_u：当前处理的这个白条是当前这行（上面行）白条中的第i_u个
	uint8_t u_num, i_u, u_left, u_right;
	uint8_t d_num, i_d, d_left, d_right;
	allrange* u_line = NULL;
	allrange* d_line = NULL;
	for (uint8_t i = NEAR_LINE; i > FAR_LINE; i--)//因为每两行每两行比较 所以循环到FAR_LINE+1
	{
		u_num = all_range[i - 1].num;
		d_num = all_range[i].num;
		u_line = &all_range[i - 1];
		d_line = &all_range[i];
		i_u = 0; i_d = 0;
		while (i_u < u_num && i_d < d_num)
		{
			u_left = u_line->range[i_u].left;
			u_right = u_line->range[i_u].right;
			d_left = d_line->range[i_d].left;
			d_right = d_line->range[i_d].right;

			if (u_left <= d_right && u_right >= d_left && d_line->range[i_d].fpoint < u_line->range[i_u].fpoint)//两个白条联通
				u_line->range[i_u].fpoint = d_line->range[i_d].fpoint;
			if (d_right > u_right)i_u++;//上左下右联通，上面行检查下一个
			if (d_right < u_right)i_d++;//上右下左联通，下面行检查下一个
			if (d_right == u_right) { i_u++; i_d++; }//对齐，同时检查下一个
		}
	}
	/*for (uint8_t i = FAR_LINE; i <= NEAR_LINE;i++)
	{
		printf("第%d行的白条子数:%d,", i, all_range[i].num);
		for (uint8_t j = 0; j < all_range[i].num; j++)
		{
			printf(" 父节点为：%d,宽度为：%d", all_range[i].range[j].fpoint, all_range[i].range[j].width);
		}
		printf("\n");
	}*/
}

uint8_t down_mid_num = 0;
void find_road()
{
	//top_road = NEAR_LINE;//赛道最高处所在行数，先初始化话为最低处
	int road_f = -1;//赛道所在连通域父节点编号，先初始化为-1，以判断是否找到赛道
	uint16_t nowline_white_num = 0, roud_nowline_white_num = 0;
	allrange* now_line = NULL;
	allrange* now_road = NULL;
	//找fpoint的众数
	//先找最下面的
	uint16_t countmax = 10;
	uint8_t wid_now = 0;
	uint8_t wid_max = 0;
	for (uint16_t roadf_now = 0; roadf_now < all_range[NEAR_LINE].num; roadf_now++)
	{
		uint16_t count = 0;
		for (int i = NEAR_LINE - 1; i > FAR_LINE; i--)
		{
			for (int j = 0; j < all_range[i].num; j++)
			{
				white_range* now_white = &all_range[i].range[j];
				if (now_white->fpoint == roadf_now)
					count++;
			}
		}
		printf("count(%d)=%d\n", roadf_now, count);
		if (count > countmax)
		{
			//countmax = count;
			wid_now = all_range[NEAR_LINE].range[roadf_now].width;
			//printf("roadf_now:%d\n", roadf_now);
			if (wid_now >= wid_max)
			{
				wid_max = wid_now;
				road_f = roadf_now;
				down_mid_num = road_f;
			}
		}
	}
	//printf("父节点为：%d\n", road_f);
	//找最宽的
	/*uint8_t wid_max = 0;
	uint8_t wid_now = 0;
	for (uint8_t i = 0; i < eachline[NEAR_LINE].num; i++)
	{
		wid_now = eachline[NEAR_LINE].whiteline[i].width;
		if (wid_now > wid_max)
		{
			wid_max = wid_now;
			road_f = i;
		}
	}*/
	//现在我们已经得到了赛道所在连通域父节点编号，接下来把所有父节点编号是road_f的所有白条放进赛道数组
	for (uint8_t i = NEAR_LINE; i >= FAR_LINE; i--)
	{
		//变量保存，避免之后写的冗杂且低效
		now_line = &all_range[i];
		now_road = &road_m[i];
		nowline_white_num = now_line->num;
		now_road->num = 0;
		roud_nowline_white_num = 0;
		for (uint8_t j = 0; j < nowline_white_num; j++)
		{
			if (now_line->range[j].fpoint == road_f)//&& now_line->whiteline[j].width>5)
			{
				//top_road = i;
				now_road->range[roud_nowline_white_num].left = now_line->range[j].left;
				now_road->range[roud_nowline_white_num].right = now_line->range[j].right;
				now_road->range[roud_nowline_white_num].width = now_line->range[j].width;
				roud_nowline_white_num++;
				now_road->num++;
			}
		}
	}
	for (int i = NEAR_LINE; i > FAR_LINE; i--)
	{
		if (road_m[i].num == 0)
		{
			road_end = i;
			break;
		}
	}
	//printf("road_end:%d\n", road_end);
	//draw_line(road_end);
	//for (uint8_t i = FAR_LINE; i < NEAR_LINE; i++)
	//{
	//	printf("第%d行：父节点为:%d,所有赛道数量为：%d,可能的道路数为%d\n",i, road_f,all_range[i].num,road_m[i].num);//meihang you yige liangge 
	//	for (int j = 0; j < road_m[i].num; j++)
	//	{
	//		uint8_t left = road_m[i].range[j].left;
	//		uint8_t right = road_m[i].range[j].right;
	//		for (uint8_t k = left; k <= right; k++)
	//		{
	//			IMG[i][k] = cyan;
	//		}
	//		IMG[i][left] = green;
	//		IMG[i][right] = blue;
	//	}
	//}
}

//////////////////////////////////////////////
////功能：向下寻找连通
////输入：哪行哪个白条
////输出：下一行哪个白条
////备注：
/////////////////////////////////////////////
//uint8_t find_down(uint8_t line, uint8_t num)
//{
//	//此函数用于向下寻找连通
//	uint8_t lined = line + 1;
//	white_range whiteline = road_m[line].range[num];
//	int wid_max = 0;
//	int wid = 0;
//	uint8_t num_return = 255;
//	//printf("num_now=%d,num_up=%d", roadp[lined].num,roadp[line].num);
//	for (uint8_t i = 0; i < road_m[lined].num; i++)
//	{
//		white_range whiteline_now = road_m[lined].range[i];
//		//printf("wid_now=%d", whiteline_now.width);
//		if (whiteline_now.left <= whiteline.right && whiteline_now.right >= whiteline.left)//重合
//		{
//			//printf("ok");
//			int left = whiteline_now.left < whiteline.left ? whiteline_now.left : whiteline.left;//最左
//			int right = whiteline_now.right > whiteline.right ? whiteline_now.right : whiteline.right;//最右
//			int wid1 = right - left;//共长
//			wid = (int)whiteline_now.width + (int)whiteline.width - wid1;//重叠面积
//			if (wid >= wid_max)//选择重叠最大的
//			{
//				num_return = i;
//				wid_max = wid;
//			}
//		}
//		//printf("wid_now=%d", wid);
//	}
//	return num_return;
//}
//
//////////////////////////////////////////////
////功能：向上寻找连通
////输入：哪行哪个白条
////输出：上一行哪个白条
////备注：
/////////////////////////////////////////////
//uint8_t find_up(uint8_t line, uint8_t num)
//{
//	//此函数用于向下寻找连通
//	uint8_t lineu = line - 1;
//	white_range whiteline = road_m[line].range[num];
//	int wid_max = 0;
//	int wid = 0;
//	uint8_t num_return = MISS;
//
//	//printf("num_now=%d,num_up=%d", roadp[lineu].num,roadp[line].num);
//	for (uint8_t i = 0; i < road_m[lineu].num; i++)
//	{
//		white_range whiteline_now = road_m[lineu].range[i];
//		//printf("wid_now=%d\n", whiteline_now.width);
//		if (whiteline_now.left <= whiteline.right && whiteline_now.right >= whiteline.left)//重合
//		{
//			//printf("ok1");
//			int left = whiteline_now.left < whiteline.left ? whiteline_now.left : whiteline.left;//最左
//			int right = whiteline_now.right > whiteline.right ? whiteline_now.right : whiteline.right;//最右
//			int wid1 = right - left;//共长
//			wid = (int)whiteline_now.width + (int)whiteline.width - wid1;//重叠面积
//			if (wid >= wid_max)//选择重叠最大的
//			{
//				num_return = i;
//				wid_max = wid;
//			}
//		}
//		//printf("wid_now=%d", wid);
//	}
//	return num_return;
//}
//
//////////////////////////////////////////////
////功能：找主干道
////输入：road_m
////输出：road
////备注：road_m有岔路，为了简化后面的处理，选择一条主干道
/////////////////////////////////////////////
//void find_my_road()
//{
//	//找一个有岔路的行开始判断
//	uint8_t i_select = road_end + 30;
//	uint8_t num_now_f = 0;
//	uint8_t num_max_f = 0;
//	for (uint8_t i = road_end + 10; i < road_end + 40; i++)
//	{
//		num_now_f = road_m[i].num;
//		printf("line:%d,num_now=%d\n", i, num_now_f);
//		if (num_now_f >= num_max_f)
//		{
//			num_max_f = num_now_f;
//			i_select = i;
//		}
//	}
//	uint8_t i_end = i_select;
//	for (uint8_t i = 40; i <= 100; i++)
//	{
//		IMG[i_end][i] = purple;
//	}
//	//if (i_select == 0)
//	//{
//	//	//每行只有一个道
//	//	for (uint8_t i = NEAR_LINE; i >= road_end; i--)
//	//	{
//	//		road[i].leftside = roadp[i].whiteline[0].leftside;
//	//		road[i].rightside = roadp[i].whiteline[0].rightside;
//	//		road[i].width = road[i].rightside - road[i].leftside;
//	//	}
//	//	return;
//	//}
//
//	printf("i_select=%d\n", i_select);
//	//uint8_t i_end = road_end + 10;
//	uint8_t temp = 5;
//	uint8_t mid_down = road_m[NEAR_LINE].range[down_mid_num].left / 2 + road_m[NEAR_LINE].range[down_mid_num].right / 2;
//	uint8_t left_down = mid_down - temp;// roadp[NEAR_LINE].whiteline[0].leftside;
//	uint8_t right_down = mid_down + temp;// roadp[NEAR_LINE].whiteline[0].rightside;
//	uint8_t num_total = road_m[i_end].num;//可以改变
//
//	uint8_t left_now = 0;
//	uint8_t right_now = 0;
//	uint8_t delta_now = 0;
//	uint8_t delta_min = 200;
//	uint8_t mid_now = 0;
//	int err_min = 9999;
//	int err_sum = 0;
//	//int err_thre = 10;
//	uint8_t k_select = MISS;
//	for (uint8_t k = 0; k < num_total; k++)
//	{
//		err_sum = 0;
//		white_range whiteline_now = road_m[i_end].range[k];
//
//		mid_now = whiteline_now.left / 2 + whiteline_now.right / 2;
//		left_now = mid_now - temp;// whiteline_now.leftside;
//		right_now = mid_now + temp;// whiteline_now.rightside;
//		delta_now = abs((int)mid_now - 70);
//		//连线，leftside画一次，rightside画一次
//
//		float base_k_l = ((float)left_now - (float)left_down) / ((float)i_end - (float)NEAR_LINE);//计算左线的斜率
//		float base_k_r = ((float)right_now - (float)right_down) / ((float)i_end - (float)NEAR_LINE);//计算右线的斜率
//		float base_k = ((float)mid_now - (float)mid_down) / ((float)i_end - (float)NEAR_LINE);//计算中线的斜率
//		float l_ref = left_now;
//		float r_ref = right_now;
//		float ref = mid_now;
//		for (uint8_t i = i_end; i < NEAR_LINE; i++)
//		{
//			//IMG[i][(uint8_t)(l_ref + 0.5)] = purple;
//			if (IMG[i][(uint8_t)(l_ref + 0.5)] == 0 || IMG[i][(uint8_t)(r_ref + 0.5)] == 0)
//				err_sum++;
//			l_ref += base_k_l;
//			r_ref += base_k_r;
//		}
//		for (uint8_t i = i_end; i < NEAR_LINE; i++)
//		{
//			if (IMG[i][(uint8_t)(ref + 0.5)] == 0 || IMG[i][(uint8_t)(l_ref + 0.5)] == 0 || IMG[i][(uint8_t)(r_ref + 0.5)] == 0)
//			{
//				err_sum++;
//			}
//			//IMG[i][(uint8_t)(ref + 0.5)] = purple;
//			l_ref += base_k_l;
//			r_ref += base_k_r;
//			ref += base_k;
//
//		}
//		//printf("err_sum=%d delta_now=%d\n", err_sum,delta_now);
//		if (err_sum < err_min)
//		{
//			/*if (delta_now<delta_min+10&&num_total>1)
//			{
//				err_min = err_sum;
//				k_select = k;
//				delta_min = delta_now;
//			}
//			else if(num_total==1)
//			{*/
//			err_min = err_sum;
//			k_select = k;
//			delta_min = delta_now;
//			//}
//		}
//		else if (err_sum == err_min && delta_now < delta_min)
//		{
//			//两者相等看斜率
//			err_min = err_sum;
//			k_select = k;
//			delta_min = delta_now;
//		}
//	}
//	//printf("k_select=%d", k_select);
//	uint8_t num_now = k_select;
//	for (uint8_t i = i_end; i <= NEAR_LINE; i++)
//	{
//		my_road[i].left = road_m[i].range[num_now].left;
//		my_road[i].right = road_m[i].range[num_now].right;
//		my_road[i].width = my_road[i].right - my_road[i].left;
//		num_now = find_down(i, num_now);//迭代出下一行的
//		//printf("line=%d,num_select=%d\n",i,num_now);
//	}
//	//上方用findup连接上
//	num_now = k_select;
//	for (uint8_t i = i_end; i > FAR_LINE; i--)
//	{
//		num_now = find_up(i, num_now);
//		if (num_now == 255)
//		{
//			road_end = i + 1;
//			break;
//		}
//		else
//		{
//			i--;
//			my_road[i].left = road_m[i].range[num_now].left;
//			my_road[i].right = road_m[i].range[num_now].right;
//			my_road[i].width = my_road[i].right - my_road[i].left;
//			i++;
//		}
//	}
//	
//	//for (uint8_t i = NEAR_LINE; i > FAR_LINE; i--)
//	//{
//	//	uint8_t j_start = my_road[i].left;
//	//	uint8_t j_end = my_road[i].right;
//	//	for (uint8_t j = j_start; j <= j_end; j++)
//	//	{
//	//		if(IMG[i][j] != purple)
//	//		IMG[i][j] = gray;
//	//	}
//	//}
//	//for (uint8_t i = 0; i < CAMERA_W; i++)
//	//{
//	//	IMG[i_end][i] = gray;
//	//}
//	//printf("\n");
//}



////////////////////////////////////////////
//功能：识别是否存在斑马线
//输入：
//输出：赋值给zebra_change，输出时表示识别到斑马线
//备注：2021.1.28
///////////////////////////////////////////
uint8_t check_zebra(void)
{
	uint8_t count = 0;
	uint8_t start = 0;

	uint8_t m_start = CAMERA_H - 5;
	uint8_t m_end = 8;

	uint8_t num_max = MISS;

	uint8_t parameter = 35;//斑马线处第一条白条子的长度阈值
	//最上端要是有无赛道部分，则不可能是斑马线
	/*for (uint8_t i = 20; i > 1; i--)
	{
		if (road_m[i].num == 0)
		{
			return MISS;
		}
	}*/

	for (uint8_t m = m_start; m >= m_end; m--)
	{
		if (road_m[m].num >= 5)
		{
			count++;
			zebra_end_line = m;
		}
		if (count == 1)
		{
			start = m;
		}
	}
	if (count >= 2)
	{
		//printf("第1个白条长度:%d\n", road_m[start].range[0].width);
		//printf("最后个白条长度:%d\n", road_m[start].range[road_m[start].num-1].width);
		if (road_m[start].range[0].width >= parameter)
		{
			garage_dir = 0;
			printf("车库在左侧\n");
		}
		else if (road_m[start].range[road_m[start].num-1].width >= 10)
		{
			garage_dir = 1;
			printf("车库在右侧\n");
		}
		printf("count:%d\n", count);
		return start;
	}
	else
	{
		printf("count:%d，斑马线数量不通过!\n", count);
		return MISS;
	}
}

////////////////////////////////////////////
//功能：斑马线补线
//输入：
//输出：
//备注：
///////////////////////////////////////////
void clean_zebra(void)
{
	uint8_t m_start = NEAR_LINE - gap;
	uint8_t m_end = FAR_LINE;
	uint8_t zebra_enter = MISS;
	uint8_t zebra_out = MISS;

	uint8_t zebra_parameter1 = 25;
	uint8_t zebra_parameter2 = 35;
	if (garage_dir == 0)
	{
		for (uint8_t m = m_start; m >= zebra_start_line; m--)
		{
			//printf("第%d行第1个白条长度:%d\n", m, road_m[m].range[0].width);
			if (road_m[m].range[0].width > road_m[m + gap].range[0].width)
			{
				if (road_m[m].range[0].width - road_m[m + gap].range[0].width > zebra_parameter1)
					zebra_enter = m + gap;
			}
			if (zebra_enter != MISS)
			{
				printf("zebra_enter:%d,", zebra_enter);
				//draw_line(zebra_enter);
				break;
			}
		}
		for (uint8_t m = (zebra_end_line - 1); m >= FAR_LINE; m--)
		{
			if (road_m[m].range[0].width - road_m[m - gap].range[0].width > zebra_parameter2)
				zebra_out = m - gap;
			if (zebra_out != MISS)
			{
				printf("zebra_out:%d\n", zebra_out);
				//draw_line(zebra_out);
				break;
			}
		}
	}
	if (zebra_enter != MISS && zebra_out != MISS)
	{
		for (uint8_t m = zebra_enter; m >= zebra_out; m--)
		{
			x_right[m] = road_m[m].range[road_m[m].num - 1].right;
			printf("x_right[%d]:%d\n", m, x_right[m]);
		}

		parameterB = (x_left[zebra_out] - (float)(x_left[zebra_enter])) / (zebra_out - zebra_enter);
		parameterA = x_left[zebra_enter] - parameterB * (zebra_enter);
		for (uint8_t b = (zebra_out); b < (zebra_enter); b++)
		{
			int left = parameterA + parameterB * b;
			if (left >= 0)
				x_left[b] = (uint8_t)left;
			else
				x_left[b] = 0;
		}
	}
	else if (zebra_out != MISS)
	{
		for (uint8_t m = (zebra_start_line + gap); m >= zebra_out; m--)
		{
			x_right[m] = road_m[m].range[road_m[m].num - 1].right;
			//printf("x_right[%d]:%d\n", m, x_right[m]);
		}
		parameterB = (x_left[zebra_out - gap] - (float)(x_left[zebra_out])) / (-gap);
		parameterA = x_left[zebra_out] - parameterB * (zebra_out);
		for (uint8_t b = (zebra_out); b < m_start; b++)
		{
			int left = parameterA + parameterB * b;
			if (left >= 0)
				x_left[b] = (uint8_t)left;
			else
				x_left[b] = 0;
		}
	}
}

////////////////////////////////////////////
//功能：寻找当前行最可能是我要行进赛道的白条编号
//输入：行数
//输出：白条编号
//备注：无
///////////////////////////////////////////
uint8_t define_my_way(uint8_t line)
{
	allrange* tmy_road = NULL;
	uint8_t  my_way_num;
	uint8_t distance1 = MISS;
	uint8_t distance2 = MISS;
	my_way_num = 0;
	tmy_road = &road_m[line];
	uint8_t max_way = 0, max_way_num = 0;
	uint8_t last_line;
	last_line = last_mid_line[line];
	if (last_line == MISS)
	{
		last_line = CAMERA_W / 2;
	}
	//printf("第%d行，last_line:%d\n", line, last_line);
	if (road_m[line].num >= 2)
	{
		for (uint8_t j = 0; j < tmy_road->num; j++)
		{
			distance1 = My_Abs((tmy_road->range[j].left + tmy_road->range[j].right) / 2 - last_line);
			distance2 = My_Abs((tmy_road->range[j].left + tmy_road->range[j].right) / 2 - CAMERA_W/2);
			if (distance2 <= 20)
			{
				if (tmy_road->range[j].width > 20)
				{
					my_way_num = j;
					break;
				}
			}
			else if (distance1 <= 20)
			{
				if (tmy_road->range[j].width > 20)
				{
					my_way_num = j;
					break;
				}
			}
		}
		printf("第%d行,1:%d,2:%d\n", line, distance1, distance2);

		if (my_way_num == 0)
		{
			for (uint8_t j = 0; j < tmy_road->num; j++)
			{
				if (tmy_road->range[j].width >= max_way)
				{
					max_way = tmy_road->range[j].width;
					max_way_num = j;
				}
				my_way_num = max_way_num;
				/*if (abs(last_mid_line[line] - (tmy_road->range[j].left + tmy_road->range[j].right) / 2) < distance)
				{
					distance = abs(last_mid_line[line] - (tmy_road->range[j].left + tmy_road->range[j].right) / 2);
					my_way_num = j;
				}*/
			}
		}
	}
	else if (road_m[line].num == 1)
	{
		my_way_num = 0;
		//printf("%d！！！\n",line);
	}
	else my_way_num = MISS;
	return my_way_num;
}

////////////////////////////////////////////
//功能：寻找边线
//输入：
//输出：
//备注：
///////////////////////////////////////////
void find_x_left_right()
{
	uint8_t num, count;
	uint8_t cut_flag = 0;
	uint8_t i_start, i_end;
	i_start = NEAR_LINE;
	i_end = FAR_LINE;
	memset(my_road_num, MISS, CAMERA_H);

	count = 0;
	for (uint8_t i = NEAR_LINE; i >= road_end; i--)
	{
		if (road_m[i].num == 1)
		{
			my_road_num[i] = 0;
			count++;
		}
		else
		{
			if (road_m[i].num == 2)
			{
				if (road_m[i].range[0].width <= 2)//稍微滤一下短道路的波
				{
					my_road_num[i] = 1;
					count++;
				}
				else if (road_m[i].range[1].width <= 2)
				{
					my_road_num[i] = 0;
					count++;
				}
			}
		}
	}
	if (count == (NEAR_LINE - road_end))//无岔路
	{
		for (uint8_t i = NEAR_LINE; i >= road_end; i--)
		{
			x_left[i] = road_m[i].range[my_road_num[i]].left;
			x_right[i] = road_m[i].range[my_road_num[i]].right;
			//printf("x_left[%d]:%d,x_right[%d]:%d\n", i, x_left[i], i, x_right[i]);
		}
		return;
	}

	for (uint8_t i = i_start; i >= i_end; i--)
	{
		my_road_num[i] = define_my_way(i);
		//printf("my_road_num[%d]:%d\n", i, my_road_num[i]);
	}

	for (uint8_t i = i_start; i >= i_end; i--)
	{
		if (my_road_num[i] == MISS)
		{
			x_left[i] = MISS;
			x_right[i] = MISS;
		}
		else
		{
			x_left[i] = road_m[i].range[my_road_num[i]].left;
			x_right[i] = road_m[i].range[my_road_num[i]].right;
			//printf("x_left[%d]:%d,x_right[%d]:%d\n", i, x_left[i], i, x_right[i]);
		}

	}
	//连通判断
	uint8_t break_line = MISS;
	for (uint8_t i = NEAR_LINE - 1; i >= 1; i--)
	{
		if ((x_left[i + 1] >= x_right[i] || x_right[i + 1] <= x_left[i]) && x_left[i] != MISS)
		{
			break_line = i;
			break;
		}
	}
	printf("break_line:%d\n", break_line);
	if (break_line != MISS)
		for (uint8_t i = break_line; i > 1; i--)
		{
			cut_flag = 0;
			num = road_m[i].num;
			if (num == 0) break;
			for (uint8_t j = 0; j <= num - 1; j++)
			{
				if (road_m[i].range[j].left <= x_right[i + 1] && road_m[i].range[j].right >= x_left[i + 1])
				{
					x_left[i] = road_m[i].range[j].left;
					x_right[i] = road_m[i].range[j].right;
					//printf("x_left[%d]:%d,x_right[%d]:%d\n", i, x_left[i], i, x_right[i]);
					break_line = i - 1;
					cut_flag = 1;
				}
			}
			if (cut_flag == 0) break;
		}
	if (break_line != MISS)
	{
		for (uint8_t i = break_line; i > 1; i--)
		{
			x_left[i] = MISS;
			x_right[i] = MISS;
		}
	}

}

////////////////////////////////////////////
//功能：确定是不是直道
//输入：数组指针，起始行，终止行
//输出：0---不直  1---直  
//备注：方差50为直道的分界线，从学长那里copy的
//日期：2021.1.29
///////////////////////////////////////////
uint8_t define_straight_line(uint8_t* line, uint8_t start_line, uint8_t end_line)
{
	uint8_t i_start, i_end;
	uint8_t i;
	for (i = end_line; i <= 15; i++)
	{
		if (*(line + i) != MISS)  break;
	}
	if (i == 15) return 0;
	i_start = start_line;
	i_end = i;

	int x1, x2;
	int y1, y2;
	int delta_x, delta_y;
	float k, b;
	int output, add_square = 0;
	float variance;

	x1 = i_start;
	x2 = i;
	y1 = *(line + i_start);
	y2 = *(line + i);

	delta_x = x1 - x2;
	delta_y = y1 - y2;
	k = (float)delta_y / (float)delta_x;
	b = y1 - (float)x1 * k;

	for (i = i_start; i >= i_end; i--)
	{
		output = k * i + b; //算
		output = output - *(line + i);//差
		output *= output;//平方
		add_square = add_square + output;//累加
	}

	variance = (float)add_square / (float)(i_start - i_end);//方差
	if (variance <= 5) return 1;
	else return 0;
}

////////////////////////////////////////////
//功能：识别是否为直道
//输入：
//输出：直道标志量：1为直道，2为左线不直，3为右线不直，4为左右均不直
//备注：
//日期：2021.1.29
///////////////////////////////////////////
uint8_t check_straight(void)
{
	//printf("x_left_straight:%d,x_right_straight:%d\n", left_flag, right_flag);
	if ((x_left_straight == 1) && (x_right_straight == 1))
	{
		mood.chuhuandao = 0;
		mood.ruhuandao = 0;
		mood.circle = 0;
		return 1;
	}
	else if ((x_left_straight == 0) && (x_right_straight == 1))
		return 2;
	else if ((x_left_straight == 1) && (x_right_straight == 0))
		return 3;
	else
		return 4;
}

////////////////////////////////////////////
//功能：十字识别,顺便找出入口
//输入：
//输出：0 不是十字   1 是第一型十字 2是第二型十字
//备注：
///////////////////////////////////////////
uint8_t check_cross(void)
{
	uint8_t i_start, i_end;
	i_start = NEAR_LINE;
	i_end = FAR_LINE;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	uint8_t left_num = 0;
	uint8_t right_num = 0;
	uint8_t tmy_road_num = 0;
	for (uint8_t m = FAR_LINE; m <= FAR_LINE + 10; m++)
	{
		//printf("x_left[%d]:%d,x_right[%d]:%d,road_m[%d].width:%d\n", m, x_left[m], m, x_right[m], m, road_m[m].range[my_road_num[m]].width);
		if ((x_left[m] <= 20 || x_right[m] <= CAMERA_W / 2) && road_m[m].range[my_road_num[m]].width <= 30)
		{
			left_num++;
		}
		if ((x_right[m] >= CAMERA_W - 20 || x_left[m] >= CAMERA_W / 2) && road_m[m].range[my_road_num[m]].width <= 30)
			right_num++;
		if (my_road_num[m] == MISS)
			tmy_road_num++;
	}
	if (left_num >= 5)
	{
		printf("上端道路过多靠左线，十字不通过！\n");
		return 0;
	}
	if (right_num >= 5)
	{
		printf("上端道路过多靠右线，十字不通过！\n");
		return 0;
	}
	if (tmy_road_num >= 5)
	{
		printf("MISS行过多，十字不通过！\n");
		return 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////



	int cosL[CHANGED_H] = { 0 };//余弦的平方左 扩大1000倍
	int cosR[CHANGED_H] = { 0 };//余弦的平方右 扩大1000倍
	int x1 = 0;
	int x2 = 0;
	int y2 = 0;

	y2 = gap * gap;

	cross_outL = MISS;
	cross_outR = MISS;
	cross_enterL = MISS;
	cross_enterR = MISS;

	fork_breakL = MISS;
	fork_breakR = MISS;

	uint8_t cross_flag = MISS;

	uint8_t l_enter = MISS;
	uint8_t r_enter = MISS;

	uint8_t l_out = MISS;
	uint8_t r_out = MISS;

	//十字的参数
	int cross_parameter1 = 600;//cos的值的阈值
	uint8_t cross_parameter2 = 8;//x_left和x_right与其自身的差值
	uint8_t cross_parameter3 = 15;//允许的中央白条离边界的距离

	//岔路的参数
	int fork_parameter1 = 500;//cos的值的阈值
	uint8_t fork_parameter2 = 10;//x_left和x_right与其自身的差值

	for (uint8_t i = (i_start - 3); i >= (i_end + gap); i--)
	{
		//printf("行数：%d,l_enter:%d\n", i, My_Abs(x_left[i - gap] - left_side[i - gap]));
		if (My_Abs(x_left[i - gap] - left_side[i - gap]) <= cross_parameter3)
		{
			l_enter = i;
			break;
		}
	}
	for (uint8_t i = (i_start - 3); i >= (i_end + gap); i--)
	{
		//printf("x_right[%d]:%d;right_side[%d]:%d\n", i, x_right[i],i,right_side[i]);
		//printf("行数：%d,r_enter:%d\n", i, My_Abs(x_right[i - gap] - right_side[i - gap]));
		if (My_Abs(x_right[i - gap] - right_side[i - gap]) <= cross_parameter3)
		{
			r_enter = i;
			break;
		}
	}
	if (l_enter == MISS || r_enter == MISS)
	{
		printf("l_enter:%d,r_enter:%d\n", l_enter, r_enter);
		printf("无可用中央白条子\n");
		return 0;
	}

	for (uint8_t i = i_start; i >= (i_end + gap); i--)
	{
		//计算左侧余弦值的平方
		x1 = (int)x_left[i - gap] - (int)x_left[i];
		x2 = (int)x_left[i] - (int)x_left[i + gap];
		cosL[i] = x1 * x2 + y2;
		cosL[i] *= cosL[i];
		cosL[i] = cosL[i] * 1000 / ((x1 * x1 + y2) * (x2 * x2 + y2));

		//计算右侧余弦值平方
		x1 = (int)x_right[i - gap] - (int)x_right[i];
		x2 = (int)x_right[i] - (int)x_right[i + gap];
		cosR[i] = x1 * x2 + y2;
		cosR[i] *= cosR[i];
		cosR[i] = cosR[i] * 1000 / ((x1 * x1 + y2) * (x2 * x2 + y2));
		//printf("行数：%d 左余弦：%d 右余弦：%d\n",i, cosL[i], cosR[i]);
		//printf("行数：%d 左线：%d 右线：%d 左余弦：%d 右余弦：%d\n", i, x_left[i], x_right[i], cosL[i], cosR[i]);
	}
	for (uint8_t j = (i_start - gap); j >= (i_end + gap * gap); j--)
	{
		if (cosL[j] <= fork_parameter1 && cosL[j] >= 0 && (x_left[j] - x_left[j - gap] >= fork_parameter2) && fork_breakL == MISS)
		{
			fork_breakL = j;
		}
		if (cosL[j] <= cross_parameter1 && cosL[j] >= 0 && (x_left[j] - x_left[j - gap] >= cross_parameter2))
		{
			cross_enterL = j;
			break;
		}
	}
	for (uint8_t j = (i_start - gap); j >= (i_end + gap * gap); j--)
	{
		if (cosR[j] <= fork_parameter1 && cosR[j] >= 0 && (x_right[j - gap] - x_right[j] >= fork_parameter2) && fork_breakR == MISS)
		{
			fork_breakR = j;
		}
		if (cosR[j] <= cross_parameter1 && cosR[j] >= 0 && (x_right[j - gap] - x_right[j] >= cross_parameter2))
		{
			cross_enterR = j;
			break;
		}
	}

	printf("左进口在：%d，右进口在%d\n", cross_enterL, cross_enterR);
	if (My_Abs(cross_enterL - cross_enterR) <= 20 && cross_enterL != MISS && cross_enterR != MISS && l_enter != MISS && r_enter != MISS)
	{
		for (uint8_t j = (cross_enterL - gap); j >= (i_end + 2); j--)
		{
			if (cosL[j] <= cross_parameter1 && cosL[j] >= 0 && (x_left[j] - x_left[j + gap] >= cross_parameter2) && (My_Abs(x_left[j - gap] - x_left[j]) <= 4))
			{
				cross_outL = j;
				break;
			}
		}
		for (uint8_t j = (cross_enterR - gap); j >= (i_end + 2); j--)
		{
			if (cosR[j] <= cross_parameter1 && cosR[j] >= 0 && (x_right[j + gap] - x_right[j] >= cross_parameter2) && (My_Abs(x_right[j] - x_right[j - gap]) <= 4))
			{
				cross_outR = j;
				break;
			}
		}
		if (cross_outL != MISS && cross_outR != MISS)
			cross_flag = 0;
	}
	else if (l_enter != MISS && r_enter != MISS)
	{
		for (uint8_t j = (i_start - gap - 1); j >= (i_end + 2); j--)
		{
			if (cosL[j] <= cross_parameter1 && cosL[j] >= 0 && (x_left[j] - x_left[j + gap] >= cross_parameter2) && (My_Abs(x_left[j - gap] - x_left[j]) <= 3))
			{
				cross_outL = j;
				break;
			}
		}
		for (uint8_t j = (i_start - gap - 1); j >= (i_end + 2); j--)
		{
			if (cosR[j] <= cross_parameter1 && cosR[j] >= 0 && (x_right[j + gap] - x_right[j] >= cross_parameter2) && (My_Abs(x_right[j] - x_right[j - gap]) <= 3))
			{
				cross_outR = j;
				break;
			}
		}
		if (cross_outL != MISS && cross_outR != MISS)
			cross_flag = 1;
	}
	printf("左出口在：%d，右出口在%d\n", cross_outL, cross_outR);
	if (cross_flag == 0)
		return 1;
	else if (cross_flag == 1)
		return 2;
	else return 0;
	/*printf("左出口在：%d，右出口在：%d\n", cross_outL, cross_outR);
	a = (cross_outL + cross_outR) / 2;
	for (int i = 0; i <= 187; i++)
	{
		IMG[a][i] = 4;
	}*/
}

////////////////////////////////////////////
//功能：十字决定双边
//输入：cross_enterL,cross_enterR,cross_outL,cross_outR,x_left,x_right
//输出：left_line,right_line
//备注：
//日期：2021.3.10
///////////////////////////////////////////
void cross_oridinary_two_line(void)
{
	if (my_cross == 1)
	{
		//确定十字存在，开始补线
		//printf("补线！\n");
		parameterB = (x_right[cross_enterR] - static_cast<float>(x_right[cross_outR])) / (cross_enterR - cross_outR);
		parameterA = x_right[cross_enterR] - parameterB * cross_enterR;
		for (uint8_t b = cross_outR; b < (cross_enterR); b++)
		{
			x_right[b] = parameterA + parameterB * b;
			//printf("right_line:%d！\n",right_line[b]);
		}

		parameterB = (x_left[cross_outL] - static_cast<float>(x_left[cross_enterL])) / (cross_outL - cross_enterL);
		parameterA = x_left[cross_enterL] - parameterB * cross_enterL;
		for (uint8_t b = cross_outL; b < (cross_enterL); b++)
		{
			int left = parameterA + parameterB * b;
			if (left >= 0)
				x_left[b] = (uint8_t)left;
			else
				x_left[b] = 0;
		}
	}
	if (my_cross == 2)
	{
		parameterB = (x_right[cross_outR] - static_cast<float>(x_right[cross_outR - 10])) / (float)10;
		parameterA = x_right[cross_outR] - parameterB * cross_outR;
		for (uint8_t b = cross_outR; b <= NEAR_LINE; b++)
		{
			x_right[b] = parameterA + parameterB * b;
		}
		parameterB = (x_left[cross_outL] - static_cast<float>(x_left[cross_outL - 10])) / (float)10;
		parameterA = x_left[cross_outL] - parameterB * cross_outL;
		for (uint8_t b = cross_outL; b <= NEAR_LINE; b++)
		{
			int left = parameterA + parameterB * (int)b;
			if (left >= 0)
				x_left[b] = (uint8_t)left;
			else
				x_left[b] = 0;
		}
	}
}

//////////////////////////////////////////////
////功能：大角度弯道修正
////输入：
////输出：输出1时进行了大角度弯道修正，输出零时未进行大角度弯道修正
////备注：直接对x_left和x_right进行修正
/////////////////////////////////////////////
uint8_t roof(void)
{
	uint8_t roofL = 0;
	uint8_t roofR = 0;

	uint8_t roof_start_line = CAMERA_H - 5;//寻找left_min和right_min的最近行
	uint8_t roof_parameter1 = 30;//left_min和right_min的绝对值差值:30
	uint8_t roof_parameter2 = 20;//往下取值的距离:20
	uint8_t roof_parameter5 = MISS;//实际取值位置:即roof_parameter+roofL/roofR
	uint8_t roof_parameter3 = 10;//盲补线的修正值:10
	uint8_t roof_parameter4 = 10;//寻找突变沿的最小参数
	//寻找第一个非miss行
	if (road_end == FAR_LINE)
		return 0;
	uint8_t left_min = MISS;
	uint8_t right_min = MISS;

	uint8_t vet = 0;
	int left = 0;
	uint8_t right = 0;
	for (uint8_t i = roof_start_line; i > road_end; i--)
	{
		if ((x_left[i] - left_side[i]) < left_min)
		{
			left_min = (x_left[i] - left_side[i]);
			//roofL = i;
		}
		if (x_left[i] <= (left_side[i] + 4))
		{
			left_min = (x_left[i] - left_side[i]);
			//roofL = i;
		}
		if ((My_Abs(x_left[i] - x_left[i + 1]) >= roof_parameter4) && (roofL == 0))
		{
			roofL = i;
		}
		//printf("行数：%d，left_min：%d\n", i, left_min);
	}
	for (uint8_t i = roof_start_line; i > road_end; i--)
	{
		if ((right_side[i] - x_right[i]) < right_min)
		{
			right_min = right_side[i] - x_right[i];
			//roofR = i;
		}
		if ((right_side[i] - x_right[i]) <= 3)
		{
			right_min = right_side[i] - x_right[i];
			//roofR = i;
		}
		if ((My_Abs(x_right[i] - x_right[i + 1]) >= roof_parameter4) && (roofR == 0))
		{
			roofR = i;
		}
		//printf("行数：%d，right_min：%d\n", i, right_min);
	}
	printf("圆弧顶修正: 左%d %d  右%d %d\n", left_min, roofL, right_min, roofR);
	if (left_min < right_min)
	{
		if (right_min - left_min > roof_parameter1)
		{
			printf("大角度弯道左修正：roofL: %d , roofR: %d\n", roofL, roofR);
			roof_parameter5 = roofL + roof_parameter2;
			if (roof_parameter5 >= NEAR_LINE)
			{
				roof_parameter5 = NEAR_LINE - 1;
			}
			vet = x_right[roof_parameter5] - x_left[roof_parameter5];
			//printf("盲补修正值：%d，L:%d，R：%d", vet, x_left[roofL + roof_parameter2], x_right[roofL + roof_parameter2]);
			for (uint8_t j = roofL; j > road_end; j--)
			{
				left = (int)x_right[j] - vet - roof_parameter3;
				if (left >= 0)
					x_left[j] = left;
				else
					x_left[j] = left_side[j];
			}
			return 1;
		}
	}
	else if (left_min > right_min)
	{
		if (left_min - right_min > roof_parameter1)
		{
			printf("大角度弯道右修正：roofL: %d , roofR: %d\n", roofL, roofR);
			roof_parameter5 = roofR + roof_parameter2;
			if (roof_parameter5 >= NEAR_LINE)
			{
				roof_parameter5 = NEAR_LINE - 1;
			}
			vet = x_right[roof_parameter5] - x_left[roof_parameter5];
			//printf("盲补修正值：%d，L:%d，R：%d", vet, x_left[roofR + roof_parameter2], x_right[roofR + roof_parameter2]);
			for (int j = roofR; j > road_end; j--)
			{
				right = x_left[j] + vet + roof_parameter3;
				if (right < CAMERA_W)
					x_right[j] = right;
				else
					x_right[j] = right_side[j] - 1;
			}
			return 1;
		}
	}
	return 0;
}

////////////////////////////////////////////
//功能：识别是否为岔道
//输入：
//输出：MISS为非岔道，1为岔道
//备注：
///////////////////////////////////////////
uint8_t if_fork(void)
{
	uint8_t i_start = FAR_LINE;
	uint8_t i_end = 0;
	uint8_t count = 0;
	uint8_t road_num = 0;
	uint8_t num_max = 0;
	for (uint8_t i = i_start; i < i_start + 30; i++)
	{
		road_num = road_m[i].num;
		//printf("line=%d,num_now=%d\n", i, num_now_f);
		if (road_num == 2)
		{
			i_end = i;
		}
		if (road_m[i].num == 1)
		{
			count++;
		}
		if (count >= 8)//有足够多的单道路时跳出循环
		{
			break;
		}
	}
	count = 0;
	for (uint8_t i = i_start; i <= i_end; i++)
	{
		//printf("第一条的长度为%d，第二条的长度为%d\n", road_m[i].range[0].width, road_m[i].range[1].width);
		if (road_m[i].range[0].width > 40 && road_m[i].range[1].width > 40)
		{
			count++;
		}
	}
	if (count >= 7)
	{
		printf("岔道已识别\n");
		draw_line(i_end);
		return i_end;
	}
	return MISS;
}

////////////////////////////////////////////
//功能：寻找中线
//输入：
//输出：
//备注：
///////////////////////////////////////////
void find_mid()
{
	memset(mid_line, MISS, CAMERA_H);
	for (int i = NEAR_LINE; i >= FAR_LINE; i--)
	{
		if (my_road_num[i] != MISS)
		{
			mid_line[i] = (x_left[i] + x_right[i]) / 2;
			IMG[i][mid_line[i]] = red;
		}
		else
		{
			for (int j = i; j <= NEAR_LINE; j++)//若中线丢失往下遍历寻找可用中线并赋值
			{
				if (my_road_num[j] != MISS)
				{
					mid_line[i] = mid_line[j];
					break;
				}
			}
		}
		last_mid_line[i] = mid_line[i];
	}
		

}

/*////////////////////////////////////////////
//绝对值
*/////////////////////////////////////////////
int My_Abs(signed int i)
{
	if (i < 0)
		return ~(--i);
	return i;
}


////////////////////////////////////////////
//功能：图像程序测试功能
//输入：标志量：img_protect\zebra_change\
//输出：控制台文字说明显示
//备注：仅用于图像组测试程序时所用，写入单片机时可引去该函数
///////////////////////////////////////////
void img_test(void)
{
	///////////////////////////////////////////////////////////////////
	//if (img_protect == 1)
	//	printf("出赛道已识别\n");
	//else
	//	printf("出赛道未识别\n");
	///////////////////////////////////////////////////////////////////
	//if (zebra_start_line != MISS)
	//	printf("斑马线已识别\n");
	//else
	//	printf("斑马线未识别\n");
	///////////////////////////////////////////////////////////////////
	//printf("中线偏差：%f\n", my_angle);
	///////////////////////////////////////////////////////////////////
	//if (my_garage > 40)
	//	printf("车库中\n");
	//else
	//	printf("已出库\n");
	///////////////////////////////////////////////////////////////////
	//if (my_fork == 1)
	//{
	//	printf("第一岔路类型，标志量为%d,", fork_flag);
	//	if (((fork_flag / 2) % 2) == 0)
	//		printf("向右补线\n");
	//	else
	//		printf("向左补线\n");
	//}
	//else if (my_fork == 2)
	//{
	//	printf("第二岔路类型，标志量为%d,", fork_flag);
	//	if (((fork_flag / 2) % 2) == 0)
	//		printf("向右补线\n");
	//	else
	//		printf("向左补线\n");
	//}
	//else
	//	printf("岔路未识别\n");
	/////////////////////////////////////////////////////////////////
	//int road_width = 0;//道路宽度
	//int i = 40;//目标行数
	//road_width = right_line[i] - left_line[i];
	//for (int j = 0; j <= 187; j++)
	//	IMG[i][j] = 4;
	//printf("第%d行的道路宽度为%d\n", i, road_width);
	///////////////////////////////////////////////////////////////////
	//if (my_straight == 1)
	//	printf("直道已识别\n");
	//else if (my_straight == 2)
	//	printf("直道未识别：左线不直\n");
	//else if (my_straight == 3)
	//	printf("直道未识别：右线不直\n");
	//else if (my_straight == 4)
	//	printf("直道未识别：左右均线不直\n");
	///////////////////////////////////////////////////////////////////
	//if (my_cross == 0)
	//	printf("十字未识别\n");
	//else if (my_cross == 1)
	//	printf("十字已识别，第一型十字\n");
	//else if (my_cross == 2)
	//	printf("十字已识别，第二型十字\n");
	/////////////////////////////////////////////////////////////////
	printf("图像阈值:%d\n", threshold);
	/////////////////////////////////////////////////////////////////
	printf("***************************************\n");
	//////////////////////////////////////////////////////////////////////////////////////赛道涂灰色
	/*uint8_t* left_side_ptr = NULL;
	uint8_t* right_side_ptr = NULL;*/
	for (int i = NEAR_LINE; i >= FAR_LINE; i--)
	{
		if (x_left[i] != MISS && x_right[i] != MISS)
		{
			for (uint8_t j = x_left[i]; j <= x_right[i]; j++)
				IMG[i][j] = sky;
			IMG[i][x_left[i]] = blue;
			IMG[i][x_right[i]] = green;
			IMG[i][mid_line[i]] = red;
		}
		/*if (left_line[i] != MISS && right_line[i] != MISS)
		{
			for (uint8_t j = x_left[i]; j <= x_right[i]; j++)
				IMG[i][j] = sky;
			IMG[i][left_line[i]] = blue;
			IMG[i][right_line[i]] = green;
			IMG[i][mid_line[i]] = red;
		}*/
		/*if (left_smooth[i] != MISS && right_smooth[i] != MISS && last_mid_line[i] != MISS)
		{
			for (uint8_t j = x_left[i]; j <= x_right[i]; j++)
				IMG[i][j] = sky;
			IMG[i][left_smooth[i]] = blue;
			IMG[i][right_smooth[i]] = green;
			IMG[i][mid_line[i]] = red;
		}*/
		/*else
		{
			printf("smooth丢失！！！\n");
		}*/
		/*left_side_ptr = &left_side[i];
		right_side_ptr = &right_side[i];
		for (uint8_t j = 0; j < CAMERA_W; j++)
		{
			if (j <= *left_side_ptr || j >= *right_side_ptr)
				IMG[i][j] = MISS;
		}*/
	}
	//draw_PROSPECT();
	/*for (int i = 0; i < 187; i++)
	{
		IMG[70][i] = 5;
		IMG[84][i] = 5;
		IMG[90][i] = 5;
	}
	for (int i = 0; i < 119; i++)
	{
		IMG[i][43] = 5;
		IMG[i][135] = 5;
	}*/
}

////////////////////////////////////////////
//功能：确定赛道元素类型
//输入：
//输出：赛道元素类型
//备注：0为斑马线，1为直道，2为十字，3为右环岛，4为左环岛，5为入环岛，6位出环岛
///////////////////////////////////////////
uint8_t define_road_flag(void)
{
	uint8_t flag = 0;
	if (zebra_start_line != MISS)
	{
		return 0;
	}
	else
	{
		my_straight = check_straight();
		printf("my_straight:%d\n", my_straight);
		if (my_straight == 1)
		{
			return 1;
		}
		else
		{
			my_cross = check_cross();
			/*if (my_cross == 0)
			{
				my_fork = trident();
			}*/
			if (my_cross != 0)
			{
				return 2;
			}
			/*else if (youhuandao())
			{
				mood.circle = 1;
				return 3;
			}
			else if (zuohuandao())
			{
				mood.circle = 1;
				return 4;
			}
			else if (ruhuandao())
			{
				mood.ruhuandao = 1;
				return 5;
			}
			else if (chuhuandao())
			{
				mood.chuhuandao = 1;
				return 6;
			}*/
			else
			{
				return 10;
			}
		}
	}
}

void image_main(void)
{
	element_init();
	THRE();
	head_clear();
	search_white_range();
	find_all_connect();
	find_road();

	zebra_start_line = check_zebra();
	if (zebra_start_line != MISS)
	{
		inf.if_zebra = 1;
		find_x_left_right();
		clean_zebra();
		//filter_two_line();
		find_mid();
		//inf.my_angle = define_PROSPECT(PROSPECT);
		img_test();
		return;
	}

	fork_point = if_fork();
	find_x_left_right();
	x_left_straight = define_straight_line(x_left, CAMERA_H - 5, 15);
	x_right_straight = define_straight_line(x_right, CAMERA_H - 5, 15);
	printf("x_left_straight:%d,x_right_straight:%d\n", x_left_straight, x_right_straight);

	uint8_t roof_xxr = roof();
	
	road_flag = define_road_flag();
	printf("赛道类型为%d\n", road_flag);

	if (my_cross != 0)
	{
		cross_oridinary_two_line();
	}
	find_mid();
	img_test();
}

