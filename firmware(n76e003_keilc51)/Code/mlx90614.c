#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_Define.h"
#include "mlx90614.h"

static uint16_t slave_address=0, newaddress=0;
static s16 temperature_vaule[2];	// temperature_vaule[0]=object temp, temperature_vaule[1]=ambient temp
static uint8_t read_temp_flag=0;	// 1-ok, 0-ng

static uint8_t crc_buffer[5] = {0x00,0x00,0x00,0x00,0x00}; //used to save the data to send
static uint8_t cnt = 0;				//used to save how many times the master device send byte

void Delay_us(uint16_t us)
{
	volatile uint16_t l_us=us;
	while(l_us)
		l_us--;
}

void SMBus_Init(void)
{
	SMBUS_SCL_1;
	SMBUS_SDA_1;
	Delay_us(US);

	SMBUS_SCL_0;
	Delay_us(1400);

	SMBUS_SCL_1;
	Delay_us(US);
}

void SMBus_Start(void)
{
	SMBUS_SDA_1;
	Delay_us(US);

	SMBUS_SCL_1;
	Delay_us(US);

	SMBUS_SDA_0;
	Delay_us(US);

	SMBUS_SCL_0;
	Delay_us(US);
}
uint8_t WaitACK(void)
{
	uint8_t ack = 2;

	SMBUS_SCL_0;
	SMBUS_SDA_1;
	Delay_us(US);

	SMBUS_SCL_1;
	Delay_us(US);

	ack = SMBUS_READ_SDA();
	if(ack != 0)
	{
		//UART_PRINTF("after %d times,nack\n",cnt);
		return 1;
	}

	SMBUS_SCL_0;
	Delay_us(1);

	return 0;
}

uint8_t SMBus_SendByte(uint8_t byte)
{
	s16 i = 0, j = 0;
	cnt++;

	for(i = 7; i >= 0; i--)
	{
		SMBUS_SCL_0;
		//Delay_us(US);

		if ( (byte & (0x80 >> j)) >> i )
		{
			SMBUS_SDA_1;
			Delay_us(US);
		}
		else
		{
			SMBUS_SDA_0;
			Delay_us(US);
		}

		SMBUS_SCL_1;
		Delay_us(US);

		j++;
	}

	return WaitACK();
}

void SMBus_SendBit(bit_t ack)
{
	SMBUS_SCL_0;
	Delay_us(US);

	if(ack == ACK)
	{
		SMBUS_SDA_0;
		Delay_us(US);
	}
	else
	{
		SMBUS_SDA_1;
		Delay_us(US);
	}

	SMBUS_SCL_1;
	Delay_us(US);

	SMBUS_SCL_0;
	Delay_us(US);
}

// ack:1 with ack, 0-no ack
uint8_t SMBus_ReadByte(uint8_t ack)
{
	s8 i = 0;
	uint8_t dat = 0;

    SMBUS_SDA_1;
	for(i = 7; i >= 0; i--)
	{
		SMBUS_SCL_0;
		Delay_us(2);

		SMBUS_SCL_1;
		Delay_us(2);

		if(SMBUS_READ_SDA())
		{
			dat |= 1 << i;
		}
	}

	SMBUS_SCL_0;
	if(ack)
	    SMBUS_SDA_0;
	else
		SMBUS_SDA_1;
	Delay_us(2);
	SMBUS_SCL_1;
	Delay_us(2);

	SMBUS_SCL_0;
	Delay_us(US);

	return dat;
}

void SMBus_Stop(void)
{
	SMBUS_SDA_0;
	Delay_us(US);

	SMBUS_SCL_1;
	Delay_us(US);

	SMBUS_SDA_1;
	Delay_us(US);
}

unsigned char CRC8(unsigned char *ptr,unsigned char len)
{
    unsigned char crc;
    unsigned char i;
    crc = 0;
    while(len--)
    {
       crc ^= *ptr++;
       for(i = 0;i < 8;i++)
       {
           if(crc & 0x80)
           {
               crc = (crc << 1) ^ 0x07;
           }
           else
		   {
			   crc <<= 1;
		   }
       }
    }
    return crc;
}

uint16_t GetSlaveAddress(void)
{
	uint16_t data_low = 0, data_high = 0, da = 0;
	uint8_t pec = 0;

	SMBus_Start();
	if(SMBus_SendByte(0) == 0)
	{
		if(SMBus_SendByte(0x2E) == 0)
		{
			SMBus_Start();
			if(SMBus_SendByte(1) == 0)
			{
				data_low = SMBus_ReadByte(1);	// with ack
				data_high =  SMBus_ReadByte(1);	// with ack
				pec = SMBus_ReadByte(0);		// no ack

				SMBus_Stop();
				//UART_PRINTF("SLAVE PEC = %#x \n", pec);
			}
			else
			{
				//UART_PRINTF("Read Command wrong!");
                return 0;
			}
        }
		else
		{
			//UART_PRINTF("Write Command Wrong!\n");
            return 0;
		}
	}
	else
	{
		//UART_PRINTF("Write Slave Address Wrong!\n");
        return 0;
	}

	da = data_low;  //Only LSByte
	return da;

}

uint8_t SetSlaveAddress(uint8_t slave_address)
{
	uint8_t i = 0;

	if(slave_address > 127)
	{
		return 1;
	}

	crc_buffer[0] = 0x00;
	crc_buffer[1] = 0x2E;
	crc_buffer[2] = slave_address;
	crc_buffer[3] = 0;
	crc_buffer[4] = CRC8(crc_buffer,4);

	SMBus_Start();

	for(i = 0; i < sizeof(crc_buffer)/sizeof(uint8_t); i++)
	{
		SMBus_SendByte(crc_buffer[i]);
	}

	SMBus_Stop();

	return 0;
}

//temp_type: 0-object temp, 1-ambient temp
uint16_t MLX_GetTemp(uint8_t slave_address, uint8_t temp_type)
{
	uint16_t dat = 0;
	u8 data_low = 0, data_high = 0, pec;
	u8 reg_add;

	if(1==temp_type)
		reg_add=0x06;
	else
		reg_add=0x07;

	SMBus_Start();
	SMBus_SendByte(slave_address << 1);
	SMBus_SendByte(reg_add);
	SMBus_Start();
	SMBus_SendByte((slave_address << 1) | (uint8_t)1);

	data_low = SMBus_ReadByte(1);
	data_high = SMBus_ReadByte(1);

	pec = SMBus_ReadByte(0);
    SMBus_Stop();

    crc_buffer[0] = slave_address<<1;
	crc_buffer[1] = reg_add;
	crc_buffer[2] = crc_buffer[0]|0x01;
    crc_buffer[3] = data_low;
	crc_buffer[4] = data_high;
	crc_buffer[5] = CRC8(crc_buffer,5);

	if(crc_buffer[5] != pec)
		read_temp_flag=0;	//fail
	else
		read_temp_flag=1;	//ok

	dat = ((u16)data_high << 8) | data_low;
	return dat;
}

uint16_t GetEmissivity(uint8_t slave_address)
{
	uint16_t data_low = 0, data_high = 0, dat = 0;
	uint8_t pec = 0;

	SMBus_Start();

	SMBus_SendByte(slave_address << 1);
	SMBus_SendByte(0x24);
	SMBus_Start();
	SMBus_SendByte((slave_address << 1) + 1);

	data_low = SMBus_ReadByte(1);
	data_high = SMBus_ReadByte(1);
	pec = SMBus_ReadByte(0);
	SMBus_Stop();

	dat = data_high << 8 | data_low;

	return dat;
}

void SetEmissivity(uint8_t slave_address, float emissivity)
{
	uint16_t emissivity_hex = (uint16_t)(emissivity * 65535);
	uint8_t i = 0;
    crc_buffer[0] = slave_address << 1;
	crc_buffer[1] = 0x24;
	crc_buffer[2] = emissivity_hex & 0xff; //LSByte
	crc_buffer[3] = emissivity_hex >> 8;   //MSByte
	crc_buffer[4] = CRC8(crc_buffer,4);

	SMBus_Start();

	for(i = 0; i < sizeof(crc_buffer)/sizeof(uint8_t); i++)
	{
		SMBus_SendByte(crc_buffer[i]);
	}

	SMBus_Stop();
}

// 注意MLX90164的SMBus的时序,SCL的速率必须在10KHZ-100KHZ,否则读出的数据可能会错误
// 所以事先必须用示波器或逻辑分析仪调试好时序
void init_mlx90614(void)
{
	u16 emiss;
	slave_address = GetSlaveAddress();
	newaddress = 0x19;
	if(slave_address != newaddress)
	{
		SetSlaveAddress(0x00);
		Delay_us(10000);
		if(SetSlaveAddress(newaddress) != 0)  //set address failed
		{
			slave_address = GetSlaveAddress();
		}
	}
	Delay_us(1000);
	slave_address = GetSlaveAddress();
	emiss =GetEmissivity(slave_address);
	SetEmissivity(slave_address, 0);
	Delay_us(2000);
	emiss =GetEmissivity(slave_address);
	SetEmissivity(slave_address, 0.9);
	emiss =GetEmissivity(slave_address);
	Delay_us(2000);
}

u8 get_temperature(s16 *temp)
{
	u8 result;
	s32 tmp;
	s32 temperture;
	static u8 i=0;
	result = 1;

	i=!i;
	//for(i=0; i<2; i++)
	{
        tmp = MLX_GetTemp(slave_address, i);		// i=0:object temp, i=1:ambient temp
        if(0==read_temp_flag)
        {
            result=0;								//fail
        }
        else
        {
            temperture = (tmp * 2 - 27315)/10;      // 0.1 C(temperture = tmp * 0.02 - 273.15;)
            if(temperture<(-9999))					// do some constrain
                temperture=(-9999);
            if(temperture>9999)
                temperture=9999;

            temperature_vaule[i]=temperture;		// updata
        }

		temp[i]=temperature_vaule[i];				//
	}

	return result;
}
