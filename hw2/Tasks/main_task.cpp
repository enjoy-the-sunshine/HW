/*
 * @Author: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @Date: 2025-10-24 19:30:36
 * @LastEditors: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @LastEditTime: 2025-10-29 21:52:14
 * @FilePath: \hy_hw2\Tasks\main_task.cpp
 * @Description: ������ ���� GM6020 �ٶ�/λ�ñջ�ʾ����ʹ�� FDCAN ͨѶ
 */

/* Includes ------------------------------------------------------------------*/
#include <cmath>
#include <cstdint>
#include <cstring>
#include "pid.hpp"      // �ٶ�/λ�� PID
#include "gm6020.hpp"   // GM6020 �����װ������/״̬/���룩
#include "HW_fdcan.hpp" // FDCAN ��װ������FdcanSendMsg �ȣ�
#include "main_task.hpp"

extern "C" {
#include "main.h"
#include "fdcan.h"
#include "tim.h"
#include <cstdio>
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==================== ���ò��� ====================
static constexpr float Ts = 0.001375f; // ����ʱ�䣨s����Լ 727 Hz
GM6020 motor1(0x205);           // GM6020 ���󣬼ٶ����/ID ��Ϣͨ�����촫��

// PID ������ʾ��ֵ��
static PidParams pos_params
{ 
    .kp=10.0f, 
    .ki=0.0f,  
    .kd=0.2f, 
    .i_limit=3000.0f, 
    .out_limit=3.0f 
};
static PidParams vel_params
{ 
    .kp=10.0f, 
    .ki=0.5f, 
    .kd=0.1f, 
    .i_limit=5000.0f, 
    .out_limit=7000.0f 
};
static Pid pid_pos(pos_params); // λ�û�
static Pid pid_vel(vel_params); // �ٶȻ�

// ==================== ���ߺ��� ====================
// ���Ƕ����Ƶ� (-pi, pi]
static inline float wrap_pi(float x) {
    while (x >  M_PI) x -= 2.0f * M_PI;
    while (x <= -M_PI) x += 2.0f * M_PI;
    return x;
}

// ==================== ����ģʽ���� ====================
// CTRL_VEL_SIN: �ٶ����Ҳο���A*sin(2*pi*f*t)��
// CTRL_POS_SEQ: λ�����У���Ԥ��Ƕ����������л���
enum CtrlMode { CTRL_VEL_SIN = 0, CTRL_POS_SEQ = 1 };
static volatile CtrlMode mode = CTRL_POS_SEQ;

static constexpr float A_vel = 6.0f;   // �ٶ���� (rad/s)
static constexpr float f_vel = 0.05f;  // �ٶ�Ƶ�� (Hz)
static float t_running = 0.0f;

static const float pos_list[] = {
    -5.0f*M_PI/6.0f,  5.0f*M_PI/6.0f,
     M_PI/3.0f,       2.0f*M_PI/3.0f,
     M_PI/4.0f,      -1.0f*M_PI
};
static int   pos_idx = 0;
static float pos_ref = 0.0f;
static uint32_t tick = 0;
float cmd_current = 0.0f;
float vel_ref = 0.0f;  // ȫ�ֱ��������ڹ۲������ٶ�

static float pos_start = 0.0f;
static float pos_target = 0.0f;
static float t_transition = 0.0f;
static constexpr float TRANSITION_TIME = 3.0f; // 3�����


// ==================== FDCAN ���ͷ�װ ====================
// ����� 4 ����������Ŀ�������Э���������͡�
// ע�⣺���͵� CAN ID ��������Ҫ��һ�£��˴�ʹ�� FdcanSendMsg ��ʵ�֣���
static void SendGm6020Currents(FDCAN_HandleTypeDef *hfdcan,
                                GM6020 *m1 = nullptr,
                                GM6020 *m2 = nullptr,
                                GM6020 *m3 = nullptr,
                                GM6020 *m4 = nullptr)
{
    uint8_t data[8] = {0};

    if (m1) {
        int16_t I1 = (int16_t)m1->getInput();
        data[0] = (I1 >> 8) & 0xFF;
        data[1] = I1 & 0xFF;
    }
    if (m2) {
        int16_t I2 = (int16_t)m2->getInput();
        data[2] = (I2 >> 8) & 0xFF;
        data[3] = I2 & 0xFF;
    }
    if (m3) {
        int16_t I3 = (int16_t)m3->getInput();
        data[4] = (I3 >> 8) & 0xFF;
        data[5] = I3 & 0xFF;
    }
    if (m4) {
        int16_t I4 = (int16_t)m4->getInput();
        data[6] = (I4 >> 8) & 0xFF;
        data[7] = I4 & 0xFF;
    }

    // ͨ��ƽ̨��װ�������ͣ����ú����ڲ��� CAN ID / ֡��ʽ��
    FdcanSendMsg(hfdcan, data, 0x1FE, 8);
}

// ==================== �ӿ�/��ʼ�� ====================
extern "C" void RobotInit(void) {
    tick = 0;
    t_running = 0.0f;
    pos_idx = 0;
    pos_ref = 0.0f;
    pid_pos.reset();
    pid_vel.reset();
}

extern "C" void MainInit(void) {
    // ��ʼ�� FDCAN ���˲����������жϣ�ʾ����
    FdcanFilterInit(&hfdcan1, FDCAN_FILTER_TO_RXFIFO0);
    HAL_FDCAN_Start(&hfdcan1);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    // ������ʱ�� TIM6������ ControlLoopStep��
    HAL_TIM_Base_Start_IT(&htim6);
}

extern "C" void MainTask(void) {
    // ���ڴ˴����ʵʱ�����ģʽ�л��߼�
}

// ==================== ����ѭ�� ====================
extern "C" void ControlLoopStep() {
    tick++;
    t_running += Ts;

//    float cmd_current = 0.0f;

    if (mode == CTRL_VEL_SIN) {
        // �ٶ����Ҳο�
        vel_ref = A_vel * std::sinf(2.0f * M_PI * f_vel * t_running);
        //const float vel_ref = A_vel * std::sinf(2.0f * M_PI * f_vel * t_running);
        cmd_current = pid_vel.pidCalc(vel_ref, motor1.vel());
    } else {
        // λ�����У�ÿ 2s �л�����һ���趨�Ƕȣ�
        if ((tick % static_cast<uint32_t>(5.0f / Ts)) == 0) {
            //pos_ref = pos_list[pos_idx];
            //pos_idx = (pos_idx + 1) % (sizeof(pos_list)/sizeof(pos_list[0]));
            pos_start = motor1.angle();
            pos_target = pos_list[pos_idx];
            pos_idx = (pos_idx + 1) % (sizeof(pos_list) / sizeof(pos_list[0]));
            t_transition = 0.0f;
        }
        if (t_transition < TRANSITION_TIME) {
            t_transition += Ts;
            float alpha = t_transition / TRANSITION_TIME;
            pos_ref = pos_start + alpha * wrap_pi(pos_target - pos_start);
        }
        //const float pos_err = wrap_pi(pos_ref - motor1.angle());
        // λ�û������ٶȲο���������Ÿ������ pidCalc Լ����
        //const float vel_ref = pid_pos.pidCalc(0.0f, -pos_err);
        // λ�������㣺ƥ��������CCW�����壬ȷ��PID�����������ת��һ��
        const float pos_err = wrap_pi(pos_ref - motor1.angle());
        // λ�û������ٶȲο����Ƴ����ţ�����ת������ԭpidCalcΪ���趨ֵ-ʵ��ֵ�������踺�ţ�
        const float vel_ref = pid_pos.pidCalc(pos_err, 0.0f);
        cmd_current = pid_vel.pidCalc(vel_ref, motor1.vel());
    }

    // ������ָ��д�������󣬲�����
    motor1.setInput(cmd_current);
    SendGm6020Currents(&hfdcan1, &motor1);
}


// ==================== ��ʱ���жϣ�TIM6���ص� ====================
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM6) {
        ControlLoopStep();
    }
}

// ==================== ģʽ�л��ӿ� ====================
extern "C" void SetModeVelocitySine() {
    mode = CTRL_VEL_SIN;
    t_running = 0.0f;
    pid_pos.reset();
    pid_vel.reset();
}

extern "C" void SetModePositionSeq() {
    mode = CTRL_POS_SEQ;
    t_running = 0.0f;
    pid_pos.reset();
    pid_vel.reset();
}
