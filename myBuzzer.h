#ifndef MYBUZZER_H_   
#define MYBUZZER_H_

#include <stdbool.h>
#include "stm32f10x.h"

#define F0		18744
#define FIS0	17691
#define G0 		16698
#define GIS0	15761
#define A0		14876
#define B0		14041
#define H0		13253

#define C1		12509
#define CIS1	11807
#define D1		11144
#define DIS1	10519
#define E1		9929
#define F1		9371
#define FIS1	8845
#define G1		8349
#define GIS1	7880
#define A1		7438
#define B1		7021
#define H1		6627

#define C2		6255 	
#define CIS2	5904
#define D2		5572
#define DIS2	5259
#define E2		4964
#define F2		4686
#define FIS2	4423
#define G2		4174
#define GIS2	3940
#define A2		3719
#define B2		3510
#define H2		3313

#define C3		3127 	
#define CIS3	2952
#define D3		2786
#define DIS3	2630
#define E3		2482
#define F3		2343
#define FIS3	2211
#define G3		2087
#define GIS3	1970
#define A3		1860
#define B3		1755
#define H3		1657

#define C4		1564 	
#define CIS4	1476
#define D4		1393
#define DIS4	1315
#define E4		1241
#define F4		1171
#define FIS4	1106
#define G4		1044
#define GIS4	985
#define A4		930
#define B4		878
#define H4		828

#define C5		782 	
#define CIS5	738
#define D5		697
#define DIS5	657
#define E5		621
#define F5		586


#define calaNuta 	1000
#define polNuta 	500
#define cwiercNuta 	250
#define osemka		125
#define szesnastka	62
#define pauze		0xFE
#define koniec 		0xFF

#define PRESCALERMIN 2000
#define PRESCALERMAX 5000

extern const uint16_t noteMelody[];
extern const uint16_t tempo[];
extern const uint16_t imperialMarchMelody[];
extern const uint16_t imperialMarchTempo[];
extern const uint16_t godfatherMelody[];
extern const uint16_t godFatherTempo[];

extern bool turnRight;
extern volatile unsigned long tim2Prescaler;
extern bool stopBuzzer;

void buzzerSyrena(void);
void playMelody(const uint16_t * noteMelody, const uint16_t * tempo, uint16_t breakTime);
void updateMyTim(unsigned long prescaler);
void disableMyTim(void);

#endif 
