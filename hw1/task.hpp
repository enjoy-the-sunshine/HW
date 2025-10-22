


#ifndef TASK_HPP
#define TASK_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stdbool.h"

// 串口通信数据结构
typedef struct {
    uint32_t tick;
    float value;
} UartCommData;

// 接收状态定义
typedef enum {
    RX_STATE_SYNC1,    // 等待0xAA
    RX_STATE_SYNC2,    // 等待0xBB
    RX_STATE_SYNC3,    // 等待0xCC
    RX_STATE_DATA      // 接收数据部分
} uart_rx_state_t;

// 函数声明
void maininit();
void maintask();
void uart_init();
void uart_encode_packet(const UartCommData* data, uint8_t* packet);
bool uart_decode_packet(const uint8_t* packet, UartCommData* data);
void uart_tx_task();
void uart_rx_task();
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void send_can_message();
#ifdef __cplusplus
}
#endif

#endif