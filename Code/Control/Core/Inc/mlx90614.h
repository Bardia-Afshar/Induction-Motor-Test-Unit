/*************************************************************************************
 Title	:   MLX90614 Thermometer Library for STM32 Using HAL Libraries
 Author:    Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Software:  STM32CubeIDE
 Hardware:  Any STM32 device
*************************************************************************************/
#ifndef _MLX90614_H_
#define _MLX90614_H_
I2C_HandleTypeDef hi2c1;
//######################### Defines ###############################
#define MLX90614_DEVICE_ADDR 0x5A                   // Default Address for MLX90614

#define OPCODE_RAM			0x00				    // OPCODE for read from RAM
#define OPCODE_EEPROM		0x20                    // OPCODE for read/write on EEPROM
#define MLX90614_TEMPOBJ1 		(OPCODE_RAM | 0x07) // Address for Object 1 Register
#define MLX90614_EMISSIVITY (OPCODE_EEPROM | 0x04)  // Address for Emissivity Register
//######################### Functions ###############################
void MLX90614_SetEmissivity(float Emissivity);		// Sets Emissivity
float MLX90614_GetEmissivity(void);		// Sets Emissivity
float MLX90614_GetTemp(void);						// Gets Temperature in centigrade
#endif
