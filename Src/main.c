/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2021 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "ds.h"
#include "eeprom.h"
#include "ccd_math.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t ccd_read_req			=0;		//czy chcemy odczytac linijke z CCD?
volatile uint16_t ccd_pix_num			=0;		//numer odczytywanego piksela
volatile uint8_t ccd_data_ready			=0;		//czy dane z ccd sa gotowe (odczytane)?

uint16_t ccd[NUM_PIX];					//miejsce na surowe probki odczytane z CCD
uint32_t ccd_avg[NUM_PIX];				//miejsce na usrednione probki z CCD
uint32_t ccd_fir[NUM_PIX];				//miejsce na odfiltrowane przebiegi z CCD

volatile uint16_t meas_num=0;					//ile pomiarow mamy?

volatile uint8_t test					=0;		//test
float f_test							=0.0;	//test

volatile float temp						=0.0;	//temperatura z DSa

uint16_t contrast						=0;		//kontrast CCD
float led_current;								//do odczytu pradu leda - bezuzyteczna zmienna

uint8_t uart_in[25];					//miejsce na dane wejsciowe z "plytki komputera"
uint8_t uart_out[50];					//miejsce na odpowiedz z pomiarem do "plytki komputera"
volatile uint8_t cal_data_save_pending	=0;		//czy mamy dane kalibracyjne gotowe do zapisu do eepromu?
uint8_t autoled_pending					=0;		//czy trzeba robic autoled?

uint16_t pix_num						=0;		//obliczony numer piksela, do przeliczen na brixy
uint32_t max_val						=0;		//wartosc odchylenia standardowego dla piksela pix_num (czyli maksymalna globalnie)
uint32_t pix_nums[PIX_NUM_AVG];					//ostatnie kilka wartosci pix_num

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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//przerwanie do odczytu linii z CCD, ok 7.5ms dla zegara 2MHz
	if(htim->Instance==TIM2)
	{
		//czy ostatnio odczytywalismy linijke z CCD?

		ICG_L;
		for(uint16_t i=0; i<5; i++) asm("NOP");
		SH_H;
		for(uint16_t i=0; i<50; i++) asm("NOP");
		SH_L;
		for(uint16_t i=0; i<99; i++) asm("NOP");
		ICG_H;
		for(uint16_t i=0; i<500; i++) asm("NOP");

		if(ccd_read_req)
		{
			ccd_read_req = 0;
			TIM1->CNT=0;
			ccd_pix_num=0;
			ccd_data_ready=0;
			HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1);
		}
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	//przerwanie do odczytu probki sygnalu z CCD, 500kHz
	//if(htim->Instance==TIM1)
	{
		HAL_ADC_Start(&hadc2);
		ccd[ccd_pix_num]=HAL_ADC_GetValue(&hadc2);
		ccd_pix_num++;
		if(ccd_pix_num>=3694){
			ccd_data_ready = 1;
			ccd_pix_num=0;
			ccd_read_req=0;
			HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_1);
			ccd_read_req=0;
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//jezeli przerwanie z UARTa do komunikacji z plytka komputera
	if(huart->Instance==USART1)
	{
		cal_data_save_pending=1;
	}
}

void getCCD(void)
{
	//memset((uint8_t*)ccd, 0, 2*NUM_PIX);
	ccd_read_req=1;
	while(!ccd_data_ready);
}

void saveCalData(void)
{
	eepromWriteInt(EEPROM_BASE_ADDR  +0, cal_data.serial);
	eepromWriteFloat(EEPROM_BASE_ADDR+2, cal_data.f);
	eepromWriteFloat(EEPROM_BASE_ADDR+6, cal_data.D);
	eepromWriteFloat(EEPROM_BASE_ADDR+10, cal_data.temp_corr);
	eepromWriteFloat(EEPROM_BASE_ADDR+14, cal_data.x_corr);
}

void readCalData(void)
{
	cal_data.serial=eepromReadInt(EEPROM_BASE_ADDR  +0);
	cal_data.f=eepromReadFloat(EEPROM_BASE_ADDR+2);
	cal_data.D=eepromReadFloat(EEPROM_BASE_ADDR+6);
	cal_data.temp_corr=eepromReadFloat(EEPROM_BASE_ADDR+10);
	cal_data.x_corr=eepromReadFloat(EEPROM_BASE_ADDR+14);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_SPI4_Init();
  MX_DAC1_Init();
  MX_ADC2_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  SCB->CPACR|=(3<<20)|(3<<22);

  setLedCurrent(0.00);
  for(uint8_t i=0; i<PIX_NUM_AVG; i++)
	  pix_nums[i]=0;
  HAL_Delay(250);

  readCalData();

  HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, 1);
  HAL_GPIO_WritePin(EEPROM_HOLD_GPIO_Port, EEPROM_HOLD_Pin, 1);
  DS_Reset();

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_Base_Start_IT(&htim2);

  HAL_Delay(2000);
  HAL_UART_Receive_IT(&huart1, uart_in, 21);

  autoLed();
  //setLedCurrent(0.0019); //bylo 0.0012
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //czyszczenie bufora i akwizycja danych
	  for(uint16_t i=0; i<NUM_PIX; i++)
		  ccd_avg[i]=0;

	  for(uint8_t i=0; i<AVG_LINES; i++)  {
		  getCCD();
		  for(uint16_t j=0; j<NUM_PIX; j++)
			  ccd_avg[j]+=ccd[j];
	  }
	  for(uint16_t j=0; j<NUM_PIX; j++)
		  ccd_avg[j] = ccd_avg[j]/AVG_LINES;

	  meas.temp=DS_GetTemp()+cal_data.temp_corr;

	  //dalsza matematyka
	  //lowpass(ccd_avg, 3650, AVG_FILTER);
	  fir(ccd_avg, ccd_fir, 5);
	  fir(ccd_fir, ccd_avg, 7);
	  fir(ccd_avg, ccd_fir, 9);
	  fir(ccd_fir, ccd_avg, 11);

	  std_dev(ccd_avg, STD_DEV_LEN);

	  //szukanie wartosci maksymalnej odchylenia standardowego
	  //przy okazji sprawdzenie, dla jakiego numeru piksela wystepuje
	  max_val=0;
	  for(uint16_t i=150; i<3500; i++) {
		  if(ccd_avg[i]>max_val) {
			  max_val=ccd_avg[i];
			  pix_num=i;
		  }
	  }

	  for(uint8_t i=0; i<PIX_NUM_AVG-1; i++) {
		  pix_nums[i]=pix_nums[i+1];
	  }

	  pix_nums[PIX_NUM_AVG-1]=pix_num;

	  if(meas_num<PIX_NUM_AVG)
		  meas_num++;

	  float pn=0.0;
	  for(int16_t i=PIX_NUM_AVG-1; i>PIX_NUM_AVG-1-meas_num; i--)
		  pn+=pix_nums[i];
	  meas.num_pix=pn/meas_num;

	  if(max_val<10000)	//brak probki, bylo 4000
	  {
		  meas_num=0;
		  for(uint8_t i=0; i<PIX_NUM_AVG-1; i++)
			  pix_nums[i]=0;
		  //HAL_Delay(1500);
		  meas.nc=-1.0;
		  //autoLed();
		  //setLedCurrent(0.0019);
		  //memset(uart_in, 0, 25);
		  HAL_UART_AbortReceive_IT(&huart1);
		  HAL_UART_Receive_IT(&huart1, uart_in, 21);
	  }
	  else
	  {
		  //gdy jest obecna probka
		  //licz wszystko
		  float ns=1.75996+((13.6e-6)*(meas.temp-20.0));// n szafiru
		  float xp=-(REAL_PIX_NUM/2.0-meas.num_pix+cal_data.x_corr)*0.008;
		  xp+=cal_data.D*pow(xp, 3.0);
		  float n_gamma=atan(xp/cal_data.f);
		  float n_beta=asin((sin(n_gamma))/ns);
		  float n_alpha=n_beta+54.0/180.0*M_PI;
		  meas.nc=ns*sin(n_alpha);
		  meas.led_curr=getLedCurrent();

		  //na zadanie p. Piotra - zaokraglamy wynik do 5 miejsc po przecinku
		  meas.nc = ((uint32_t)(meas.nc * 100000 + 0.5) / 100000.0);

		  //obliczaj korekcje
		  float brix=0.0, realbrix=0.0;

		  //szukanie w tabeli n_brix
		  for(uint16_t i=0; i<(9201*2-1); i++)
		  {
			if(meas.nc-n_brix[i]<=0.0)
			{
				brix=-2.00+i*0.005;
				break;
			}
		  }

		  uint8_t x, y;	//pomocnicze zmienne do obliczen

		  realbrix=brix;
		  if(brix<0)
			brix=0.0;

		  x=(uint8_t)brix/5;
		  y=(uint8_t)meas.temp-10;

		  float px=brix-((uint8_t)brix/5)*5; px/=5.0;
		  float py=meas.temp-(uint8_t)meas.temp;

		  float f00=temp_brix[y][x], f11=temp_brix[y+1][x+1], f01=temp_brix[y+1][x], f10=temp_brix[y][x+1];
		  float corr=f00*(1-px)*(1-py) + f10*px*(1-py) + f01*(1-px)*py + f11*px*py;

		  brix=realbrix-corr;
		  meas.brix=realbrix;
		  meas.brixk=brix;

		  meas.nck=n_brix[(uint16_t)(meas.brixk*200.0+400.0)];

		  //wypluj dane przez uart gdy sa gotowe
		  uart_out[0]='!';
		  uart_out[1]='M';
		  memcpy(&uart_out[2], (uint8_t*)&(meas.brix), 4);
		  memcpy(&uart_out[6], (uint8_t*)&(meas.brixk), 4);
		  memcpy(&uart_out[10], (uint8_t*)&(meas.led_curr), 4);
		  memcpy(&uart_out[14], (uint8_t*)&(meas.nc), 4);
		  memcpy(&uart_out[18], (uint8_t*)&(meas.nck), 4);
		  memcpy(&uart_out[22], (uint8_t*)&(meas.temp), 4);
		  memcpy(&uart_out[26], (uint8_t*)&(cal_data.D), 4);
		  memcpy(&uart_out[30], (uint8_t*)&(cal_data.f), 4);
		  memcpy(&uart_out[34], (uint8_t*)&(cal_data.serial), 2);
		  memcpy(&uart_out[36], (uint8_t*)&(cal_data.temp_corr), 4);
		  memcpy(&uart_out[40], (uint8_t*)&(cal_data.x_corr), 4);

		  uint8_t sum=0;

		  for(uint8_t i=0; i<44; i++)
			  sum+=uart_out[i];

		  uart_out[44]=sum;

		  //wylaczone na czas debugowania
		  //sprintf(uart_out, "brixk=%d, num_pix=%d===", (int16_t)(meas.brixk*100.0), (uint16_t)(meas.num_pix*10));
		  HAL_UART_Transmit(&huart1, uart_out, 45, 100);
	  }

	  //jezeli w miedzyczasie wpadly dane kalibracyjne przez uart - przepisz je do struktury i zapisz w eepromie
	  if(cal_data_save_pending)
	  {
		  uint8_t sum=0;

		  for(uint8_t i=0; i<20; i++)
			  sum+=uart_in[i];

		  if(uart_in[0]=='!' && uart_in[1]=='C' && sum==uart_in[20])
		  {
			  memcpy((uint8_t*)&(cal_data.serial), &uart_in[2], 2);
			  memcpy((uint8_t*)&(cal_data.f), &uart_in[4], 4);
			  memcpy((uint8_t*)&(cal_data.D), &uart_in[8], 4);
			  memcpy((uint8_t*)&(cal_data.temp_corr), &uart_in[12], 4);
			  memcpy((uint8_t*)&(cal_data.x_corr), &uart_in[16], 4);
			  memset(uart_in, 0, 25);
			  saveCalData();
		  }

		  cal_data_save_pending=0;
		  HAL_UART_Receive_IT(&huart1, uart_in, 21);
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_CSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.CSIState = RCC_CSI_ON;
  RCC_OscInitStruct.CSICalibrationValue = RCC_CSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 3;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 12;
  PeriphClkInitStruct.PLL2.PLL2N = 288;
  PeriphClkInitStruct.PLL2.PLL2P = 4;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
