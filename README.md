# Primitive_WAV_player-Prj.Mercury-
It's primitive WAV player on AVR microcontroller

Description:
  Primitive wav player on ATMEGA8a (possible to change MCU on other AVR MCU than has not less then 8kB flash memory).
  Automaticaly reads some needed data from RIFF chain on begin of file.
   - File size;
   - Sample rate;
   - Channels amount;
   
   
  This parameters automaticaly applies befor file start play.
  Can works only with 8-bit unsigned int PCM sound;
  One and two channels(8000 - 32000 Samples/s for 1 Channel and 8000 - 16000 Samples/s for 2 channels).
  Works only with SD (cads who has les then 2GB memory) In ATMEGA8a not enough memory to add SDHC maintaining.
  Two playing modes APM (plays files only from root dir) and CCPM mode (plays files from CCPM dir, controled only by UART)

Features:
	Auto playing mode(APM)
		3 control buttons: Prev, Play/Pause, Next
		Buttons duplicates by UART commands: Prev = 'B', Play/Pause = 'P', Next = 'N'
		If you want to activate this mode press any key or send UARD command('B', 'P' or 'N')
	Command controllable playing mode(CCPM)
		Available UART commands:
		'S' - Start line
		'.' - End line and play
		'O' - Enable output mode
		'F' - Finish CCPM
		'R' - Read CCPM DIR  
		'<' - Stop line reading
		'B' - Previous track (only for APM)
		'P' - Play / Pause	 (only for APM)
		'N' - Next track	 (only for APM)
	If you want to enable CCPM mode you need send 'S' command and then name (without.WAV) and '.' - end line command
	after, file with chosen name will play from CCPM dir on SD card. If the file will finish player will return 'F'.
	If file with chosen name is not existed, player will return 'E' and will go to IDLE state.
	If you want to abort CCPM playing send 'F' command.
	If you want to abort CCPM playing send 'F' command.
