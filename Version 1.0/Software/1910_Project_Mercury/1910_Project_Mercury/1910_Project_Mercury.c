//-------------------------------------------------------------------------------------------------------------------------*
// Actual version: 0.1a
//_________________________________________________________________________________________________________________________*
//   _____   _____ 	 _____       _   _____	 _____	 _______	 __    __   _____   _____	_____   _   _   _____  __    __*
//  |  _  \	|  _  \	/  _  \     | | |  ___|	/  _  \	|__   __|	|  \  /  | |  ___| |  _  \ /  _  \ | | | | |  _  \ \ \  / /*
//  | |_| |	| |_| |	| | | |     | | | |___	| | |_|	   | |		|   \/   | | |___  | |_| | | | |_| | | | | | |_| |  \ \/ / *
//  |  ___/	|    _/	| | | |  _  | | |  ___|	| |  _	   | |		| |\  /| | |  ___| |    _/ | |  _  | | | | |    _/   \  /  *
//  | |		| |\ \	| |_| | | |_| | | |___	| |_| |	   | |		| | \/ | | | |___  | |\ \  | |_| | | |_| | | |\ \    / /   *
//  |_|		|_| \_\	\_____/ \_____/ |_____|	\_____/	   |_|		|_|    |_| |_____| |_| \_\ \_____/ \_____/ |_| \_\  /_/    *
//_________________________________________________________________________________________________________________________*
//
//	Author: VAL
//	Created: 05 Oct 2019
//_________________________________________________________________________________________________________________________*
//
//	Story of versions:
//_________________________________________________________________________________________________________________________*
//
//	08 OCT 2019:
//	version 0.1:
//			Project Mercury V1.0 Firmware V0.1
//			Very simple 8-bits MCU WAV PLAYER
//						  _____________________________		
//			Handle next: |Sample rate|Channels|PSM type|
//						 |-----------|--------|--------|
//						 |32000		 |    1   |uint 8  |
//						 |-----------|--------|--------|
//					     |24000		 |	  1	  |uint 8  |
//						 |-----------|--------|--------|
//						 |22100		 |	  1   |uint 8  |				
//						 |-----------|--------|--------|
//						 |16000		 |  1, 2  |uint 8  |
//						 |-----------|--------|--------|
//						 |8000		 |  1, 2  |uint 8  |
//						 |___________|________|________|
//
//			Features:
//			Auto playing mode(APM)
//				3 control buttons: Prev, Play/Pause, Next
//				Buttons duplicates by UART commands: Prev = 'B', Play/Pause = 'P', Next = 'N'
//				If you want to activate this mode press any key or send UARD command('B', 'P' or 'N')
//			Command controllable playing mode(CCPM)
//				Available UART commands:
//				'S' - Start line
//				'.' - End line and play
//				'O' - Enable output mode
//				'F' - Finish CCPM
//				'R' - Read CCPM DIR  
//				'<' - Stop line reading
//				'B' - Previous track (only for APM)
//				'P' - Play / Pause	 (only for APM)
//				'N' - Next track	 (only for APM)
//			If you want to enable CCPM mode you need send 'S' command and then name (without.WAV) and '.' - end line command
//			after, file with chosen name will play from CCPM dir on SD card. If the file will finish player will return 'F'.
//			If file with chosen name is not existed, player will return 'E' and will go to IDLE state.
//			If you want to abort CCPM playing send 'F' command.
//	
//	10 OCT 2019
//	version 0.1a
//		
//			Fixed buf when at stereo tracks not read new data.
//_________________________________________________________________________________________________________________________*

#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "pff.h"
#include "diskio.h"
#include "standart_interface.h"

#define LED_PORT	PORTD
#define LED_DDR		DDRD
#define LED_PIN		5

#define B_PREV_PORT PORTD
#define B_PREV_DDR	DDRD
#define B_PREV_PIN	2

#define B_PP_PORT   PORTD
#define B_PP_DDR	DDRD
#define B_PP_PIN	3

#define B_NEXT_PORT PORTD
#define B_NEXT_DDR	DDRD
#define B_NEXT_PIN	4

#define PWM_A_PORT  PORTB
#define PWM_A_DDR   DDRB
#define PWM_A_PIN   1

#define PWM_B_PORT  PORTB
#define PWM_B_DDR   DDRB
#define PWM_B_PIN   2

#define BUF_SIZE    512

//Buttons
#define PREVIOUS    1
#define PAUSE		2
#define NEXT		3
#define NONE		0

//Main FSM states
#define POR			0			// First state, active after power-on or after reset
#define IDLE		1			// Idle mode after POR, wait button action or UART command
#define CCPM_I		10			// Initiated by UART
#define CCPM_OD		11			// 
#define CCPM_EOF	12			//
#define CCPM_STOP	13			//
#define CCPM_RD		14			//
#define APM_I		20			// Initializate auto-playing mode
#define APM_CF		21			// Count tracks(songs) amount on disc
#define APM_OD		22			// Open root dir
#define APM_RD		23			// Read dir
#define APM_OF		24			// Open needed file
#define APM_SC		25			// Set playing parameters 
#define APM_WB		26			// Wait EOF or buttons event
#define APM_EOF		27			// End of file procedure 
#define APM_BUT		28			// Buttons procedure
#define FAIL		255			// Failure mode. Blinking LED in cycle 

//UART FSM states
#define WAIT		0
#define LINE		1
#define	PLAY		2

//Player struct
typedef struct wav_data
{
	uint16_t size;
	uint16_t sampleRate;
	uint8_t  timerCoef;
	uint16_t fileCounter;
	uint16_t neededFile;
	uint16_t currentFile;
} WAV;

void initWAV(WAV* obj);
void incFC(WAV* obj);
void decSize(WAV* obj);
void setSize(WAV* obj, char *buffer);
void setSampleRate(WAV* obj, char *buffer);

//Variables used in interrupts
char buf[BUF_SIZE];			//Data buffer, 
uint16_t sell = 0;			
uint8_t	 channel = 1;
uint8_t mode = POR;			//Main FSM state
uint8_t cmode = WAIT;
uint8_t UART_B_Event = NONE;
uint8_t UART_L_Pointer = 0;
uint8_t UART_GOTO_CCPM = 0;
bool	UART_OUT_DATA  = false;
char name[13];				//File name, can be write from UART
char flname[18] = {'C','C','P','M','/'};

FATFS fs;
DIR dir;
FILINFO fno;
WORD s1;
FRESULT result;

ISR(TIMER2_COMP_vect)
{
	TCNT2 = 0;
	OCR1A = buf[sell];
	OCR1B = (channel > 1)? buf[sell+1]:buf[sell];
	if(sell > 509){
		sell = 0;}
	else
		sell += channel;
};

ISR(USART_RXC_vect)
{
	UART_B_Event = NONE;
	uint8_t UARTtemp = UDR;
	switch(cmode)
	{
		case WAIT:
			if (UARTtemp == 'S') 
			{
				for(uint8_t i = 0; i < 13; i++)
					name[i] = 0;
				cmode = LINE;
				break;	
			}
			else if (UARTtemp == 'R')
				mode = CCPM_RD;
			else if (UARTtemp == 'O')
				UART_OUT_DATA = ~UART_OUT_DATA;				
			else if(mode == APM_WB || mode == IDLE)
			{
					 if(UARTtemp == 'B') UART_B_Event = PREVIOUS;
				else if(UARTtemp == 'P') UART_B_Event = PAUSE;
				else if(UARTtemp == 'N') UART_B_Event = NEXT;
				break;
			}			
			break;
		case LINE:
			if(UARTtemp == 8)
			{
				if(UART_L_Pointer != 0) UART_L_Pointer--;
			}
			else if(UARTtemp == '<')
				cmode = WAIT;			
			else if(UARTtemp != '.')
			{
				name[UART_L_Pointer] = UARTtemp;
				UART_L_Pointer++;
			}
			else
			{
				name[UART_L_Pointer] = '.';
				name[UART_L_Pointer+1] = 'W';
				name[UART_L_Pointer+2] = 'A';
				name[UART_L_Pointer+3] = 'V';
				UART_L_Pointer = 0;
				UART_GOTO_CCPM = 1;
				UART_B_Event = 0;
				if(UART_OUT_DATA)
				{
					uart_send_array(name);
					uart_write_char('\n');
					uart_write_char(13);
				}					
				cmode = PLAY;
			}
			break;
		case PLAY:
			if(UARTtemp == 'F')
			{
				UART_GOTO_CCPM = 255;
				cmode = WAIT;	
				UART_B_Event = 0;
				UART_L_Pointer = 0;
			}					
			break;
	}
	if(UART_OUT_DATA)
	{
		uart_write_char(UARTtemp);
		uart_write_char('\n');
		uart_write_char(13);
	}		
};

int8_t isWAV(char *string);
int8_t buttons();

int main(void)
{		
	WAV player;
	initWAV(&player);
	uint16_t current_song = 0;
	uint16_t needed_song = 1;
	uint8_t temp8 = 0;
	bool playing = false;
	
    while(1)
    {
		switch(mode)
		{
//----------------------------------------------------------------------------------------------------------------------
// mode = 0 // POR MODE // Start initialization after power on or after reset.
//----------------------------------------------------------------------------------------------------------------------
			case POR:
				//Set up GPIO
				//LED
				LED_DDR    |=  (1 << LED_PIN);
				//PWM
				PWM_A_DDR  |=  (1 << PWM_A_PIN);
				PWM_B_DDR  |=  (1 << PWM_B_PIN);
				PWM_A_PORT &= ~(1 << PWM_A_PIN);
				PWM_B_PORT &= ~(1 << PWM_B_PIN);
				//Buttons
				//B_PREV_DDR &= ~(1 << B_PREV_PIN);
				//B_PP_DDR   &= ~(1 << B_PP_PIN);
				//B_NEXT_DDR &= ~(1 << B_NEXT_PIN);
				B_PREV_PORT |= (1 << B_PREV_PIN);
				B_PP_PORT   |= (1 << B_PP_PIN);
				B_NEXT_PORT |= (1 << B_NEXT_PIN);
				//Init SD
				for(uint8_t i = 0; i < 5; i++)	//Try to initialize SD card. 5 tries with 1s gap between tries.
				{
					_delay_ms(1000);
					result = pf_mount(&fs);
					if(result == FR_OK)
					{
						mode = IDLE;							// If try is successful go to IDLE mode 
						break;
					}						
					else
						mode = FAIL;							// If all tries are failed go to FAIL mode
				}
			    //Init UART
				uart_init();
				asm("sei");
				break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 1 // IDLE MODE // player wait UART command or buttons request
//----------------------------------------------------------------------------------------------------------------------
			case IDLE:
					LED_PORT &= ~(1 << LED_PIN);
					temp8 = buttons();
					if((temp8 != NONE) || (UART_B_Event != NONE))
					{
						UART_B_Event = NONE; 
						mode = APM_I;
					}		
					if(UART_GOTO_CCPM == 1)
					{
						UART_GOTO_CCPM = 0;
						mode = CCPM_I;				
					}					
					break;
					// Here not described UART behavior because all UART commands handle in UART receive interruption
//----------------------------------------------------------------------------------------------------------------------
// mode =  20// APM_I MODE // Auto playing mode initialization. Read all in root DIR and count all WAV files. 
//----------------------------------------------------------------------------------------------------------------------
			case APM_I:
					result = pf_opendir(&dir,"");
					if(result != FR_OK)							//If read access is failed go to FAIL mode
					{
						mode = FAIL;
						break;
					}
					else
					{
						while(1)
						{
							result = pf_readdir(&dir,&fno);		// Get file or DIR name
							if(result != FR_OK)					// If read access if failed go to FAIL mode.
							{
								mode = FAIL;
								break;
							}
							else
							{
								int8_t temp = isWAV(fno.fname);
								if(temp == 1)
								{
								// increment songs counter
									incFC(&player);				// If current read file if WAV increment files counter
								}
								else if(temp == -1)				// If current read file has 0 length name then it is the last file  
								{
								// stop counting
									mode = APM_OD;				// All file are read, go to APM_OD mode
									break;
								}
							}							
						}
					}					
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 22// APM_OD MODE // Auto playing mode open DIR, Open root dir to start read files from begin.
//----------------------------------------------------------------------------------------------------------------------
			case APM_OD:
					result = pf_opendir(&dir,"");
					if(result != FR_OK)							//If read access is failed go to FAIL mode
					{
						mode = FAIL;
						break;
					}
					mode = APM_RD;								//If read access successful go to APM_RD mode 
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 23// APM_RD MODE // Auto playing mode read DIR, Read root dir to find needed file.
//----------------------------------------------------------------------------------------------------------------------
			case APM_RD:
					result = pf_readdir(&dir,&fno);
					if(result != FR_OK)							//IF read access is failed go to FAIL mode
					{
						mode = FAIL;
						break;
					}
					mode = APM_OF;								//IF read access successful go to APM_OF mode 
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 24// APM_OF MODE // Auto playing mode open file. Open needed file. 
//----------------------------------------------------------------------------------------------------------------------
			case APM_OF:
					if(isWAV(fno.fname) == 1)					//If last read file is WAV
					{
						current_song++;
						if(current_song == needed_song){		//If this file is needed for me
							pf_open(fno.fname);					//Open this file and read metadata
							pf_read(&buf, BUF_SIZE, &s1);
							setSampleRate(&player, &buf);		//Get sample rate
							setSize(&player, &buf);				//Get file size
							pf_read(&buf, BUF_SIZE, &s1);		//Read next block to start playing
							if(UART_OUT_DATA)					//If output mode enabled
							{	// Print filename in console
								uart_send_array(fno.fname);		
								uart_write_char('\n');
								uart_write_char(13);
								// Print sample rate i console
								uart_send_array("Sample rate - ");
								uart_write_char(48 +  player.sampleRate          / 10000);
								uart_write_char(48 + (player.sampleRate % 10000)/ 1000);
								uart_write_char(48 + (player.sampleRate % 1000)/ 100);
								uart_write_char(48 + (player.sampleRate % 100)/ 10);
								uart_write_char(48 + player.sampleRate % 10);
								uart_write_char('\n');
								uart_write_char(13);
								// Print channels amount in console
								uart_send_array("Chan - ");
								uart_write_char(channel+48);
								uart_write_char('\n');
								uart_write_char(13);
								// Print timer coefficient in console
								uart_send_array("TC - ");
								uart_write_char(player.timerCoef		 / 100 + 48);
								uart_write_char((player.timerCoef % 100) / 10 + 48);
								uart_write_char(player.timerCoef % 10		 +48);
								uart_write_char('\n');
								uart_write_char(13);
							}
							mode = APM_SC;}						//Start playing go to APM_SC mode
						else									//If this file isn't needed for me
							mode = APM_RD;						//Read next file
					}
					else										//If last read file isn't WAV
						mode = APM_RD;							//Read next file
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode =  25// APM_SC MODE // auto playing mode set configuration // Preparing to playing, set timers configurations. 
//----------------------------------------------------------------------------------------------------------------------
			case APM_SC:
					TCCR2 |= (1 << CS21);
					TIMSK |= (1 << OCF2);
					OCR2 = player.timerCoef;
					TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM10);
					TCCR1B |= (1 << WGM12)|(1 << CS10);
					LED_PORT |= (1<<LED_PIN);
					playing = true;
					mode = APM_WB;
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 26// APM_WB MODE // Auto playing mode wait buttons or EOF. Wait buttons event or end of file
//----------------------------------------------------------------------------------------------------------------------
			case APM_WB:	
						if(sell >= 509)							// If pointer near end of buffer read fresh data 	
						{
							pf_read(&buf, BUF_SIZE, &s1);
							decSize(&player);					// Decrease amount of blocks to end of file.
						}
						if(player.size == 0)					// If end of file read play new *.WAV file
						{
							needed_song++;
							current_song = 0;
							mode = APM_OD;
						}
						temp8 = buttons();
						if(temp8 != NONE || UART_B_Event != NONE)						// If buttons request go to buttons handler
							mode = APM_BUT;
						if(UART_GOTO_CCPM == 1)
							mode = CCPM_I;					
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 27// APM_BUT MODE // Auto playing mode buttons hander.
//----------------------------------------------------------------------------------------------------------------------
			case APM_BUT:
						if(temp8 == PREVIOUS || UART_B_Event == PREVIOUS)	//Previous track (handles button event and UART event)
						{
							UART_B_Event = NONE;
							if(current_song == 1)							//If current song first - play last in queue
							{
								needed_song = player.fileCounter;
								current_song = 0;
								mode = APM_OD;
							}
							else											//If current song not first - play previous track in queue
							{
								needed_song = current_song-1;
								current_song = 0;
								mode = APM_OD;
							}
							TIMSK &= ~(1 << OCIE2);
							_delay_ms(200);
						}	 						
						else if(temp8 == PAUSE || UART_B_Event == PAUSE)	//Play/pause track (handles button event and UART event)
						{
							UART_B_Event = NONE;
							if(playing)										//If track playing - pause
							{
								playing = false;
								TIMSK &= ~(1 << OCIE2);
								_delay_ms(200);
							}
							else											//If pause - cotinue to play 
							{
								playing = true;
								_delay_ms(200);
								TIMSK |= (1 << OCIE2);
							}
							mode = APM_WB;
						}							
						else if(temp8 == NEXT || UART_B_Event == NEXT)	//Next track (handles button event and UART event)
						{
							UART_B_Event = NONE;
							if(current_song == player.fileCounter)		//If current song last in queue - play first
							{
								needed_song = 1;
								current_song = 0;
								mode = APM_OD;
							}
							else										//If current track not last - play next track
							{
								needed_song = current_song+1;
								current_song = 0;
								mode = APM_OD;
							}
							TIMSK &= ~(1 << OCIE2);
							_delay_ms(200);
							}
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 10// CCPM_I //Command controllable playing mode initialization
//----------------------------------------------------------------------------------------------------------------------
			case CCPM_I:			
					for(uint8_t i = 5; i < 18; i++)				//Merge directive path and file name
					{
						flname[i] = name[i-5]; 
					}
					result = pf_open(flname);					//Open file in CCPM directive(folder)
					if(result != FR_OK)							//If read access is failed go to FAIL mode
					{
						uart_write_char('E');					//Error
						mode = IDLE;
						cmode = WAIT;
						break;
					}
					pf_read(&buf, BUF_SIZE, &s1);
					setSampleRate(&player, &buf);				//Get sample rate
					setSize(&player, &buf);						//Get file size
					pf_read(&buf, BUF_SIZE, &s1);				//Read next block to start playing
					TCCR2 |= (1 << CS21);
					TIMSK |= (1 << OCF2);
					OCR2 = player.timerCoef;
					TCCR1A |= (1 << COM1A1)|(1 << COM1B1)|(1 << WGM10);
					TCCR1B |= (1 << WGM12)|(1 << CS10);
					LED_PORT |= (1<<LED_PIN);
					playing = true;
					mode = CCPM_STOP;
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 11// CCPM_STOP //Command controllable playing mode wait stop event
//----------------------------------------------------------------------------------------------------------------------
			case CCPM_STOP:
					if(sell >= 509)									// If pointer near end of buffer read fresh data
					{
						pf_read(&buf, BUF_SIZE, &s1);
						decSize(&player);							// Decrease amount of blocks to end of file.
					}
					if(player.size == 0 || UART_GOTO_CCPM == 255)	// If end of file read play new *.WAV file
					{
						current_song = 0;
						mode = CCPM_EOF;
						break;
					}					
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 12// CCPM_EOF //Command controllable playing mode end of file
//----------------------------------------------------------------------------------------------------------------------
			case CCPM_EOF:											//If end of file return 'F' and return to IDLE mode
					uart_write_char('F');
					mode = IDLE;
					cmode = WAIT;
					TIMSK &= ~(1 << OCIE2);
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode = 14// CCPM_RD //Command controllable playing mode read directive
//----------------------------------------------------------------------------------------------------------------------			
			case CCPM_RD:										//Return all file names from CCPM DIR
					result = pf_opendir(&dir,"CCPM");
					if(result != FR_OK)							//If read access is failed go to FAIL mode
					{
						uart_write_char('E');
						mode = IDLE;
						cmode = WAIT;
						break;
					}
					while(1)
					{
						pf_readdir(&dir,&fno);
						uart_send_array(fno.fname);
						uart_write_char('\n');
						uart_write_char(13);
						if(fno.fname[0] == '\0')
						{
							mode = IDLE;
							break;		
						}											
					}
					break;
//----------------------------------------------------------------------------------------------------------------------
// mode =  255// FAIL MODE// Blink LED in infinite loop. One possible way to go out is reset.
//----------------------------------------------------------------------------------------------------------------------
			case FAIL: 
					while(1)
					{
						LED_PORT ^= (1<<LED_PIN);
						_delay_ms(300);
					}
					break;
		}			
    }
}

void initWAV(WAV* obj)						// Init. structure
{
	obj->currentFile = 0;
	obj->fileCounter = 0;
	obj->neededFile = 0;
	obj->sampleRate = 0;
	obj->size = 0;
	obj->timerCoef = 0;
}

void incFC(WAV* obj)						// Increment file counter
{
	obj->fileCounter++;
}

void decSize(WAV* obj)						// Decrement file size
{
	obj->size--;
}

void setSize(WAV* obj, char *buffer)		// Set file size 
{
	uint32_t temp = ((uint32_t)(*(buffer+7))|(((uint32_t)*(buffer+6))<<8)|(((uint32_t)*(buffer+5))<<16)|(((uint32_t)*(buffer+4))<<24));
	obj->size = (uint16_t)(temp>>9);
}

void setSampleRate(WAV* obj, char *buffer)	// Set sample rate and time coefficient
{
	channel = *(buffer+22);
	//channel = 2;
	obj->sampleRate = (((uint16_t)(*(buffer + 25)))<<8) | *(buffer+24);
	obj->timerCoef = (uint8_t)((2000000/obj->sampleRate)+1);
}


int8_t buttons()							// Read buttons state
{
	if(!(PIND & (1 << B_PREV_PIN)))
		return PREVIOUS;
	if(!(PIND & (1 << B_PP_PIN)))
		return PAUSE;
	if(!(PIND & (1 << B_NEXT_PIN)))
		return NEXT;
	return NONE;	
}

int8_t isWAV(char *string)					//Check if the most resently read file is *.WAV
{
	for(uint8_t i = 0; i < 13; i++)
	{
		if(*(string+i) == '\0')				//If name have 0 length it is end of directive 
		{
			if(i == 0)
				return -1;					
			return 0;
		}
		else if(((*(string+i) == '.') && (*(string+i+1) == 'W') && (*(string+i+2) == 'A') && (*(string+i+3) == 'V'))||
				((*(string+i) == '.') && (*(string+i+1) == 'w') && (*(string+i+2) == 'a') && (*(string+i+3) == 'v')))
		return 1;
	}
}