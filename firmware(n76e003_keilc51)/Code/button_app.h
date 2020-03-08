/***************************************  File Info **********************************************
** File name:			button.c
** Descriptions:        ����������
** Created by:
** Created date:
*************************************************************************************************/

#define CHECK_KEY_LOW(a)		((!a)?(delay(2),((!a)?1:0)):0)	//���͵�ƽ,����
#define CHECK_KEY_HIGH(a)		((a)?(delay(2),(a?1:0)):0)		//���ߵ�ƽ,����

#define KEY_OFF			0		//�ߵ�ƽ,����
#define KEY_ONCE_ON		1		//�½���
#define KEY_LONG_ON		2		//�͵�ƽ
#define KEY_ONCE_UP		5		//������

typedef struct STRUCT_KEY
{
    u8 mode;
    u8 hold;
}struct_key_status;

extern struct_key_status key;

void key_init_sta(void);
void key_detect(void);
void key_proc(void);

