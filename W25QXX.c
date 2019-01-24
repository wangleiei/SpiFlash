#include "W25QXX.h"

#include  "Interface.h"

static void W25QCSXX_Write_Enable(W25QCS_BASE * base);  
static void W25QCSXX_Write_Disable(W25QCS_BASE * base); 
static void W25QCSXX_Write_NoCheck(W25QCS_BASE * base,uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);  
static uint32_t W25QCSXX_Wait_Busy(W25QCS_BASE * base,uint32_t ms); 
static uint32_t W25QCSXX_Write_Page(W25QCS_BASE * base,uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
static uint8_t W25QCSXX_ReadSR123(W25QCS_BASE * base,uint8_t sr);
/***************************内部命令**************/
enum W25QCOMMAND {
	W25X_WriteEnable		= 0x06 ,
	W25X_WriteDisable		= 0x04 ,
	W25X_ReadStatusReg1		= 0x05 ,
	W25X_ReadStatusReg2		= 0x35 ,
	W25X_ReadStatusReg3		= 0x15 ,
	W25X_WriteStatusReg		= 0x01 ,
	W25X_ReadData			= 0x03 ,
	W25X_FastReadData		= 0x0B ,
	W25X_FastReadDual		= 0x3B ,
	W25X_PageProgram		= 0x02 ,
	W25X_BlockErase			= 0xD8 ,
	W25X_SectorErase		= 0x20 ,
	W25X_ChipErase			= 0xC7 ,
	W25X_PowerDown			= 0xB9 ,
	W25X_ReleasePowerDown	= 0xAB ,
	W25X_DeviceID			= 0xAB ,
	W25X_ManufactDeviceID	= 0x90 ,
	W25X_JedecDeviceID		= 0x9F ,
	W25X_BlockSectorUlock	= 0x39 ,
	W25X_BlockSectorLock	= 0x36 
};

uint8_t W25QCSXX_ReadSR123(W25QCS_BASE * base,uint8_t sr)   {  
	uint8_t byte=0;
	if((3>=sr)&&(sr>=1)){
		base->W25QCS_CS_H();
		//W25QCS_DELAYUS_U8(10);   
		base->W25QCS_CS_L_enable();
		if(1 == sr){
			base->W25QCS_SPI_RW(W25X_ReadStatusReg1);   		
		}else if(2 == sr){
			base->W25QCS_SPI_RW(W25X_ReadStatusReg2);
		}else if(3 == sr){
			base->W25QCS_SPI_RW(W25X_ReadStatusReg3);
		}                     
		byte = base->W25QCS_SPI_RW(0); 

		base->W25QCS_CS_H();                               
		return byte;
	}else{
		return 0;
	}
} 
void W25QCSXX_Write_SR(W25QCS_BASE * base,uint8_t sr)   {   
	base->W25QCS_CS_L_enable();                           
	base->W25QCS_SPI_RW(W25X_WriteStatusReg); 
	base->W25QCS_SPI_RW(sr);             
	base->W25QCS_CS_H();                         
}   
static void W25QCSXX_Write_Enable(W25QCS_BASE * base)   
{
	base->W25QCS_CS_L_enable();                           
    base->W25QCS_SPI_RW(W25X_WriteEnable);     
	base->W25QCS_CS_H();                               
} 
static void W25QCSXX_Write_Disable(W25QCS_BASE * base)   {  
	base->W25QCS_CS_L_enable();                          
    base->W25QCS_SPI_RW(W25X_WriteDisable);     
	base->W25QCS_CS_H();                                  
} 		
uint16_t W25QCSXX_ReadID(W25QCS_BASE * base)
{
	uint16_t Temp = 0;	  
	base->W25QCS_CS_L_enable();	
	for(Temp = 0;Temp<1000;Temp++){}		
	base->W25QCS_SPI_RW(0x90);   
	base->W25QCS_SPI_RW(0x00); 	    
	base->W25QCS_SPI_RW(0x00); 	    
	base->W25QCS_SPI_RW(0x00); 	
	Temp|=base->W25QCS_SPI_RW(0x00);
	Temp= Temp<<8;
	Temp|=base->W25QCS_SPI_RW(0x00);	 
	base->W25QCS_CS_H();				    
	return Temp;
}
void W25QCSXX_ReadUniquidID(W25QCS_BASE * base,uint8_t *ptr,uint8_t len){
	uint8_t i = 0;
	base->W25QCS_CS_H();
	//W25QCS_DELAYUS_U8(10);
	base->W25QCS_CS_L_enable();
	base->W25QCS_SPI_RW(0x9F);
	// 4个dummy
	for(i = 0;i < 4;i++){
		base->W25QCS_SPI_RW(0);
	}
	for(i = 0;((i<4)&&(i<len));i++){
		*(ptr+i) = base->W25QCS_SPI_RW(0);
	}
	//W25QCS_DELAYUS_U8(10);
	base->W25QCS_CS_H();
}
/**********************************************************************************************************
*	函 数 名: uint8_t W25Q_Test(W25QCS_BASE * base)
*	功能说明: 测试flash是否正常，
*	传    参: 
*	返 回 值: 1 测试不通过，器件可能有问题，0：测试通过
*   说    明: 
*********************************************************************************************************/
uint8_t W25Q_Test(W25QCS_BASE * base){
	uint8_t buf[4] = {0,0,0,0};
	uint16_t w25temp = 0;
	W25QCSXX_ReadUniquidID(base,buf,4);
	// w25temp = buf[0]+buf[1]+buf[2]+buf[3];
	if((buf[0] != 0) && (0xff != buf[0])){
		return 0;
	}
	return 1;
}
void W25QCSXX_Read(W25QCS_BASE * base,uint8_t* pBuffer,uint32_t ReadAddr,uint16_t len)   
{ 
 	uint16_t i;
 	W25QCSXX_Write_Enable(base);
	base->W25QCS_CS_H();
	//W25QCS_DELAYUS_U8(10);
	base->W25QCS_CS_L_enable();	                          
    base->W25QCS_SPI_RW(W25X_ReadData);        
    base->W25QCS_SPI_RW((uint8_t)((ReadAddr)>>16));  
    base->W25QCS_SPI_RW((uint8_t)((ReadAddr)>>8));   
    base->W25QCS_SPI_RW((uint8_t)ReadAddr);   
    for(i = 0;i < len;i ++)
	{  
        pBuffer[i] = base->W25QCS_SPI_RW(0XFF);  
    }
	base->W25QCS_CS_H();  				    	      
}
/**********************************************************************************************************
*	函 数 名: void W25QCS_Write(W25QCS_BASE * base,uint32_t addr,uint8_t* ptr,uint16_t len){
*	功能说明: 往flash里写入数据，
*	传    参: 
	@ptr：数据指针
	@addr：flash存储地址
	@len：数据长度，最长0xffff
*	返 回 值: 
*   说    明: 
*********************************************************************************************************/
uint8_t W25QCS_Write(W25QCS_BASE * base,uint32_t addr,uint8_t* ptr,uint16_t len){
	
	uint8_t i = 0;
	uint8_t page_num = len>>8;
	uint8_t relase_num = len&0x00ff;
	// TST_Print_EX("addr %xptr %xlen %d\n",addr,ptr,len);
	if(len>256){
		for(i = 0;i<page_num;i++){//16
			W25QCSXX_Write_Page(base,(uint8_t *)(ptr+i*256),addr+i*256,256);
		}
		if(0 != relase_num){
			W25QCSXX_Write_Page(base,(uint8_t *)(ptr+i*256),addr+i*256,relase_num);		
		}
		
	}
	else{
		W25QCSXX_Write_Page(base,ptr,addr,len);
	}
}  
static uint32_t W25QCSXX_Write_Page(W25QCS_BASE * base,uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{//test ok
 	uint16_t i;  
    W25QCSXX_Write_Enable(base);                  
	base->W25QCS_CS_L_enable();                            
    base->W25QCS_SPI_RW(W25X_PageProgram);     
    base->W25QCS_SPI_RW((uint8_t)((WriteAddr)>>16)); 
    base->W25QCS_SPI_RW((uint8_t)((WriteAddr)>>8));   
    base->W25QCS_SPI_RW((uint8_t)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++){
    	base->W25QCS_SPI_RW(pBuffer[i]);
    }
	base->W25QCS_CS_H();  

	return W25QCSXX_Wait_Busy(base,0);
} 
static void W25QCSXX_Write_NoCheck(W25QCS_BASE * base,uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 			 		 
	uint16_t pageremain;	   
	pageremain=256-WriteAddr%256; 	 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;
	while(1)
	{	   
		W25QCSXX_Write_Page(base,pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;
	 	else 
		{
			pBuffer += pageremain;
			WriteAddr += pageremain;	
			NumByteToWrite-=pageremain;			 
			if(NumByteToWrite>256)pageremain=256; 
			else pageremain=NumByteToWrite; 	 
		}
	};	    
}    
void W25QCSXX_Erase_Chip(W25QCS_BASE * base)   {                                 
    W25QCSXX_Write_Enable(base);                  
    W25QCSXX_Wait_Busy(base,1);   
  	base->W25QCS_CS_L_enable();                            
    base->W25QCS_SPI_RW(W25X_ChipErase);      
	base->W25QCS_CS_H();                                
	W25QCSXX_Wait_Busy(base,100);//
}  
uint8_t W25QCSXX_Erase_Sector(W25QCS_BASE * base,uint32_t Dst_Addr)   {  
	uint8_t buf = 0;
    W25QCSXX_Write_Enable(base);//解除擦除编程锁
    base->W25QCS_CS_H();
    //W25QCS_DELAYUS_U8(10);
    base->W25QCS_CS_L_enable();
    base->W25QCS_SPI_RW(W25X_BlockSectorUlock);
    base->W25QCS_SPI_RW((uint8_t)((Dst_Addr)>>16));  
    base->W25QCS_SPI_RW((uint8_t)((Dst_Addr)>>8));   
    base->W25QCS_SPI_RW((uint8_t)Dst_Addr);
    base->W25QCS_CS_H(); //解除擦除编程锁
    W25QCSXX_Write_Enable(base);                  
    W25QCSXX_Wait_Busy(base,1);   
  	base->W25QCS_CS_L_enable();                           
    base->W25QCS_SPI_RW(W25X_SectorErase);      
    base->W25QCS_SPI_RW((uint8_t)((Dst_Addr)>>16));  
    base->W25QCS_SPI_RW((uint8_t)((Dst_Addr)>>8));   
    base->W25QCS_SPI_RW((uint8_t)Dst_Addr);  
   	base->W25QCS_CS_H();                                
    W25QCSXX_Wait_Busy(base,2);//擦除结束

    W25QCSXX_Read(base,&buf,Dst_Addr,1);
    if(0xff == buf)return 0;
    else return 1;
}  
//擦除一块大概需要0.1s
static uint32_t W25QCSXX_Wait_Busy(W25QCS_BASE * base,uint32_t ms){   
	uint8_t Safe_Count_u8 = 0;
	while(((W25QCSXX_ReadSR123(base,1)&0x01)==0x01)&&(Safe_Count_u8++<300)){
		base->DELAY_MS(ms);
	}  
	if(Safe_Count_u8 > 300){
		return 1;
	}
	return 0;
}  
void W25QCSXX_PowerDown(W25QCS_BASE * base){
	W25QCSXX_Write_Disable(base);
  	base->W25QCS_CS_L_enable();                           
    base->W25QCS_SPI_RW(W25X_PowerDown);        
	base->W25QCS_CS_H();                              
}   
void W25QCSXX_WAKEUP(W25QCS_BASE * base){  
  	base->W25QCS_CS_L_enable();                           
    base->W25QCS_SPI_RW(W25X_ReleasePowerDown);     
	base->W25QCS_CS_H();                                
}  
/**********************************************************************************************************
*	函 数 名: uint8_t W25Q_Init
*	功能说明: 器件接口初始化
*	传    参: @W25QCS_CS_L_enable，使能芯片有效函数指针
*	传    参: @W25QCS_CS_H，使能芯片无效函数指针
*	传    参: @W25QCS_SPI_RW，spi读写函数指针
*	传    参: @size_MB 芯片存储容量，单位字节
*	返 回 值: 
*   说    明: 
*********************************************************************************************************/
void W25Q_Init(W25QCS_BASE * base,
					void (* W25QCS_CS_L_enable)(void),
					void (* W25QCS_CS_H)(void),
					// spi时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
					uint8_t (*W25QCS_SPI_RW)(uint8_t a_u8),
					uint8_t (*DELAY_MS)(uint32_t a_u8),
					uint32_t size_MB,
					 DEV_TYPE dev_type
					){
	// base->dev_sta = W25QCS_IDLE;
	base->good_trans_count_u16 = 0;
	base->bad_trans_count_u16 = 0;
	base->good_recv_count_u16 = 0;
	base->W25QCS_CS_L_enable = W25QCS_CS_L_enable;
	base->W25QCS_CS_H = W25QCS_CS_H;
	// spi时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
	base->W25QCS_SPI_RW = W25QCS_SPI_RW;
	base->W25QCS_SIZE_MB = size_MB;
	base->DELAY_MS = DELAY_MS;
} 
                    
