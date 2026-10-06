/**
 ******************************************************************************
 * @file    lcd24.c
 * @brief   2.4寸 LCD 驱动：SPI 接口，字符/汉字/图片显示
 ******************************************************************************
 */

#include "lcd24_desc.h"
#include "lcd24.h"
#include "delay.h"
#include "spi.h"
#include "spi_desc.h"
#include "front.h"
#include "front_desc.h"
#include "image.h"
#include "image_desc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lvgl/lvgl.h"
# include "lv_port_disp_template.h"
//#include "queue.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#define MAX_OBJ 64
#define IMG_SIZE 16
typedef enum{
	LV_OBJ_TYPE_BG,
	LV_OBJ_TYPE_TEXT,
	LV_OBJ_TYPE_IMAGE
}lv_obj_type_t;
//对象表(因为lvgl的对象如果重复创建就会形成重影)
typedef struct{
	lv_obj_t*obj;
	//坐标当成查找obj
	uint16_t x,y;
	//lv对象的类型
	lv_obj_type_t type;
}ui_obj_entry_t;

//对象表
static ui_obj_entry_t obj_table[MAX_OBJ]={0};
static uint8_t obj_count=0;

//lvgl里的图片对象
static lv_img_dsc_t img_dsc_cache[IMG_SIZE]={0};
static uint8_t img_dsc_count=0;

static SemaphoreHandle_t write_gram_handler=NULL;
//static SemaphoreHandle_t lcd_write_gram_mutex=NULL;
static void lcd_init_send_cmd(lcd24_handler lcd24_x);
static void lcd_write_gram_dma(lcd24_handler lcd24x,uint8_t*data,uint32_t length,
	bool issignalcolour);
// lcd24.c
uint16_t lcd_get_width(void)
{
    return 240;   // 改成你屏幕的横向分辨率，比如2.4寸屏一般240
}

uint16_t lcd_get_height(void)
{
    return 320;   // 屏幕纵向分辨率
}

//查询对象
static lv_obj_t*find_obj(uint16_t x,uint16_t y)
{
	for(uint8_t i=0;i<obj_count;++i)
	{
		if(x==obj_table[i].x&&y==obj_table[i].y)
		{
			return obj_table[i].obj;
		}
	}
	return NULL;
}

//将图片对象转换成lvgl对象
static const lv_img_dsc_t*image_to_lvgl_dsc(image_handle img)
{
	if(img==NULL)
	{
		return NULL;
	}
	//查找一下有没有这个图片对象
	for(uint8_t i=0;i<img_dsc_count;++i)
	{
		if(img_dsc_cache[i].data==img->pdata_arr)
		{
			return &img_dsc_cache[i];
		}
	}
	//如果没有就新建
	//这两个必须填0，lvgl规定的
	img_dsc_cache[img_dsc_count].header.always_zero=0;
	img_dsc_cache[img_dsc_count].header.reserved=0;
	img_dsc_cache[img_dsc_count].header.w=img->weight;
	img_dsc_cache[img_dsc_count].header.h=img->height;
	//设置颜色格式为同屏幕同色深，没有透明通道
	img_dsc_cache[img_dsc_count].header.cf=LV_IMG_CF_TRUE_COLOR;
	img_dsc_cache[img_dsc_count].data=img->pdata_arr;
	img_dsc_cache[img_dsc_count].data_size=img->height*img->weight*2;
	return &img_dsc_cache[img_dsc_count++];
}

extern const lv_font_t *lv_font_maple_get(front_handler frontx);

static const lv_font_t *front_to_lvgl_font(front_handler frontx)
{
    return lv_font_maple_get(frontx);
}


//适配lvgl的color_hex(0xRRGGBB)
static uint32_t rgb565_to_rrggbb(uint16_t color)
{
	uint8_t r=(color>>11)&0x1f;
	uint8_t g=(color>>5)&0x3f;
	uint8_t b=(color)&0x1f;
	//因为这里最大只有8位，如果只是进行移位那么低位就有可能是0，因此要将高位补到低位进行填充
	r=(r<<3)|(r>>2);
	g=(g<<2)|(g>>4);
	b=(b<<3)|(b>>2);
	return ((uint32_t)r<<16)|((uint32_t)g<<8)|b;
}

//添加对象
static void add_obj(lv_obj_t*obj,uint16_t x,uint16_t y,lv_obj_type_t type)
{
	if(obj&&obj_count<MAX_OBJ)
	{
		obj_table[obj_count].obj=obj;
		obj_table[obj_count].type=type;
		obj_table[obj_count].x=x;
		obj_table[obj_count].y=y;
		obj_count++;
	}
}


static void lcd_dma_init(lcd24_handler lcd24x)
{
	DMA_InitTypeDef DMA_InStructer;
	DMA_StructInit(&DMA_InStructer);
	DMA_InStructer.DMA_Channel=DMA_Channel_0;
	DMA_InStructer.DMA_PeripheralBaseAddr=(uint32_t)&lcd24x->spi_x->spinum->DR;
//	DMA_InStructer.DMA_PeripheralBurst=DMA_PeripheralBurst_INC8;
	DMA_InStructer.DMA_DIR=DMA_DIR_MemoryToPeripheral;
	DMA_InStructer.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;
	DMA_InStructer.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InStructer.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;
//	意思是：当 FIFO 条件满足时，DMA 会一次性从内存连续读取 8 个数据单元
//	（这里单元是 16 位半字），一次性填满 FIFO，这个操作高度优化，能有效利用总线带宽。
	DMA_InStructer.DMA_MemoryBurst=DMA_MemoryBurst_INC8;
	DMA_InStructer.DMA_Mode=DMA_Mode_Normal;
	DMA_InStructer.DMA_Priority=DMA_Priority_High;
	DMA_InStructer.DMA_FIFOMode=DMA_FIFOMode_Enable;
//	设置 FIFO 的触发阈值。Full 表示只有当 FIFO 完全填满时，才进行一次突发传输。
//例如：FIFO 深度为 4 个字（对于半字来说就是 8 个半字）
//	，那么只有当 8 个半字都装进 FIFO 后，DMA 才会向 SPI 数据寄存器连续爆发写入多次数据。
	DMA_InStructer.DMA_FIFOThreshold=DMA_FIFOThreshold_Full;
//	外设端的突发长度设为单次（1 拍）。
//因为 SPI 的数据寄存器每次只能写入一个 16 位数据，所以外设端不能连续爆发写
//，只能按 FIFO 的数据节奏一个一个地发送。外设突发单次意味着每次突发只执行一次外设写操作。
	
	DMA_InStructer.DMA_PeripheralBurst=DMA_PeripheralBurst_Single;
	SPI_I2S_DMACmd(lcd24x->spi_x->spinum,SPI_I2S_DMAReq_Tx,ENABLE);
	DMA_Init(DMA1_Stream4,&DMA_InStructer);
}
static void lcd_io_init(lcd24_handler lcd24x)
{
	GPIO_InitTypeDef GPIO_InStructer;
	GPIO_StructInit(&GPIO_InStructer);
	GPIO_InStructer.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InStructer.GPIO_OType=GPIO_OType_PP;
	GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InStructer.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_InStructer.GPIO_Pin=lcd24x->GPIO_DC_PIN;
	GPIO_Init(lcd24x->GPIO_DC_PORT,&GPIO_InStructer);
	GPIO_InStructer.GPIO_Pin=lcd24x->GPIO_RESET_PIN;
	GPIO_Init(lcd24x->GPIO_RESET_PORT,&GPIO_InStructer);
  GPIO_SetBits(lcd24x->GPIO_DC_PORT, lcd24x->GPIO_DC_PIN);
  GPIO_SetBits(lcd24x->GPIO_RESET_PORT, lcd24x->GPIO_RESET_PIN);

}
static void lcd_dma_interrupt_init()
{
	write_gram_handler=xSemaphoreCreateBinary();
	configASSERT(write_gram_handler);
//	lcd_write_gram_mutex=xSemaphoreCreateMutex();
//	configASSERT(lcd_write_gram_mutex);
//	xSemaphoreGive(lcd_write_gram_mutex);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitTypeDef NVIC_InStructer;
	memset(&NVIC_InStructer,0,sizeof(NVIC_InStructer));
	NVIC_InStructer.NVIC_IRQChannel=DMA1_Stream4_IRQn;
	NVIC_InStructer.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InStructer.NVIC_IRQChannelPreemptionPriority=8;
	NVIC_InStructer.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InStructer);
	//NVIC_SetPriority(DMA1_Stream4_IRQn, 15);
	DMA_ITConfig(DMA1_Stream4,DMA_IT_TC,ENABLE);
}
static void lcd_init_init(lcd24_handler lcd24x)
{
	lcd_io_init(lcd24x);
	m_spi_init(lcd24x->spi_x);
	lcd_dma_init(lcd24x);
	lcd_dma_interrupt_init();
	lcd_init_send_cmd(lcd24x);
}
static void lcd_reset(lcd24_handler lcd24_x)
{
	GPIO_ResetBits(lcd24_x->GPIO_RESET_PORT,lcd24_x->GPIO_RESET_PIN);
	//至少保持10ms
	//delay_ms(15);
	vTaskDelay(pdMS_TO_TICKS(15));
	GPIO_SetBits(lcd24_x->GPIO_RESET_PORT,lcd24_x->GPIO_RESET_PIN);
	//等待至少120ms
	//delay_ms(120);
	vTaskDelay(pdMS_TO_TICKS(120));
}
static void lcd_write_cmd(lcd24_handler lcd24_x,uint8_t cmd,uint8_t *data,uint32_t len)
{
	//先将spi的传输格式改成8bit
	SPI_DataSizeConfig(lcd24_x->spi_x->spinum,SPI_DataSize_8b);
	//先拉低片选线
	GPIO_ResetBits(lcd24_x->spi_x->GPIO_CS_PORT,lcd24_x->spi_x->GPIO_CS_PIN);
	//拉低dc线表示发送命令
	GPIO_ResetBits(lcd24_x->GPIO_DC_PORT,lcd24_x->GPIO_DC_PIN);
	spi_send_data(lcd24_x->spi_x,&cmd);
	//如果这里不等的话会发生数据还没有发送完成就dc被拉成数据模式
	while(SPI_I2S_GetFlagStatus(lcd24_x->spi_x->spinum,SPI_I2S_FLAG_BSY));
	GPIO_SetBits(lcd24_x->GPIO_DC_PORT,lcd24_x->GPIO_DC_PIN);
	if(data&&len)
	{
		//命令后面带了参数
		for(uint32_t i=0;i<len;++i)
		{
			spi_send_data(lcd24_x->spi_x,&data[i]);
		}
	}
	//最后这里也要等待一下busy,因为如果不等就会发生数据还没发完片选线就被拉高了
	while(SPI_I2S_GetFlagStatus(lcd24_x->spi_x->spinum,SPI_I2S_FLAG_BSY)==SET);
	GPIO_SetBits(lcd24_x->spi_x->GPIO_CS_PORT,lcd24_x->spi_x->GPIO_CS_PIN);
}
static void lcd_set_range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,lcd24_handler lcd24_x)
{
	lcd_write_cmd(lcd24_x,0x2A,(uint8_t[]){((x1>>8)&0xff),x1&0xff,(x2>>8)&0xff,x2&0xff},4);
	lcd_write_cmd(lcd24_x,0x2B,(uint8_t[]){((y1>>8)&0xff),y1&0xff,(y2>>8)&0xff,y2&0xff},4);
}
static bool lcd_is_range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	return (x1<=x2&&y1<=y2&&x1<LCD24_WIDTH&&y1<LCD24_HEIGHT&&x2<LCD24_WIDTH&&y2<LCD24_HEIGHT);
}
static void lcd_set_gram_mode(lcd24_handler lcd24_x)
{
	lcd_write_cmd(lcd24_x,0x2C,NULL,0);
}
void lcd_fill_colour(lcd24_handler lcd24_x,uint16_t x1\
	,uint16_t x2,uint16_t y1,uint16_t y2,uint16_t colour )
{
//	xSemaphoreTake(lcd_write_gram_mutex,portMAX_DELAY);
	if(!lcd_is_range(x1,y1,x2,y2)||!lcd24_x)
	{
//		xSemaphoreGive(lcd_write_gram_mutex);
		return ;
	}
	//如果是进行刷屏
	if(x1==0&&y1==0&&x2>=239&&y2>=319)
	{
		lv_obj_clean(lv_scr_act());
		memset(obj_table,0,sizeof(obj_table));
		obj_count=0;
		lv_obj_set_style_bg_color(lv_scr_act(),lv_color_hex(rgb565_to_rrggbb(colour)),LV_PART_MAIN);
		lv_obj_set_style_bg_opa(lv_scr_act(),LV_OPA_COVER,LV_PART_MAIN);
		return;
	}
	//普通着色
	lv_obj_t*ret=find_obj(x1,y1);
	if(ret)
	{
		lv_obj_set_style_bg_color(ret,lv_color_hex(rgb565_to_rrggbb(colour)),LV_PART_MAIN);
		//return;
	}
	else {
	ret=lv_obj_create(lv_scr_act());
	lv_obj_set_style_bg_color(ret,
        lv_color_hex(rgb565_to_rrggbb(colour)), 0 );
	lv_obj_set_style_bg_opa(ret,LV_OPA_COVER,0);
  lv_obj_set_style_border_width(ret, 0 , 0 );
  lv_obj_set_style_radius(ret, 0 , 0 );
  lv_obj_set_pos(ret, x1, y1);
  lv_obj_set_size(ret, x2 - x1 + 1 , y2 - y1 + 1 );
  add_obj(ret, x1, y1, LV_OBJ_TYPE_BG);
	}
	
//	lcd_set_range(x1,y1,x2,y2,lcd24_x);
//	lcd_set_gram_mode(lcd24_x);
//	uint8_t data[]={(colour>>8)&0xff,colour&0xff};
//	uint32_t nums=(x2-x1+1)*(y2-y1+1);
//	GPIO_ResetBits(lcd24_x->spi_x->GPIO_CS_PORT,lcd24_x->spi_x->GPIO_CS_PIN);
//	GPIO_SetBits(lcd24_x->GPIO_DC_PORT,lcd24_x->GPIO_DC_PIN);
//	for(uint32_t i=0;i<nums;++i)
//	{
//		spi_send_data(lcd24_x->spi_x,&data[0]);
//		spi_send_data(lcd24_x->spi_x,&data[1]);
//	}
//	while(SPI_I2S_GetFlagStatus(lcd24_x->spi_x->spinum,SPI_I2S_FLAG_BSY)==SET);
//	GPIO_SetBits(lcd24_x->spi_x->GPIO_CS_PORT,lcd24_x->spi_x->GPIO_CS_PIN);
//		lcd_write_gram_dma(lcd24_x,(uint8_t*)&colour,nums*2,true);
//	xSemaphoreGive(lcd_write_gram_mutex);
}

static void lcd_init_send_cmd(lcd24_handler lcd24_x)
{
	lcd_reset(lcd24_x);
	lcd_write_cmd(lcd24_x,0x11,NULL,0);
	//delay_ms(5);
	vTaskDelay(pdMS_TO_TICKS(5));
	lcd_write_cmd(lcd24_x,0x36, (uint8_t[]){0x00}, 1);
	lcd_write_cmd(lcd24_x,0x3A, (uint8_t[]){0x55}, 1);
	lcd_write_cmd(lcd24_x,0xB7, (uint8_t[]){0x05}, 1);
	lcd_write_cmd(lcd24_x,0xBB, (uint8_t[]){0x3F}, 1);
	lcd_write_cmd(lcd24_x,0xC0, (uint8_t[]){0x2C}, 1);
	lcd_write_cmd(lcd24_x,0xC5,(uint8_t []){0x1A},1);
	lcd_write_cmd(lcd24_x,0xB2,(uint8_t []){0x05, 0x05, 0x00, 0x33, 0x33},5);
	lcd_write_cmd(lcd24_x,0xC3,(uint8_t []){0x0F},1);
	lcd_write_cmd(lcd24_x,0xE8,(uint8_t []){0x03},1);
	lcd_write_cmd(lcd24_x,0xE9,(uint8_t []){0x09, 0x09, 0x08},3);
	lcd_write_cmd(lcd24_x,0xC2, (uint8_t[]){0x01}, 1);
	lcd_write_cmd(lcd24_x,0xC4, (uint8_t[]){0x20}, 1);
	lcd_write_cmd(lcd24_x,0xC6, (uint8_t[]){0x01}, 1);
	lcd_write_cmd(lcd24_x,0xD0, (uint8_t[]){0xA4,0xA1}, 2);
	lcd_write_cmd(lcd24_x,0xD6, (uint8_t[]){0xA1}, 1);
	lcd_write_cmd(lcd24_x,0xE0,(uint8_t []){0xD0, 0x05, 0x09, 0x09,\
	0x08, 0x14, 0x28, 0x33, 0x3F, 0x07, 0x13, 0x14, 0x28, 0x30},14);
	lcd_write_cmd(lcd24_x,0xE1, (uint8_t[]){0xD0, 0x05, 0x09, 0x09,\
	0x08, 0x03, 0x24, 0x32, 0x32, 0x3B, 0x14, 0x13, 0x28, 0x2F}, 14);
	lcd_write_cmd(lcd24_x,0x20, NULL,0);
	lcd_write_cmd(lcd24_x,0x29,NULL,0);
}

void lcd_init(lcd24_handler lcd24_x)
{
//	m_spi_init(lcd24_x->spi_x);
	lcd_init_init(lcd24_x);
	lv_init();
	extern void lv_font_maple_init(void);
	lv_font_maple_init();
	lv_port_disp_init();
	//默认显示
	lcd_fill_colour(lcd24_x,0,LCD24_WIDTH-1,0,LCD24_HEIGHT-1,0x0000);
}

static void lcd_write_gram_dma(lcd24_handler lcd24x,uint8_t*data,uint32_t length,
	bool issignalcolour)
{
		//xSemaphoreTake(lcd_write_gram_mutex,portMAX_DELAY);
		//将spi的传输转换成16bit的格式
		SPI_DataSizeConfig(lcd24x->spi_x->spinum,SPI_DataSize_16b);
	//因为这个是uint8_t的数组，传输的时候是按照16bit为一个单元传输，因此要/2
		length>>=1;
		GPIO_ResetBits(lcd24x->spi_x->GPIO_CS_PORT,lcd24x->spi_x->GPIO_CS_PIN);
		GPIO_SetBits(lcd24x->GPIO_DC_PORT,lcd24x->GPIO_DC_PIN);
		do{
			uint16_t singal_size=length>65535?65535:length;
			if(issignalcolour)
			{
				//单色地址不自增
					DMA1_Stream4->CR&=~DMA_SxCR_MINC;
			}
			else 
			{
				DMA1_Stream4->CR|=DMA_SxCR_MINC;
			}
			DMA1_Stream4->M0AR = (uint32_t)data;
      DMA1_Stream4->NDTR =singal_size;
			
//			GPIO_ResetBits(lcd24x->spi_x->GPIO_CS_PORT,lcd24x->spi_x->GPIO_CS_PIN);
//			GPIO_SetBits(lcd24x->GPIO_DC_PORT,lcd24x->GPIO_DC_PIN);
			DMA_Cmd(DMA1_Stream4,ENABLE);
			//这里标志位的后面的数字是对应stream的编号
//			while(DMA_GetFlagStatus(DMA1_Stream4,DMA_FLAG_TCIF4)==RESET);
//			DMA_ClearFlag(DMA1_Stream4,DMA_FLAG_TCIF4);
			xSemaphoreTake(write_gram_handler,portMAX_DELAY);
			if(!issignalcolour)
			{
				data+=singal_size*2;
			}
			length-=singal_size;
		}while(length>0);
		while(SPI_GetFlagStatus(lcd24x->spi_x->spinum,SPI_FLAG_BSY)==SET);
		GPIO_SetBits(lcd24x->spi_x->GPIO_CS_PORT,lcd24x->spi_x->GPIO_CS_PIN);
		//xSemaphoreGive(lcd_write_gram_mutex);
}

static void lcd_show_char_mode(lcd24_handler lcd24x,uint16_t colour_acs\
	,uint16_t colour_bg,const uint8_t *ps,uint16_t height,uint16_t weight)
{
	uint16_t real_row=(weight+7)/8;
			/*
			由于是逐行来存储数据的，因此每一行(宽度)的数据位数如果不是8的整数倍，那就会补齐到8的整数倍，而
			行数(高度)则不存在说补齐，因为每一行存的已经是8的整数倍了
			*/
			/*
				因为dma的传输是以16bit为单位的，所以涉及到内存对齐的问题，因此要用强制对齐的中转数组转换
			*/
		static uint8_t  __attribute__((aligned(2))) data[76*76*2]={0};
		memset(data,0,sizeof(data));
		uint8_t *pbuf_data=data;
		for(uint32_t i=0;i<height;++i)
		{
			for(uint32_t j=0;j<weight;++j)
			{
				uint8_t isLight=(ps[i*real_row+(j/8)]>>(7-(j%8)))&0x01;
				uint16_t colour=isLight?colour_acs:colour_bg;
				*pbuf_data++=colour&0xff;
				*pbuf_data++=(colour>>8)&0xff;
			}
		}
		lcd_write_gram_dma(lcd24x,data,pbuf_data-data,false);
}

static void lcd_show_singal_acs(lcd24_handler lcd24x,uint16_t x,uint16_t y\
	,uint16_t colour_acs,uint16_t colour_bg,char ch\
	,front_handler frontx	)
{
	if(frontx==NULL||lcd_is_range(x,y,x+frontx->weight/2-1,y+frontx->height-1)==false\
		||ch>0x7E||ch<0x20)
	{
		return;
	}
	
	lcd_set_range(x,y,x+frontx->weight/2-1,y+frontx->height-1,lcd24x);
	lcd_set_gram_mode(lcd24x);
	//acs字符一般是半宽
	uint16_t row=(frontx->height)*(((frontx->weight/2)+7)/8);
	const uint8_t*ps=NULL;

	if(frontx->map)
	{
		const uint8_t* map=frontx->map;
		do{
			if(*map==ch)
			{
				ps=(const uint8_t*)frontx->data_arr+(map-frontx->map)*row;
				break;
			}
		}while(*(++map)!=0);
		
	}
	else 	if(frontx->data_arr){
	//找到字符开头所在的位置
	uint32_t index=(ch-' ')*row;
	ps=frontx->data_arr+index;
	}
		lcd_show_char_mode(lcd24x,colour_acs,colour_bg,ps,frontx->height,frontx->weight/2);
}

static bool isgb2312(char ch)
{
	return((unsigned char)ch>=0xA1&&(unsigned char)ch<=0xF7);
}

static void lcd_show_singal_chinese(lcd24_handler lcd24x,uint16_t x,uint16_t y\
	,uint16_t colour_acs,uint16_t colour_bg,char*word\
		,front_handler frontx	)
{
		if(frontx==NULL||word==NULL||frontx->chinese==NULL\
			||lcd_is_range(x,y,x+frontx->weight-1,y+frontx->height-1)==false)
			{
				return;
			}
			lcd_set_range(x,y,x+frontx->weight-1,y+frontx->height-1,lcd24x);
			lcd_set_gram_mode(lcd24x);
			for(uint32_t i=0;i<frontx->len;++i)
			{
				if(!strcmp(frontx->chinese[i].str,word))
				{
					lcd_show_char_mode(lcd24x,colour_acs,colour_bg,\
					frontx->chinese[i].data_mode,frontx->height,frontx->weight);
					break;
				}
			}
}

static int utf8_char_len(uint8_t ch)
{
    if (ch < 0x80) return 1;                 // ASCII
    if ((ch & 0xE0) == 0xC0) return 2;       // 2字节（很少用于中文）
    if ((ch & 0xF0) == 0xE0) return 3;       // 中文常用范围 0xE4-0xE9
    if ((ch & 0xF8) == 0xF0) return 4;       // 4字节（如emoji）
    return 0;                                // 无效
}

void lcd_show_str(lcd24_handler lcd24x,uint16_t x,uint16_t y\
	,front_handler frontx,char*str,uint32_t length,uint16_t colour_acs,uint16_t colour_bg)
{
		if(length==0||str==NULL||lcd24x==NULL||frontx==NULL)
		{
			return;
		}
		
		lv_obj_t *label = find_obj(x, y); if (label) { // 已存在，改文字和颜色
			lv_label_set_text(label, str);
        lv_obj_set_style_text_color(label,
            lv_color_hex(rgb565_to_rrggbb(colour_acs)), 0 );
				//设置文字不透明就是防止文字和背景混在一起
        lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0 );
        lv_obj_set_style_bg_color(label,
            lv_color_hex(rgb565_to_rrggbb(colour_bg)), 0 );
				//设置背景不透明就是用于盖住旧的像素
				lv_obj_set_style_bg_opa(label, LV_OPA_COVER, 0 );
    } else { // 不存在，创建新label
			 label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, str);
        lv_obj_set_style_text_color(label,
            lv_color_hex(rgb565_to_rrggbb(colour_acs)), 0 );
        lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0 );
        lv_obj_set_style_bg_color(label,
            lv_color_hex(rgb565_to_rrggbb(colour_bg)), 0 );
        lv_obj_set_style_bg_opa(label, LV_OPA_COVER, 0 );
        lv_obj_set_style_text_font(label,
            front_to_lvgl_font(frontx), 0 );
        lv_obj_set_pos(label, x, y);
        add_obj(label, x, y, 1 );
    }
		
//	xSemaphoreTake(lcd_write_gram_mutex,portMAX_DELAY);
//	if(length>0&&str)
//	{
//		while(length>0)
//		{
//			uint8_t len=utf8_char_len(*str);
//			if(len==1)
//			{
//				lcd_show_singal_acs(lcd24x,x,y,colour_acs,colour_bg,*str,frontx);
//				x+=frontx->weight/2;
//				--length;
//				str++;
//			}
//			else if(len==2||len==3){
//				char chinese_arr[5]={0};
//				strncpy(chinese_arr,str,len);
//				lcd_show_singal_chinese(lcd24x,x,y,colour_acs,colour_bg,chinese_arr,frontx);
//				x+=frontx->weight;
//				length-=strlen(chinese_arr);
//				str+=strlen(chinese_arr);
//			}
//			else
//			{
//				str++;
//			}
//		}
//	}
//	xSemaphoreGive(lcd_write_gram_mutex);
}

void lcd_show_image(lcd24_handler lcd24x,uint16_t x,uint16_t y,image_handle imagex)
{
//	xSemaphoreTake(lcd_write_gram_mutex,portMAX_DELAY);
		if(imagex==NULL||lcd24x==NULL||\
		lcd_is_range(x,y,imagex->weight+x-1,imagex->height+y-1)==false)
		{
//			xSemaphoreGive(lcd_write_gram_mutex);
			return;
		}
		lv_obj_t *img = find_obj(x, y); 
		if (img) { // 已存在，换图片源 
			lv_img_set_src(img, image_to_lvgl_dsc(imagex));
    } else { // 不存在，创建新图片对象 
			img = lv_img_create(lv_scr_act());
        lv_img_set_src(img, image_to_lvgl_dsc(imagex));
        lv_obj_set_pos(img, x, y);
        add_obj(img, x, y, 2 );
		}
		
//		uint32_t nums=imagex->height*imagex->weight*2;
//		lcd_set_range(x,y,x+imagex->weight-1,y+imagex->height-1,lcd24x);
//		lcd_set_gram_mode(lcd24x);
//		GPIO_ResetBits(lcd24x->spi_x->GPIO_CS_PORT,lcd24x->spi_x->GPIO_CS_PIN);
//		GPIO_SetBits(lcd24x->GPIO_DC_PORT,lcd24x->GPIO_DC_PIN);
//		const uint8_t*pindex=imagex->pdata_arr;
//		for(uint32_t i=0;i<nums;i+=2)
//		{
//			//因为一个像素点是用2个字节表示的
//			//,所以放到uint8_t的数组里就会被切成两半,并且是小端存储,因此要倒过来
//			spi_send_data(lcd24x->spi_x,&(pindex[i+1]));
//			spi_send_data(lcd24x->spi_x,&pindex[i]);
//		}
//		while(SPI_I2S_GetFlagStatus(lcd24x->spi_x->spinum,SPI_I2S_FLAG_BSY));
//		GPIO_SetBits(lcd24x->spi_x->GPIO_CS_PORT,lcd24x->spi_x->GPIO_CS_PIN);
			/*
					因为dma的传输是以16bit为单位的，所以涉及到内存对齐的问题，因此要用强制对齐的中转数组转换
			*/
//			static uint8_t  __attribute__((aligned(2))) temp[240*180*2]={0};
//			memset(temp,0,sizeof(temp));
//			uint8_t *pbuf=temp;
//			for(uint32_t i=0;i<nums;i+=2)
//			{
//					*pbuf++=pindex[i];
//					*pbuf++=pindex[i+1];
//			}
//			lcd_write_gram_dma(lcd24x,(uint8_t *)temp,pbuf-temp,false);
//				lcd_write_gram_dma(lcd24x,(uint8_t *)pindex,nums,false);
//			xSemaphoreGive(lcd_write_gram_mutex);
}


void DMA1_Stream4_IRQHandler(void)
{
		if(DMA_GetITStatus(DMA1_Stream4,DMA_IT_TCIF4)==SET)
		{
			BaseType_t pxHigherPriorityTaskWoken;
			xSemaphoreGiveFromISR(write_gram_handler,&pxHigherPriorityTaskWoken);
//			if(pxHigherPriorityTaskWoken==pdTRUE)
//			{
				//如果pxHigherPriorityTaskWoken是pdTrue说明唤醒了比当前优先级更高的任务
			//而portYIELD_FROM_ISR就是当入参为pdTrue时才执行任务切换
				portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
//			}
			DMA_ClearITPendingBit(DMA1_Stream4,DMA_IT_TCIF4);
		}
}


// 区域刷新，LVGL把一整块绘制好的buffer传给LCD驱动，刷到屏幕指定区域
void lcd_show_buffer(lcd24_handler lcd24_x, uint16_t x1, uint16_t x2\
	, uint16_t y1, uint16_t y2, const uint16_t *buf)
{
    // 在这里调用你LCD的填充函数，把buf的数据刷到屏幕[x1,y1] ~ [x2,y2]
    // 示例：LCD_ShowArea(x1,y1,x2,y2,buf);
		if(!lcd_is_range(x1,y1,x2,y2)||!buf||!lcd24_x)
		{
			return;
		}
		lcd_set_range(x1,y1,x2,y2,lcd24_x);
		lcd_set_gram_mode(lcd24_x);
		uint32_t nums=(x2-x1+1)*(y2-y1+1);
		lcd_write_gram_dma(lcd24_x,(uint8_t*)buf,
		nums*sizeof(uint16_t)/sizeof(uint8_t),false);
}
