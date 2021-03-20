#include "image.h"
#include <iostream>
#include"math.h"

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

uint8_t IMG[CAMERA_H][CAMERA_W];//二值化图像数组
allrange all_range[CAMERA_H];
allrange road_m[CAMERA_H];//可能的赛道白条
road_t my_road[CAMERA_H];
uint8_t all_num = 0;//编号从0开始
uint8_t road_end = FAR_LINE;//可视道路尽头

void THER(void)
{
	for (uint8_t i = 0; i < CAMERA_H; i++)
	{
		for (uint8_t j = 0; j < CAMERA_W; j++)
		{
			if (temp_IMG[i][j] <= 120)
				IMG[i][j] = 0;
			else
				IMG[i][j] = 1;
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
				if (i > 80)
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
		printf("count%d=%d", roadf_now, count);
		if (count > countmax)
		{
			//countmax = count;
			wid_now = all_range[NEAR_LINE].range[roadf_now].width;
			if (wid_now >= wid_max)
			{
				wid_max = wid_now;
				road_f = roadf_now;
				down_mid_num = road_f;
			}
		}
	}
	printf("父节点为：%d\n", road_f);
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
	for (uint8_t i = FAR_LINE; i < NEAR_LINE; i++)
	{
		if (road_m[i].num > 0)
		{
			road_end = i + 1;
			break;
		}
	}
	for (uint8_t i = FAR_LINE; i < NEAR_LINE; i++)
	{
		printf("父节点为:%d,所有赛道数量为：%d,可能的道路数为%d\n", road_f,all_range[i].num,road_m[i].num);//meihang you yige liangge 
		for (int j = 0; j < road_m[i].num; j++)
		{
			//printf("%d",)
			uint8_t left = road_m[i].range[j].left;
			uint8_t right = road_m[i].range[j].right;
			for (uint8_t k = left; k <= right; k++)
			{
				IMG[i][k] = gray;
			}
			IMG[i][left] = red;
			IMG[i][right] = blue;
		}
	}
}


void image_main(void)
{
	THER();
	search_white_range();
	find_all_connect();
	find_road();
}