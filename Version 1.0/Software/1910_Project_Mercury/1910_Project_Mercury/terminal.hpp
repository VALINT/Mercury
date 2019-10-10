/*------------------------------------------------------------------------------------------------------
 * Terminal (serial connection with PC or other devices)
 * Realize like Arduino serial connection for debugging or data communication.
 *______________________________________________________________________________________________________
 *  __    __ __ __     __   _____	 _____  ____  ____  __  __  _  __  _    _    _
 *  \ \  / //  \\ \    \_\ / ___/	|_   _|| ___|| |\ \|  \/  || ||  \| |  / \  | |
 *   \ \/ //    \\ \___ |/ \___ \	  | |  | ___|| |/ /| |\/| || || |\  | / _ \ | |__
 *    \__//__/\__\\____\   /____/     |_|  |____||_|\_\|_|  |_||_||_| \_|/_/ \_\|____|
 *_______________________________________________________________________________________________________
 *
 * Created: 14-Oct-2018 23:05:14
 *  Author: VAL
 *///------------------------------------------------------------------------------------------------------ 
//For true work needed to use font - "terminal"


#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "standart_interface.h"

//#define PuTTy

char line[16];

#define ARRAY &(line[0])
#define PrintLn1(text1) sprintf(line, text1); uart_send_array(ARRAY); uart_write_char('\n'); uart_write_char(13);
#define PrintLn2(text1, text2) sprintf(line, text1, text2); uart_send_array(ARRAY); uart_write_char('\n'); uart_write_char(13);
#define PrintLn3(text1, text2, text3) sprintf(line, text1, text2, text3); uart_send_array(ARRAY); uart_write_char('\n'); uart_write_char(13);
#define PrintLn4(text1, text2, text3, text4) sprintf(line, text1, text2, text3, text4); uart_send_array(ARRAY); uart_write_char('\n'); uart_write_char(13);

#define PrintStr1(text1) sprintf(line, text1,'\0'); uart_send_array(ARRAY);
#define PrintStr2(text1, text2) sprintf(line, text1, text2,'\0'); uart_send_array(ARRAY);
#define PrintStr3(text1, text2, text3) sprintf(line, text1, text2, text3,'\0'); uart_send_array(ARRAY);
#define PrintStr4(text1, text2, text3, text4) sprintf(line, text1, text2, text3, text4); uart_send_array(ARRAY);

#define NewLine  uart_write_char('\n'); uart_write_char(13);

//Next Function Print next in terminal.
//*______________________________________________________________________________________________________");
//*  __    __ __ __     __   _____	 _____  ____  ____  __  __  _  __  _    _    _
//*  \ \  / //  \\ \    \_| / ___/	|_   _|| ___|| |\ \|  \/  || ||  \| |  / \  | |
//*   \ \/ //    \\ \___ \/ \___ \	  | |  | ___|| |/ /| |\/| || || |\  | / _ \ | |__
//*    \__//__/\\__\\____\   /____/   |_|  |____||_|\_\|_|  |_||_||_| \_|/_/ \_\|____|
//*______________________________________________________________________________________________________");
/*
void hi(void)
{
	PrintLn1("*______________________________________________________________________________________________________");
	PrintLn1("*  __    __ __ __     __   _____	 _____  ____  ____  __  __  _  __  _    _    _");
	PrintLn1("*  \\ \\  / //  \\\\ \\    \\_| / ___/	|_   _|| ___|| |\\ \\|  \\/  || ||  \\| |  / \\  | |");
	PrintLn1("*   \\ \\/ //    \\\\ \\___ \\/ \\___ \\	  | |  | ___|| |/ /| |\\/| || || |\\  | / _ \\ | |__");
	PrintLn1("*    \\__//__/\\__\\\\____\\   /____/          |_|  |____||_|\\_\\|_|  |_||_||_| \\_|/_/ \\_\\|____|");
	PrintLn1("*______________________________________________________________________________________________________");
}*/
#endif /* TERMINAL_H_ */