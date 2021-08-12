 /*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-10     years       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include <onenet.h>


#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        10
#define SAMPLE_UART_NAME       "uart1"    /* 串口设备名称 */
//#define REFER_VOLTAGE       330         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
//#define CONVERT_BITS        (1 << 16)   /* 转换位数为16位 */

static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);


static rt_device_t serial;    /* 串口设备句柄 */


static rt_thread_t tid1 = RT_NULL;
static rt_thread_t oneNetUp = RT_NULL;

/* 邮箱控制块 */
static struct rt_mailbox mb;
/* 用于放邮件的内存池 */
static char mb_pool[128];

static rt_timer_t timer1;


ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;


#define ADC_CONVERTED_DATA_BUFFER_SIZE   ((uint32_t)  240)   /* Size of array aADCxConvertedData[], Aligned on cache line size */
ALIGN_32BYTES (static uint16_t aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE]) __attribute__((section(".ARM.__at_0x24000000")));
rt_uint32_t Adc_aver[3] = {0};


static void getValue_entry(void *parameter)
{
//    unsigned int tmp, tick = 0;
//    rt_uint32_t  vol;

//    uint16_t adc[11] = {1,2,3,4,5,6,7,8,9,19,255};

    MX_DMA_Init();
    MX_ADC1_Init();

    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED, ADC_SINGLE_ENDED) != HAL_OK)
   {
     Error_Handler();
   }

    if (HAL_ADC_Start_DMA(&hadc1,(uint32_t *)aADCxConvertedData,ADC_CONVERTED_DATA_BUFFER_SIZE) != HAL_OK)
    {
           Error_Handler();
     }
    rt_timer_start(timer1);

}

static void timeout1(void *parameter)
{


    rt_kprintf("time is ok  \r\n");
    rt_device_write(serial,0,aADCxConvertedData,sizeof(aADCxConvertedData));

    for(int8_t i = 0;i < ADC_CONVERTED_DATA_BUFFER_SIZE; )
    {
        Adc_aver[0] += aADCxConvertedData[i++];
        Adc_aver[1] += aADCxConvertedData[i++];
        Adc_aver[2] += aADCxConvertedData[i++];
    }

    rt_mb_send(&mb, (rt_uint32_t)&Adc_aver);


}

static void oneNetUp_entry(void *parameter)
{
    rt_uint32_t *p;
    while(1)
    {
        if (rt_mb_recv(&mb, (rt_uint32_t *)&p, RT_WAITING_FOREVER) == RT_EOK)
        {
            onenet_mqtt_upload_digit("adc_x",*p/80);
            onenet_mqtt_upload_digit("adc_y",*(p+1)/80);
            onenet_mqtt_upload_digit("adc_z",*(p+2)/80);

            for( int i = 0 ; i < 3;i++)
            {
                Adc_aver[i] = 0;
            }
        }

        rt_thread_delay(100);
    }

}




int adc_getValue(void)
{

    rt_err_t result;


    serial = rt_device_find(SAMPLE_UART_NAME);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", SAMPLE_UART_NAME);
        return RT_ERROR;
    }




    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_DMA_TX);


    timer1 = rt_timer_create("timer1", timeout1,
                             RT_NULL, 1500,
                             RT_TIMER_FLAG_PERIODIC);



       /* 初始化一个 mailbox */
       result = rt_mb_init(&mb,
                           "mbt",                      /* 名称是 mbt */
                           &mb_pool[0],                /* 邮箱用到的内存池是 mb_pool */
                           sizeof(mb_pool) / 4,        /* 邮箱中的邮件数目，因为一封邮件占 4 字节 */
                           RT_IPC_FLAG_FIFO);          /* 采用 FIFO 方式进行线程等待 */
       if (result != RT_EOK)
       {
           rt_kprintf("init mailbox failed.\n");
           return -1;
       }

    /* 创建线程 1，名称是 thread1，入口是 thread1_entry*/
    tid1 = rt_thread_create("getValue",
                            getValue_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    /* 如果获得线程控制块，启动这个线程 */
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);

    }
    oneNetUp = rt_thread_create("oneNetUp",
                            oneNetUp_entry, RT_NULL,
                            2048,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    if (oneNetUp != RT_NULL)
    {
        rt_thread_startup(oneNetUp);
    }


    return 0;
}


MSH_CMD_EXPORT(adc_getValue, adc_getValue );












void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
   /* Invalidate Data Cache to get the updated content of the SRAM on the second half of the ADC converted data buffer: 32 bytes */
  SCB_InvalidateDCache_by_Addr((uint32_t *) &aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE/2], ADC_CONVERTED_DATA_BUFFER_SIZE);

}


static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);


}



static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void DMA1_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream0_IRQn 0 */

  /* USER CODE END DMA1_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Stream0_IRQn 1 */

  /* USER CODE END DMA1_Stream0_IRQn 1 */
}

/**
  * @brief This function handles ADC1 and ADC2 global interrupts.
  */
void ADC_IRQHandler(void)
{
  /* USER CODE BEGIN ADC_IRQn 0 */

  /* USER CODE END ADC_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc1);
  /* USER CODE BEGIN ADC_IRQn 1 */

  /* USER CODE END ADC_IRQn 1 */
}




