/***************************************  File Info **********************************************
** File name:			timer.h
** Descriptions:        定时器相关函数
** Created by:			csh
** Created date:
*************************************************************************************************/

void t2_start(void);
void t2_stop(void);

uint16_t timeout_chk(uint32_t *ptrTimer, uint32_t dwTimeOutVal);

