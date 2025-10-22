/* Includes ------------------------------------------------------------------*/
#include "stm32f103xb.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "main_task.hpp"
#include "math.h"
#include "tim.h"
#include "stdint.h"
#include "gpio.h"
#include "usart.h"
#include "can.h"
#include "HW_can.hpp"
#include "string.h"

uint32_t tick = 0; //��¼��Ƭ��Ŀǰ���е�ʱ��
float value =0.0f;  //����value
//uint8_t can_tx_data[8] = {0};

// ���巢�ͻ���
bool uart_tx_busy = false;
uint8_t uart_tx_buf[9];
uint8_t uart_rx_buffer[9]={0};     // ÿ��DMA����1�ֽ�
uint8_t uart_payload[6];  // ��Ч���ݻ�����

UartCommData uart_data_tx;  //���ڷ�������
UartCommData uart_data_rx;  //���ڽ�������

// ״̬������
static uint8_t uart_state = 0;
static uint8_t uart_index = 0;

//CAN���ݶ���
typedef struct
{
    uint32_t tick;
    float value1;
    uint8_t value2;
    bool flag1;
    bool flag2;
    bool flag3;
    bool flag4;
} CANCommData;

CANCommData can_data_tx;
CANCommData can_data_rx;

//CAN��������
void CAN_Encode(CANCommData *data, uint8_t *buf);
void CAN_Decode(uint8_t *buf, CANCommData *data);
void CAN_Send(CANCommData *data);


void RobotInit()
{
    tick = 0;
}

void MainInit(void)
{
    tick = 0;
    // ���� CAN��Loopback ģʽ��
    CanFilter_Init();
    HAL_CAN_Start(&hcan);
    HAL_CAN_ActivateNotification(&hcan,CAN_IT_RX_FIFO0_MSG_PENDING);

    //��������
    HAL_UART_Receive_DMA(&huart2, uart_rx_buffer, 1);
    uart_data_tx.tick = tick;
    uart_data_tx.value = 0.0f;

    HAL_TIM_Base_Start_IT(&htim4);  //������ʱ��4
}

void MainTask(void)
{
    tick++;
    value = sinf(tick / 1000.0f);

    if (tick % 1000 == 0)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); //���ĵ��
    }

    //�����շ�
    if (tick % 100 == 0 && !uart_tx_busy)
    {
        uart_data_tx.tick = tick;
        uart_data_tx.value = value;

        encode(&uart_data_tx, uart_tx_buf);
        uart_tx_busy = true;
        HAL_UART_Transmit_DMA(&huart1, uart_tx_buf, sizeof(uart_tx_buf));
      //HAL_UART_Transmit(&huart1, uart_tx_buf, sizeof(uart_tx_buf), 0xffff);

    }

    //CAN����
    if (tick % 10 == 1)
    {
        can_data_tx.tick = tick;
        can_data_tx.value1 = value;
        can_data_tx.value2 = (uint8_t)(tick & 0xFF);
        can_data_tx.flag1 = (tick / 200) % 2;
        can_data_tx.flag2 = 1;
        can_data_tx.flag3 = 0;
        can_data_tx.flag4 = 1;

        CAN_Send(&can_data_tx);
    }
    
}

//��ʱ���жϻص�����
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim4)
    {
        MainTask();
    }
}


//uart2���ջص�����
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart2)
    {
        //data process

        uint8_t byte = uart_rx_buffer[0];

        switch (uart_state)
        {
        case 0: // S0 �ȴ� 0xAA
            if (byte == 0xAA)
                uart_state = 1;
            else
                uart_state = 0;
            break;

        case 1: // S1 �ȴ� 0xBB
            if (byte == 0xBB)
                uart_state = 2;
            else
                uart_state = 0;
            break;

        case 2: // S2 �ȴ� 0xCC
            if (byte == 0xCC)
            {
                uart_state = 3;
                uart_index = 0;
            }
            else
                uart_state = 0;
            break;

        case 3: // S3 ������Ч���ݣ�6�ֽڣ�
            uart_payload[uart_index++] = byte;
            if (uart_index >= 6)
            {
                // ƴ����֡ 0xAA 0xBB 0xCC + 6�ֽ�
                uint8_t full_buf[9];
                full_buf[0] = 0xAA;
                full_buf[1] = 0xBB;
                full_buf[2] = 0xCC;
                memcpy(&full_buf[3], uart_payload, 6);

                if (decode(full_buf, &uart_data_rx))
                {
                    // ����ɹ���������ʾ
                    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
                }

                uart_state = 0; // �ص���ʼ״̬
            }
            break;

        default:
            uart_state = 0;
            break;
        }
        //decode(rx_byte);

        HAL_UART_Receive_DMA(&huart2,uart_rx_buffer,1);
    }
}

//uart1DMA������ɻص�
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uart_tx_busy = false;
    }
}


// CAN �����жϻص�
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

    if (rx_header.StdId == 0x100 && rx_header.DLC == 8)
    {
        CAN_Decode(rx_data, &can_data_rx);
    }
}

// CAN ���뺯��
void CAN_Encode(CANCommData *data, uint8_t *buf)
{
    uint32_t tick = data->tick;
    memcpy(&buf[0], &tick, 4);

    int16_t val = (int16_t)(data->value1 * 30000);
    buf[4] = (val >> 8) & 0xFF;
    buf[5] = (val)&0xFF;

    buf[6] = data->value2;
    buf[7] = (data->flag1 << 3) | (data->flag2 << 2) | (data->flag3 << 1) | (data->flag4);
}

// CAN ���뺯��
void CAN_Decode(uint8_t *buf, CANCommData *data)
{
    data->tick = ((uint32_t)buf[0]) |
                 ((uint32_t)buf[1] << 8) |
                 ((uint32_t)buf[2] << 16) |
                 ((uint32_t)buf[3] << 24);

    int16_t val = ((int16_t)buf[4] << 8) | buf[5];
    data->value1 = ((float)val) / 30000.0f;
    data->value2 = buf[6];

    uint8_t flags = buf[7];
    data->flag1 = (flags >> 3) & 1;
    data->flag2 = (flags >> 2) & 1;
    data->flag3 = (flags >> 1) & 1;
    data->flag4 = (flags >> 0) & 1;
}

// CAN ���ͺ���
void CAN_Send(CANCommData *data)
{
    CAN_TxHeaderTypeDef tx_header;
    tx_header.StdId = 0x100;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 8;

    uint8_t buf[8];
    CAN_Encode(data, buf);

    uint32_t mailbox;
    HAL_CAN_AddTxMessage(&hcan, &tx_header, buf, &mailbox);
}

//���ڱ���
void encode(UartCommData *data, uint8_t *buf)
{
    int16_t val = (int16_t)(data->value * 30000);

    buf[0] = 0xAA;
    buf[1] = 0xBB;
    buf[2] = 0xCC;

    buf[3] = (data->tick >> 24) & 0xFF;
    buf[4] = (data->tick >> 16) & 0xFF;
    buf[5] = (data->tick >> 8) & 0xFF;
    buf[6] = (data->tick >> 0) & 0xFF;

    buf[7] = (val >> 8) & 0xFF;
    buf[8] = (val >> 0) & 0xFF;
}

//���ڽ���
uint8_t decode(uint8_t *buf, UartCommData *data)
{
    // ֡ͷУ��
    if (buf[0] != 0xAA || buf[1] != 0xBB || buf[2] != 0xCC)
        return 0;  // ����֡

    data->tick = ((uint32_t)buf[3] << 24) |
                 ((uint32_t)buf[4] << 16) |
                 ((uint32_t)buf[5] << 8)  |
                 ((uint32_t)buf[6]);

    int16_t val = ((int16_t)buf[7] << 8) | buf[8];
    data->value = ((float)val) / 30000.0f;
    return 1; // �ɹ�
}
