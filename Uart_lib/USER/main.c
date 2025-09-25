#include "stm32f10x.h"                  // Device header
#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_usart.h"            // Keil::Device:StdPeriph Drivers:USART
#include "uart.h"
#include "lora.h"
#include "lora_ota.h"




uint8_t loraHandle(void);
static void printHexArray(const uint8_t *array, uint16_t length);

int main()
{
	Usart_Config(USART1, 9600);
	Usart_Send_String("\n Hello - Start - \n");
//	Usart_SendNumber(4.5);
//	LoRa_Init();
	while(1)
	{
		if(Usart_CheckFlag() == 1)	
		{
			if(loraHandle() == 0)
			{
				Usart_Send_String("Handle Ota success");
			}
		}
	}
}


uint8_t loraHandle(void)
{
	LORA_frame_t decode_frame;
	memset(&decode_frame, 0, sizeof(decode_frame));
	Usart_Send_Char('2');
	uint16_t length_buffer_receive_fsm = lora_fsm_frame.save_length;
	uint8_t buffer_receive_fsm[150];
	memcpy(buffer_receive_fsm, lora_fsm_frame.buffer, length_buffer_receive_fsm);
	
	LORA_Frame_Status_t decode_status = loRaDecodeFrame(buffer_receive_fsm, length_buffer_receive_fsm , &decode_frame);
	if(decode_status != LORA_STATUS_DECODE_FRAME_SUCCESS)
	{
//		Usart_Send_Char('\n');
		Usart_SendNumber(decode_status);
//		Usart_SendNumber(lora_fsm_frame.save_length);
//		printHexArray(lora_fsm_frame.buffer,lora_fsm_frame.save_length);
//		Usart_Send_Char('\n');
		return -1;
	}	
	
	Usart_Send_Char('3');	
	
	if(decode_frame.message_type == OTA_TYPE_START)
	{
		otaNodeInit();
	}
	
//	Usart_SendNumber(decode_frame.packet_type);
//	Usart_Send_Char('\n');
//	Usart_SendNumber(decode_frame.message_type);
//	Usart_Send_Char('\n');
	LORA_OtaStatus_t st;
	st = loRaOtaReceiveHandler(decode_frame.message_type, decode_frame.data_payload, decode_frame.data_len);
	if(st < 0) 
	{
//		Usart_SendNumber(decode_frame.message_type);
//		Usart_SendNumber(decode_frame.data_len);
//		Usart_Send_Char('\n');
		Usart_SendNumber(st);
		return -1;
	}
	
	Usart_Send_Char('4');	
	
	return 0;
}


static void printHexArray(const uint8_t *array, uint16_t length) 
{
    for (uint16_t i = 0; i < length; i++) {
        Usart_Send_Char(array[i]);
    }
}

//			if(Usart_Compare("hello") == 1)
//			{
//				Usart_Send_String("hi");
//			}