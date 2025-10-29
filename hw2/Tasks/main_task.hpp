/*
 * @Author: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @Date: 2025-10-13 09:20:14
 * @LastEditors: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @LastEditTime: 2025-10-14 10:54:16
 * @FilePath: \hy\Tasks\main_task.hpp
 * @Description: ����Ĭ������,������`customMade`, ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __MAIN_TASK_HPP__
#define __MAIN_TASK_HPP__

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/


void RobotInit(void);
void MainInit(void);
void MainTask(void);


void SetModeVelocitySine(void);     // �л������ٶ����Ҳο���
void SetModePositionSeq(void);   // �л�����λ�����н�Ծ��

void ControlLoopStep(void); // �����Ʋ��������ɶ�ʱ���ж������Ե���

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_TASK_HPP__ */