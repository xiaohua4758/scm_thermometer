/***************************************  File Info **********************************************
** File name:			main.c
** Descriptions:
** Created by:			csh
** Created date:		2020.03.05
*************************************************************************************************/
#include <stdio.h>
#include "string.h"
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_Define.h"
#include "myfunction.h"
#include "timer.h"
#include "lcd.h"
#include "button_app.h"
#include "mlx90614.h"

// TFT LCD 80 x 160 Dots
int main(void)
{
	s16 temp_low, temp_high;
	s16 temperture[2];
	u16 delay_mlx_init=0;

	io_init();
	delay(3000);
	t2_start();
    sys_adc_init(7);
	Lcd_Init();
	readout_data();
    //init_mlx90614();
	memset(&lcd_show, 1, sizeof(lcd_show));						//redraw all
	sys_para.sys_mode=1;
	while(1)
	{
		if(delay_mlx_init<9000)
		{
			delay_mlx_init++;
            if(2==delay_mlx_init)
            {
                init_mlx90614();
            }
		}

		if(delay_mlx_init>2)
		{
            if(get_temperature(temperture)&&(!sys_para.hold))   //get data from mlx90614
            {
                sys_para.obj_temperture=temperture[0];
                sys_para.amb_temperture=temperture[1];

                switch(sys_para.sys_mode)
                {
                    case 0: temp_low=358; temp_high=373; break;
                    case 1: temp_low=120; temp_high=500; break;
                }

                if(sys_para.obj_temperture<temp_low)
                    sys_para.result=0;
                else if(sys_para.obj_temperture<temp_high)
                    sys_para.result=1;
                else
                    sys_para.result=2;
            }
		}

        //sys_para.bat=sys_para.bat<20?sys_para.bat+1:0;
        bat_adc_prc();
	    lcd_show_temp();
        key_proc();
	    //delay(2000);
	}
}
