/*
 * @Author: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @Date: 2025-10-24 19:30:36
 * @LastEditors: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @LastEditTime: 2025-10-29 21:52:14
 * @FilePath: \hy_hw2\Tasks\main_task.cpp
 * @Description: 主任务 —— GM6020 速度/位置闭环示例，使用 FDCAN 通讯
 */

/* Includes ------------------------------------------------------------------*/
#include <cmath>
#include <cstdint>
#include <cstring>
#include "pid.hpp"      // 速度/位置 PID
#include "gm6020.hpp"   // GM6020 电机封装（解码/状态/输入）
#include "HW_fdcan.hpp" // FDCAN 封装函数（FdcanSendMsg 等）
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

// ==================== 配置参数 ====================
static constexpr float Ts = 0.001375f; // 采样时间（s），约 727 Hz
GM6020 motor1(0x205);           // GM6020 对象，假定电机/ID 信息通过构造传入

// PID 参数（示例值）
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
static Pid pid_pos(pos_params); // 位置环
static Pid pid_vel(vel_params); // 速度环

// ==================== 工具函数 ====================
// 将角度限制到 (-pi, pi]
static inline float wrap_pi(float x) {
    while (x >  M_PI) x -= 2.0f * M_PI;
    while (x <= -M_PI) x += 2.0f * M_PI;
    return x;
}

// ==================== 控制模式定义 ====================
// CTRL_VEL_SIN: 速度正弦参考（A*sin(2*pi*f*t)）
// CTRL_POS_SEQ: 位置序列（按预设角度在两秒内切换）
enum CtrlMode { CTRL_VEL_SIN = 0, CTRL_POS_SEQ = 1 };
static volatile CtrlMode mode = CTRL_POS_SEQ;

static constexpr float A_vel = 6.0f;   // 速度振幅 (rad/s)
static constexpr float f_vel = 0.05f;  // 速度频率 (Hz)
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
float vel_ref = 0.0f;  // 全局变量，用于观察期望速度

static float pos_start = 0.0f;
static float pos_target = 0.0f;
static float t_transition = 0.0f;
static constexpr float TRANSITION_TIME = 3.0f; // 3秒过渡


// ==================== FDCAN 发送封装 ====================
// 把最多 4 个电机对象的目标电流按协议打包并发送。
// 注意：发送的 CAN ID 与电机驱动要求一致（此处使用 FdcanSendMsg 的实现）。
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

    // 通过平台封装函数发送（检查该函数内部的 CAN ID / 帧格式）
    FdcanSendMsg(hfdcan, data, 0x1FE, 8);
}

// ==================== 接口/初始化 ====================
extern "C" void RobotInit(void) {
    tick = 0;
    t_running = 0.0f;
    pos_idx = 0;
    pos_ref = 0.0f;
    pid_pos.reset();
    pid_vel.reset();
}

extern "C" void MainInit(void) {
    // 初始化 FDCAN 过滤并启动接收中断（示例）
    FdcanFilterInit(&hfdcan1, FDCAN_FILTER_TO_RXFIFO0);
    HAL_FDCAN_Start(&hfdcan1);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    // 启动定时器 TIM6（用于 ControlLoopStep）
    HAL_TIM_Base_Start_IT(&htim6);
}

extern "C" void MainTask(void) {
    // 可在此处理非实时任务或模式切换逻辑
}

// ==================== 控制循环 ====================
extern "C" void ControlLoopStep() {
    tick++;
    t_running += Ts;

//    float cmd_current = 0.0f;

    if (mode == CTRL_VEL_SIN) {
        // 速度正弦参考
        vel_ref = A_vel * std::sinf(2.0f * M_PI * f_vel * t_running);
        //const float vel_ref = A_vel * std::sinf(2.0f * M_PI * f_vel * t_running);
        cmd_current = pid_vel.pidCalc(vel_ref, motor1.vel());
    } else {
        // 位置序列（每 2s 切换到下一个设定角度）
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
        // 位置环产生速度参考（反向符号根据你的 pidCalc 约定）
        //const float vel_ref = pid_pos.pidCalc(0.0f, -pos_err);
        // 位置误差计算：匹配电机正向（CCW）定义，确保PID输出方向与电机转向一致
        const float pos_err = wrap_pi(pos_ref - motor1.angle());
        // 位置环产生速度参考：移除负号，避免转向反向（若原pidCalc为“设定值-实际值”，无需负号）
        const float vel_ref = pid_pos.pidCalc(pos_err, 0.0f);
        cmd_current = pid_vel.pidCalc(vel_ref, motor1.vel());
    }

    // 将电流指令写入电机对象，并发送
    motor1.setInput(cmd_current);
    SendGm6020Currents(&hfdcan1, &motor1);
}


// ==================== 定时器中断（TIM6）回调 ====================
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM6) {
        ControlLoopStep();
    }
}

// ==================== 模式切换接口 ====================
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
