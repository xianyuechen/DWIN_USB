PROJECT: DWIN_CH376
DESCRIPTION: Include Keil project and Code about dwin USB driver.

FILE STRUCTURE
|||
 |
 |
 V

main.c
|
|
app-----/app_dgus-----/usb_dgus.c
|		|		|
|		|		 -----/usb_dgus.h
|		|	
|		/app_usb-----/app_interface.c
|		|		|
|		|		 -----/app_interface.h
|		|		|
|		|		 -----/file_sys.c
|		|		|
|		|		 -----/file_sys.h
|		|
|		/
|
|
driver------/system-----/sys.c
|			|	  |
|			|	   -----/sys.h
|			|
|			/uart-----/uart.c
|			|	|
|			|	 -----/uart.h
|			|
|			/dgus-----/dgus.c
|			|	|
|			|	 -----/dgus.h
|			|
|			/usb-----/ch376.h
|			|  |
|			|   -----/para_port.c
|			|  |
|			|   -----/para_port.h
|			|
|			/
|
obj------/1.bat
| |
|  ------/srec_cat.exe
|		  
|