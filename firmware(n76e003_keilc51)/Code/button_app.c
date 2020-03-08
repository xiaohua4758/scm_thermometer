/***************************************  File Info **********************************************
** File name:			button.c
** Descriptions:        按键处理函数
** Created by:
** Created date:
*************************************************************************************************/
#include <stdio.h>
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_Define.h"
#include "myfunction.h"
#include "timer.h"
#include "lcd.h"
#include "button_app.h"

struct_key_status key;

unsigned char pre_key=0;	//按键前一个状态
unsigned char cur_key=0;	//按键当前状态

#define MODE_KEY		(P1&BIT5)
#define HOLD_KEY		(P1&BIT2)

//初始化按键状态
void key_init_sta(void)
{
    u8 i;
    for(i=0; i<5; i++)
        key_detect();
}

/**************************************************************************
*function:  void key_detect(void)
*input:     NULL
*output:    NULL
*describe:  按键检测
**************************************************************************/
void key_detect(void)
{
	if(CHECK_KEY_LOW(MODE_KEY)) cur_key|=BIT0;		//BIT0----MODE_KEY
	else cur_key &= NEG_BIT0;

	if(CHECK_KEY_LOW(HOLD_KEY)) cur_key|=BIT1;		//BIT1----HOLD_KEY
	else cur_key &= NEG_BIT1;

	if(cur_key&BIT0)
		key.mode=(pre_key&BIT0)?KEY_LONG_ON:KEY_ONCE_ON;
	else
        key.mode=(pre_key&BIT0)?KEY_ONCE_UP:KEY_OFF;

    if(cur_key&BIT1)
        key.hold=(pre_key&BIT1)?KEY_LONG_ON:KEY_ONCE_ON;
    else
        key.hold=(pre_key&BIT1)?KEY_ONCE_UP:KEY_OFF;

	pre_key = cur_key;	//保存当前按键
}


void do_with_key(void)
{
	if(key.mode==KEY_ONCE_ON)
	{
		sys_para.sys_mode=sys_para.sys_mode>=1?0:sys_para.sys_mode+1;
		lcd_refresh_all();
	}
	if(key.hold==KEY_ONCE_ON)
	{
		sys_para.hold=!sys_para.hold;
	}
}

/* 处理 */
void key_proc(void)
{
	key_detect();
	do_with_key();
}

