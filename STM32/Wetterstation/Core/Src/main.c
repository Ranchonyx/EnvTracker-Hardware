/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Erik Lauter.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "HP20x.h"
#include "SHT4x.h"
#include "SEN5x.h"
#include "INA3221.h"
#include "crc.h"
#include "kalman.h"
#include "command_processor.h"
#include "LoRa_E5.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef unsigned int    uint;
typedef unsigned char   uchar;
typedef unsigned long   ulong;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KALMAN_INIT_COUNT 3 // Anzahl der Messwerte zur Initialisierung des Kalman-Filters
#define MAX_COMMAND_LENGTH 512
#define DEFAULT_INTERVAL_MINUTES 10
#define LOOP_DELAY_MS 10 // Delay in milliseconds for each loop iteration
#define LOOP_COUNT (DEFAULT_INTERVAL_MINUTES * 60 * 1000 / LOOP_DELAY_MS + 6000) // Number of iterations for the desired interval
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
KalmanFilter t_filter; // Temperature filter
KalmanFilter p_filter; // Pressure filter
KalmanFilter a_filter; // Altitude filter

char uart1_rx_buffer[MAX_COMMAND_LENGTH];
char uart2_rx_buffer[MAX_COMMAND_LENGTH];
uint8_t uart1_rx_index = 0;
uint8_t uart2_rx_index = 0;

uint8_t interval_minutes = DEFAULT_INTERVAL_MINUTES;
volatile uint32_t loop_counter = 0;
volatile uint8_t uart1_command_ready = 0;
volatile uint8_t uart2_command_ready = 0;
uint8_t rx_buff1[1]; // UART reception buffer for UART1
uint8_t rx_buff2[1]; // UART reception buffer for UART2



float bus_voltages[3];
float currents[3];
float shunt_resistance = 1; // Shunt-Widerstand in Ohm (z. B. 0.1 Ohm)

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HP20x_Init(void);
void HP20x_ReadSensorData(float* temperature, float* pressure, float* altitude);
float HP20x_ReadTemperature(void);
float HP20x_ReadPressure(void);
float HP20x_ReadAltitude(void);
void HP20x_IIC_WriteCmd(uint8_t cmd);
ulong HP20x_IIC_ReadData(uint8_t cmd);

void SEN5X_Init(void);
void SEN5X_ReadSensorData(float* pm1p0, float* pm2p5, float* pm4p0, float* pm10, float* humidity, float* temperature, float* voc_index, float* nox_index);

void SHT4X_Init(void);
void SHT4X_ReadSensorData(float* temperature, float* humidity);

void Transmit_AllSensorData(void);
void Enter_SleepMode(void);
void WakeUp_FromSleep(void);
void ProcessCommand(char* command);

void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
void RTC_GetTime(uint8_t* hours, uint8_t* minutes, uint8_t* seconds);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Transmit All Data ---------------------------------------------------------*/

void Transmit_AllSensorData(void) {
    float hp20x_temp, hp20x_press, hp20x_alt;
    float sen5X_pm1p0, sen5X_pm2p5, sen5X_pm4p0, sen5X_pm10, sen5X_humidity, sen5X_temp, sen5X_voc, sen5X_nox;
    float sht4X_temp, sht4X_humidity;
    float bus_voltages[3], currents[3];
//    float shunt_resistance = 0.001; // Shunt-Widerstand in Ohm

    // Read data from all sensors
    HP20x_ReadAllMeasuredValues(&hp20x_temp, &hp20x_press, &hp20x_alt);
    SHT4X_MeasureHighPrecision(&sht4X_temp, &sht4X_humidity);
    SEN5X_ReadAllMeasuredValues(&sen5X_pm1p0, &sen5X_pm2p5, &sen5X_pm4p0, &sen5X_pm10, &sen5X_humidity, &sen5X_temp, &sen5X_voc, &sen5X_nox);

    // Read INA3221 data
    if (INA3221_ReadAll(&hi2c1, shunt_resistance, bus_voltages, currents) != HAL_OK) {
        // Fehler beim Auslesen der INA3221-Daten
        for (int i = 0; i < 3; i++) {
            bus_voltages[i] = -1.0; // Fehlerwert
            currents[i] = -1.0;
        }
    }

    // Turn off sensors to save power
    HP20x_IIC_WriteCmd(HP20X_CMD_SOFT_RST); // Reset command can be used to power off HP206C

    // Format and transmit all sensor data in a single short string
    char short_msg[512];
    snprintf(short_msg, sizeof(short_msg),
            "HP20x:%.2f,%.2f,%.2f;SEN55:%.1f,%.1f,%.1f,%.1f,%.2f,%.2f,%.1f,%.1f;SHT45:%.2f,%.2f;INA3221:%.3f,%.3f,%.3f,%.3f,%.3f,%.3f;\r\n",
             hp20x_temp, hp20x_press, hp20x_alt,
             sen5X_pm1p0, sen5X_pm2p5, sen5X_pm4p0, sen5X_pm10, sen5X_humidity, sen5X_temp, sen5X_voc, sen5X_nox,
             sht4X_temp, sht4X_humidity,
             bus_voltages[0], currents[0], bus_voltages[1], currents[1], bus_voltages[2], currents[2]);
//    HAL_UART_Transmit(&huart1, (uint8_t*)short_msg, strlen(short_msg), HAL_MAX_DELAY);
//    if (!LoRa_E5_Send(short_msg)) {
//        HAL_UART_Transmit(&huart1, (uint8_t *)"LoRa transmission failed!\r\n", 27, HAL_MAX_DELAY);
//    }

    // Debug output with detailed information
    char debug_msg[1024];
    snprintf(debug_msg, sizeof(debug_msg),
             "HP20x - Temperature: %.2f C, Pressure: %.2f hPa, Altitude: %.2f m; "
             "SEN55 - PM1.0: %.1f \u00b5g/m\u00b3, PM2.5: %.1f \u00b5g/m\u00b3, PM4.0: %.1f \u00b5g/m\u00b3, PM10: %.1f \u00b5g/m\u00b3, Humidity: %.2f %%, Temperature: %.2f C, VOC: %.1f, NOx: %.1f; "
             "SHT45 - Temperature: %.2f C, Humidity: %.2f %% "
             "INA3221 - Bus1: %.3f V, Current1: %.3f A; Bus2: %.3f V, Current2: %.3f A; Bus3: %.3f V, Current3: %.3f A\r\n",
             hp20x_temp, hp20x_press, hp20x_alt,
             sen5X_pm1p0, sen5X_pm2p5, sen5X_pm4p0, sen5X_pm10, sen5X_humidity, sen5X_temp, sen5X_voc, sen5X_nox,
             sht4X_temp, sht4X_humidity,
             bus_voltages[0], currents[0], bus_voltages[1], currents[1], bus_voltages[2], currents[2]);
    HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);
}


/* Transmit All Data END------------------------------------------------------*/

void Enter_SleepMode(void) {
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void WakeUp_FromSleep(void) {
    SystemClock_Config();
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

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  HP20x_Init();
  SEN5X_Init();
  SHT4X_Init();
  printf("INIT\n");
  INA3221_Init(&hi2c1);

  HAL_UART_Receive_IT(&huart1, rx_buff1, 1);
  HAL_UART_Receive_IT(&huart2, rx_buff2, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  while (1)
  {
	SEN5X_StartMeasurement();
    HAL_Delay(10000);
	Transmit_AllSensorData();
//    SEN5X_StopMeasurement();
//    HAL_Delay(1000);
/*    loop_counter++;
    if(loop_counter + 6000 == LOOP_COUNT) {
    	SEN5X_StartMeasurement();
    }


    if (loop_counter >= LOOP_COUNT) {
        loop_counter = 0;
        Transmit_AllSensorData();
        SEN5X_StopMeasurement();
    }

    if (uart1_command_ready) {
        ProcessCommand(uart1_rx_buffer);
        uart1_command_ready = 0;
    }

    if (uart2_command_ready) {
        ProcessCommand(uart2_rx_buffer);
        uart2_command_ready = 0;
    }
*/
//    HAL_Delay(LOOP_DELAY_MS);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Check for UART commands
    // UART interrupts will handle command processing
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        uart1_rx_buffer[uart1_rx_index++] = (char)rx_buff1[0];
        if (uart1_rx_buffer[uart1_rx_index - 1] == '\n') {
            uart1_rx_buffer[uart1_rx_index - 1] = '\0';
            uart1_command_ready = 1;
            uart1_rx_index = 0;
        }
        HAL_UART_Receive_IT(&huart1, rx_buff1, 1);
    } else if (huart->Instance == USART2) {
        uart2_rx_buffer[uart2_rx_index++] = (char)rx_buff2[0];
        if (uart2_rx_buffer[uart2_rx_index - 1] == '\n') {
            uart2_rx_buffer[uart2_rx_index - 1] = '\0';
            uart2_command_ready = 1;
            uart2_rx_index = 0;
        }
        HAL_UART_Receive_IT(&huart2, rx_buff2, 1);
    }
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
/*  RTC_AlarmTypeDef sAlarm;
  HAL_RTC_GetAlarm(hrtc,&sAlarm,RTC_ALARM_A,FORMAT_BIN);
  if(sAlarm.AlarmTime.Seconds>58) {
    sAlarm.AlarmTime.Seconds=0;
  }else{
    sAlarm.AlarmTime.Seconds=sAlarm.AlarmTime.Seconds+1;
  }
    while(HAL_RTC_SetAlarm_IT(hrtc, &sAlarm, FORMAT_BIN)!=HAL_OK){}
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
*/}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
