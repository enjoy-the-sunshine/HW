/*
 * @Author: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @Date: 2025-10-25 16:34:00
 * @LastEditors: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @LastEditTime: 2025-10-29 20:01:12
 * @FilePath: \hy_hw2\Tasks\gm6020.hpp
 * @Description: ����Ĭ������,������`customMade`, ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef GM6020_HPP
#define GM6020_HPP

#include "stdint.h"
#include "algorithm"   // ���� std::clamp��C++17��
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
    uint32_t id_;   // ��������ID
    float angle_;   // �Ƕȣ���ǰʵ�����ǡ�ԭʼֵ/δ���㡱��
    float angle_offset_ {0.0f};   // ���ƫ��
    float vel_; // �ٶȣ���ǰʵ�����ǡ�ԭʼֵ/δ���㡱��
    float current_; // ʵ�������������ԭʼֵ��
    float temp_;    // �¶�
    float input_;   // ��Ҫ��������Ŀ�����
};

#endif
