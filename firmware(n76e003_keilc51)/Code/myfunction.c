/***************************************  File Info **********************************************
** File name:			myfunction.c
** Descriptions:
** Created by:			csh
** Created date:
*************************************************************************************************/
#include "N76E003.h"          //包含用到的外设的头文件
#include "string.h"
#include "SFR_Macro.h"
#include "Function_Define.h"
#include "lcd.h"
#include "timer.h"
#include "flash.h"
#include "myfunction.h"

system_parameter sys_para={0};
lcd_show_flags lcd_show;		//系统显示标志

unsigned int save_delay=0;		//保存延时
/**************************************************************************
*function:  void delay(unsigned int t)
*input:     NULL
*output:    NULL
*describe:  稍长延时
**************************************************************************/
void delay(unsigned int t)
{
	volatile unsigned int i;
	while(t--){
		i=30;
		while(i--); //做一些没有用的事，打发时间
	}
}

void io_init(void)
{
	P13_Quasi_Mode;P13=1;

	P16_Quasi_Mode;P16=1;
	P02_Quasi_Mode;P02=1;

	P30_PushPull_Mode;P30=1;        //lcd sda
	P17_PushPull_Mode;P17=1;        //lcd scl
	P04_PushPull_Mode;P04=0;        //lcd rst
	P01_PushPull_Mode;P01=0;        //lcd dc
	P03_PushPull_Mode;P03=0;        //lcd cs
	P05_PushPull_Mode;P05=0;		//lcd led_k

	P11_Input_Mode;					//power adc
	P15_Quasi_Mode;P15=1;			//mode key
	P12_Quasi_Mode;P12=1;			//hold key

	P13_PushPull_Mode;P13=1;        //mlx90614 scl
	P14_OpenDrain_Mode;P14=1;		//mlx90614 sda

	P10_PushPull_Mode;P10=0;		//test led
}

/**************************************************************************
*function:  unsigned char get_adc_value(void)
*input:     NULL
*output:    ADC转换值,仅取高8位
*describe:  启动一次ADC转换,并返回8位ADC值
**************************************************************************/
unsigned char get_adc_value(void)
{
    clr_ADCF;
    set_ADCS;           // Each time ADC start trig signal
    while(ADCF == 0);

    return (ADCRH);     //读出电压
}

/**************************************************************************
*function:  void bat_adc_prc(void)
*input:     NULL
*output:    NULL
*describe:  电池电压采样处理函数
**************************************************************************/
void bat_adc_prc(void)
{
	code u8 VOLTAGE_1V2=102;		//vref 3.0 (1.2*255/3.0=102)
	code u8 VOLTAGE_1V1=93;

    u8 tmp;
	static u8 pre_adc=0xff;

	tmp=get_adc_value();
	if(tmp==pre_adc)				//when get the same value twice, update bat.
	{
		if(tmp>VOLTAGE_1V2)
		{
			sys_para.bat=21;		//more than 1.2V
		}
		else if(tmp>VOLTAGE_1V1)
		{
			sys_para.bat=(tmp-VOLTAGE_1V1)*5/2;
		}
		else						//less 1.1v
		{
			sys_para.bat=0;
		}
	}
	pre_adc=tmp;
}

/*function:  sys_adc_init(unsigned char acd_ch)
*input:     acd_ch--ADC采样端口
*output:    NULL
*describe:  adc初始化,单次转换,左对齐,预分频1/2
**************************************************************************/
void sys_adc_init(unsigned char acd_ch)
{
    switch(acd_ch)
    {
        case 0:Enable_ADC_AIN0; break;
        case 1:Enable_ADC_AIN1; break;
        case 2:Enable_ADC_AIN2; break;
        case 3:Enable_ADC_AIN3; break;
        case 4:Enable_ADC_AIN4; break;
        case 5:Enable_ADC_AIN5; break;
        case 6:Enable_ADC_AIN6; break;
        case 7:Enable_ADC_AIN7; break;
    }
    get_adc_value();    //ignore first value
}


void u32_to_num(u32 num_in, u8 *num_out_buff, u8 len)
{
	u8 i;

	for(i=0; i<len; i++)
	{
        num_out_buff[len-1-i] = num_in%10;
        num_in = num_in/10;
	}
}

/**************************************************************************
*function:  void readout_data(void)
*input:     NULL
*output:    NULL
*describe:  读出保存数据处理函数
**************************************************************************/
u8 readout_data(void)
{
	/*system1 para*/
	sys_para.backup_dat=read_APROM_BYTE(SYS_BACK_DAT);

    return 1;
}

/**************************************************************************
*function:  void hook_save_data(void)
*input:     NULL
*output:    NULL
*describe:  延时保存数据
**************************************************************************/
void hook_save_100ms(u8 time)
{
	save_delay=time;
}

void save_data(void)
{
    volatile u8 xdata wr_dat[10];

    wr_dat[0]=sys_para.backup_dat;
    write_DATAFLASH_BYTE(SYS_BACK_DAT, wr_dat, 1);
}


/**************************************************************************
*function:  void save_proc(void)
*input:     NULL
*output:    NULL
*describe:  保存数据处理函数
**************************************************************************/
void save_proc(void)
{
    static u32 f100ms_save=0;
	if(save_delay>0)
	{
        if(timeout_chk(&f100ms_save, 50)==SUCCESS)            //定义节拍50ms
        {
            timeout_chk(&f100ms_save, 0);
    		save_delay--;
    		if(save_delay==0)
    		{
                save_data();
    		}
        }
	}
}

void lcd_refresh_all(void)
{
	memset(&lcd_show, 1, sizeof(lcd_show));
}


// value:0-20
void show_bat(u8 x,u8 y, u8 value, u8 redraw_flag)
{
	u16 x1, y1, x2, y2;
    extern code unsigned char gImage_bat32x17[];

	if(redraw_flag)
	    LCD_ShowPicture(gImage_bat32x17,x,y,x+32-1,y+17-1);

	if(value>19)
		value=21;

    x1=x+4;
    x2=x1+value;

	y1=y+4;
	y2=y+12;
    LCD_Fill(x1, y1, x2, y2, 0xd69a);

	if(value<21)
	{
        x1=x2;
        x2=x+25;
        LCD_Fill(x1, y1, x2, y2, BLACK);
	}
}

//
void lcd_show_temp(void)
{
    extern code unsigned char gImage_temp29x26[];

	static u8 pre_bat=0xff, pre_resut=0xff, pre_hold=0xff;
	u8 x,y,x1,y1;
	u8 buf[6];
	u16 tmp, color;

    if(lcd_show.title)
    {
        x=(160-16*2)/2-8; y=0;
		switch(sys_para.sys_mode)
		{
			case 0: buf[0]=10; buf[1]=11; break;		//体10,温11
			case 1: buf[0]=1; buf[1]=2; break;			//测1,量2
			case 2: buf[0]=3; buf[1]=4; break;			//记3,录4
		}

        LCD_ShowChinese(x,y,buf[0],24,YELLOW); x+=24;
        LCD_ShowChinese(x,y,buf[1],24,YELLOW);
    }
	// temp. logo
    if(lcd_show.global_refresh)
    {
        x=10;y=22;
        LCD_ShowChar16x32(x,y,13,LIGHTBLUE|0xc30c);		// 'logo'13
    }

	if((sys_para.hold!=pre_hold)||lcd_show.global_refresh)
	{
		x=12;y=4;
		if(sys_para.hold)
		{
			LCD_ShowString_Rev(x,y,"HOLD",WHITE,BLUE);
		}
		else
		{
			LCD_ShowString(x,y,"REC ",MAGENTA);
		}
		//show_num_with_bord(x,y,buf[0],color);x+=16;		// record with bord
	}

	// object temp. value
	color=(sys_para.hold)?WHITE:CYAN;
    x=10+24;y=22;
	if(sys_para.obj_temperture>0)
	{
	    tmp = sys_para.obj_temperture;
		LCD_ShowChar16x32(x,y,12,color);			// '+'12
	}
	else
	{
		tmp = 0-sys_para.obj_temperture;
		LCD_ShowChar16x32(x,y,11,color);			// '-'11
	}

    u32_to_num(tmp, buf, 4);
	x+=16+1; y=22;
	if(buf[0])
    	LCD_ShowChar16x32(x,y,buf[0],color);
    else
    	LCD_Fill(x, y, x+16, y+32, BLACK);
    x+=16+1;

    if(buf[0]||buf[1])
    	LCD_ShowChar16x32(x,y,buf[1],color);
	else
    	LCD_Fill(x, y, x+16, y+32, BLACK);
    x+=16+1;

    LCD_ShowChar16x32(x,y,buf[2],color);
    x+=16+1;
    if(lcd_show.global_refresh||(sys_para.hold!=pre_hold))
	    LCD_ShowChar16x32(x,y,10,color); 		//dot
	x+=8+1;
    LCD_ShowChar16x32(x,y,buf[3],color); x+=18;
    if(lcd_show.global_refresh||(sys_para.hold!=pre_hold))
	    LCD_ShowChinese(x,y,0,32,color);			//C - unit

	//-- ambient temperature --
	tmp = (sys_para.amb_temperture>0)?sys_para.amb_temperture:(0-sys_para.amb_temperture);
    u32_to_num(tmp, buf, 4);
	x=160-32-6*8-4;y=80-16-8;					//
	x1=x;y1=y;
    if(lcd_show.global_refresh)
    {
        LCD_ShowChinese(x+2,y+2,0,16,WHITEBLUE);x=x+16;     //环0
        LCD_ShowChinese(x+2,y+2,1,16,WHITEBLUE);x=x+16;     //境1
    }
    else
    	x=x+32;

	x+=4; y+=2;
    buf[6]=(sys_para.amb_temperture>0)?'+':'-';
	LCD_ShowChar(x,y,buf[6],0,WHITEBLUE);
	x+=8;

	if(buf[0])
	{
	    LCD_ShowChar(x,y,buf[0]+'0',0,WHITEBLUE);
		x+=8;
	}

    if(buf[0]||buf[1])
    {
        LCD_ShowChar(x,y,buf[1]+'0',0,WHITEBLUE);
        x+=8;
    }

	LCD_ShowChar(x,y,buf[2]+'0',0,WHITEBLUE); x+=8;
    LCD_ShowChar(x,y,'.',0,WHITEBLUE); x+=8;
	LCD_ShowChar(x,y,buf[3]+'0',0,WHITEBLUE); x+=8;
	while(x<(x1+32+4+48))
	{
        LCD_ShowChar(x,y,' ',0,WHITEBLUE); x+=8;
	}

	// result
    x=10; y=80-27;
	if((lcd_show.global_refresh)||(pre_resut!=sys_para.result))
	{
		if(0==sys_para.sys_mode)
		{
            switch(sys_para.result)
            {
                case 0: buf[0]=7; buf[1]=8; color=MAGENTA; break;		//过7, 低8
                case 1: buf[0]=5; buf[1]=6; color=HIGHGREEN;break;		//正5, 常6
                case 2: buf[0]=7; buf[1]=9; color=YELLOW;break;			//过7, 高9
            }
		}
		else if(1==sys_para.sys_mode)
		{
            switch(sys_para.result)
            {
                case 0: buf[0]=8; buf[1]=11; color=MAGENTA; break;			//低8, 温11
                case 1: buf[0]=12; buf[1]=11; color=HIGHGREEN;break;		//中12,温11
                case 2: buf[0]=9; buf[1]=11; color=YELLOW;break;			//高9, 温11
            }
		}

        LCD_ShowChinese(x,y,buf[0],24,color); x+=24;
        LCD_ShowChinese(x,y,buf[1],24,color);
	}

	//battery
    if((lcd_show.bat)||(pre_bat!=sys_para.bat))
	{
        x=160-36;y=4;
        if(sys_para.bat<=21)
	        show_bat(x, y, sys_para.bat, lcd_show.global_refresh|(pre_bat==0xff));
	}

	pre_resut=sys_para.result;
	pre_bat = sys_para.bat;
	pre_hold=sys_para.hold;
	memset(&lcd_show, 0, sizeof(lcd_show));
}

