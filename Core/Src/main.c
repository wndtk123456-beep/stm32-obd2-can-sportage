/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
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
CAN_HandleTypeDef hcan;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, 10);
    return len;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
    {
        /* 수신 데이터 Shape: RxData [8] (1D Array, 상대방이 쏴준 8바이트 날것의 데이터) */

        // OBD-II ECU response
        if (RxHeader.StdId == 0x7E8 && RxData[1] == 0x41)
        {
            if (RxData[2] == 0x0C) { // RPM
                uint16_t raw_rpm = (RxData[3] << 8) | RxData[4]; /* Shape: [1] (스칼라, 16비트 통합) */
                int real_rpm = raw_rpm / 4;                      /* Shape: [1] (스칼라, 실제 RPM 값) */
                printf("RPM=%d\r\n", real_rpm);
            }
            else if (RxData[2] == 0x0D) { // 속도
                int real_speed = RxData[3];                      /* Shape: [1] (스칼라, 실제 속도 km/h) */
                printf("SPEED=%d\r\n", real_speed);
            }
            else if (RxData[2] == 0x11) {
                            /* Shape: RxData[3] [1] (스칼라, 0~255 범위의 8비트 날것의 데이터) */

                            // 공식: A * 100 / 255 (0~100% 백분율로 변환)
            int real_throttle = (RxData[3] * 100) / 255;

                            /* Shape: real_throttle [1] (스칼라, 실제 쓰로틀 개도량 % 값) */

            printf("THROTTLE=%d\r\n", real_throttle);
            }
                            /* Shape: 출력 문자열 [Variable Length] (시리얼 텍스트 스트림) */
            else if (RxData[2] == 0x2F) { // 연료계 잔량
                int real_fuel = (RxData[3] * 100) / 255;         /* Shape: [1] (스칼라, 연료 백분율 %) */
                printf("FUEL=%d\r\n", real_fuel);
            }
            else if (RxData[2] == 0x10) { // 공기 흡입량 (연비 계산용)
                uint16_t raw_maf = (RxData[3] << 8) | RxData[4]; /* Shape: [1] (스칼라, 16비트 통합) */
                int real_maf = raw_maf / 100;                    /* Shape: [1] (스칼라, 공기 흡입량 g/s) */
                printf("MAF=%d\r\n", real_maf);
            }
        }

        // Experimental gear broadcast candidate found during vehicle analysis.
        else if (RxHeader.StdId == 0x43F)
        {
            uint8_t gear_val = RxData[0]; /* Shape: [1] (스칼라, 첫 번째 바이트 기어 데이터) */

            if (gear_val == 0x00) printf("GEAR=P\r\n");
            else if (gear_val == 0x07) printf("GEAR=R\r\n");
            else if (gear_val == 0x06) printf("GEAR=N\r\n");
            else if (gear_val == 0x05) printf("GEAR=D\r\n");
            // Vehicle model years may use different values. Log raw data first.
            // else printf("GEAR_RAW=%02X\r\n", gear_val);
        }


    }
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
  MX_CAN_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  CAN_FilterTypeDef sFilterConfig;
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

  /* Accept all frames for OBD-II responses and broadcast ID analysis. */
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

  /* OBD-II functional request header */
  TxHeader.StdId = 0x7DF;
  TxHeader.ExtId = 0x01;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.DLC = 8;               // 보낼 데이터 길이 8바이트
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
    {
        uint32_t TxMailbox;
        uint8_t TxData[8] = {0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        /* Shape: TxData [8] (1D Array, 송신용 질문 페이로드) */

        /* ----- 1. RPM 질문 (0x0C) ----- */
        TxData[2] = 0x0C;
        HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
        HAL_Delay(100);

        /* ----- 2. 속도 질문 (0x0D) ----- */
        TxData[2] = 0x0D;
        HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
        HAL_Delay(100);

        /* ----- 3. 연료계 질문 (0x2F) ----- */
        TxData[2] = 0x2F;
        HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
        HAL_Delay(100);

        /* ----- 4. 공기 흡입량(MAF) 질문 (0x10) - 나중에 연비 계산용 ----- */
        TxData[2] = 0x10;
        HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
        HAL_Delay(100);
        /* ----- 5. 쓰로틀(Throttle) 질문 쏘기 (0x11) 추가 ----- */
        TxData[2] = 0x11; // 세 번째 칸을 쓰로틀 코드로 변경
        HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox);
            /* Shape: CAN 메시지 송신 (ID: 0x7DF, 데이터: TxData) */

        HAL_Delay(100); // 자동차가 대답하고 처리할 시간 확보 (0.1초)

        /* Gear candidate 0x43F is a broadcast frame and is not requested here. */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 6;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
