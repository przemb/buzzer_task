#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "core_cm3.h"
#include "myBuzzer.h"

#define DEBUGMODE 0
#define DEBUGLED 1
#define LINELEN 50


char statsBuffer[512] = {0};
char * statsBufferPtr = &statsBuffer[0];

void ledInit(void);
void uartConfig(void);
void timerTwoConfig(void);
void clockConfig(void);
void sendChar(char c);
void sendString(const char* s);
void ITMsendString(const char * s);

// RTOS task
void vRedLedTask(void *p);
void vGreenLedTask(void *p);
void vGatekeeperTask(void *p);
void vGetStatsTask(void *p);
void vBuzzerTask(void *p);

void vAlarmTimerCallback(TimerHandle_t xTimer);

void queuePrintf(char * message, ...);
void directPrintf(char * message, unsigned long param);

TimerHandle_t xAlarmTimer = NULL;

QueueHandle_t xShortMsgQueue, xLongMsgQueue;


volatile unsigned long ulHighFrequencyTimerTicks = 0;


int main(void){
    //clockConfig();
	//Configure GPIO for LED
    ledInit();
	uartConfig();
	timerTwoConfig();
	
	xLongMsgQueue = xQueueCreate(8, sizeof(char *));
	xShortMsgQueue = xQueueCreate(8, LINELEN);
	xAlarmTimer = xTimerCreate("Alarm Timer", 			pdMS_TO_TICKS(15000), pdTRUE, ( void * ) 0, vAlarmTimerCallback);
	
	if((xShortMsgQueue != NULL) && (xLongMsgQueue != NULL)){
		xTaskCreate(vGreenLedTask, 		(const char*) "GreenLedTask     ", 128, NULL, 2, NULL);
		//xTaskCreate(vGetStatsTask, 		(const char*) "GetMyStatsTask   ", 128, NULL, 2, NULL);
		xTaskCreate(vGatekeeperTask, 	(const char*) "GatekeeperTask   ", 128, NULL, 1, NULL);
		//xTaskCreate(vRedLedTask, 		(const char*) "RedLedTask       ", 128, NULL, 2, NULL);
		xTaskCreate(vBuzzerTask, 		(const char*) "BuzzerTask       ", 128, NULL, 2, NULL);
		
		if( xAlarmTimer != NULL ){
			xTimerStart( xAlarmTimer, 0);	 
		}
	}
	sendString("System start\n");
	//Start RTOS scheduler
	vTaskStartScheduler();
    for(;;);
}



void vGreenLedTask(void *p){
    for (;;){
        GPIOA->ODR ^= GPIO_Pin_5;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vRedLedTask(void *p){
    for (;;){
        GPIOC->ODR ^= GPIO_Pin_0;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void vGatekeeperTask(void *p){
	char * longMsgPtr;
	char shortMsgBuffer[LINELEN];
	
	for(;;){
		if(xQueueReceive( xShortMsgQueue, shortMsgBuffer, pdMS_TO_TICKS(500)))
			sendString(shortMsgBuffer);
		if(DEBUGMODE){
			if(xQueueReceive( xLongMsgQueue, &longMsgPtr, pdMS_TO_TICKS(500)))
				ITMsendString(longMsgPtr);
		}
	}
}

void vGetStatsTask(void *p){
	for (;;){
		//vTaskGetRunTimeStats(statsBuffer);
		xQueueSendToBack(xLongMsgQueue, &statsBufferPtr, 0);
				
		//queuePrintf("ulHighFrequencyTimerTicks = %lu\n", ulHighFrequencyTimerTicks);
		vTaskDelay(pdMS_TO_TICKS(1000));		
    }
}

void vBuzzerTask(void *p){
	for (;;){
		playMelody(imperialMarchMelody, imperialMarchTempo, 50);
		//playMelody(godfatherMelody, godFatherTempo, 50);
		//buzzerSyrena();
	}
}

void vAlarmTimerCallback(void *p){
	queuePrintf("\nAlarm!\n\n");
}


void timerTwoConfig(void){
	TIM_TimeBaseInitTypeDef tim;
	NVIC_InitTypeDef nvic;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	//81818,(18)
	tim.TIM_CounterMode = TIM_CounterMode_Up; 
	tim.TIM_Prescaler = 3719 - 1;
	tim.TIM_Period =  22 - 1;
	TIM_TimeBaseInit(TIM2, &tim);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);	
	
	nvic.NVIC_IRQChannel = TIM2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
}

void TIM2_IRQHandler(void){
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET){
		
		//buzzer!
		if(!stopBuzzer)
			GPIOC->ODR ^= GPIO_Pin_6;
	
		//red led
		if(DEBUGLED)
			GPIOC->ODR ^= GPIO_Pin_0;
			
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void clockConfig(void){
	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
}

void uartConfig(void){
	GPIO_InitTypeDef gpio;
	USART_InitTypeDef uart;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //alternatywne linie IO
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //USART2

	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = GPIO_Pin_3;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio);

	USART_StructInit(&uart);
	uart.USART_BaudRate = 9600;
	USART_Init(USART2, &uart);

	USART_Cmd(USART2, ENABLE);
}

void sendString(const char * s){
	while (*s)
		sendChar(*s++);
	//sendChar('\n');	
}

void ITMsendString(const char * s){
	while (*s)
		ITM_SendChar(*s++);
	ITM_SendChar('\n');
}

void sendChar(char c){
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
		;
	USART_SendData(USART2, c);
}

void ledInit(){
    GPIO_InitTypeDef  gpio;
    //Configure PA5, push-pull output
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_8| GPIO_Pin_9;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &gpio);
	GPIO_Init(GPIOC, &gpio);
}

void queuePrintf(char * message, ...){
	static char buffer[LINELEN] = {0}; 
	
	va_list args;

	va_start(args, message);
	vsprintf(buffer, message, args);
	va_end(args);
	
	//ex.
	//sprintf(buffer, "Counter value = %zu\n", ulCount);
	
	xQueueSendToBack(xShortMsgQueue, buffer, 0);	
}

void directPrintf(char * message, unsigned long param){
	static char buffer[LINELEN] = {0};
	static char * ptrBuffer = &buffer[0];

	sprintf(buffer, message, param);

	sendString(ptrBuffer);
}

