#include <c8051f020.h>
#include <lcd.h>
#include<stdio.h>
#include<stdlib.h>
 
long score_2, score, score_1, high_score = 0;
sbit player_sw = P1^7;
char switches, bonk = 0; 
bit player, num_player = 0;
int int_cnt, t_cnt, t_cnt_init, pad_w, pot_val, count = 0;
char ball_1, ball_2 = 3;
char xpos, ypos, xangle, yangle = 0;
code unsigned char ball[] = {0x0E, 0x1F, 0x1F, 0x1F, 0x0E};
code unsigned char sine[] = { 176, 217, 244, 254, 244, 217, 176, 128, 80, 39, 12, 2, 12, 39, 80, 128 };
xdata unsigned char blocks_1[11][5] = {
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1}
    }; 
xdata unsigned char blocks_2[11][5] = {
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1}
    };
unsigned long sum = 0;
sbit button = P2^6;

void disp_char(unsigned char row, unsigned char col, char single_char);
void disp_score(int x, int y, unsigned long score);
void display();
void mov_ball();
unsigned char draw_ball(int x, int y);
void wait_screen();
void game_over();
void turn_end();
void draw_paddle(char x, char padd_w);

//void adc_init(){
//	REF0CN = 0x7;
//	ADC0CF = 0x40;
//	ADC0CN = 0x8C;
//	AMX0SL = 0;
//}
unsigned char phase = sizeof(sine)-1;	// current point in sine to output

unsigned int duration = 0;		// number of cycles left to output

void timer4(void) interrupt 16
{
	T4CON = T4CON^0x80;
	DAC0H = sine[phase];
	if ( phase < sizeof(sine)-1 )	// if mid-cycle
	{				// complete it
		phase++;
	}
	else if ( duration > 0 )	// if more cycles left to go
	{				// start a new cycle
		phase = 0;
		duration--;
	}
	if (duration == 0){
		T4CON = 0x00;
		}
}

void pot() interrupt 15{
	
	unsigned long samp = 0;
	unsigned int D;
	AD0INT = 0;

	D = (ADC0L|(ADC0H << 8));

	samp = (89L - pad_w)*D/4096;
	sum += samp;
	count++;
	
	if(count % 7 == 0)
	{
		pot_val = sum >> 3;
		sum = 0;
	}

}

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

void disp_score(int x, int y, unsigned long score){
	int thou = 0;
	int hund = 0;
	int tens = 0;
	int ones = 0;
	thou = score/1000;
	score = score%1000;
	hund = score/100;
	score = score%100;
	tens =score/10;
	ones = score%10;
	disp_char(x, y, thou + '0');
	disp_char(x, y+6, hund + '0');
	disp_char(x, y+12, tens + '0');
	disp_char(x, y+18, ones + '0');
}

void display()
{
	int i;

	if(score > high_score)
	{
		high_score = score;
	}

	
	for(i = 0; i < 82; i++)
	{
		screen[i] |= 3;
	}
	for(i = 0; i < 8; i++)
	{
		screen[i*128] |= 255;
		screen[i*128 + 1] |= 255;
		screen[i*128 + 81] |= 255;
		screen[i*128 + 80] |= 255;
	}
	disp_char(0, 89, 'H');
	disp_char(0, 95, 'I');
	disp_char(0, 101, 'G');
	disp_char(0, 107, 'H');
	disp_char(0, 113, ':');

	disp_score(1, 93, high_score);

	disp_char(2, 87, 'S');
	disp_char(2, 93, 'C');
	disp_char(2, 99, 'O');
	disp_char(2, 105, 'R');
	disp_char(2, 111, 'E');
	disp_char(2, 117, ':');
	
	//update for two players
	disp_score(3, 93, score);

	disp_char(4, 99, 'P');
	disp_char(4, 105, '1');
	if(ball_1 == 3){
		disp_char(5, 87, '*');
		disp_char(5, 102, '*');
		disp_char(5, 117, '*');
		}
	else if (ball_1 == 2){
		disp_char(5, 87, '*');
		disp_char(5, 102, '*');
		}
	else if(ball_1 == 1) {
		disp_char(5, 87, '*');
		}
	if(num_player == 1)
	{
		disp_char(6, 99, 'P');
		disp_char(6, 105, '2');
	}
	if(ball_2 == 3){
		disp_char(7, 87, '*');
		disp_char(7, 102, '*');
		disp_char(7, 117, '*');
		}
	else if (ball_2 == 2){
		disp_char(7, 87, '*');
		disp_char(7, 102, '*');
		}
	else if(ball_2 == 1) {
		disp_char(7, 87, '*');
		}

	//potentially update player scores here
	if(player == 0)
	{
		disp_char(4, 93, '-');
		disp_char(4, 111, '-');
	}
	else
	{
		disp_char(6, 93, '-');
		disp_char(6, 111, '-');
	}

	refresh_screen();
}

void wait_screen(){
	TR2 = 0;
	xpos = 40;//middle of screen
	ypos = 40;//one pixel below the bricks
	xangle = 1;
	yangle = 1;
	blank_screen();
	
	
	disp_char(2, 30, 'P');
	disp_char(2, 36, 'R');
	disp_char(2, 42, 'E');
	disp_char(2, 48, 'S');
	disp_char(2, 54, 'S');
	disp_char(3, 30, 'S');
	disp_char(3, 36, 'T');
	disp_char(3, 42, 'A');
	disp_char(3, 48, 'R');
	disp_char(3, 54, 'T');
	//refresh_screen();
	display();
	if(ball_1 == 3 && ball_2 == 3){ 
		while (button == 1){
		pad_w = P1&3;
		t_cnt_init = P1&24;
		t_cnt_init = t_cnt_init >>3;
		num_player = player_sw;
		
			}
		if(pad_w == 3){
			pad_w = 24;
		}
		else{
			pad_w = pad_w*4 +8;
		}
		t_cnt_init = t_cnt_init*10 + 30; 
		t_cnt = t_cnt_init;
		if(num_player == 0){
		ball_2 = 0;
		}
	}
	else{
		while (button == 1){};
	}
	TR2 = 1;	
}


void game_over()
{
	TR2 = 0;

	blank_screen();
	disp_char(2, 30, 'G');
	disp_char(2, 36, 'A');
	disp_char(2, 42, 'M');
	disp_char(2, 48, 'E');
	disp_char(3, 30, 'O');
	disp_char(3, 36, 'V');
	disp_char(3, 42, 'E');
	disp_char(3, 48, 'R');

	display();
	while(1){}
	
}

void turn_end()
{
	t_cnt = t_cnt_init;
	if(num_player == 1){
		
		if(player == 0){
			ball_1 = ball_1 -1;
			player = 1;
			score_1 = score;
			score = score_2;
		}
		else{
			if(ball_2 == 1){
				ball_2 -= 1;
				game_over();}
			else{
				ball_2 = ball_2 -1;
				player = 0;
				score_2 = score;
				score = score_1;
			}
		}
	}

	else{
		if(ball_1 == 1){
			game_over();}
		else{
			ball_1 = ball_1 -1;
			}
	}
	wait_screen();
}

void draw_bricks(){
	int i;
	int j;
	int k;
	int zero_cnt = 0;
	xdata unsigned char blocks[11][5] = {0};
	if(player == 0)
	{
		for(i= 0; i < 11; i ++)
		{
			for(j = 0; j < 5; j ++)
			{
				blocks[i][j] = blocks_1[i][j];
			}
		}		
	}
	else
	{
		for(i= 0; i < 11; i ++)
		{
			for(j = 0; j < 5; j ++)
			{
				blocks[i][j] = blocks_2[i][j];
			}
		}	
	}

	for(i= 0; i < 11; i ++)
	{
		for(j = 0; j < 5; j ++)
		{
			if(blocks[i][j] == 1)
			{
				if(j%2 == 1)
				{
					for(k = 0; k < 6; k++)
					{
						screen[((j-1)/2)*128 + i*7 +k +131] |= 0x70;
					}
				}
				else
				{
					for(k = 0; k < 6; k++)
					{
						screen[(j/2)*128 + i*7 + k +131] |= 0x07;
					}
				}
			}
			else
			{
				zero_cnt ++;
				if(j%2 == 1)
				{
					for(k = 0; k < 6; k++)
					{
						screen[(j-1)/2*128 + i*7 + k + 131] &= 0x07;
						
					}
				}
				else
				{
					for(k = 0; k < 6; k++)
					{
						screen[j/2*128 + i*7 + k + 131] &= 0x70;
					}
				}
			}
		}
	}
	if(zero_cnt == 55 && ypos > 40){
		if(player == 0)
		{
			for(i= 0; i < 11; i ++)
			{
				for(j = 0; j < 5; j ++)
				{
				blocks_1[i][j] = 1;
				}
			}		
		}
		else
		{
			for(i= 0; i < 11; i ++)
			{
				for(j = 0; j < 5; j ++)
				{
				blocks_2[i][j] = 1;
				}
			}	
		}
	} 
}

void draw_paddle(char x, char padd_w)
{
	int i;
	pad_w = (int)pad_w;
	for(i = 0; i < padd_w; i ++)
	{
		screen[898+x+i] |= 0xc0;
	}
}

unsigned char draw_ball(int x, int y)
{	unsigned char row, col, shift, j, hit;
	int i;
	
	col = x-2;
	row = y - 2;
	shift = row%8;
	row = row/8;
	//row >> 3;
	hit = 0;
	for(j = 0, i = row*128+col; j < 5; i++, j++)
	{
		int mask = (int)ball[j] << shift;
		hit |= screen[i]&(unsigned char)mask;
		screen[i] |= mask;

		if(mask & 0xFF00)
		{
			hit |= screen[i+128]&(unsigned char)(mask >> 8);
			screen[i + 128] |= (unsigned char)(mask >> 8);
		}
	}
	if((x<5 || x > 78) && y < 3)
	{
		yangle = -1*yangle;
		xangle = -1*xangle;
		return -2;
	}
	else if(x<5 || x > 78)
	{
		xangle = -1*xangle;
		return -2;
	}

	else if (y < 3)
	{
		yangle = -1*yangle;
		return -2;
	}
	else if(y > 61)
	{
		turn_end();
		return -2;
	}
	if( y == 60 && hit > 0){

		char col = xpos - pot_val -2;
		int div = pad_w/4;
		if( col < div || col > 3*div)
		{
			xangle = 2;
			yangle = -1;
		}
		else
		{
			xangle = 1;
			yangle = -2;
		}
		if(col < div *2)
		{
			xangle = -1*xangle;
		}
		//score += 1;
		return -1;
	}
	else if( hit > 0)
	{
		
		int x_b, y_b;
		score += 1;
		if(yangle < 0)
		{
			y_b = (ypos -10)/4;
		}
		else{
			y_b = (ypos -6)/4;
		}
		if(xangle < 0)
		{
			x_b = (xpos -5)/7;
		}
		else
		{
			x_b = (xpos -1)/7;
		}
			
		if(player == 0)
		{
			if(blocks_1[x_b][y_b] == 0)
				{
					if(xangle < 0)
						{
							y_b = (ypos -8)/4;
							blocks_1[x_b-1][y_b] = 0;
						}
					else
						{
							y_b = (ypos -8)/4;
							blocks_1[x_b+1][y_b] = 0;
						}
					xangle = -1*xangle;
				}
			else
			{
				blocks_1[x_b][y_b] = 0;
				yangle = -1*yangle;
			}
		}
		else
		{
			if(blocks_2[x_b][y_b] == 0)
			{
				if(xangle < 0)
				{
					blocks_2[x_b-1][y_b] = 0;
				}
				else
				{
					blocks_2[x_b+1][y_b] = 0;
				}
					xangle = -1*xangle;
			}
			else
			{
				blocks_2[x_b][y_b] = 0;
				yangle = -1*yangle;
			}	
		}
		if(y_b <= 3)
		{
			t_cnt -= (t_cnt*(1/8));
		}
	}
	return hit;
}


void mov_ball() {
	xpos += xangle;
	ypos += yangle;
	bonk = draw_ball(xpos, ypos);
	display();
}

void timer2(void) interrupt 5
{
	
	TF2 = 0;
	int_cnt ++;
		
}





void main()
{
	WDTCN = 0xde; //disable watchdog
	WDTCN = 0xad;
	XBR2 = 0x40;  //port output
	//XBR0 = 4;	  //enable uart 0
	OSCXCN = 0x67; //crystal enabled
	TMOD = 0x20;  //wait 1ms: T1 mode 2
	TH1 = 167;	  // 1ms/(1/(2Mhz/12)) = 166.666
	TR1 = 1;
	while( TF1 == 0) {}	//1 ms wait for flag
	while( !(OSCXCN & 0x80)) {} //stabilize crystal
	OSCICN = 8;   // switch to 22.1184Mhz
	//SCON0 = 0x50;	//8-bit, var baud, recieve enable
	//TH1 = -6;		//9600 baud
	
	IE = 0xA0;
	EIE2 = 0x06; //Timer 4 and ADC
	
	T4CON = 0x00; //timer 4, auto reload
	RCAP4H = -1;	//timer 4
	RCAP4L = -144;	//timer 4
	//REF0CN = 3;		//set up refrence voltage
	DAC0CN = 0x9C;	//DAC0CN

	T2CON = 0x00;
	RCAP2H = -2211 >> 8;
	RCAP2L = -2211;

	ADC0CN = 0x8c;
	REF0CN = 0x07;
	ADC0CF = 0x40;
	AMX0SL = 0x0;
	CKCON = 0x40;
	//EXF2 = 1;
	TR2 = 1;
	T4CON = T4CON^0x04;
	
	init_lcd();
	//t_cnt = 50; //change depending on switches
	xpos = 40;//middle of screen
	ypos = 40;//one pixel below the bricks

	xangle = 1;
	yangle = 1;
	pad_w = 8;
	ball_1 = 3;
	ball_2 = 3;
	draw_ball(xpos, ypos);
	//draw_bricks();
	display();
	wait_screen();
	
	
	
	
	while(1)
	{
		if(int_cnt==t_cnt) {
		blank_screen();
		draw_paddle(pot_val, pad_w);
		draw_bricks();
		if(bonk > 0){
			RCAP4L = -2097;				// set up for 659Hz
	   		duration = 400;				// one second
			T4CON = T4CON^0x04;
		}
		else if(bonk ==-1)
		{
			RCAP4L = -2642;				// set up for 523Hz
	   		duration = 400;
			T4CON = T4CON^0x04;
		}
		else if(bonk == -2)
		{
			RCAP4L = -2354;				// set up for 587Hz
	   		duration = 400;
			T4CON = T4CON^0x04;
		}
		mov_ball();
		int_cnt = 0;
	}
	}


}