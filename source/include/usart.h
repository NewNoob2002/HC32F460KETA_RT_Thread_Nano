#ifndef __USART_H__
#define __USART_H__
#include "hc32_ddl.h"

#define DEBUG_USART_CH   (M4_USART2)
  
/* USART baudrate definition */
#define DEBUG_USART_BAUDRATE     (115200ul)

/* USART RX Port/Pin definition */
#define DEBUG_USART_RX_PORT   (PortA)
#define DEBUG_USART_RX_PIN     (Pin05)
#define DEBUG_USART_RX_FUNC     (Func_Usart2_Rx)
  
/* USART TX Port/Pin definition */
#define DEBUG_USART_TX_PORT   (PortA)
#define DEBUG_USART_TX_PIN     (Pin04)
#define DEBUG_USART_TX_FUNC   (Func_Usart2_Tx)


#ifdef __cplusplus
extern "C"
{
#endif
	
void DebugUsartInit(void);
	
#ifdef __cplusplus
}
#endif
#endif

