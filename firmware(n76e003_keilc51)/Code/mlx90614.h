#ifndef __MLX90614_H__
#define __MLX90614_H__


#define SMBUS_SCL_1 (P13=1)
#define SMBUS_SCL_0 (P13=0)
#define SMBUS_SDA_1 (P14=1)
#define SMBUS_SDA_0 (P14=0)
#define SMBUS_READ_SDA() (P14)

#define US 1

typedef enum Bit
{
	ACK = 0 ,
	NACK
}bit_t;

uint16_t GetSlaveAddress(void);
uint8_t SetSlaveAddress(uint8_t slave_address);
void SetEmissivity(uint8_t slave_address, float emissivity);
uint16_t GetEmissivity(uint8_t slave_address);
void init_mlx90614(void);
u8 get_temperature(s16 *temp);

#endif
