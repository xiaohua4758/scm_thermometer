/***************************************  File Info **********************************************
** File name:			myfunction.h
** Descriptions:
** Created by:
** Created date:
** Change date:
*************************************************************************************************/

#define enableInterrupts()      (set_EA)
#define disableInterrupts()     (clr_EA)

/* bit define */
#define BIT0  	(0X01)
#define BIT1  	(0X02)
#define BIT2  	(0X04)
#define BIT3  	(0X08)
#define BIT4  	(0X10)
#define BIT5  	(0X20)
#define BIT6  	(0X40)
#define BIT7  	(0X80)
#define BIT8  	(0X100)

/* bit define */
#define NEG_BIT0  	(0xfe)
#define NEG_BIT1  	(0xfd)
#define NEG_BIT2  	(0xfb)
#define NEG_BIT3  	(0xf7)
#define NEG_BIT4  	(0xef)
#define NEG_BIT5  	(0xdf)
#define NEG_BIT6  	(0xbf)
#define NEG_BIT7  	(0x7f)

/* ���ݴ�ŵ�ַ */
/* SYS1 PARA EEPROM ADDRESS */
#define SYS_BACK_DAT				(0x4780)    //ȫAPROMʱ�����һҳ

typedef struct SYSTEM_PARAMETER_STR
{
	u8 backup_dat;

    u8 sys_mode;				// 0-����ģʽ, 1-��ͨģʽ, 2-��¼ģʽ(�鿴)
	u8 hold;

	s16 obj_temperture;			// �����¶�,�ȶѴ�������Ӧ����Ŀ������¶�
	s16 amb_temperture;			// �����¶�,̽ͷ���������¶�
	u8 result;					// 0-normal, 1-low, 2-high
	u8 bat;						// bat 0-21
}system_parameter;

extern system_parameter sys_para;

// lcd��ʾ���ؽṹ��,��ʾ��־λ
typedef struct LCD_DISPLAY_FLAG
{
	u8 title;
	u8 result;
	u8 global_refresh;
	u8 bat;
}lcd_show_flags;
extern lcd_show_flags lcd_show;		//ϵͳ��ʾ��־

#define FAILED      (0)
#define SUCCESS     (1)

void io_init(void);
void delay(unsigned int t);
void u32_to_num(u32 num_in, u8 *num_out_buff, u8 len);
void hook_save_100ms(u8 time);
void save_proc(void);
u8 readout_data(void);
void lcd_refresh_all(void);
void lcd_show_temp(void);
void sys_adc_init(unsigned char acd_ch);
void bat_adc_prc(void);


