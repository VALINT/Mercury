/*------------------------------------------------------------------------------------------------------
 * Standard interface module
 *______________________________________________________________________________________________________
 *  __    __ __ __     __   _____	 _____ _____   __    __  _  ____    __    ____  _____
 *  \ \  / //  \\ \    \_\ / ___/	/ ___/|_   _| /  \  |  \| ||  _ \  /  \  | |\ \|_   _|
 *   \ \/ //    \\ \___ |/ \___ \	\___ \  | |  /    \ | |\  || |_|| /    \ | |/ /  | |
 *    \__//__/\__\\____\   /____/	/____/  |_| /__/\__\|_| \_||____//__/\__\|_|\_\  |_|
 *					 _  __  _  _____  ____  ____  ____    _    ____  ____
 *					| ||  \| ||_   _|| ___|| |\ \| ___|  / \  /  __\| ___|
 *					| || |\  |  | |  | ___|| |/ /| ___| / _ \ | |__ | ___|
 *					|_||_| \_|  |_|  |____||_|\_\|_|   /_/ \_\\____/|____|
 *_______________________________________________________________________________________________________
 *
 * Created: 18-Sep-2018 23:05:14
 *  Author: VAL
 *------------------------------------------------------------------------------------------------------- 
 *	This module consist program realization of communication interfaces.
 *	Using hardware interfaces (need for easy programing if you need using interface)
 *-------------------------------------------------------------------------------------------------------
 *	History:
 *		-	18-Sep-2018
 *
 *-------------------------------------------------------------------------------------------------------
 *	Features:
 *		-	Using hardware I2C interface (protocol) 
 *
 *-------------------------------------------------------------------------------------------------------
 */

#ifndef STANDART_INTERFACE_H_
#define STANDART_INTERFACE_H_

#include <avr/io.h>
#include <stdint.h>

#ifndef F_CPU	//If FCPU is not defined, set default value
	#define F_CPU 16000000UL
	//#error: "Used default F_CPU value" 
#endif

#include <util/delay.h>

#ifndef BAUD	//If baudrate not is not defined, set default value
	#define BAUD 9600L // Baud rate
#endif
#define ATMEGA8
#define UBRRL_value (F_CPU/(BAUD*16))-1

//-------------------------------------------------------------------------------------------------------
//	UART/USART HARDWARE part
//-------------------------------------------------------------------------------------------------------

//Procedures and functions needed for this interface work. 

void uart_init(void);

void uart_write_char(char data);

void uart_send_array(char* data);

//-------------------------------------------------------------------------------------------------------
//	I2C HARDWARE part
//-------------------------------------------------------------------------------------------------------

//Procedures and functions needed for this interface work. 

void i2c_init(void);

void i2c_start_operation(void);

void i2c_stop_operation(void);

void i2c_send_byte(uint8_t byte);

void i2c_send_data(uint8_t data, uint8_t addres);

uint8_t i2c_reseive_byte(void);

uint8_t i2c_reseive_last_byte(void);

//-------------------------------------------------------------------------------------------------------
//	SPI HARDWARE part
//-------------------------------------------------------------------------------------------------------
 
//Procedures and functions needed for this interface work. 

#define SPI_PORTX   PORTB
#define SPI_DDRX    DDRB

#define SPI_MISO   4
#define SPI_MOSI   3
#define SPI_SCK    5
#define SPI_SS     0

#define SPI_DisableSS_m(ss)  do{SPI_PORTX |= (1<<(ss)); }while(0)								//Disable SPI device
#define SPI_EnableSS_m(ss)   do{SPI_PORTX &= ~(1<<(ss)); }while(0)								//Enable SPI device
#define SPI_StatSS_m(ss)    (!(SPI_PORTX & (1<<(ss))))											//Device status
#define SPI_WriteByte_m(data)  do{ SPDR = data; while(!(SPSR & (1<<SPIF))); }while(0)			//Send SPI byte
#define SPI_ReadByte_m(data)  do{ SPDR = 0xff; while(!(SPSR & (1<<SPIF))); data = SPDR;}while(0)//Read SPI byte
inline static uint8_t SPI_ReadByte_i(void)														//Get byte on SPI
{
   SPDR = 0xff;
   while(!(SPSR & (1<<SPIF)));
   return SPDR;   
}

// Init SPI module accordingly to defined parameters above 
void SPI_Init(void); 
//
void SPI_WriteByte(uint8_t data); 
//
uint8_t SPI_ReadByte(void);
//
uint8_t SPI_WriteReadByte(uint8_t data);
//
void SPI_WriteArray(uint8_t num, uint8_t *data);
//
void SPI_WriteReadArray(uint8_t num, uint8_t *data);

#endif /* STANDART_INTERFACE_H_ */