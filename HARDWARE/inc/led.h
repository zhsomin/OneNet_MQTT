/*-------------------------------------------------*/
/*            悦为电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*              实现LED功能的头文件                */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __LED_H
#define __LED_H

#define LED1_OUT(x)   GPIO_WriteBit(GPIOB, GPIO_Pin_5,  (BitAction)x)  //设置PA5  的电平，可以点亮熄灭LED1
//#define LED2_OUT(x)   GPIO_WriteBit(GPIOA, GPIO_Pin_7,  (BitAction)x)  //设置PB1  的电平，可以点亮熄灭LED2
#define LED3_OUT(x)   GPIO_WriteBit(GPIOB, GPIO_Pin_0, (BitAction)x)  //设置PB10 的电平，可以点亮熄灭LED3

#define LED1_IN_STA   GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5)  //PA5  控制LED1,读取电平状态，可以判断LED1是点亮还是熄灭
//#define LED2_IN_STA   GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_7)  //PB1  控制LED2,读取电平状态，可以判断LED2是点亮还是熄灭
#define LED3_IN_STA   GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_0) //PB10 控制LED3,读取电平状态，可以判断LED3是点亮还是熄灭

#define LED1_ON       GPIO_ResetBits(GPIOB, GPIO_Pin_5)         //共阳极，拉低PA5电平，点亮LED1
#define LED1_OFF      GPIO_SetBits(GPIOB, GPIO_Pin_5)           //共阳极，拉高PA5电平，熄灭LED1

//#define LED2_ON       GPIO_ResetBits(GPIOA, GPIO_Pin_7)         //共阳极，拉低PB1电平，点亮LED2
//#define LED2_OFF     GPIO_SetBits(GPIOA, GPIO_Pin_7)           //共阳极，拉高PB1电平，熄灭LED2

#define LED3_ON       GPIO_ResetBits(GPIOB, GPIO_Pin_0)        //共阳极，拉低PB10电平，点亮LED3
#define LED3_OFF      GPIO_SetBits(GPIOB, GPIO_Pin_0)          //共阳极，拉高PB10电平，熄灭LED3
         

void LED_Init(void);               //初始化	
void LED_AllOn(void);              //点亮所有LED
void LED_AllOff(void);             //熄灭所有LED

#endif
