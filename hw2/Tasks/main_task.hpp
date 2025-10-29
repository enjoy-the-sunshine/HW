/*
 * @Author: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @Date: 2025-10-13 09:20:14
 * @LastEditors: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @LastEditTime: 2025-10-14 10:54:16
 * @FilePath: \hy\Tasks\main_task.hpp
 * @Description: ï¿½ï¿½ï¿½ï¿½Ä¬ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½,ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½`customMade`, ï¿½ï¿½koroFileHeaderï¿½é¿´ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
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


void SetModeVelocitySine(void);     // ÇÐ»»µ½¡°ËÙ¶ÈÕýÏÒ²Î¿¼¡±
void SetModePositionSeq(void);   // ÇÐ»»µ½¡°Î»ÖÃÐòÁÐ½×Ô¾¡±

void ControlLoopStep(void); // ¡°¿ØÖÆ²½½ø¡±£ºÓÉ¶¨Ê±Æ÷ÖÐ¶ÏÖÜÆÚÐÔµ÷ÓÃ

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_TASK_HPP__ */