//; 15 adc interupt enable these
//;5 timer2 interupt enable these
#include <c8051f020.h>
#include <lab07.h>

void send_c(char single_char)
{
	int i = x * 5 * y * 100;
	x = x*5;
	y = y * 256;
	for(i = x + y; i < 5; i ++) {
		screen[i] = font5x8[single_char + 20 *5 + j];
		}
}

void send_temp( int x, int y, char *temp){
	while(*temp) {
		screen[x] = send_c(x, y, *temp);
	}
}

void end() {
	while(1){}
}

void main()
{
	WDTCN = 0xde; //disable watchdog
	WDTCN = 0xad;
	XBR2 = 0x40;  //port output
	XBR0 = 4;	  //enable uart 0
	OSCXN = 0x67; //crystal enabled
	TMOD = 0x20;  //wait 1ms: T1 mode 2
	TH1 = 167;	  // 1ms/(1/(2Mhz/12)) = 166.666
	TR1 = 1;
	while( TF1 == 0) {}	//1 ms wait for flag
	while( !(OSCXCN & 0x80)) {} //stabilize crystal
	OSCICN = 8;   // switch to 22.1184Mhz
	SCON0 = 0x50;	//8-bit, var baud, recieve enable
	TH1 = -6;		//9600 baud
	AMX0CF = 0x00;  
	AMX0SL = 0x0f;	//Temp sensor
	ADC0CF = 0x40;	
	AD0CN = 0x80;

	REF0CN = 0x03;
	init_lcd();
	send_temp(1, 1, "start up");
	refresh_lcd();
	end();
}
