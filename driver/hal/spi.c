#include "spi.h"
#include "spi_desc.h"
static void spi_af_config(spi_handler spix)
{
		if(spix->spinum==SPI1)
		{
			GPIO_PinAFConfig(spix->GPIO_MISO_PORT,spix->GPIO_MISO_PIN_source,GPIO_AF_SPI1);
			GPIO_PinAFConfig(spix->GPIO_MOSI_PORT,spix->GPIO_MOSI_PIN_source,GPIO_AF_SPI1);
			GPIO_PinAFConfig(spix->GPIO_SCK_PORT,spix->GPIO_SCK_PIN_source,GPIO_AF_SPI1);
		}
		else 	if(spix->spinum==SPI2)
		{
			GPIO_PinAFConfig(spix->GPIO_MISO_PORT,spix->GPIO_MISO_PIN_source,GPIO_AF_SPI2);
			GPIO_PinAFConfig(spix->GPIO_MOSI_PORT,spix->GPIO_MOSI_PIN_source,GPIO_AF_SPI2);
			GPIO_PinAFConfig(spix->GPIO_SCK_PORT,spix->GPIO_SCK_PIN_source,GPIO_AF_SPI2);
		}
		else	if(spix->spinum==SPI3)
		{
			GPIO_PinAFConfig(spix->GPIO_MISO_PORT,spix->GPIO_MISO_PIN_source,GPIO_AF_SPI3);
			GPIO_PinAFConfig(spix->GPIO_MOSI_PORT,spix->GPIO_MOSI_PIN_source,GPIO_AF_SPI3);
			GPIO_PinAFConfig(spix->GPIO_SCK_PORT,spix->GPIO_SCK_PIN_source,GPIO_AF_SPI3);
		}
		else if(spix->spinum==SPI4)
		{
			GPIO_PinAFConfig(spix->GPIO_MISO_PORT,spix->GPIO_MISO_PIN_source,GPIO_AF_SPI4);
			GPIO_PinAFConfig(spix->GPIO_MOSI_PORT,spix->GPIO_MOSI_PIN_source,GPIO_AF_SPI4);
			GPIO_PinAFConfig(spix->GPIO_SCK_PORT,spix->GPIO_SCK_PIN_source,GPIO_AF_SPI4);
		}
		else if(spix->spinum==SPI5)
		{
			GPIO_PinAFConfig(spix->GPIO_MISO_PORT,spix->GPIO_MISO_PIN_source,GPIO_AF_SPI5);
			GPIO_PinAFConfig(spix->GPIO_MOSI_PORT,spix->GPIO_MOSI_PIN_source,GPIO_AF_SPI5);
			GPIO_PinAFConfig(spix->GPIO_SCK_PORT,spix->GPIO_SCK_PIN_source,GPIO_AF_SPI5);
		}
		else if(spix->spinum==SPI6)
		{
			GPIO_PinAFConfig(spix->GPIO_MISO_PORT,spix->GPIO_MISO_PIN_source,GPIO_AF_SPI6);
			GPIO_PinAFConfig(spix->GPIO_MOSI_PORT,spix->GPIO_MOSI_PIN_source,GPIO_AF_SPI6);
			GPIO_PinAFConfig(spix->GPIO_SCK_PORT,spix->GPIO_SCK_PIN_source,GPIO_AF_SPI6);
		}
}

void  m_spi_init(spi_handler spix)
{
			if(spix)
			{
			GPIO_InitTypeDef GPIO_InStructer;
			GPIO_StructInit(&GPIO_InStructer);
			GPIO_InStructer.GPIO_Mode=GPIO_Mode_AF;
			GPIO_InStructer.GPIO_OType=GPIO_OType_PP;
			GPIO_InStructer.GPIO_Pin=spix->GPIO_MOSI_PIN;
			GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_NOPULL;
			GPIO_InStructer.GPIO_Speed=GPIO_High_Speed;
			GPIO_Init(spix->GPIO_MOSI_PORT,&GPIO_InStructer);
			GPIO_InStructer.GPIO_Pin=spix->GPIO_MISO_PIN;
			GPIO_Init(spix->GPIO_MISO_PORT,&GPIO_InStructer);
			GPIO_InStructer.GPIO_Pin=spix->GPIO_SCK_PIN;
			GPIO_Init(spix->GPIO_SCK_PORT,&GPIO_InStructer);
			GPIO_InStructer.GPIO_Mode=GPIO_Mode_OUT;
			GPIO_InStructer.GPIO_OType=GPIO_OType_PP;
			GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_NOPULL;
			GPIO_InStructer.GPIO_Pin=spix->GPIO_CS_PIN;
			GPIO_Init(spix->GPIO_CS_PORT,&GPIO_InStructer);
			GPIO_SetBits(spix->GPIO_CS_PORT,spix->GPIO_CS_PIN);
			spi_af_config(spix);
			
			SPI_InitTypeDef SPI_InStructer;
			SPI_StructInit(&SPI_InStructer);
				//因为一个周期的最小150ns,spix是挂载在apbx总线上的,因此要分频小于最大频率
				if(spix->spinum==SPI2||spix->spinum==SPI3)
				{
					//apb1总线，42Mhz
				SPI_InStructer.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_8;
				}
				else 
				{
					//apb2总线，84Mhz
					SPI_InStructer.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_16;
				}
				SPI_InStructer.SPI_CPHA=SPI_CPHA_1Edge;
				SPI_InStructer.SPI_CPOL=SPI_CPOL_Low;
				SPI_InStructer.SPI_DataSize=SPI_DataSize_8b;
				SPI_InStructer.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
				SPI_InStructer.SPI_FirstBit=SPI_FirstBit_MSB;
				SPI_InStructer.SPI_Mode=SPI_Mode_Master;
				SPI_InStructer.SPI_NSS=SPI_NSS_Soft;
				SPI_Init(spix->spinum,&SPI_InStructer);
				SPI_Cmd(spix->spinum,ENABLE);
			}
}


void spi_send_data(spi_handler spix,const uint8_t *data)
{
	if(spix)
	{
		SPI_I2S_SendData(spix->spinum,*data);
		while(SPI_I2S_GetFlagStatus(spix->spinum,SPI_I2S_FLAG_TXE)==RESET);
	}
}
