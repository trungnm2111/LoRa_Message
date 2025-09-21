	/*
	  ******************************************************************************
	  * @file				uart.h                                                                  *
	  * @author	    Nguyen Minh Trung                                                   *
	  * @date				00/00/0000                                                         *
	  ******************************************************************************
	*/
		
	#ifndef _UART_H
	#define _UART_H


	#ifdef __cplusplus
	 extern "C" {
	#endif
	/******************************************************************************/
	#include "stm32f10x.h"                  // Device header
	#include "stm32f10x_usart.h"            // Keil::Device:StdPeriph Drivers:USART
	#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
	#include "stdio.h"
	#include "lora.h"

	extern char string_receive[50];
	/******************************************************************************/	 
	void Usart_Config (USART_TypeDef * USARTx, uint32_t BaudRate );
	void Usart_Send_Char(char _chr);
	void Usart_Send_String(char *str);
	void Usart_SendNumber(float number);
	void Usart_Send_Uint16(uint16_t number);
	uint16_t Usart_Compare(char *string);
	uint8_t Usart_CheckFlag (void );
	void Usart_Put_Char_Receive (void);
	/******************************************************************************/	 
	#ifdef __cplusplus
	}
	#endif

	#endif

	/********************************* END OF FILE ********************************/
	/******************************************************************************/
