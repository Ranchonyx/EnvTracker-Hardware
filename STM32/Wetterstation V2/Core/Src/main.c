/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "CommHandler.h"
#include "LoRa_E5.h"
#include "SEN5X.h"
#include "SHT4x.h"
#include "HP20x.h"
#include "INA3221.h"
#include "command_processor.h"
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
#define UART1_RX_BUFFER_SIZE 512
#define UART2_RX_BUFFER_SIZE 512
#define SHORT_MSG_SIZE 512
uint8_t uart1_rx_buffer[UART1_RX_BUFFER_SIZE];
uint8_t uart2_rx_buffer[UART2_RX_BUFFER_SIZE];

uint8_t uart2_command_buffer[UART2_RX_BUFFER_SIZE]; // Zum Speichern von UART2-Kommandos
uint16_t uart2_command_length = 0; // Länge des aktuellen UART2-Kommandos

uint32_t measurement_interval_ms = 15000; // Standard: 1 Minute
uint8_t interval_minutes = 5; // Standardwert, falls nichts gesetzt ist
uint32_t last_measurement_time = 0;       // Zeitpunkt der letzten Messung
uint8_t sen5x_active = 0;                 // Flag, ob der SEN5X-Sensor aktiv ist

float bus_voltages[3];
float currents[3];
float shunt_resistance = 1; // Shunt-Widerstand in Ohm (z. B. 0.1 Ohm)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void ProcessUART1Data(uint8_t *buffer, uint16_t length);
void ProcessUART2Data(uint8_t *buffer, uint16_t length);

/* USER CODE END PFP */
void HandleIncomingCommands(void);
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    // Ändere huart2 auf den gewünschten UART-Handle
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
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
  /* USER CODE BEGIN 2 */
  HP20x_Init();
  SEN5X_Init();
  SHT4X_Init();
  INA3221_Init(&hi2c1);

  void Start_UART_DMA(void) {
      // UART1 DMA RX starten
	  HAL_UART_Receive_DMA(&huart1, lora_rx_buffer, LORA_BUFFER_SIZE);


      // UART2 DMA RX starten
      HAL_UART_Receive_DMA(&huart2, uart2_rx_buffer, UART2_RX_BUFFER_SIZE);

      // IDLE Interrupt für UART1 und UART2 aktivieren
      __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
      __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
  }

  Start_UART_DMA();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init(); // UART für LoRa
  MX_USART2_UART_Init(); // UART für Debug-Konsole

  // DMA RX starten und IDLE-Interrupt aktivieren
  HAL_UART_Receive_DMA(&huart1, uart1_rx_buffer, UART1_RX_BUFFER_SIZE);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

  // Kommunikationshandler initialisieren
  CommHandler_Init();

  // Sende ein Beispielkommando an das LoRa-Modul
  LoRa_E5_SetConfiguration("868,SF12,125,12,15,14,ON,OFF,OFF");
//  LoRa_E5_SendMessage("1234");
//  printf("Fin\n");

  while (1)
  {

	  SEN5X_StartMeasurement();
	  HAL_Delay(15000);
	  ReadAndSendSensorData(&hi2c1);
	  SEN5X_StopMeasurement();
	  HAL_Delay(75000);

//	  SEN5X_StopMeasurement();
//	  HAL_Delay(50000);

//	  HandleIncomingCommands(); // Überprüft und verarbeitet eingehende Befehle
//	         HAL_Delay(100); // Kleine Pause zur Reduzierung der CPU-Last
//	     }
//	 }
//
//	 /* USER CODE BEGIN 4 */
//	 void HandleIncomingCommands(void) {
//	     // Verarbeiten von LoRa-Befehlen
//	     LoRa_E5_ReceiveMessage(); // Diese Funktion ruft `ProcessCommand` direkt auf
//
//	     // Verarbeiten von UART2-Befehlen
//	     if (uart2_command_length > 0) { // Wenn ein Befehl empfangen wurde
//	         uart2_command_buffer[uart2_command_length] = '\0'; // Terminieren als String
//	         ProcessCommand((char *)uart2_command_buffer); // Befehl an den Command Processor weitergeben
//	         uart2_command_length = 0; // Länge zurücksetzen
//	     }
//	 }

//	 // UART2 IDLE-Interrupt-Callback
//	 void HAL_UART_IDLECallback(UART_HandleTypeDef *huart) {
//	     if (huart == &huart2) {
//	         uint16_t dma_index = UART2_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx); // Datenlänge berechnen
//	         memcpy(uart2_command_buffer, uart2_rx_buffer, dma_index); // Daten in den Befehlsbuffer kopieren
//	         uart2_command_length = dma_index; // Befehlslänge setzen
//	         HAL_UART_Receive_DMA(&huart2, uart2_rx_buffer, UART2_RX_BUFFER_SIZE); // DMA erneut starten
	     }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */


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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_10;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */
  void DMA_RX_CompleteCallback(UART_HandleTypeDef *huart, uint8_t *buffer, uint16_t buffer_size)
  {
      uint16_t dma_index = buffer_size - __HAL_DMA_GET_COUNTER(huart->hdmarx);

      if (huart == &huart1)
      {
          // UART1-Daten verarbeiten
          ProcessUART1Data(buffer, dma_index);
      } else if (huart == &huart2)
      {
          // UART2-Daten verarbeiten
          ProcessUART2Data(buffer, dma_index);
    }
  }


  void ProcessUART1Data(uint8_t *buffer, uint16_t length) {
      for (uint16_t i = 0; i < length; i++) {
          printf("UART1: %c\n", buffer[i]);
      }
  }

  void ProcessUART2Data(uint8_t *buffer, uint16_t length) {
      for (uint16_t i = 0; i < length; i++) {
          printf("UART2: %c\n", buffer[i]);
      }
  }

  void HandleUART1Data(uint8_t *buffer, uint16_t length) {
      for (uint16_t i = 0; i < length; i++) {
          printf("UART1 received: %c\n", buffer[i]); // Ausgabe der empfangenen Daten
      }
  }

  // Verarbeitung von Daten, die über UART2 empfangen wurden
  void HandleUART2Data(uint8_t *buffer, uint16_t length) {
      static uint16_t last_position = 0;
      uint16_t current_position = UART2_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);

      if (current_position != last_position) {
          if (current_position > last_position) {
              // Normale Verarbeitung, wenn der Puffer nicht umschlägt
              for (uint16_t i = last_position; i < current_position; i++) {
                  printf("UART2: %c\n", buffer[i]);
              }
          } else {
              // Puffer schlägt um (Wraparound)
              for (uint16_t i = last_position; i < UART2_RX_BUFFER_SIZE; i++) {
                  printf("UART2: %c\n", buffer[i]);
              }
              for (uint16_t i = 0; i < current_position; i++) {
                  printf("UART2: %c\n", buffer[i]);
              }
          }

          // Aktualisiere die letzte Position
          last_position = current_position;
      }
  }



  void ReadAndSendSensorData(I2C_HandleTypeDef *hi2c) {
      // Sensorwerte
      float hp20x_temp = 0, hp20x_press = 0, hp20x_alt = 0;
      float sen5X_pm1p0 = 0, sen5X_pm2p5 = 0, sen5X_pm4p0 = 0, sen5X_pm10 = 0;
      float sen5X_humidity = 0, sen5X_temp = 0, sen5X_voc = 0, sen5X_nox = 0;
      float sht4X_temp = 0, sht4X_humidity = 0;
      float bus_voltages[3] = {0}, currents[3] = {0};

      // Sensoren auslesen
      HP20x_ReadAllMeasuredValues(&hp20x_temp, &hp20x_press, &hp20x_alt);
      SEN5X_ReadAllMeasuredValues(&sen5X_pm1p0, &sen5X_pm2p5, &sen5X_pm4p0, &sen5X_pm10,
                                  &sen5X_humidity, &sen5X_temp, &sen5X_voc, &sen5X_nox);
      SHT4X_MeasureHighPrecision(&sht4X_temp, &sht4X_humidity);

      // INA3221 auslesen
      if (INA3221_ReadAll(&hi2c1, shunt_resistance, bus_voltages, currents) != HAL_OK) {
          // Fehler beim Auslesen der INA3221-Daten
          for (int i = 0; i < 3; i++) {
              bus_voltages[i] = -1.0; // Fehlerwert
              currents[i] = -1.0;
          }
      }
      currents[2] = currents[2] - currents[1];

      HP20x_IIC_WriteCmd(HP20X_CMD_SOFT_RST); // Reset command can be used to power off HP206C

      // Nachricht formatieren
      char short_msg[SHORT_MSG_SIZE];
      snprintf(short_msg, sizeof(short_msg),
               "HP20x:%.2f,%.2f,%.2f;SEN55:%.1f,%.1f,%.1f,%.1f,%.2f,%.2f,%.1f,%.1f;SHT45:%.2f,%.2f;INA3221:%.3f,%.3f,%.3f,%.3f,%.3f,%.3f;\r\n",
               hp20x_temp, hp20x_press, hp20x_alt,
               sen5X_pm1p0, sen5X_pm2p5, sen5X_pm4p0, sen5X_pm10,
               sen5X_humidity, sen5X_temp, sen5X_voc, sen5X_nox,
               sht4X_temp, sht4X_humidity,
               bus_voltages[0], currents[0],
               bus_voltages[1], currents[1],
               bus_voltages[2], currents[2]);


      // Nachricht über LoRa senden
      LoRa_E5_SendMessage(short_msg);

      // Debug-Ausgabe
      printf("Original Message: %s\n", short_msg);

  }
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
