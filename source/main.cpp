// #include "hc32_ddl.h"
#include "global.h"
#include "settings.h"
// #include "Arduino.h"

static struct rt_thread i2cSlave_thread;
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t i2cSlave_thread_stack[1024];
static void i2c_slave_task(void *e);
static rt_uint8_t i2cSlave_thread_priority = 6;

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

uint8_t chargeStatus                = notCharge;
float batteryLevelPercent           = 50; // SOC measured from fuel gauge, in %. Used in multiple places (display, serial debug, log)
float batteryVoltage                = 0.0;
float batteryChargingPercentPerHour = 0.0;
float batteryTempC                  = 0.0;
present_device present_devices;
online_device online_devices;

// system device
unsigned short reg_value        = 0;
static uint32_t lastPowerOnTime = 0;
SystemParameter DisplayPannelParameter;
/**
 * @brief I2c communication mode enum
 */
typedef enum {
    Mode_Send = 0u,
    Mode_Rev  = 1u,
} stc_i2c_com_mode_t;

/**
 * @brief I2c communication status enum
 */
typedef enum {
    I2C_ComBusy = 0u,
    I2C_ComIdle = 1u,
} stc_i2c_com_status_t;

/**
 * @brief I2c communication structure
 */
typedef struct
{
    stc_i2c_com_mode_t enMode;        /*!< I2C communication mode*/
    uint32_t u32Length;               /*!< I2C communication data length*/
    uint8_t *pBuf;                    /*!< I2C communication data buffer pointer*/
    __IO uint32_t u32DataIndex;       /*!< I2C communication data transfer index*/
    stc_i2c_com_status_t enComStatus; /*!< I2C communication status*/
} stc_i2c_communication_t;

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Define I2C unit used for the example */
#define I2C_UNIT (M4_I2C2)
/* Define slave device address for example */
#define SLAVE_ADDRESS (0x11u)
// #define I2C_10BITS_ADDRESS              (1u)

/* Define port and pin for SDA and SCL */
#define I2C_SCL_PORT      (PortA)
#define I2C_SCL_PIN       (Pin09)
#define I2C_SDA_PORT      (PortA)
#define I2C_SDA_PIN       (Pin08)
#define I2C_GPIO_SCL_FUNC (Func_I2c2_Scl)
#define I2C_GPIO_SDA_FUNC (Func_I2c2_Sda)

#define I2C_INT_EEI_DEF   (INT_I2C2_EEI)
#define I2C_INT_RXI_DEF   (INT_I2C2_RXI)
#define I2C_INT_TXI_DEF   (INT_I2C2_TXI)
#define I2C_INT_TEI_DEF   (INT_I2C2_TEI)

#define I2C_FCG_USE       (PWC_FCG1_PERIPH_I2C2)

/* Define Write and read data length for the example */
#define TEST_DATA_LEN (256u)
/* Define i2c baudrate */
#define I2C_BAUDRATE (400000ul)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
uint8_t u8RxBuf[TEST_DATA_LEN];
static stc_i2c_communication_t stcI2cCom;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 ******************************************************************************
 ** \brief  Slave receive data
 **
 ** \param  pu8RxData             Pointer to the data buffer
 ** \param  u32Size               Data size
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                    Success
 **         - OperationInProgress:   Busy
 ******************************************************************************/
static en_result_t I2C_Slave_Receive_IT(uint8_t *pu8RxData, uint32_t u32Size)
{
    en_result_t enRet = Ok;

    if (I2C_ComIdle == stcI2cCom.enComStatus) {
        stcI2cCom.enComStatus = I2C_ComBusy;

        stcI2cCom.u32DataIndex = 0u;
        stcI2cCom.enMode       = Mode_Rev;
        stcI2cCom.u32Length    = u32Size;
        stcI2cCom.pBuf         = pu8RxData;

        I2C_Cmd(I2C_UNIT, Enable);
        /* Config slave address match interrupt function*/
        I2C_IntCmd(I2C_UNIT, I2C_CR2_SLADDR0EN | I2C_CR2_RFULLIE, Enable);
    } else {
        enRet = OperationInProgress;
    }

    return enRet;
}

/**
 ******************************************************************************
 ** \brief  Slave transmit data
 **
 ** \param  pu8TxData             Pointer to the data buffer
 ** \param  u32Size               Data size
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                    Success
 **         - OperationInProgress:   Busy
 ******************************************************************************/
static en_result_t I2C_Slave_Transmit_IT(uint8_t *pu8TxData, uint32_t u32Size)
{
    en_result_t enRet = Ok;

    if (I2C_ComIdle == stcI2cCom.enComStatus) {
        stcI2cCom.enComStatus = I2C_ComBusy;

        stcI2cCom.u32DataIndex = 0u;
        stcI2cCom.enMode       = Mode_Send;
        stcI2cCom.u32Length    = u32Size;
        stcI2cCom.pBuf         = pu8TxData;

        I2C_Cmd(I2C_UNIT, Enable);
        /* Config slave address match interrupt function*/
        I2C_IntCmd(I2C_UNIT, I2C_CR2_SLADDR0EN, Enable);
    } else {
        enRet = OperationInProgress;
    }

    return enRet;
}

/**
 *******************************************************************************
 ** \brief static function for buffer write.
 **
 ** \param  u8Data the data to be write.
 **
 ** \retval None
 **
 ******************************************************************************/
static void BufWrite(uint8_t u8Data)
{
    if (stcI2cCom.u32DataIndex < stcI2cCom.u32Length) {
        u8RxBuf[stcI2cCom.u32DataIndex] = u8Data;
        stcI2cCom.u32DataIndex++;
    }
}

/**
 *******************************************************************************
 ** \brief static function for buffer read.
 **
 ** \param None
 **
 ** \retval uint8_t the data read from the buffer.
 **
 ******************************************************************************/
static uint8_t BufRead(void)
{
    uint8_t temp;
    if (stcI2cCom.u32DataIndex < stcI2cCom.u32Length) {
        temp = u8RxBuf[stcI2cCom.u32DataIndex];
        stcI2cCom.u32DataIndex++;
    } else {
        temp = 0xFFu;
    }

    return temp;
}

/**
 *******************************************************************************
 ** \brief I2C EEI(communication error or event) interrupt callback function
 **
 ** \param  None
 **
 ** \retval None
 **
 ******************************************************************************/
void I2C_EEI_Callback(void)
{
    /* If address interrupt occurred */
    if (Set == I2C_GetStatus(I2C_UNIT, I2C_SR_SLADDR0F)) {
        I2C_ClearStatus(I2C_UNIT, I2C_CLR_SLADDR0FCLR | I2C_CLR_NACKFCLR);

        if ((Mode_Send == stcI2cCom.enMode) && (Set == I2C_GetStatus(I2C_UNIT, I2C_SR_TRA))) {
            /* Enable tx end interrupt function*/
            I2C_IntCmd(I2C_UNIT, I2C_CR2_TENDIE, Enable);
            /* Write the first data to DTR immediately */
            I2C_WriteData(I2C_UNIT, BufRead());

            /* Enable stop and NACK interrupt */
            I2C_IntCmd(I2C_UNIT, I2C_CR2_STOPIE | I2C_CR2_NACKIE, Enable);
        } else if ((Mode_Rev == stcI2cCom.enMode) && (Reset == I2C_GetStatus(I2C_UNIT, I2C_SR_TRA))) {
            /* Config rx buffer full interrupt function*/
            I2C_IntCmd(I2C_UNIT, I2C_CR2_RFULLIE, Enable);

            /* Enable stop and NACK interrupt */
            I2C_IntCmd(I2C_UNIT, I2C_CR2_STOPIE | I2C_CR2_NACKIE, Enable);
        }
    }
    /* If NACK interrupt occurred */
    else if (Set == I2C_GetStatus(I2C_UNIT, I2C_SR_NACKF)) {
        /* clear NACK flag*/
        I2C_ClearStatus(I2C_UNIT, I2C_CLR_NACKFCLR);
        /* Stop tx or rx process*/
        if (Set == I2C_GetStatus(I2C_UNIT, I2C_SR_TRA)) {
            /* Config tx end interrupt function disable*/
            I2C_IntCmd(I2C_UNIT, I2C_CR2_TENDIE, Disable);
            I2C_ClearStatus(I2C_UNIT, I2C_CLR_TENDFCLR);

            /* Read DRR register to release */
            I2C_ReadData(I2C_UNIT);
        } else {
            /* Config rx buffer full interrupt function disable */
            I2C_IntCmd(I2C_UNIT, I2C_CR2_RFULLIE, Disable);
        }
    }
    /* If stop interrupt occurred */
    else if (Set == I2C_GetStatus(I2C_UNIT, I2C_SR_STOPF)) {
        /* Disable all interrupt enable flag except SLADDR0IE*/
        I2C_IntCmd(I2C_UNIT,
                   I2C_CR2_TENDIE | I2C_CR2_RFULLIE |
                       I2C_CR2_STOPIE | I2C_CR2_NACKIE,
                   Disable);
        /* Clear STOPF flag */
        I2C_ClearStatus(I2C_UNIT, I2C_CLR_STOPFCLR);
        I2C_Cmd(I2C_UNIT, Disable);

        stcI2cCom.enComStatus = I2C_ComIdle;

        I2C_Cmd(I2C_UNIT, Enable);
        /* Config slave address match interrupt function*/
        I2C_IntCmd(I2C_UNIT, I2C_CR2_SLADDR0EN | I2C_CR2_RFULLIE, Enable);
    }
}

/**
 *******************************************************************************
 ** \brief I2C TEI(transfer end) interrupt callback function
 **
 ** \param  None
 **
 ** \retval None
 **
 ******************************************************************************/
void I2C_TEI_Callback(void)
{
    if ((Set == I2C_GetStatus(I2C_UNIT, I2C_SR_TENDF)) &&
        (Reset == I2C_GetStatus(I2C_UNIT, I2C_SR_ACKRF))) {
        I2C_WriteData(I2C_UNIT, BufRead());
    }
}

/**
 *******************************************************************************
 ** \brief I2C RXI(receive buffer full) interrupt callback function
 **
 ** \param  None
 **
 ** \retval None
 **
 ******************************************************************************/
void I2C_RXI_Callback(void)
{
    if (Set == I2C_GetStatus(I2C_UNIT, I2C_SR_RFULLF)) {
        BufWrite(I2C_ReadData(I2C_UNIT));
    }
}

/**
 ******************************************************************************
 ** \brief  Initialize the I2C peripheral for slave
 **
 ** \param  None
 **
 ** \retval en_result_t                Enumeration value:
 **          - Ok:                     Success
 **          - ErrorInvalidParameter:  Invalid parameter
 ******************************************************************************/
static en_result_t Slave_Initialize(void)
{
    en_result_t enRet;
    stc_i2c_init_t stcI2cInit;
    stc_irq_regi_conf_t stcIrqRegiConf;
    float32_t fErr;

    I2C_DeInit(I2C_UNIT);

    stcI2cCom.enComStatus = I2C_ComIdle;
    MEM_ZERO_STRUCT(stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV2;
    stcI2cInit.u32Baudrate = 400000ul;
    stcI2cInit.u32SclTime  = 5U;
    enRet                  = I2C_Init(I2C_UNIT, &stcI2cInit, &fErr);

    if (Ok == enRet) {
        /* Set slave address*/
#ifdef I2C_10BITS_ADDRESS
        I2C_SlaveAdr0Config(I2C_UNIT, Enable, Adr10bit, SLAVE_ADDRESS);
#else
        I2C_SlaveAdr0Config(I2C_UNIT, Enable, Adr7bit, SLAVE_ADDRESS);
#endif
        /* Register EEI Int to Vect.No.001 */
        stcIrqRegiConf.enIRQn = Int001_IRQn;
        /* Select I2C Error or Event interrupt function */
        stcIrqRegiConf.enIntSrc = I2C_INT_EEI_DEF;
        /* Callback function */
        stcIrqRegiConf.pfnCallback = &I2C_EEI_Callback;
        /* Registration IRQ */
        enIrqRegistration(&stcIrqRegiConf);
        /* Clear Pending */
        NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
        /* Set priority */
        NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
        /* Enable NVIC */
        NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

        /* Register RXI Int to Vect.No.002 */
        stcIrqRegiConf.enIRQn = Int002_IRQn;
        /* Select I2C receive full interrupt function */
        stcIrqRegiConf.enIntSrc = I2C_INT_RXI_DEF;
        /* Callback function */
        stcIrqRegiConf.pfnCallback = &I2C_RXI_Callback;
        /* Registration IRQ */
        enIrqRegistration(&stcIrqRegiConf);
        /* Clear Pending */
        NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
        /* Set priority */
        NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
        /* Enable NVIC */
        NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

        /* Register TEI Int to Vect.No.003 */
        stcIrqRegiConf.enIRQn = Int003_IRQn;
        /* Select I2C TX end interrupt function */
        stcIrqRegiConf.enIntSrc = I2C_INT_TEI_DEF;
        /* Callback function */
        stcIrqRegiConf.pfnCallback = &I2C_TEI_Callback;
        /* Registration IRQ */
        enIrqRegistration(&stcIrqRegiConf);
        /* Clear Pending */
        NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
        /* Set priority */
        NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
        /* Enable NVIC */
        NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
    }
    return enRet;
}
/**
 *******************************************************************************
 ** \brief  Main function of template project
 **
 ** \param  None
 **
 ** \retval int32_t return value, if needed
 **
 ******************************************************************************/
int32_t main(void)
{
    //ledInit();
    pinMode(PB14, OUTPUT);
    keyInit();
    memset(&DisplayPannelParameter, 0, sizeof(SystemParameter));
    memcpy(DisplayPannelParameter.hw_version, HW_VERSION, strlen(HW_VERSION));
    memcpy(DisplayPannelParameter.sw_version, SW_VERSION, strlen(SW_VERSION));
    /* Initialize I2C port*/
    PORT_SetFunc(I2C_SCL_PORT, I2C_SCL_PIN, I2C_GPIO_SCL_FUNC, Disable);
    PORT_SetFunc(I2C_SDA_PORT, I2C_SDA_PIN, I2C_GPIO_SDA_FUNC, Disable);

    /* Enable I2C Peripheral*/
    PWC_Fcg1PeriphClockCmd(I2C_FCG_USE, Enable);
    /* Initialize I2C peripheral and enable function*/
    Slave_Initialize();
    /* main thread */
    user_task_init();
    rt_thread_init(&i2cSlave_thread,
                   "i2cSlave",
                   i2c_slave_task,
                   RT_NULL,
                   &i2cSlave_thread_stack,
                   sizeof(i2cSlave_thread_stack),
                   i2cSlave_thread_priority,
                   100);

    rt_thread_startup(&i2cSlave_thread);
}

static void i2c_slave_task(void *e)
{
    LOG_INFO("Entry %s", __func__);
    while (1) {
        while (Ok != I2C_Slave_Receive_IT(u8RxBuf, TEST_DATA_LEN)) {
            rt_thread_yield();
        }
        rt_thread_mdelay(100);
    }
}
