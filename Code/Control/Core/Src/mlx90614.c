/*************************************************************************************
 Title	:   MLX90614 Thermometer Library for STM32 Using HAL Libraries
 Author:    Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Software:  STM32CubeIDE
 Hardware:  Any STM32 device
*************************************************************************************/
#include "stm32f0xx_hal.h"
#include "mlx90614.h"
#include "math.h"
I2C_HandleTypeDef hi2c1;
uint16_t Calculated_Emissivity=0;
//######################### CRC Code Calculation ###############################
uint8_t CRC8_Calc(uint8_t *data, uint8_t len)
{
  uint8_t crc = 0;
  uint8_t inputbyte=0;
  uint8_t carry=0;
  while (len--)
  {
	  inputbyte= *data++;
    for (uint8_t i = 8; i; i--)
    {
      carry = (crc ^ inputbyte) & 0x80;
      crc <<= 1;
      if (carry)
        crc ^= 0x7;
      inputbyte <<= 1;
    }
  }
  return crc;
}
//######################## Register Write Function #############################
void MLX90614_Write(uint8_t deviceAddr, uint8_t registerAddr, uint16_t data) {
	uint8_t data2send[5];                 		// Variable for DATA
	data2send[0] = (deviceAddr<<1); 	   		// Device Address
	data2send[1] = registerAddr;		   		// Register Address
	data2send[2] = 0;					  		// Clears Register first(LSB)
	data2send[3] = 0;					   		// Clears Register first(MSB)
	data2send[4] = CRC8_Calc(data2send, 4); 	// CRC Code Calculation

	HAL_I2C_Mem_Write(&hi2c1, (deviceAddr<<1), registerAddr, I2C_MEMADD_SIZE_8BIT, &data2send[2], 3, 100); // Sends Data (in first attempt, data=0)
	HAL_Delay(10);

	data2send[2] = data & 0x00FF;		        // Calculates Real data (LSB)
	data2send[3] = data >> 8;					// Calculates Real data (MSB)
	data2send[4] = CRC8_Calc(data2send, 4);	    // CRC code Calculation

	HAL_I2C_Mem_Write(&hi2c1, (deviceAddr<<1), registerAddr, I2C_MEMADD_SIZE_8BIT, &data2send[2], 3, 100); // Sends Real Data (calculated)
	HAL_Delay(20);
return;
}
//######################## Register Read Function #############################
uint16_t MLX90614_Read(uint8_t deviceAddr, uint8_t registerAddr) {
	uint16_t data=0;							// Extracted Data
	uint8_t Raw_Data[3];    				// RAW Data which received from I2C
	uint8_t CRC_Check[5]={0,0,0,0,0} ;					// Variable to check CRC
	uint8_t	crc=0;							// Calculated CRC Value

	HAL_I2C_Mem_Read(&hi2c1, (deviceAddr<<1), registerAddr, I2C_MEMADD_SIZE_8BIT, Raw_Data, 3, 100); // Read command to get data from MLX90614

	CRC_Check[0] = (deviceAddr<<1);       	// Putting data in right place in order to calculate CRC
	CRC_Check[1] = registerAddr;			// Putting data in right place in order to calculate CRC
	CRC_Check[2] = (deviceAddr<<1) + 1;		// Putting data in right place in order to calculate CRC
	CRC_Check[3] = Raw_Data[0];				// Putting data in right place in order to calculate CRC
	CRC_Check[4] = Raw_Data[1];				// Putting data in right place in order to calculate CRC
	crc = CRC8_Calc(CRC_Check, 5);			// Putting data in right place in order to calculate CRC

	data = (Raw_Data[0]|(Raw_Data[1] <<8)); // Extracting received data from RAW Data

	if (crc != Raw_Data[2]) {				// Checking if CRC is OK
	data = 0x0000;
	}
	return data;
}
//######################## Material Emissivity Set #############################
void MLX90614_SetEmissivity(float Emissivity){

										                        // Emissivity is between 0-1
	if(Emissivity>1.00){
	Emissivity=1.00;
	}
	if(Emissivity<0.01){
	Emissivity=0.01;
	}

	Calculated_Emissivity =(uint16_t)((65536.0 * Emissivity)-1);   // Calculates tuning value for Emissivity Register
	MLX90614_Write(MLX90614_DEVICE_ADDR,MLX90614_EMISSIVITY,Calculated_Emissivity);// Writes data to Emissivity Register

}
//######################## Material Emissivity Set #############################
float MLX90614_GetEmissivity(void){
	float emi=0;	   												   // Emissivity variable
	uint16_t data=0;
	data = MLX90614_Read(MLX90614_DEVICE_ADDR, MLX90614_EMISSIVITY);   // Data Read from MLX90614
	emi=(float)((data/65535.0f)+0.01);								   // Calculating EMI
	emi=floor(pow(10,2)*emi)/pow(10,2); 							   // limiting to two decimal digits
	return emi;
}
//######################## Gets temperature in Centigrade #############################
float MLX90614_GetTemp(void) {
	float temp=0;	   												   // Temperature variable
	uint16_t data=0; 												   // Raw Data variable

	data = MLX90614_Read(MLX90614_DEVICE_ADDR, MLX90614_TEMPOBJ1); 	   // Gets Raw DATA from sensor
	temp = data*0.02 - 273.15;									       // Calculates temperature in centigrade
	return temp;
}
