/*-----------------------------------------------------*/
/*              悦为电子STM32系列开发板                */
/*-----------------------------------------------------*/
/*                     程序结构                        */
/*-----------------------------------------------------*/
/*USER     ：包含程序的main函数，是整个程序的入口      */
/*HARDWARE ：包含开发板各种功能外设的驱动程序          */
/*CORE     ：包含STM32的核心程序，官方提供，我们不修改 */
/*STLIB    ：官方提供的库文件，我们不修改              */
/*-----------------------------------------------------*/
/*                                                     */
/*           程序main函数，入口函数源文件              */
/*                                                     */
/*-----------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "main.h"       //包含需要的头文件
#include "delay.h"      //包含需要的头文件
#include "usart1.h"     //包含需要的头文件
#include "iic.h"        //包含需要的头文件
//#include "24c02.h" 			//包含需要的头文件
#include "usart2.h"     //包含需要的头文件
#include "timer1.h"     //包含需要的头文件
#include "timer3.h"     //包含需要的头文件
#include "timer4.h"     //包含需要的头文件
#include "wifi.h"	    	//包含需要的头文件
#include "led.h"        //包含需要的头文件
#include "mqtt.h"       //包含需要的头文件
#include "key.h"        //包含需要的头文件
#include "bsp_tempad.h"
#include "timer2.h"
#include "bsp_dht11.h"

char *CMD1 = "open_led";    //开关控制命令，状态翻转，开->关 关->开  
//char *CMD2 = "APP+OneSW=?";    //开关查询命令，回复服务器当前开关状态
//char *CMD3 = "APP+OneSWCD";    //设置开关倒计时，倒计时时间到的时候，开关状态翻转
int   CDTime= 0;               //记录开关倒计时时间
extern __IO u16 ADC_ConvertedValue;
u16 Current_Temp;	  //这是使用ADC的芯片内部温度
DHT11_Data_TypeDef DHT11_Data;
int main(void) 
{	
	//
	Delay_Init();                   //延时功能初始化              
	Usart1_Init(9600);              //串口1功能初始化，波特率9600
	Usart2_Init(115200);            //串口2功能初始化，波特率115200	
	TIM4_Init(300,7200);            //TIM4初始化，定时时间 300*7200*1000/72000000 = 30ms
  LED_Init();	                    //LED初始化
//	Temp_ADC1_Init();
   DHT11_Init();

//KEY_Init();	//按键初始化
	 
	WiFi_ResetIO_Init();            //初始化WiFi的复位IO
  MQTT_Buff_Init();               //初始化接收,发送,命令数据的 缓冲区 以及各状态参数
	OneNetIoT_Parameter_Init();	    //初始化连接OneNet云IoT平台MQTT服务器的参数	
	while(1)                        //主循环
	{		
		
	     Current_Temp=(V25-ADC_ConvertedValue)/AVG_SLOPE+25;
		//	printf("\r\n The IC current tem= %3d ℃\r\n", Current_Temp);	      

		/*--------------------------------------------------------------------*/
		/*   Connect_flag=1同服务器建立了连接,我们可以发布数据和接收推送了    */
		/*--------------------------------------------------------------------*/
		if(Connect_flag==1){     
			/*-------------------------------------------------------------*/
			/*                     处理发送缓冲区数据                      */
			/*-------------------------------------------------------------*/
				if(MQTT_TxDataOutPtr != MQTT_TxDataInPtr){                //if成立的话，说明发送缓冲区有数据了
				//3种情况可进入if
				//第1种：0x10 连接报文
				//第2种：0x82 订阅报文，且ConnectPack_flag置位，表示连接报文成功
				//第3种：SubcribePack_flag置位，说明连接和订阅均成功，其他报文可发
				if((MQTT_TxDataOutPtr[1]==0x10)||((MQTT_TxDataOutPtr[1]==0x82)&&(ConnectPack_flag==1))||(SubcribePack_flag==1)){    
					u1_printf("发送数据:0x%x\r\n",MQTT_TxDataOutPtr[1]);  //串口提示信息
					MQTT_TxData(MQTT_TxDataOutPtr);                       //发送数据
					MQTT_TxDataOutPtr += BUFF_UNIT;                       //指针下移
					if(MQTT_TxDataOutPtr==MQTT_TxDataEndPtr)              //如果指针到缓冲区尾部了
						MQTT_TxDataOutPtr = MQTT_TxDataBuf[0];            //指针归位到缓冲区开头
				} 				
			}//处理发送缓冲区数据的else if分支结尾
			
			/*-------------------------------------------------------------*/
			/*                     处理接收缓冲区数据                      */
			/*-------------------------------------------------------------*/
			if(MQTT_RxDataOutPtr != MQTT_RxDataInPtr){  //if成立的话，说明接收缓冲区有数据了														
				u1_printf("接收到数据:");
				/*-----------------------------------------------------*/
				/*                    处理CONNACK报文                  */
				/*-----------------------------------------------------*/				
				//if判断，如果一共接收了4个字节，第一个字节是0x20，表示收到的是CONNACK报文
				//接着我们要判断第4个字节，看看CONNECT报文是否成功
				if((MQTT_RxDataOutPtr[0]==4)&&(MQTT_RxDataOutPtr[1]==0x20)){             			
				    switch(MQTT_RxDataOutPtr[4]){					
						case 0x00 : u1_printf("CONNECT报文成功\r\n");                            //串口输出信息	
								    ConnectPack_flag = 1;                                        //CONNECT报文成功，订阅报文可发
									break;                                                       //跳出分支case 0x00                                              
						case 0x01 : u1_printf("连接已拒绝，不支持的协议版本，准备重启\r\n");     //串口输出信息
									Connect_flag = 0;                                            //Connect_flag置零，重启连接
									break;                                                       //跳出分支case 0x01   
						case 0x02 : u1_printf("连接已拒绝，不合格的客户端标识符，准备重启\r\n"); //串口输出信息
									Connect_flag = 0;                                            //Connect_flag置零，重启连接
									break;                                                       //跳出分支case 0x02 
						case 0x03 : u1_printf("连接已拒绝，服务端不可用，准备重启\r\n");         //串口输出信息
									Connect_flag = 0;                                            //Connect_flag置零，重启连接
									break;                                                       //跳出分支case 0x03
						case 0x04 : u1_printf("连接已拒绝，无效的用户名或密码，准备重启\r\n");   //串口输出信息
									Connect_flag = 0;                                            //Connect_flag置零，重启连接						
									break;                                                       //跳出分支case 0x04
						case 0x05 : u1_printf("连接已拒绝，未授权，准备重启\r\n");               //串口输出信息
									Connect_flag = 0;                                            //Connect_flag置零，重启连接						
									break;                                                       //跳出分支case 0x05 		
						default   : u1_printf("连接已拒绝，未知状态，准备重启\r\n");             //串口输出信息 
									Connect_flag = 0;                                            //Connect_flag置零，重启连接					
									break;                                                       //跳出分支case default 								
					}				
				}			
				//if判断，如果一共接收了5个字节，第一个字节是0x90，表示收到的是SUBACK报文
				//接着我们要判断订阅回复，看看是不是成功
				else if((MQTT_RxDataOutPtr[0]==5)&&(MQTT_RxDataOutPtr[1]==0x90)){ 
						switch(MQTT_RxDataOutPtr[5]){					
						case 0x00 :
						case 0x01 : u1_printf("订阅成功\r\n");            //串口输出信息
							        SubcribePack_flag = 1;                //SubcribePack_flag置1，表示订阅报文成功，其他报文可发送
									Ping_flag = 0;                        //Ping_flag清零
   								    TIM3_ENABLE_30S();                    //启动30s的PING定时器
						          TIM2_ENABLE_2S();
						          Tempeature_State();
									break;                                //跳出分支                                             
						default   : u1_printf("订阅失败，准备重启\r\n");  //串口输出信息 
									Connect_flag = 0;                     //Connect_flag置零，重启连接
									break;                                //跳出分支 								
					}					
				}
				//if判断，如果一共接收了2个字节，第一个字节是0xD0，表示收到的是PINGRESP报文
				else if((MQTT_RxDataOutPtr[0]==2)&&(MQTT_RxDataOutPtr[1]==0xD0)){ 
					u1_printf("PING报文回复\r\n"); 		  //串口输出信息 
					if(Ping_flag==1){                     //如果Ping_flag=1，表示第一次发送
						 Ping_flag = 0;    				  //要清除Ping_flag标志
					}else if(Ping_flag>1){ 				  //如果Ping_flag>1，表示是多次发送了，而且是2s间隔的快速发送
						Ping_flag = 0;     				  //要清除Ping_flag标志
						TIM3_ENABLE_30S(); 				  //PING定时器重回30s的时间
					}				
				}	
				//if判断，如果第一个字节是0x30，表示收到的是服务器发来的推送数据
				//我们要提取控制命令
				else if(MQTT_RxDataOutPtr[1]==0x30){ 
					u1_printf("服务器等级0推送\r\n"); 		   		//串口输出信息 
					MQTT_DealPushdata_Qs0(MQTT_RxDataOutPtr);   //处理等级0推送数据
				}				
								
				MQTT_RxDataOutPtr += BUFF_UNIT;                     //指针下移
				if(MQTT_RxDataOutPtr==MQTT_RxDataEndPtr)            //如果指针到缓冲区尾部了
					MQTT_RxDataOutPtr = MQTT_RxDataBuf[0];          //指针归位到缓冲区开头                        
			}//处理接收缓冲区数据的else if分支结尾
			
			/*-------------------------------------------------------------*/
			/*                     处理命令缓冲区数据                      */
			/*-------------------------------------------------------------*/
			if(MQTT_CMDOutPtr != MQTT_CMDInPtr){                             //if成立的话，说明命令缓冲区有数据了			
				MQTT_CMDOutPtr[MQTT_CMDOutPtr[0]+1] = '\0';                  //加入字符串结束符        
				u1_printf("命令:%s\r\n",&MQTT_CMDOutPtr[1]);                 //串口输出信息
				if(!memcmp(&MQTT_CMDOutPtr[1],CMD1,strlen(CMD1))){           //判断指令，如果是CMD1
					LED1_OUT(!LED1_IN_STA);                                  //开关1状态翻转 如果点亮就熄灭，反之如果熄灭就点亮
				//	LED_CD_State();                                          //判断开关和倒计时状态，并发布给服务器
				}
//				else if(!memcmp(&MQTT_CMDOutPtr[1],CMD2,strlen(CMD2))){     //判断指令，如果是CMD2，回复开关状态
//				//	LED_CD_State();                                          //判断开关和倒计时状态，并发布给服务器				
//				}else if(!memcmp(&MQTT_CMDOutPtr[1],CMD3,strlen(CMD3))){     //判断指令，如果是CMD3，设置倒计时时间 
//					//CDTime = MQTT_CMDOutPtr[12];                             //记录倒计时时间
//					if(CDTime!=0){                                           //如果CDTime不等于0，进入if
//						u1_printf("倒计时:%d分钟\r\n",CDTime);               //串口输出信息
//						TIM1_ENABLE_60S();                                   //启动定时器1 60s钟定时
//					}else{                                                   //反之CDTime等于0，进入else，取消定时
//					    u1_printf("取消倒计时\r\n");                         //串口输出信息
//						TIM_Cmd(TIM1,DISABLE);                               //关闭TIM1
//					}
//					LED_CD_State();                                          //判断开关和倒计时状态，并发布给服务器							
//				}else u1_printf("未知指令\r\n");                             //串口输出信息
				MQTT_CMDOutPtr += BUFF_UNIT;                             	 //指针下移
				if(MQTT_CMDOutPtr==MQTT_CMDEndPtr)           	             //如果指针到缓冲区尾部了
					MQTT_CMDOutPtr = MQTT_CMDBuf[0];          	             //指针归位到缓冲区开头				
			}//处理命令缓冲区数据的else if分支结尾	
		}//Connect_flag=1的if分支的结尾
		
		/*--------------------------------------------------------------------*/
		/*      Connect_flag=0同服务器断开了连接,我们要重启连接服务器         */
		/*--------------------------------------------------------------------*/
		else{ 
			u1_printf("需要连接服务器\r\n");                 //串口输出信息
			TIM_Cmd(TIM4,DISABLE);                           //关闭TIM4 
			TIM_Cmd(TIM3,DISABLE);                           //关闭TIM3  
			WiFi_RxCounter=0;                                //WiFi接收数据量变量清零                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //清空WiFi接收缓冲区 
			if(WiFi_Connect_IoTServer()==0){   			     //如果WiFi连接云服务器函数返回0，表示正确，进入if
				u1_printf("建立TCP连接成功\r\n");            //串口输出信息
				Connect_flag = 1;                            //Connect_flag置1，表示连接成功	
				WiFi_RxCounter=0;                            //WiFi接收数据量变量清零                        
				memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);      //清空WiFi接收缓冲区 
				MQTT_Buff_ReInit();                          //重新初始化发送缓冲区                    
			}				
		}
	}
}
/*-------------------------------------------------*/
/*函数名：判断开关和倒计时状态，并发布给服务器     */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
//void LED_CD_State(void)
//{
////	int i=0;
//	char temp[20];                  		//定义一个临时缓冲区
//	//int ww[20]={1,2,3,4,5,6,7,9};
//	
//	memset(temp,0,20);             		    //清空临时缓冲区
//	//sprintf(temp,"tem");          	    //构建回复数据	
//	
//	if(LED1_IN_STA) temp[13] = '0';  		//如果LED1是高电平，说明是熄灭状态，开关1状态位置0
//	else            temp[13] = '1';			//反之，说明是点亮状态，开关1状态位置1	
//	temp[14] = CDTime;                       //加入剩余倒计时时间
//// sprintf(temp,"CurrentHumidity:%d.CurrentTemperature:%d",10,20);
//	MQTT_PublishQs0(P_TOPIC_NAME,temp,0);   //添加数据，发布给服务器	

////   	sprintf(temp,"{\"method\":%d}",Current_Temp); 
////		MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //添加数据，发布给服务器	
////	u1_printf("\r\n The IC current tem= %3d ℃\r\n", Current_Temp);	      

//	
//	
//	 //构建回复湿度温度数据
//}

 void Tempeature_State(void)
	 {
   char temp[256];
	memset(temp,0,256);
	while(1){
		 DHT11_Read_TempAndHumidity(&DHT11_Data);
		if(DHT11_Read_TempAndHumidity(&DHT11_Data)==SUCCESS)
			{
				u1_printf("湿度：%d.%d,温度：%d.%d",DHT11_Data.humi_int,DHT11_Data.humi_deci,DHT11_Data.temp_int,DHT11_Data.temp_deci);	
				break; 
		}else{
		//	u1_printf("读取温度失败");
		}
      
	}
	sprintf(temp,"{\"temperature\":%d.%d,\"humity\":%d.%d}",DHT11_Data.temp_int,DHT11_Data.temp_deci,DHT11_Data.humi_int,DHT11_Data.humi_deci); 
		MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));

}



