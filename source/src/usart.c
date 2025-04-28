#include "usart.h"

void DebugUsartInit(void)
{
	en_result_t enRet = Ok;
	uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART1 | PWC_FCG1_PERIPH_USART2 | \
	PWC_FCG1_PERIPH_USART3 | PWC_FCG1_PERIPH_USART4;
	const stc_usart_uart_init_t stcInitCfg = {
	UsartIntClkCkNoOutput,
	UsartClkDiv_1,
	UsartDataBits8,
	UsartDataLsbFirst,
	UsartOneStopBit,
	UsartParityNone,
	UsartSampleBit8,
	UsartStartBitFallEdge,
	UsartRtsEnable,
	};

	/* Enable peripheral clock */
	PWC_Fcg1PeriphClockCmd(u32Fcg1Periph, Enable);

	/* Initialize USART IO */
	PORT_SetFunc(DEBUG_USART_RX_PORT, DEBUG_USART_RX_PIN, DEBUG_USART_RX_FUNC, Disable);
	PORT_SetFunc(DEBUG_USART_TX_PORT, DEBUG_USART_TX_PIN, DEBUG_USART_TX_FUNC, Disable);

	/* Initialize UART */
	enRet = USART_UART_Init(DEBUG_USART_CH, &stcInitCfg);
	if (enRet != Ok)
	{
	while (1)
	{
	}
	}

	/* Set baudrate */
	enRet = USART_SetBaudrate(DEBUG_USART_CH, DEBUG_USART_BAUDRATE);
	if (enRet != Ok)
	{
	while (1)
	{
}
}

	/*Enable RX && TX function*/
	USART_FuncCmd(DEBUG_USART_CH, UsartRx, Enable);
	USART_FuncCmd(DEBUG_USART_CH, UsartTx, Enable);
}
