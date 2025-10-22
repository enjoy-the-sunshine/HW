/*
 * @Author: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @Date: 2025-10-13 09:20:14
 * @LastEditors: enjoy-the-sunshine 159221072+enjoy-the-sunshine@users.noreply.github.com
 * @LastEditTime: 2025-10-14 10:54:16
 * @FilePath: \hy\Tasks\main_task.hpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __MAIN_TASK_HPP__
#define __MAIN_TASK_HPP__

typedef struct 
{
    uint32_t tick;
    float value;
}UartCommData;


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

void MainInit(void);

void MainTask(void);

void encode(UartCommData *data, uint8_t *buf);
uint8_t decode(uint8_t *buf, UartCommData *data);


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_TASK_HPP__ */