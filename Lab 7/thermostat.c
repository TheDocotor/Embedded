//; 15 adc interupt enable these
//;5 timer2 interupt enable these
#include <c8051f020.h>
#include <lab07.h>
void adc_init(void);
unsigned int adc_val();
void disp_temp(unsigned int tempC);
void disp_set(unsigned int set_temp);
void led(unsigned int temp, unsigned int set_temp);
unsigned int pot();
void disp_char(unsigned char row, unsigned char col, char single_char)
{
	int i, j;
	unsigned char k;
	i = 128*row+col;
	j = (single_char - 0x20)*5;
	for(k = 0; k < 5; k ++) {
		screen[i+k] = font5x8[j + k];
		}
}
void adc_init(){
	REF0CN = 0x7;
	ADC0CF = 0x41;
	ADC0CN = 0x8C;
	AMX0SL = 8;
} 
unsigned int adc_val(){
	unsigned long sum = 0;
	unsigned long samp = 0;
	unsigned int D;
	int i;
	ADC0CF = 0x41;
	ADC0CN = 0x8C;
	AMX0SL = 8;
	for(i = 0; i < 512; ++i){
		while(AD0INT == 0){}
		AD0INT = 0;
		D = (ADC0L |(ADC0H << 8));
		samp = ((D-2475)*12084L)>>16;
		sum += samp;
		}
	return (unsigned long)(sum/512);
}

unsigned int pot(){
	unsigned long sum = 0;
	unsigned long samp = 0;
	unsigned int D;
	int i;
	AMX0SL = 0x00;
	ADC0CF = 0x40;

	for(i = 0; i < 512; ++i){
		while(AD0INT == 0){}
		AD0INT = 0;
		D = (ADC0L|(ADC0H << 8));
		samp = 55 + D*31L/4096;
		sum += samp;
		}
	return (unsigned long)(sum/512);
}

void disp_temp(unsigned int tempC){
	int tempF = 0;
	int tens = 0;
	int ones = 0;
	tempF = (9/5)*tempC +10;
	tens = tempF/10;
	ones = tempF%10;
	disp_char(2, 15, tens + '0');
	disp_char(2, 20, ones + '0');
	refresh_screen();
}

void disp_set(unsigned int set_temp){
	int tens = 0;
	int ones = 0;
	tens = (set_temp/10)%10;
	ones = set_temp%10;
	disp_char(2, 50, tens + '0');
	disp_char(2, 55, ones + '0');
	refresh_screen();
}

void led(unsigned int temp, unsigned int set_temp){
	if(temp < set_temp){
		P3 = 0x00;
		P2 = 0x00;
	}
	else{
		P3 = 0xFF;
		P2 = 0xFF;
	}
}
		
void main()
{
	unsigned int temp, set;
	WDTCN = 0xde; //disable watchdog
	WDTCN = 0xad;
	XBR2 = 0x40;  //port output
	XBR0 = 4;	  //enable uart 0
	OSCXCN = 0x67; //crystal enabled
	TMOD = 0x20;  //wait 1ms: T1 mode 2
	TH1 = 167;	  // 1ms/(1/(2Mhz/12)) = 166.666
	TR1 = 1;
	while( TF1 == 0) {}	//1 ms wait for flag
	while( !(OSCXCN & 0x80)) {} //stabilize crystal
	OSCICN = 8;   // switch to 22.1184Mhz
	SCON0 = 0x50;	//8-bit, var baud, recieve enable
	TH1 = -6;		//9600 baud
	
	RCAP2H = -843 >> 8;
	RCAP2L = -843;
	TR2 = 1;
	init_lcd();
	adc_init();

	while(1)
	{
		temp = adc_val();
		set = pot();
		disp_temp(temp);
		disp_set(set);
		led(temp + 10, set);
	}
}
	
