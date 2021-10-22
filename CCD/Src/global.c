#include "global.h"

volatile uint8_t ccd_read_req			=0;		//czy chcemy odczytac linijke z CCD?
volatile uint16_t ccd_pix_num			=0;		//numer odczytywanego piksela
volatile uint8_t ccd_data_ready			=0;		//czy dane z ccd sa gotowe (odczytane)?

volatile uint16_t meas_num				= 0;	//ile pomiarow mamy?
volatile uint8_t tim5_flag 				= 0;

volatile uint8_t test					=0;		//test
float f_test								=0.0;	//test

volatile float temp						=0.0;	//temperatura z DSa

uint16_t contrast						=0;		//kontrast CCD
float led_current						=0;		//do odczytu pradu leda - bezuzyteczna zmienna

volatile uint8_t cal_data_save_pending	=0;		//czy mamy dane kalibracyjne gotowe do zapisu do eepromu?
uint8_t autoled_pend					=1;		//czy trzeba robic autoled?

uint16_t pix_num						=0;		//obliczony numer piksela, do przeliczen na brixy
uint32_t max_val						=0;		//wartosc odchylenia standardowego dla piksela pix_num (czyli maksymalna globalnie)
