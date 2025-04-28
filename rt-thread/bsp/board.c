/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */

#include <stdint.h>
#include <rtthread.h>
// #include "hc32_ddl.h"
#include "systick.h"
#include "usart.h"
#include "debug.h"

static void sysTick_Init(void);
static void SysClkConfig(void);
char rt_hw_console_getchar(void);
void rt_hw_console_output(const char *str);

static void SysClkConfig(void)
{
    stc_clk_xtal_cfg_t stcXtalCfg;
    stc_clk_mpll_cfg_t stcMpllCfg;
    en_clk_sys_source_t enSysClkSrc;
    stc_clk_sysclk_cfg_t stcSysClkCfg;

    MEM_ZERO_STRUCT(enSysClkSrc);
    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);

    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv  = ClkSysclkDiv1;
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;
    CLK_SysClkConfig(&stcSysClkCfg);

    /* Switch system clock source to MPLL. */
    /* Use Xtal as MPLL source. */
    stcXtalCfg.enMode        = ClkXtalModeOsc;
    stcXtalCfg.enDrv         = ClkXtalLowDrv;
    stcXtalCfg.enFastStartup = Enable;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* MPLL config. */
    stcMpllCfg.pllmDiv = 1u;  /* XTAL 8M / 1 */
    stcMpllCfg.plln    = 50u; /* 8M*50 = 400M */
    stcMpllCfg.PllpDiv = 2u;  /* MLLP = 100M */
    stcMpllCfg.PllqDiv = 2u;  /* MLLQ = 100M */
    stcMpllCfg.PllrDiv = 2u;  /* MLLR = 100M */
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_MpllConfig(&stcMpllCfg);

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_4);
    EFM_Lock();

    /* Enable MPLL. */
    CLK_MpllCmd(Enable);

    /* Wait MPLL ready. */
    while (Set != CLK_GetFlagStatus(ClkFlagMPLLRdy)) {
    }

    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);
}

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE 4096
static uint32_t rt_heap[RT_HEAP_SIZE]; // heap default size: 4K(1024 * 4)
RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

void core_init()
{
    SysClkConfig();
    sysTick_Init();
}
/**
 * This function will initial your board.
 */
void rt_hw_board_init()
{
    /* Call components board initial (use INIT_BOARD_EXPORT()) */
    core_init();
    DebugUsartInit();
    LOG_INFO("MCU_TYPE: HC32F460 \nSystemCoreClock : %d Mhz", SystemCoreClock / 1000000);
    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}

void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();
    ticks_ms++;

    /* leave interrupt */
    rt_interrupt_leave();
}

static void sysTick_Init(void)
{
    stc_clk_freq_t stcClkFreq;

    /* configure structure initialization */

    MEM_ZERO_STRUCT(stcClkFreq);

    /* Config 1 sec trigger interrupt*/
    CLK_GetClockFreq(&stcClkFreq);
    SysTick_Config(stcClkFreq.sysclkFreq / RT_TICK_PER_SECOND);
}

void rt_hw_console_output(const char *str)
{
    rt_size_t i = 0, size = 0;
    char a = '\r';

    size = rt_strlen(str);
    for (i = 0; i < size; i++) {
        if (*(str + i) == '\n') {
            DEBUG_USART_CH->DR = a;
            while (0ul == DEBUG_USART_CH->SR_f.TC) {
                ;
            }
        }
        DEBUG_USART_CH->DR = *(str + i);
        while (0ul == DEBUG_USART_CH->SR_f.TC) {
            ;
        }
    }
}

char rt_hw_console_getchar(void)
{
    int ch = -1;

    if (Set == USART_GetStatus(DEBUG_USART_CH, UsartRxNoEmpty)) {
        ch = DEBUG_USART_CH->DR_f.RDR;
    } else {
        if (Set == USART_GetStatus(DEBUG_USART_CH, UsartOverrunErr)) {
            USART_ClearStatus(DEBUG_USART_CH, UsartOverrunErr);
        }
        rt_thread_mdelay(10);
    }
    return ch;
}
