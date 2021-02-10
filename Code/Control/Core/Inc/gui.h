/*************************************************************************************
 Title	:   GUI for Universal Motor Test Unit
 Author:    Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Software:  STM32CubeIDE
 Hardware:  Any STM32 device
*************************************************************************************/
#ifndef _GUI_H
#define _GUI_H
#include "main.h"
#include "lcd.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "mlx90614.h"
#include "math.h"
//##################################### Defines #####################################
#define KEYR         HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)
#define KEY1		 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)
#define KEY2		 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5)
#define RELAY1H 	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 	//Relay 1 ON
#define RELAY1L		 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //Relay 1 OFF
#define RELAY2H		 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);	//Relay 2 ON
#define RELAY2L		 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //Relay 2 OFF
//#################################### Variables ####################################
ADC_HandleTypeDef hadc;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim16;
uint8_t cursor=0;										 // Cursor Position Variable
int power=0;										 // Power Percentage Variable
int32_t Dev=0;                                           // Counter Variable - Increases Every 100 uS
int rpm=0;                                               // Rotation per minute Variable
int set=0;                                               // Variable for External Interrupt Change Detection
float current =4.0;										 // Predefine Current Variable
float rcurrent=0;										 // Real-Time Current Variable
float emi=1.00;											 // Emissivity Variable
float temp=0;											 // temperature Variable
bool MainmenuFlag=0;
bool SettingFlag=0;
bool RunFlag=0;
bool setflag=0;
char c[20];												 // String for LCD
// ############################ ADC Read ################################
int adc1_read(void){
int value=0;
HAL_ADC_Init(&hadc);									// ADC Initializing
HAL_ADC_Start(&hadc);									// ADC Starting
if(HAL_ADC_PollForConversion(&hadc, 500)==HAL_OK){		// Waits for conversation & gets value
value=HAL_ADC_GetValue(&hadc);
}
HAL_ADC_Stop(&hadc);									// ADC Stop
return value;
}
// ############################ Current Calculator ################################
float Get_AC_RMS(void){
float RMS_value=0;
uint32_t value=0;
value=adc1_read();
RMS_value=((value*3300)/4095);   // Calculating Voltage in mV
RMS_value/=33;					 // Converting Voltage to current equivalent
RMS_value*=0.1;
return RMS_value;
}
// ############################ RPM Read Function ################################
int Get_RPM(void){
return rpm;
}
// ############################ Configuration Function ################################
void gui_lcd_config(void){
lcd_Init();                   // Sets LCD in 8 BIT MODE
HAL_Delay(50);
lcd_Init();					  // Sets LCD in 8 BIT MODE
lcd_clrscr();                 // Clears Screen
lcd_gotoxy(0,0);              // Moves Cursor to 0,0
return;
}
//###############################   GUI Functions   ####################################
// ----------------------------------- Main Menu -------------------------------------
void main_menu(void){
lcd_clrscr();				               // Clears LCD
lcd_pos_puts(14,cursor,"<");  			   // Puts cursor on desired Position
lcd_pos_puts(0,0,"1-RUN");
lcd_pos_puts(0,1,"2-SETTING");
}
// ----------------------------------- Setting Menu ---------------------------------
void setting_menu(void){
lcd_clrscr();				              // Clears LCD
lcd_pos_puts(19,cursor,"<");              // Puts cursor on desired Position
sprintf(c,"CURRENT   =%4.1f A",current);
lcd_pos_puts(0,0,c);
sprintf(c,"EMISSIVITY= %1.2f",emi);
lcd_pos_puts(0,1,c);
lcd_pos_puts(0,2,"BACK");
}
// -------------------------------- Parameter Set Function ---------------------------
void set_param(bool cursor){
lcd_clrscr();                                 // Clears Screen
setflag=1;									  // Sets Parameter Setting flag to 1 and other Flags to 0
MainmenuFlag=0;
SettingFlag=0;
RunFlag=0;
while(1){
switch(cursor){								  // Updates Screen based on cursor position
case 0:
sprintf(c,"CURRENT   =%4.1f A",current);
lcd_pos_puts(0,0,c);
break;
case 1:
sprintf(c,"EMISSIVITY= %1.2f",emi);
lcd_pos_puts(0,1,c);
break;
}
if(KEYR==0){								 // Returns to Setting Menu
HAL_Delay(200);
setflag=0;									 // Sets Parameter Setting flag to 0
SettingFlag=1;								 // Sets Setting flag to 1
MLX90614_SetEmissivity(emi);
setting_menu();								 // Loads Setting Menu
break;										 // Breaks the Loop
}
}
}
// ------------------------------------- RUN Function ----------------------------------------
void run(void){
lcd_clrscr();
RunFlag=1;
RELAY1H;							// Relay 1 ON
HAL_Delay(200);
RELAY2H;							// Relay 2 ON
while(1){
HAL_Delay(20);
// -------------------------- Power Control And Show
sprintf(c,"POWER  =%3d",power);
lcd_pos_puts(0,0,c);
HAL_Delay(10);
lcd_puts("%");
// -------------------------- Current control and show
if(power!=0){
rcurrent=Get_AC_RMS();			// Gets RMS current
// -------------------------- High current detection
if(rcurrent>current){
RELAY1L;
RELAY2L;
HAL_Delay(200);
lcd_clrscr();
lcd_pos_puts(4,0,"OVER CURRENT");
HAL_Delay(5000);
power=0;
RunFlag=0;
MainmenuFlag=1;
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
HAL_Delay(50);
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
main_menu();
break;
}
// --------------------------
sprintf(c,"CURRENT=%4.1f/",rcurrent);
lcd_pos_puts(0,1,c);
sprintf(c,"%4.1f A",current);
lcd_puts(c);
}
else if(power==0){
rcurrent=0;
sprintf(c,"CURRENT=%4.1f/",rcurrent);
lcd_pos_puts(0,1,c);
sprintf(c,"%4.1f A",current);
lcd_puts(c);
}
// ------------------------- TEMP -------------------------
if(KEY1==0){
temp=MLX90614_GetTemp();
sprintf(c,"TEMP(C)= %3.1f",temp);
lcd_pos_puts(0,2,c);
}
if(KEY1==1){
sprintf(c,"TEMP(C)= N/A       ");
lcd_pos_puts(0,2,c);
}
// -------------------------- RPM --------------------------
if(KEY2==0){
sprintf(c,"RPM    = %4d",Get_RPM());
lcd_pos_puts(0,3,c);
}
if(KEY2==1){
sprintf(c,"RPM    = N/A       ");
lcd_pos_puts(0,3,c);
rpm=0;
}
if(KEYR==0){
HAL_Delay(150);
RELAY1L;
RELAY2L;
power=0;
RunFlag=0;
MainmenuFlag=1;
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
HAL_Delay(50);
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
main_menu();
break;
}
}
}
// ------------------------------------- Setting Function ----------------------------------------
void setting(void){
cursor=0;
SettingFlag=1;
setting_menu();
while(1){
if(KEYR==0){
HAL_Delay(200);
if(cursor==2){ 				// Backs to Main Menu
MainmenuFlag=1;				// Sets Main Menu Flag to 1
SettingFlag=0;				// Sets Setting Menu Flag to 0
cursor=1;					// Puts Cursor Position to 1
main_menu();				// Loads Main Menu
break;						// Breaks Loop
}
else {
setflag=1;
set_param(cursor);
}
}
}
}
// ----------------------------- GUI Main Function ---------------------------------
void gui(void){
gui_lcd_config();                	// LCD Setup
main_menu();					 	// Loads Main Menu
MainmenuFlag=1;					 	// Sets Main Menu Flag to 1
temp=MLX90614_GetTemp();
HAL_Delay(200);
emi=MLX90614_GetEmissivity();
while(1){
if(KEYR==0){					   // KEYR Pressed
HAL_Delay(200);
MainmenuFlag=0;
cursor==0 ? run(): setting();      // Decides between Run or Setting Based on Cursor Position
}
}
}
//############################# Timer Interrupt Routine ##############################
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
if(htim->Instance==TIM14){
Dev+=65535;
}
}
//############################ External Interrupt Routine ############################
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
if(GPIO_Pin==GPIO_PIN_14&&RunFlag==1&&KEY2==0){                            // External Interrupt on PIN8
set++;
if(set%2==1){                                // First Falling Edge Detection
htim14.Instance->CNT = 0;                    // Counter Variable = 0
HAL_TIM_Base_Start(&htim14);                 // Start Counting (Getting Time)
}
if(set%2==0){                                // Second Falling Edge Detection
HAL_TIM_Base_Stop(&htim14);                  // Stop Counting
Dev=htim14.Instance->CNT;                    // Putting Counter value in DEV Variable ( Dev*100uS = Pulse Duration )
rpm=600000/Dev;                              // Calculating RPM from Pulse Duration
}
}
// Rotary Encoder Function;
if(GPIO_Pin==GPIO_PIN_1){
// ------------------------------------- Main Menu GUI Control-------------------------------------------
if(MainmenuFlag==1){ 								// Main Menu Flag
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==1)){		// Rotary Encoder ClockWise Detection
cursor^=1;											// Changes Cursor Position
}
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==0)){		// Rotary Encoder Counter - ClockWise Detection
cursor^=1;											// Changes Cursor Position
}
main_menu();										// Updates Main menu with new cursor position
}
// ------------------------------------ Setting Menu GUI Control -----------------------------------------
if(SettingFlag==1){ 								// Main Menu Flag
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==1)){		// Rotary Encoder ClockWise Detection
cursor--;											// Changes Cursor Position
if(cursor<0)cursor=2;
}
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==0)){		// Rotary Encoder Counter - ClockWise Detection
cursor++;											// Changes Cursor Position
if(cursor>2)cursor=0;
}
setting_menu();										// Updates Main menu with new cursor position
}
// ------------------------------------ Parameter Setting GUI Control -------------------------------------
//  --------------------     Current Setting
if(setflag==1&&cursor==0){ 							// Sets Parameter setting flag to 1
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==1)){		// Rotary Encoder ClockWise Detection
current-=0.1;										// Changes Value of Current - Decrement
if(current<0)current=0;
}
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==0)){		// Rotary Encoder Counter - ClockWise Detection
current+=0.1;										// Changes Value of Current - Increment
if(current>10)current=10;
}
}
//  --------------------      Emissivity Setting
if(setflag==1&&cursor==1){ 							// Sets Parameter setting flag to 1
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==1)){		// Rotary Encoder ClockWise Detection
emi-=0.01;											// Changes Value of Emissivity - Increment
if(emi<0)emi=0;
}
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==0)){		// Rotary Encoder Counter - ClockWise Detection
emi+=0.01;											// Changes Value of Emissivity - Decrement
if(emi>1)emi=1;
}
}
// ------------------------------------ Power Control -------------------------------------
if(RunFlag==1){ 									// Sets Parameter setting flag to 1
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==1)){		// Rotary Encoder ClockWise Detection
power--;											// Changes Value of Emissivity - Decrement
if(power<0)power=0;
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // Sends Pulse to Dimmer Board
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);	  // Sends Pulse to Dimmer Board
}
if(((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2))==0)){		// Rotary Encoder Counter - ClockWise Detection
power++;											// Changes Value of Emissivity - Increment
if(power>100)power=100;
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);// Sends Pulse to Dimmer Board
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);  // Sends Pulse to Dimmer Board
}
}
}
}
#endif
