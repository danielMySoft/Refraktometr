#ifndef _GLOBAL_
#define _GLOBAL_

#include "main.h"

extern volatile uint8_t ccd_read_req;		//czy chcemy odczytac linijke z CCD?
extern volatile uint16_t ccd_pix_num;		//numer odczytywanego piksela
extern volatile uint8_t ccd_data_ready;		//czy dane z ccd sa gotowe (odczytane)?

uint16_t ccd[NUM_PIX];					//miejsce na surowe probki odczytane z CCD
uint16_t ccd_fir16[NUM_PIX];
uint32_t ccd_avg[NUM_PIX];				//miejsce na usrednione probki z CCD
uint16_t ccd_fir[NUM_PIX];				//miejsce na odfiltrowane przebiegi z CCD

extern volatile uint16_t meas_num;					//ile pomiarow mamy?
extern volatile uint8_t tim5_flag;

extern volatile uint8_t test;		//test
extern float f_test;	//test

extern volatile float temp;	//temperatura z DSa

extern uint16_t contrast;		//kontrast CCD
extern float led_current;								//do odczytu pradu leda - bezuzyteczna zmienna

uint8_t uart_in[25];					//miejsce na dane wejsciowe z "plytki komputera"
uint8_t uart_out[50];					//miejsce na odpowiedz z pomiarem do "plytki komputera"
extern volatile uint8_t cal_data_save_pending;		//czy mamy dane kalibracyjne gotowe do zapisu do eepromu?
extern uint8_t autoled_pend;		//czy trzeba robic autoled?

extern uint16_t pix_num;		//obliczony numer piksela, do przeliczen na brixy
extern uint32_t max_val;		//wartosc odchylenia standardowego dla piksela pix_num (czyli maksymalna globalnie)
uint32_t pix_nums[PIX_NUM_AVG];					//ostatnie kilka wartosci pix_num
float temperature[PIX_NUM_AVG];
uint32_t test1;

struct calib
{
	uint16_t serial;	//numer seryjny
	float f;			//ogniskowa
	float D;			//dystorsja
	float temp_corr;	//korekcja temperatury
	float x_corr;		//korekcja polozenia
} cal_data;

struct measurement
{
	float temp;			//temperatura zmierzona
	float nc;			//wspolczynnik zalamania zmierzony w temperaturze pomiaru
	float nck;			//wspolczynnik zalamania obliczony dla temp. 20*C
	float brix;			//brixy zmierzone
	float brixk;		//brixy po korekcji temperaturowej
	float led_curr;		//prad LEDa w mA
	float num_pix;		//numer granicznego piksela obliczony
} meas;

#endif
