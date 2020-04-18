
#include "main.h"
#include "usb_device.h"

#include "device_conf.h"
#include "curemisc.h"
#include "curebuffer.h"
#include "usbd_midi_if.h"
#include "usbd_midi.h"

#define HYST 50

ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

PCD_HandleTypeDef hpcd_USB_FS;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
static void MX_USB_PCD_Init(void);

uint16_t ADC_val[8];
uint16_t ADC_val_old[8];
uint8_t dial[8];
uint8_t dial_mapping[8] = {1, 2, 3, 4, 5, 6, 7, 0};
uint16_t dial_div[8] = {32, 32, 32, 32, 32, 32, 32, 256};

uint8_t midi_packet[4] = {0x0B, 0xB0, 0x00, 0x00};

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USB_MIDI_INIT();

  HAL_ADC_Start_DMA(&hadc, ADC_val, 8);


  if(FUNC_ERROR == midiInit() ){
	  while(1){
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, SET);
		  HAL_Delay(500);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, RESET);
		  HAL_Delay(500);
	  }
  }

  //Wait usb configuration.
  while(1){

	  if(USBD_STATE_CONFIGURED == hUsbDeviceFS.dev_state){
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, SET);
		  break;
	  }else{
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, RESET);
	  }
  }

  while (1)
  {
    while(1){
      if(USBD_STATE_CONFIGURED == hUsbDeviceFS.dev_state){
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, SET);
        break;
      }else{
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, SET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, RESET);
        HAL_Delay(200);
      }
    }

    midiProcess();
    for(uint8_t i = 0; i <= 7; i++){
      if(ADC_val[dial_mapping[i]] >= (ADC_val_old[dial_mapping[i]] + HYST) || ADC_val[dial_mapping[i]] <= (ADC_val_old[dial_mapping[i]] - HYST)){
        ADC_val_old[dial_mapping[i]] = ADC_val[dial_mapping[i]];
        dial[i] = (uint8_t)((ADC_val[dial_mapping[i]]/dial_div[i]) & 0x7F);
        midi_packet[3] = dial[i];
        midi_packet[1] = 0xB0 + i;
        sendMidiMessage(midi_packet, 4);
        USBD_MIDI_SendPacket();
        HAL_Delay(2);
      }
    }
    //USBD_MIDI_SendData(&hUsbDeviceFS, midi_packet, 4);


  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

}

static void MX_ADC_Init(void)
{

  ADC_ChannelConfTypeDef sConfig = {0};

  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  HAL_ADC_Init(&hadc);

  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  sConfig.Channel = ADC_CHANNEL_1;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_2;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_3;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_4;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_5;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_6;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_7;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

  sConfig.Channel = ADC_CHANNEL_8;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);
}

static void MX_USB_PCD_Init(void)
{

  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  HAL_PCD_Init(&hpcd_USB_FS);
}


static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}


static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  while(1){
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,1);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,0);
    HAL_Delay(100);
  }
}
