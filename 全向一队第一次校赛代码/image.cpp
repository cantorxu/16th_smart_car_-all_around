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

////////////////////////////////////////////
//功能：初始化
//输入：
//输出：
//备注：将全局变量和数组初始化
///////////////////////////////////////////
void element_init(void)
{
	all_num = 0;
	road_end = FAR_LINE;
	for (uint8_t i = NEAR_LINE; i >= FAR_LINE; i--)
	{
		my_road[i].left = 0;
		my_road[i].right = 0;
		my_road[i].width = 0;
		all_range[i].num = 0;
		road_m[i].num = 0;
	}
}

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
		printf("count(%d)=%d\n", roadf_now, count);
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
	printf("road_end:%d\n", road_end);
	for (uint8_t i = FAR_LINE; i < NEAR_LINE; i++)
	{
		//printf("第%d行：父节点为:%d,所有赛道数量为：%d,可能的道路数为%d\n",i, road_f,all_range[i].num,road_m[i].num);//meihang you yige liangge 
		for (int j = 0; j < road_m[i].num; j++)
		{
			//printf("%d",)
			uint8_t left = road_m[i].range[j].left;
			uint8_t right = road_m[i].range[j].right;
			for (uint8_t k = left; k <= right; k++)
			{
				IMG[i][k] = sky;
			}
			IMG[i][left] = red;
			IMG[i][right] = blue;
		}
	}
}

////////////////////////////////////////////
//功能：向下寻找连通
//输入：哪行哪个白条
//输出：下一行哪个白条
//备注：
///////////////////////////////////////////
uint8_t find_down(uint8_t line, uint8_t num)
{
	//此函数用于向下寻找连通
	uint8_t lined = line + 1;
	white_range whiteline = road_m[line].range[num];
	int wid_max = 0;
	int wid = 0;
	uint8_t num_return = 255;
	//printf("num_now=%d,num_up=%d", roadp[lined].num,roadp[line].num);
	for (uint8_t i = 0; i < road_m[lined].num; i++)
	{
		white_range whiteline_now = road_m[lined].range[i];
		//printf("wid_now=%d", whiteline_now.width);
		if (whiteline_now.left <= whiteline.right && whiteline_now.right >= whiteline.left)//重合
		{
			//printf("ok");
			int left = whiteline_now.left < whiteline.left ? whiteline_now.left : whiteline.left;//最左
			int right = whiteline_now.right > whiteline.right ? whiteline_now.right : whiteline.right;//最右
			int wid1 = right - left;//共长
			wid = (int)whiteline_now.width + (int)whiteline.width - wid1;//重叠面积
			if (wid >= wid_max)//选择重叠最大的
			{
				num_return = i;
				wid_max = wid;
			}
		}
		//printf("wid_now=%d", wid);
	}
	return num_return;
}

////////////////////////////////////////////
//功能：向上寻找连通
//输入：哪行哪个白条
//输出：上一行哪个白条
//备注：
///////////////////////////////////////////
uint8_t find_up(uint8_t line, uint8_t num)
{
	//此函数用于向下寻找连通
	uint8_t lineu = line - 1;
	white_range whiteline = road_m[line].range[num];
	int wid_max = 0;
	int wid = 0;
	uint8_t num_return = 255;

	//printf("num_now=%d,num_up=%d", roadp[lineu].num,roadp[line].num);
	for (uint8_t i = 0; i < road_m[lineu].num; i++)
	{
		white_range whiteline_now = road_m[lineu].range[i];
		//printf("wid_now=%d\n", whiteline_now.width);
		if (whiteline_now.left <= whiteline.right && whiteline_now.right >= whiteline.left)//重合
		{
			//printf("ok1");
			int left = whiteline_now.left < whiteline.left ? whiteline_now.left : whiteline.left;//最左
			int right = whiteline_now.right > whiteline.right ? whiteline_now.right : whiteline.right;//最右
			int wid1 = right - left;//共长
			wid = (int)whiteline_now.width + (int)whiteline.width - wid1;//重叠面积
			if (wid >= wid_max)//选择重叠最大的
			{
				num_return = i;
				wid_max = wid;
			}
		}
		//printf("wid_now=%d", wid);
	}
	return num_return;
}


////////////////////////////////////////////
//功能：找主干道
//输入：road_m
//输出：road
//备注：road_m有岔路，为了简化后面的处理，选择一条主干道
///////////////////////////////////////////
void find_my_road()
{
	//找一个有岔路的行开始判断
	uint8_t i_select = road_end + 30;
	uint8_t num_now_f = 0;
	uint8_t num_max_f = 0;
	for (uint8_t i = road_end + 10; i < road_end + 40; i++)
	{
		num_now_f = road_m[i].num;
		//printf("line=%d,num_now=%d\n", i, num_now_f);
		if (num_now_f >= num_max_f)
		{
			num_max_f = num_now_f;
			i_select = i;
		}
	}
	uint8_t i_end = i_select;
	for (uint8_t i = 30; i <= 60; i++)
	{
		//IMG[i_end][i] = purple;
	}
	//if (i_select == 0)
	//{
	//	//每行只有一个道
	//	for (uint8_t i = NEAR_LINE; i >= road_end; i--)
	//	{
	//		road[i].leftside = roadp[i].whiteline[0].leftside;
	//		road[i].rightside = roadp[i].whiteline[0].rightside;
	//		road[i].width = road[i].rightside - road[i].leftside;
	//	}
	//	return;
	//}

	//printf("i_select=%d\n", i_select);
	//uint8_t i_end = road_end + 10;
	uint8_t temp = 5;
	uint8_t mid_down = road_m[NEAR_LINE].range[down_mid_num].left / 2 + road_m[NEAR_LINE].range[down_mid_num].right / 2;
	uint8_t left_down = mid_down - temp;// roadp[NEAR_LINE].whiteline[0].leftside;
	uint8_t right_down = mid_down + temp;// roadp[NEAR_LINE].whiteline[0].rightside;
	uint8_t num_total = road_m[i_end].num;//可以改变

	uint8_t left_now = 0;
	uint8_t right_now = 0;
	uint8_t delta_now = 0;
	uint8_t delta_min = 200;
	uint8_t mid_now = 0;
	int err_min = 9999;
	int err_sum = 0;
	//int err_thre = 10;
	uint8_t k_select = MISS;
	for (uint8_t k = 0; k < num_total; k++)
	{
		err_sum = 0;
		white_range whiteline_now = road_m[i_end].range[k];

		mid_now = whiteline_now.left / 2 + whiteline_now.right / 2;
		left_now = mid_now - temp;// whiteline_now.leftside;
		right_now = mid_now + temp;// whiteline_now.rightside;
		delta_now = abs((int)mid_now - 70);
		//连线，leftside画一次，rightside画一次

		float base_k_l = ((float)left_now - (float)left_down) / ((float)i_end - (float)NEAR_LINE);
		float base_k_r = ((float)right_now - (float)right_down) / ((float)i_end - (float)NEAR_LINE);
		float base_k = ((float)mid_now - (float)mid_down) / ((float)i_end - (float)NEAR_LINE);
		float l_ref = left_now;
		float r_ref = right_now;
		float ref = mid_now;
		//for (uint8_t i = i_end; i < NEAR_LINE; i++)
		//{
		//	//IMG[i][(uint8_t)(l_ref + 0.5)] = purple;
		//	if (IMG[i][(uint8_t)(l_ref + 0.5)] == 0 || IMG[i][(uint8_t)(r_ref + 0.5)] == 0)
		//		err_sum++;
		//	l_ref += base_k_l;
		//	r_ref += base_k_r;
		//}
		for (uint8_t i = i_end; i < NEAR_LINE; i++)
		{
			if (IMG[i][(uint8_t)(ref + 0.5)] == 0 || IMG[i][(uint8_t)(l_ref + 0.5)] == 0 || IMG[i][(uint8_t)(r_ref + 0.5)] == 0)
			{
				err_sum++;
			}
			//IMG[i][(uint8_t)(ref + 0.5)] = purple;
			l_ref += base_k_l;
			r_ref += base_k_r;
			ref += base_k;

		}
		//printf("err_sum=%d delta_now=%d\n", err_sum,delta_now);
		if (err_sum < err_min)
		{
			/*if (delta_now<delta_min+10&&num_total>1)
			{
				err_min = err_sum;
				k_select = k;
				delta_min = delta_now;
			}
			else if(num_total==1)
			{*/
			err_min = err_sum;
			k_select = k;
			delta_min = delta_now;
			//}
		}
		else if (err_sum == err_min && delta_now < delta_min)
		{
			//两者相等看斜率
			err_min = err_sum;
			k_select = k;
			delta_min = delta_now;
		}
	}
	//printf("k_select=%d", k_select);
	uint8_t num_now = k_select;
	for (uint8_t i = i_end; i <= NEAR_LINE; i++)
	{
		my_road[i].left = road_m[i].range[num_now].left;
		my_road[i].right = road_m[i].range[num_now].right;
		my_road[i].width = my_road[i].right - my_road[i].left;
		num_now = find_down(i, num_now);//迭代出下一行的
		//printf("line=%d,num_select=%d\n",i,num_now);
	}
	//上方用findup连接上
	num_now = k_select;
	for (uint8_t i = i_end; i > FAR_LINE; i--)
	{
		num_now = find_up(i, num_now);
		if (num_now == 255)
		{
			road_end = i + 1;
			break;
		}
		else
		{
			i--;
			my_road[i].left = road_m[i].range[num_now].left;
			my_road[i].right = road_m[i].range[num_now].right;
			my_road[i].width = my_road[i].right - my_road[i].left;
			i++;
		}
	}
	
	for (uint8_t i = NEAR_LINE; i > FAR_LINE; i--)
	{
		uint8_t j_start = my_road[i].left;
		uint8_t j_end = my_road[i].right;
		for (uint8_t j = j_start; j <= j_end; j++)
		{
			if(IMG[i][j] != purple)
			IMG[i][j] = gray;
		}
	}
	for (uint8_t i = 0; i < CAMERA_W; i++)
	{
		IMG[i_end][i] = gray;
	}
	printf("\n");
}

////////////////////////////////////////////
//功能：识别是否为岔道
//输入：
//输出：0为非岔道，1为岔道
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
		printf("第一条的长度为%d，第二条的长度为%d\n", road_m[i].range[0].width, road_m[i].range[1].width);
		if (road_m[i].range[0].width > 40 && road_m[i].range[1].width > 40)
		{
			count++;
		}
	}
	if (count >= 7)
	{
		printf("岔道已识别\n");
		for (uint8_t i = 30; i < 100; i++)
		{
			IMG[i_end][i] = purple;
		}
	}
	return 0;
}

void image_main(void)
{
	element_init();
	THER();
	head_clear();
	search_white_range();
	find_all_connect();
	find_road();
	//uint8_t my_fork = if_fork();
	find_my_road();
	printf("******************************************\n");
}