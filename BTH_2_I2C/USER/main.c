#include "stm32f10x.h"                 
#include "stm32f10x_i2c.h"              
#include "stm32f10x_usart.h"            
#include <stdio.h>                  

#define BH1750_ADDRESS 0x46             //  BH1750
#define BH1750_POWER_ON 0x01
#define BH1750_RESET 0x07
#define BH1750_CONTINUOUS_HIGH_RES_MODE 0x10

void I2C_Config(void) {
  
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStruct;

    //  clock cho I2C1 và GPIOB
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // chân PB6 và PB7 cho I2C
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD; // Alternate Function Open-Drain
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Cau hinh  I2C1
    I2C_InitStruct.I2C_ClockSpeed = 100000; // 100kHz
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStruct);

    
    I2C_Cmd(I2C1, ENABLE);
}

void USART_Config(void) {
    
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    // clock cho USART1 và GPIOA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // cau  hình chân PA9 và PA10 cho USART
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9; // TX
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; // Alternate Function Push-Pull
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10; // RX
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // Input floating
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Cau  hình USART1
    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_InitStruct);

    // Kick  USART1
    USART_Cmd(USART1, ENABLE);
}

void USART_SendString(USART_TypeDef* USARTx, const char* str) {
    while (*str) {
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
        USART_SendData(USARTx, *str++);
    }
}

void BH1750_Init(void) {
    //  cho BH1750
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1, BH1750_ADDRESS, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C1, BH1750_POWER_ON);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTOP(I2C1, ENABLE);

    
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1, BH1750_ADDRESS, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C1, BH1750_CONTINUOUS_HIGH_RES_MODE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTOP(I2C1, ENABLE);
}

uint16_t BH1750_ReadLight(void) {
    uint8_t light_data[2];
    uint16_t light_level;

    
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

   
    I2C_Send7bitAddress(I2C1, BH1750_ADDRESS, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

   
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    light_data[0] = I2C_ReceiveData(I2C1);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    light_data[1] = I2C_ReceiveData(I2C1);

    
    I2C_GenerateSTOP(I2C1, ENABLE);

    
    light_level = (light_data[0] << 8) | light_data[1];
    return light_level;
}

int main(void) {
    
    volatile int i;
    char buffer[16];

  
    I2C_Config();
    USART_Config();

   
    BH1750_Init();

    while (1) {
       
        uint16_t light_level = BH1750_ReadLight()/1.2;

        // send qua UART
        sprintf(buffer, "Light: %d lx\r\n", light_level);
        USART_SendString(USART1, buffer);

        // Delay 
        for (i = 0; i < 7200000; i++);
    }
}
