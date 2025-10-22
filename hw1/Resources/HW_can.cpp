/**
 *******************************************************************************
 * @file      :HW_can.cpp
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
#include "HW_can.hpp"
#include "stdint.h"

/* Private macro -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static CAN_RxHeaderTypeDef rx_header;
static uint8_t can_rx_data[8];
uint32_t pTxMailbox;

/* External variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief
 * @param        *hcan:
 * @retval       None
 * @note        None
 */
void CanFilter_Init() {
  CAN_FilterTypeDef canfilter;

  canfilter.FilterMode = CAN_FILTERMODE_IDLIST;
  canfilter.FilterScale = CAN_FILTERSCALE_16BIT;

  canfilter.FilterActivation = ENABLE;
  canfilter.SlaveStartFilterBank = 14;
  canfilter.FilterFIFOAssignment = CAN_FilterFIFO0;

  canfilter.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilter.FilterScale = CAN_FILTERSCALE_32BIT;
  canfilter.FilterIdHigh = 0x0000;
  canfilter.FilterIdLow = 0x0000;
  canfilter.FilterMaskIdHigh = 0x0000;
  canfilter.FilterMaskIdLow = 0x0000;
  canfilter.FilterBank = 0;
  canfilter.FilterActivation = ENABLE;
  if (HAL_CAN_ConfigFilter(&hcan, &canfilter) != HAL_OK) {
    Error_Handler();
  }
}

uint32_t can_rec_times = 0;
uint32_t can_success_times = 0;
uint32_t can_receive_data = 0;

/**
 * @brief   CAN�?�?的回调函数，全部数据解析都在该函数中
 * @param   hcan为CAN句柄
 * @retval  none
 * @note
 **/

/*
//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  can_rec_times++;

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, can_rx_data) ==
      HAL_OK) // 获得接收到的数据头和数据
  {
    if (rx_header.StdId == 0x1FF) {
      can_success_times++;
      can_receive_data=(can_rx_data[0] << 24) | (can_rx_data[1] << 16) | (can_rx_data[2] << 8) | can_rx_data[3];
    }
  }
  HAL_CAN_ActivateNotification(
      hcan, CAN_IT_RX_FIFO0_MSG_PENDING); // 再�?�使能FIFO0接收�?�?
}
*/


/**
 * @brief   向can总线发送数�?，抄官方�?
 * @param   hcan为CAN句柄
 * @param	msg为发送数组�?�地址
 * @param	id为发送报文id
 * @param	len为发送数�?长度（字节数�?
 * @retval  none
 * @note    主控发送都是len=8字节，再加上帧间�?3位，理�?�上can总线1ms最多传�?9�?
 **/
void CAN_Send_Msg(CAN_HandleTypeDef *hcan, uint8_t *msg, uint32_t id,
                  uint8_t len) {
  CAN_TxHeaderTypeDef TxMessageHeader = {0};
  TxMessageHeader.StdId = id;
  TxMessageHeader.IDE = CAN_ID_STD;
  TxMessageHeader.RTR = CAN_RTR_DATA;
  TxMessageHeader.DLC = len;
  if (HAL_CAN_AddTxMessage(hcan, &TxMessageHeader, msg, &pTxMailbox) !=
      HAL_OK) {
  }
}
