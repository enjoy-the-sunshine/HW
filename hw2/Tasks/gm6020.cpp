#include "gm6020.hpp"

GM6020::GM6020(uint32_t id)
    : id_(id), angle_(0.0f), vel_(0.0f), current_(0.0f), temp_(0.0f), input_(0.0f)
{
}

uint32_t GM6020::txId(void)
{
    return 0x1FF;  // 常见“控制一组电机”的下发ID
}

uint32_t GM6020::rxId(void)
{
    return id_; // 本电机的反馈ID
}


float GM6020::angle(void)
{
    return angle_;
}

float GM6020::vel(void)
{
    return vel_;
}

float GM6020::current(void)
{
    return current_;
}

float GM6020::temp(void)
{
    return temp_;
}

void GM6020::setInput(float current)
{
#if __cplusplus >= 201703L
    // 限幅到 ±16000（兼容DJI常见范围）
    input_ = std::clamp(current, -16000.0f, 16000.0f);
#else
    if (current > 16000.0f)
        input_ = 16000.0f;
    else if (current < -16000.0f)
        input_ = -16000.0f;
    else
        input_ = current;
#endif
}

float GM6020::getInput(void) const
{
    return input_;
}

void GM6020::decode(const uint8_t* data)
{
    int16_t raw_angle   = (int16_t)((data[0] << 8) | data[1]);
    int16_t raw_vel_dps = (int16_t)((data[2] << 8) | data[3]);
    int16_t raw_current = (int16_t)((data[4] << 8) | data[5]);
    uint8_t raw_temp    = data[6];

    // 角度：将 0~8191 映射到 [0, 2π)
    float angle_now = (float)raw_angle * (2.0f * (float)M_PI / 8192.0f);

    // 角度解缠（把圈数连起来，避免 2π 跳变）
    static float angle_last = 0.0f;
    float d = angle_now - angle_last;
    if (d >  M_PI) angle_offset_ -= 2.0f * M_PI;
    if (d < -M_PI) angle_offset_ += 2.0f * M_PI;
    angle_ = angle_now + angle_offset_;
    angle_last = angle_now;

    // 速度：°/s -> rad/s
    vel_     = ((float)raw_vel_dps) * ((float)M_PI / 180.0f);
    current_ = (float)raw_current;     // 仍保存为“原始电流单位”
    temp_    = (float)raw_temp;
}


//把目标电流装进 2 字节
void GM6020::encode(uint8_t* tx_buf)
{
    int16_t value = static_cast<int16_t>(input_);
    tx_buf[0] = (value >> 8) & 0xFF;    // 高位
    tx_buf[1] = (value & 0xFF); // 低位
}
