#ifndef _IMAGE_H
#define _IMAGE_H
#include <stdio.h>
#include <stdlib.h>
#include "cv.h"
#include "highgui.h"
#include <math.h>
#include "image.h"

#define MISS 255
#define CAMERA_H  60                            //图片高度
#define CAMERA_W  130                           //图片宽度
#define FAR_LINE 1//图像处理上边界
#define NEAR_LINE 55//图像处理下边界
#define LEFT_SIDE 0//图像处理左边界
#define RIGHT_SIDE 129//图像处理右边界
#define MISS 255
#define white_num_MAX 8//每行最多允许白条数
#define CHANGED_H 60
#define CHANGED_W 130

//#define our_test bool
#define our_test uint8_t

/////////////////////////////
#define black 0
#define white 1
#define blue  2
#define green 3
#define red   4
#define gray  MISS
#define purple 6
#define sky 7
#define cyan 8
///////////////////////////

extern uint8_t IMG[CAMERA_H][CAMERA_W];//二值化图像数组
extern uint8_t image_Buffer_0[CAMERA_H][CAMERA_W];
extern uint8_t* fullBuffer;//指向灰度图的首地址

extern uint8_t temp_IMG[CAMERA_H][CAMERA_W];//二值化图像数组
extern uint8_t last_mid_line[CAMERA_H];

///////////////////////////
//与控制交接结构体定义
typedef struct {
	float my_angle;//偏差
	float last_angle;//上一次偏差
	uint8_t if_out;//出赛道判断 0-未出  1-出赛道
	uint8_t if_zebra;//斑马线检测 0-无斑马线 1-斑马线
	uint8_t if_acceleration;//是否加速 0-不加速 1-加速
	uint8_t now_road_type;//赛道类型，便于调试
	uint8_t zebra_start_line;//斑马线起始行 未找到时为255
	uint8_t general_threshold1;//中间部分阈值(普通二值化)
	uint8_t general_threshold2;//两边部分阈值（普通二值化）
	uint8_t general_threshold3;
	uint8_t if_stage_5;

}all_infomation;
extern all_infomation inf;

///////////////////////////
//环岛结构体定义
typedef struct {
	uint8_t circle;
	uint8_t ruhuandao;
	uint8_t chuhuandao;
	uint8_t cross;
	uint8_t straight;
}MOOD;
extern MOOD mood;
///////////////////////////
//图像基本处理的定义

void head_clear(void);
void THRE(void);
void thre1(void);
void get_HistGram(void);
void GetOSTUThreshold(float* HistGram);
void map(void);
void element_init(void);

///////////////////////////
//基础找路找线函数的定义

uint8_t find_f(uint8_t a);
void search_white_range();
void find_all_connect();
void find_road();
uint8_t find_down(uint8_t line, uint8_t num);
uint8_t find_up(uint8_t line, uint8_t num);
void find_my_road();
uint8_t find_continue(uint8_t i_start, uint8_t j_start);
void ordinary_two_line(void);
void image_main();
void get_mid_line(void);

///////////////////////////
//出赛道及斑马线识别的定义

uint8_t check_zebra(void);
void clean_zebra(void);
uint8_t check_out(void);

///////////////////////////
//测试函数的定义

void find_mid(void);
float define_PROSPECT(uint8_t PROSPECT);

void draw_PROSPECT(void);

void img_test(void);
///////////////////////////
//工具函数的定义

void my_memset(uint8_t* ptr, uint8_t num, uint8_t size);
void regression(int type, int startline, int endline);
int My_Abs(signed int i);
float my_sqrt(float x);
void check_right_line(uint8_t start, uint8_t end);
void Cal_Line(float k, float b, uint8_t start_point, uint8_t end_point, uint8_t ArryName);
void make_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void paint(void);
void draw_line(uint8_t line);

///////////////////////////

//岔路函数

void check_fork(void);
void check_whitefork(void);
uint8_t if_fork(void);
uint8_t trident(void);

//车库函数

uint8_t check_garage(void);
void out_garage(void);

//十字函数

uint8_t check_cross(void);

//直道函数

uint8_t check_straight(void);
uint8_t define_straight_line(uint8_t* line, uint8_t start_line, uint8_t end_line);

//大角度弯道函数
uint8_t roof(void);

///////////////////////////
//基本处理函数

void THRE();
void denoise(void);
uint8_t define_my_way(uint8_t line);
void if_connect(void);
void if_smooth(void);
void filter_two_line(void);
void if_edge_continuos(void);
void find_x_right_left(void);

uint8_t check_bottom(void);
uint8_t check_through(void);

///////////////////////////
//处理主函数

uint8_t define_road_type(void);//辛老板
uint8_t define_road_flag(void);

///////////////////////////
//决定双边

void straight_oridinary_two_line(void);
void cross_oridinary_two_line(void);

///////////////////////////
//辛老板环岛版本

uint8_t prefect_stright();
uint8_t youhuandao();
uint8_t zuohuandao();
uint8_t shizi();
uint8_t ruhuandao();
uint8_t chuhuandao();
uint8_t jinshizi();

void find_leftdown_point(uint8_t start_point, uint8_t end_point, int32_t plus_minus);
void find_rightdown_point(uint8_t start_point, uint8_t end_point, int32_t plus_minus);
void find_leftup_point(uint8_t start_point, uint8_t end_point, int32_t plus_minus);
void find_rightup_point(uint8_t start_point, uint8_t end_point, int32_t plus_minus);

void advanced_LeastSquareCalc_Curve(uint8_t type, uint8_t StartLine1, uint8_t EndLine1, uint8_t StartLine2, uint8_t EndLine2);

///////////////////////////

#endif //