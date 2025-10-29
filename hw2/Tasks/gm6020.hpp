/*
 * @Author: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @Date: 2025-10-25 16:34:00
 * @LastEditors: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @LastEditTime: 2025-10-29 20:01:12
 * @FilePath: \hy_hw2\Tasks\gm6020.hpp
 * @Description: 锟斤拷锟斤拷默锟斤拷锟斤拷锟斤拷,锟斤拷锟斤拷锟斤拷`customMade`, 锟斤拷koroFileHeader锟介看锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef GM6020_HPP
#define GM6020_HPP

#include "stdint.h"
#include "algorithm"   // 锟斤拷锟斤拷 std::clamp锟斤拷C++17锟斤拷
#include "math.h"

class GM6020
{
public:
    explicit GM6020(uint32_t id = 0x205);

    uint32_t txId(void);
    uint32_t rxId(void);

    float angle(void);
    float vel(void);
    float current(void);
    float temp(void);

    void setInput(float current);
    float getInput(void) const;

    void decode(const uint8_t* data);
    void encode(uint8_t* tx_buf);

private:
    uint32_t id_;   // 本机接收ID
    float angle_;   // 角度（当前实现里是“原始值/未换算”）
    float angle_offset_ {0.0f};   // 解缠偏移
    float vel_; // 速度（当前实现里是“原始值/未换算”）
    float current_; // 实际相电流反馈（原始值）
    float temp_;    // 温度
    float input_;   // 你要发给它的目标电流
};

#endif
