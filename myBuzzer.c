#include <stdint.h>
#include "myBuzzer.h"
#include "FreeRTOS.h"
#include "task.h"



const uint16_t testMelody[] = {C1, C1, G1, G1, A1, A1, G1, F1, F1, E1, E1, D1, D1, C1, koniec};

const uint16_t testTempo[] = 	{cwiercNuta, cwiercNuta, cwiercNuta, cwiercNuta, cwiercNuta, cwiercNuta, polNuta, 
								 cwiercNuta, cwiercNuta, cwiercNuta, cwiercNuta, cwiercNuta, cwiercNuta, polNuta, koniec};
	

const uint16_t imperialMarchMelody[] = {A1, A1, A1, F1, C2, 	A1, F1, C2, A1, 			E2, E2, E2, F2, C2,
										GIS1, F1, C2, A1, 		A2, A1, A1, A2, GIS2, G2, 	FIS2, F2, FIS2, B1, DIS2, D2, CIS2,
										C2, H1, C2, F1, GIS1, 	F1, GIS1, C2, H1, C2, A1, 	koniec					};

const uint16_t imperialMarchTempo[] = 	{cwiercNuta, cwiercNuta, cwiercNuta, osemka, osemka, 	
										 cwiercNuta, osemka, osemka, polNuta, 
										 cwiercNuta, cwiercNuta, cwiercNuta, osemka, osemka, 	
										 cwiercNuta, osemka, osemka, polNuta, 	
										 cwiercNuta, osemka, osemka, cwiercNuta, osemka, osemka, 
										 osemka, osemka, osemka, osemka, cwiercNuta, osemka, osemka, 
										 osemka, osemka, osemka, osemka, cwiercNuta, osemka, osemka, 
										 cwiercNuta, osemka, osemka, polNuta, 
										 koniec
										};

const uint16_t godfatherMelody[] =	{G1, C2, E2,						D2, C2, E2, C2, D2, C2, A1, B1, 
									 G1, G1, C2, E2, 					D2, C2, E2, C2, D2, C2, G1, F1,
									 F1, F1, A1, B1, 					D2, F1, A1, B1,							C2, C1, E1, B1,
									 A1, G1, B1, A1, A1, G1, G1, B0, 	C1, C2, C2, B1,
									 koniec,
									};
						
const uint16_t godFatherTempo[] = 	{cwiercNuta, osemka, osemka,				osemka, osemka, osemka, osemka, osemka, osemka, osemka, osemka,
									 polNuta, cwiercNuta, osemka, osemka, 		osemka, osemka, osemka, osemka, osemka, osemka, osemka, osemka,
									 polNuta, cwiercNuta, osemka, osemka, 		polNuta, cwiercNuta, osemka, osemka, 								polNuta, cwiercNuta, osemka, osemka,
									 osemka, osemka, osemka, osemka, osemka, osemka, osemka, osemka,		polNuta, cwiercNuta, osemka, osemka,
									 koniec,
									};

bool turnRight = 1; //1 - right, 0 - left
bool stopBuzzer = 0;

//volatile unsigned long tim2Prescaler = 3719;
volatile unsigned long tim2Prescaler = 0;
									
uint16_t debugNoteMelody;
uint16_t debugTempo;
							 

void playMelody(const uint16_t * noteMelody, const uint16_t * tempo, uint16_t breakTime){
	uint16_t noteLen = 0;
	
	while(*tempo != koniec){
		debugNoteMelody = *noteMelody;
		debugTempo = *tempo;
		
		updateMyTim(*noteMelody);
		noteLen = *tempo;
		vTaskDelay(pdMS_TO_TICKS(noteLen));
			
		noteMelody++;
		tempo++;
		
		//break between notes
		stopBuzzer = 1;
		vTaskDelay(pdMS_TO_TICKS(breakTime));
		stopBuzzer = 0;
	}
	disableMyTim();
	vTaskSuspend(NULL);

}

void updateMyTim(unsigned long prescaler){
	TIM_TimeBaseInitTypeDef tim;
	tim.TIM_Prescaler = prescaler - 1;
	tim.TIM_Period =  22 - 1;
	TIM_TimeBaseInit(TIM2, &tim);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}


void disableMyTim(void){
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
}


void buzzerSyrena(void){
	if((tim2Prescaler > PRESCALERMIN) && (tim2Prescaler < PRESCALERMAX)){
		if(turnRight==1)
			tim2Prescaler=tim2Prescaler + 1;
		else
			tim2Prescaler=tim2Prescaler - 1;
	}
	else if(tim2Prescaler == PRESCALERMAX){
		tim2Prescaler--;
		turnRight = !turnRight;
	}
	else{
		tim2Prescaler++;
		turnRight = !turnRight;
	}	
}
