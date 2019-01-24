#ifndef __W25QCSFLASHCS
#define __W25QCSFLASHCS
// spi时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
// 1注意：如果出现数据异常，尝试芯片断电一会在上电
#include <stdint.h>

typedef enum DEV_TYPE {W25Q,MX25}DEV_TYPE;//支持两种芯片
// enum DEV_STA1_ {W25QCS_BUSY,W25QCS_IDLE,W25QCS_ERROR,W25QCS_READY,};
typedef struct W25QCS_BASE{
	// enum DEV_STA1_ dev_sta;

	uint16_t W25QCS_SIZE_MB;
	void (* W25QCS_CS_L_enable)(void);
	void (* W25QCS_CS_H)(void);
	// spi时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
	uint8_t (*W25QCS_SPI_RW)(uint8_t a_u8);
	uint8_t (*DELAY_MS)(uint32_t a_u8);
	DEV_TYPE dev_type;

	uint16_t good_trans_count_u16;
	uint16_t bad_trans_count_u16;
	uint16_t good_recv_count_u16;     
}W25QCS_BASE;


void W25Q_Init(W25QCS_BASE * base,
					void (* W25QCS_CS_L_enable)(void),
					void (* W25QCS_CS_H)(void),
					// spi时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
					uint8_t (*W25QCS_SPI_RW)(uint8_t a_u8),
					uint8_t (*DELAY_MS)(uint32_t a_u8),
					uint32_t size_MB,
					 DEV_TYPE dev_type
					);
uint8_t W25Q_Test(W25QCS_BASE * base); //用于测试 SPI FLASH 能工作就会返回'k' 否者返回‘n’
void W25QCSXX_Read(W25QCS_BASE * base,uint8_t* pBuffer,uint32_t ReadAddr,uint16_t len); //这可以读出整个flash的内容
uint8_t W25QCS_Write(W25QCS_BASE * base,uint32_t addr,uint8_t* ptr,uint16_t len);//任意位置都可以写入
uint8_t W25QCSXX_Erase_Sector(W25QCS_BASE * base,uint32_t Dst_Addr); 
void W25QCSXX_Erase_Chip(W25QCS_BASE * base);  

void W25QCSXX_PowerDown(W25QCS_BASE * base);  
void W25QCSXX_WAKEUP(W25QCS_BASE * base);  
void W25QCSXX_ReadUniquidID(W25QCS_BASE * base,uint8_t *ptr,uint8_t len);
uint16_t W25QCSXX_ReadID(W25QCS_BASE * base);
uint8_t W25QCSXX_ReadSR(W25QCS_BASE * base);  


#endif

