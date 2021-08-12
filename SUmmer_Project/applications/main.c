/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-02     RT-Thread    first version
 */
//#include "netdev_ipaddr.h"
//#include "netdev.h"
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include <onenet.h>

#define LED_PIN GET_PIN(I, 8)
//struct netdev *netdev= RT_NULL;

extern int adc_getValue(void);
extern void wlan_autoconnect_init(void);
extern rt_sem_t dynamic_sem;


int main(void)
{
    rt_uint32_t count = 1;
    rt_err_t result;
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);


    /* init Wi-Fi auto connect feature */
    wlan_autoconnect_init();
    /* enable auto reconnect on WLAN device */
    rt_wlan_config_autoreconnect(RT_TRUE);

//
//    do
//    {
//        netdev = netdev_get_first_by_flags(NETDEV_FLAG_LINK_UP);
//        rt_thread_mdelay(50);
//    }while(netdev == RT_NULL);

    rt_kprintf("start init onenet mqtt \n");
   if(onenet_mqtt_init()<0)
   {
       rt_kprintf("onenet_mqtt_init failed\n");

   }
   else
   {
       rt_kprintf("onenet_mqtt_init finished\n");
       result = rt_sem_take (dynamic_sem, RT_WAITING_FOREVER);
       if (result == RT_EOK)
       {
           rt_sem_delete(dynamic_sem);
           adc_getValue();
       }
   }



    while(count++)
    {

        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
    }
    return RT_EOK;
}

#include "stm32h7xx.h"
static int vtor_config(void)
{
    /* Vector Table Relocation in Internal QSPI_FLASH */
    SCB->VTOR = QSPI_BASE;
    return 0;
}
INIT_BOARD_EXPORT(vtor_config);


