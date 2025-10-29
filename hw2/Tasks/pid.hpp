#pragma once
#include <algorithm>
#include <cstdint>

struct PidParams {
    float kp{0.0f};
    float ki{0.0f};
    float kd{0.0f};
    float i_limit{0.0f};     // �����޷�
    float out_limit{0.0f};   // ����޷�
};

struct PidData {
    float ref{0.0f};    // Ŀ��ֵ
    float fdb{0.0f};    // ����ֵ
    float err{0.0f};    // ��ǰ��� ref - fdb
    float err_prev{0.0f};   // ��һ�����
    float integral{0.0f};   // ������
    float derivative{0.0f}; // ΢����
    float out{0.0f};    // ���
};

class Pid {
public:
    explicit Pid(PidParams &params) 
    { 
        params_ = params; 
    }
    ~Pid() = default;

    void setParams(PidParams &params)   //���� PID ���������״̬
    { 
        params_ = params; 
        reset(); 
    }
    PidParams getParams(void)   //���ص�ǰ����
    { 
        return params_; 
    }

    float pidCalc(const float ref, const float fdb) {
        //���㵱ǰ���
        datas_.ref = ref;
        datas_.fdb = fdb;
        datas_.err = ref - fdb;

        //������������޷�
        datas_.integral += datas_.err;
        if (params_.i_limit > 0.0f) {
            const float lim = params_.i_limit / std::max(params_.ki, 1e-6f);
            datas_.integral = std::clamp(datas_.integral, -lim, lim);
        }

        //΢�������
        datas_.derivative = datas_.err - datas_.err_prev;

        //PID �������
        float out = params_.kp * datas_.err
                  + params_.ki * datas_.integral
                  + params_.kd * datas_.derivative;

        //����޷�
        if (params_.out_limit > 0.0f)
            out = std::clamp(out, -params_.out_limit, params_.out_limit);

        //����״̬������
        datas_.out = out;
        datas_.err_prev = datas_.err;
        last_datas_ = datas_;
        return out;
    }

    //���ú���
    void reset() 
    { 
        datas_ = {}; 
        last_datas_ = {}; 
    }

private:
    PidParams params_{};    // ���Ʋ���
    PidData datas_{};   // ��ǰ����
    PidData last_datas_{};  // ��һ���������ݣ�������ԣ�
};
