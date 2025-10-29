/**
 *******************************************************************************
 * @file      :HW_fdcan.cpp
 * @brief     :
 * @history   :
 *  Version     Date            Author          Note
 *  V0.9.0      yyyy-mm-dd      <author>        1. <note>
 *******************************************************************************
 * @attention :
 *******************************************************************************
 *  Copyright (c) 2023 Hello World Team, Zhejiang University.
 *  All Rights Reserved.
 *******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "HW_fdcan.hpp"
#include "stdint.h"
#include "gm6020.hpp"
/* Private macro -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static FDCAN_RxHeaderTypeDef rx_header1, rx_header3;
static uint8_t can1_rx_data[8], can3_rx_data[8];
uint32_t pTxMailbox;
extern GM6020 motor1;
extern FDCAN_HandleTypeDef hfdcan1;

/* External variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief
 * @param        *hcan:
 * @retval       None
 * @note        None
 */
void FdcanFilterInit(FDCAN_HandleTypeDef *hfdcan, uint32_t fifo) {

  FDCAN_FilterTypeDef filter_config{
      .IdType = FDCAN_STANDARD_ID,
      .FilterIndex = 0,
      .FilterType = FDCAN_FILTER_MASK,
      .FilterID1 = 0x000,
      .FilterID2 = 0x000,
      .RxBufferIndex = 0,
      .IsCalibrationMsg = 0,
  };
  filter_config.FilterConfig = fifo;

  if (HAL_FDCAN_ConfigFilter(hfdcan, &filter_config) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief   CAN�?�?的回调函数，全部数据解析都在该函数中
 * @param   hcan为CAN句柄
 * @retval  none
 * @note
 **/
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,uint32_t RxFifo0ITs) {
  if (hfdcan == &hfdcan1) {
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header1,can1_rx_data) == HAL_OK) // 获得接收到的数据头和数据
    {
      if (rx_header1.Identifier == 0x205) { // 帧头校验
        motor1.decode(can1_rx_data);
        // 校验通过进�?�具体数�?处理
      }
    }
  } else if (hfdcan == &hfdcan2) {
  }
  HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                 0); // 再�?�使能FIFO0接收�?�?
}

/**
 * @brief   CAN�?�?的回调函数，全部数据解析都在该函数中
 * @param   hcan为CAN句柄
 * @retval  none
 * @note
 **/
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo1ITs) {
  if (hfdcan == &hfdcan3) {
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &rx_header3,
                               can3_rx_data) ==
        HAL_OK) // 获得接收到的数据头和数据
    {
      if (rx_header3.Identifier == 0x200) { // 帧头校验
        // 校验通过进�?�具体数�?处理
      }
    }
  }
  HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
                                 0); // 再�?�使能FIFO0接收�?�?
}

/**
 * @brief   向can总线发送数�?，抄官方�?
 * @param   hcan为CAN句柄
 * @param	msg为发送数组�?�地址
 * @param	id为发送报�?
 * @param	len为发送数�?长度（字节数�?
 * @retval  none
 * @note    主控发送都是len=8字节，再加上帧间�?3位，理�?�上can总线1ms最多传�?9�?
 **/
void FdcanSendMsg(FDCAN_HandleTypeDef *hfdcan, uint8_t *msg, uint32_t id,
                  uint8_t len) {
  FDCAN_TxHeaderTypeDef TxMessageHeader = {0};

  TxMessageHeader.Identifier = id;                // 32位ID
  TxMessageHeader.IdType = FDCAN_STANDARD_ID;     // 标准ID
  TxMessageHeader.TxFrameType = FDCAN_DATA_FRAME; // 数据�?
  TxMessageHeader.DataLength = len;               // 数据长度
  TxMessageHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  TxMessageHeader.BitRateSwitch = FDCAN_BRS_OFF;           // 关闭速率切换
  TxMessageHeader.FDFormat = FDCAN_CLASSIC_CAN;            // 传统的CAN模式
  TxMessageHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS; // 无发送事�?
  TxMessageHeader.MessageMarker = 0;
  if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxMessageHeader, msg) != HAL_OK) {
  }
}
