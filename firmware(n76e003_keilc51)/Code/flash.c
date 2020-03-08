#include "N76E003.h"          //�����õ��������ͷ�ļ�
#include "string.h"
#include "SFR_Macro.h"
#include "Function_Define.h"
#include "myfunction.h"

bit BIT_TMP;

/**************************************************************************
*function:  unsigned char read_APROM_BYTE(unsigned long addr)
*input:     eeprom��ַ
*output:    ����
*describe:  ��ȡ�����EERPOM�е�ֵַΪaddr������
**************************************************************************/
u8 read_APROM_BYTE(u16 code *u16_addr)
{
    u8 rdata;
    rdata = *u16_addr>>8;
    return rdata;
}


//!!!!!!   ע����ļ����ܿ�����Ż�,����99%���ᶼ��дFLASHʧ��

/*****************************************************************************************************************
write_DATAFLASH_BYTE :
user can copy all this subroutine into project, then call this function in main.
******************************************************************************************************************/
volatile void write_DATAFLASH_BYTE(u16 u16_addr,u8 *u8_data, u8 len)
{
	volatile u8 u8_addrl_r;
	volatile u8 xdata i;

    //������Ҫ��ҳ(512 byte)̽�� ��̰��ֽڱ��
    disableInterrupts();
    //u16_addr=SYS1_LOCK_EPROM_ADDR;  //��ַ����
    //Erase APROM DATAFLASH page
    IAPAL = u16_addr;
    IAPAH = u16_addr>>8;
    IAPFD = 0xFF;
    set_IAPEN;
    set_APUEN;
    IAPCN = 0x22;
    set_IAPGO;
    //Save changed RAM data to APROM DATAFLASH
	u8_addrl_r = u16_addr;
	if (u8_addrl_r<0x80)
	{
		u8_addrl_r =0;
	}
	else
	{
		u8_addrl_r = 0x80;
	}
    IAPAL = u8_addrl_r;
    IAPAH = u16_addr>>8;
    set_IAPEN;
    set_APUEN;
    IAPCN = 0x21;
    for(i=0;i<len;i++)
    {
        IAPFD = u8_data[i];
        set_IAPGO;
        IAPAL++;
    }
    clr_APUEN;
    clr_IAPEN;

    enableInterrupts();
}

