#include <c8051f020.h>
#include <lcd.h>
#include<stdio.h>
#include<stdlib.h>

long score_2, score, score_1, high_score = 0;
sbit player_sw = P1^7;
char switches, bonk = 0; 
bit player, num_player, su = 0;
int int_cnt, t_cnt, t_cnt_init, pad_w, pad_w2, pot_val, count = 0;
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
////////////////////////////////////////////////////////////////////////////////////
//						Timer 4 Interupt: Used for Game audio
//	Occurs when timer four flag is raised.
// 	This plays the sound bite for the desired duration.
// 	Audio is set up for certain frequencies and then played through this interupt.
//
////////////////////////////////////////////////////////////////////////////////////

unsigned char phase = sizeof(sine)-1;	// current point in sine to outputcode 

unsigned int duration = 0;		// number of cycles left to output

void timer4(void) interrupt 16
{
	T4CON = T4CON^0x80;
	DAC0H = sine[phase];
	if ( phase < sizeof(sine)-1 )	// if mid-cycle
	{				
		phase++;					// complete it
	}
	else if ( duration > 0 )	// if more cycles left to go
	{				
		phase = 0;				// start a new cycle
		duration--;
	}
	if (duration == 0){			//Turn off timer when the sound is done
		T4CON = 0x00;
		}
}

////////////////////////////////////////////////////////////////////////////////////
//					Potentiometer Reading: Used for moving the paddle
//	Occurs when ADC flag is raised and it samples the potentiometer.
// 	Takes several samples from the potentiometer and averages them.
//
////////////////////////////////////////////////////////////////////////////////////
void pot() interrupt 15{
	
	unsigned long samp = 0;
	unsigned int D;
	AD0INT = 0;

	D = (ADC0L|(ADC0H << 8)); //Takes in the Data from the potentiometer
	if(player == 0){
	samp = (89L - pad_w)*D/4096;	//This normalizes the sample to fit within the play area for player 1
	}
	else{
	samp = (89L - pad_w2)*D/4096;	//This normalizes the sample to fit within the play area for player 2
	}
	sum += samp;					//Takes the sum of 8 samples
	count++;
	
	if(count % 7 == 0) 				//after 8 samples take the average and update the potentiometers
	{
		pot_val = sum >> 3;			//bit shift for division
		sum = 0;
	}

}

////////////////////////////////////////////////////////////////////////////////////
//						Displays Characters to Screen
//	Input: 
// 		-Row: 0-7 On the screen
//		-Col: 0-127 Desired column to start the character
//		-single_char: One Keyboard character to be printed to the screen.
// 	Output:
//		-Displays single character on the LCD display
////////////////////////////////////////////////////////////////////////////////////
void disp_char(unsigned char row, unsigned char col, char single_char)
{
	int i, j;
	unsigned char k;
	i = 128*row+col; 				//takes the row and column and translates it to the exact position on LCD
	j = (single_char - 0x20)*5;		//translates the single char to be comptible with the font function.
	for(k = 0; k < 5; k ++) {
		screen[i+k] = font5x8[j + k]; //calls font and prints the char
		}
}

////////////////////////////////////////////////////////////////////////////////////
//						Displays Score to Screen
//	Input: 
// 		-X: 0-7 row on the screen
//		-Y: 0-127 Desired column to start the character
//		-Score: Four digit score to be printed on the screen
// 	Output:
//		-Displays entire score on the LCD display
////////////////////////////////////////////////////////////////////////////////////
void disp_score(int x, int y, unsigned long score){
	int thou = 0;
	int hund = 0;
	int tens = 0;
	int ones = 0;
	thou = score/1000; 	//takes the thousands place
	score = score%1000;	//remove the thousands place
	hund = score/100;	//takes the hundreds place
	score = score%100;	//remove the hundreds place
	tens =score/10;		//takes the tens place
	ones = score%10;	//takes the ones place
	disp_char(x, y, thou + '0');	//display thousands
	disp_char(x, y+6, hund + '0');	//display hundreds
	disp_char(x, y+12, tens + '0');	//display tens
	disp_char(x, y+18, ones + '0');	//display ones
}

////////////////////////////////////////////////////////////////////////////////////
//						Display configuration
//	This function sets up the display area for the game. 
//	Prints the boarders for the play area 79x62 with two bit boarder on the outside
// 	Displays high score, current player score, player 1's balls left,
//	and player two's balls left.
////////////////////////////////////////////////////////////////////////////////////
void display()
{
	int i;

	if(score > high_score)	//updates high score if current score is greater
	{
		high_score = score;	
	}

	
	for(i = 0; i < 82; i++)
	{
		screen[i] |= 3; 	//Prints the top boarder
	}
	for(i = 0; i < 8; i++)	//print the side walls
	{
		screen[i*128] |= 255;		//255 is a full column on a page
		screen[i*128 + 1] |= 255;	//left two boarders
		screen[i*128 + 81] |= 255;	//right two boarders
		screen[i*128 + 80] |= 255;
	}

	//All scores and player's balls are printed on the right side of the screen.

	//Display High Score
	disp_char(0, 89, 'H');
	disp_char(0, 95, 'I');
	disp_char(0, 101, 'G');
	disp_char(0, 107, 'H');
	disp_char(0, 113, ':');

	disp_score(1, 93, high_score);

	//display current player's score
	disp_char(2, 87, 'S');
	disp_char(2, 93, 'C');
	disp_char(2, 99, 'O');
	disp_char(2, 105, 'R');
	disp_char(2, 111, 'E');
	disp_char(2, 117, ':');
	
	disp_score(3, 93, score);
	
	//Display player ones balls left
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

	//If more than one player display player two's info
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

	//Displays "-P*-" according to who's turn it is
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

	//displays the screen
	refresh_screen();
}

void wait_screen(){
	
	TR2 = 0;
	xpos = 40;//middle of screen
	ypos = 40;//one pixel below the bricks
	xangle = 1;
	yangle = 1;
	blank_screen();
	
	
	disp_char(2, 24, 'P');
	disp_char(2, 30, 'L');
	disp_char(2, 36, 'A');
	disp_char(2, 42, 'Y');
	disp_char(2, 48, 'E');
	disp_char(2, 54, 'R');

	if(player == 0){
		disp_char(3, 36, 'O');
		disp_char(3, 42, 'N');
		disp_char(3, 48, 'E');
	}
	else{
		disp_char(3, 36, 'T');
		disp_char(3, 42, 'W');
		disp_char(3, 48, 'O');
	}
	disp_char(4, 30, 'R');
	disp_char(4, 36, 'E');
	disp_char(4, 42, 'A');
	disp_char(4, 48, 'D');
	disp_char(4, 54, 'Y');

	display();
	if(ball_1 == 3 && ball_2 == 3){ 
		while (button == 1){
		pad_w = P1&3;
		pad_w2 = P1&96;
		pad_w2 = pad_w2 >>5;
		t_cnt_init = P1&24;
		t_cnt_init = t_cnt_init >>3;
		num_player = player_sw;
		
			}
		if(pad_w == 3){
			pad_w = 24;
		}
		else{
			pad_w = pad_w2*4 +8;
		}
		if(pad_w2 == 3){
			pad_w2 = 24;
		}
		else{
			pad_w2 = pad_w2*4 +8;
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
	RCAP4H = -1763>>8;			// set up for 784Hz
	RCAP4L = -1763;				// set up for 784 Hz
	duration = 196;				// quarter second
	T4CON = T4CON^0x04;			//enable timer 4
	su = 0;
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

void draw_paddle(char x)
{
	int i;
	int padd_w;
	if(player == 0){
		padd_w = (int)pad_w;
		
	}
	else{
		padd_w = (int)pad_w2;
	}
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
		return -3;
	}
	if( y == 60 && hit > 0){

		char col = xpos - pot_val -2;
		int div = 0;
		if(player == 0){
			div = pad_w/4;
			}
		else{
			div = pad_w2/4;
			}
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
				if(x_b >= 0 || y_b >= 0){
					blocks_1[x_b][y_b] = 0;
					yangle = -1*yangle;
				}
			}
		}
		else
		{
			if(blocks_2[x_b][y_b] == 0)
			{
				if(xangle < 0)
				{
					y_b = (ypos -8)/4;
					blocks_2[x_b-1][y_b] = 0;
				}
				else
				{
					y_b = (ypos -8)/4;
					blocks_2[x_b+1][y_b] = 0;
				}
					xangle = -1*xangle;
			}
			else
			{	
				if(x_b >= 0 || y_b >= 0){
					blocks_2[x_b][y_b] = 0;
					yangle = -1*yangle;
				}
			}	
		}
		if(hit > 0 && ypos <= 21 && su ==0)
		{
			t_cnt = t_cnt*0.6;
			su = 1;
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
	OSCXCN = 0x67; //crystal enabled
	TMOD = 0x20;  //wait 1ms: T1 mode 2
	TH1 = 167;	  // 1ms/(1/(2Mhz/12)) = 166.666
	TR1 = 1;	  //enable timer 1
	while( TF1 == 0) {}	//1 ms wait for flag
	while( !(OSCXCN & 0x80)) {} //stabilize crystal
	OSCICN = 8;   // switch to 22.1184Mhz
	
	IE = 0xA0;	 //Timer 2 interrupt enable
	EIE2 = 0x06; //Timer 4 and ADC
	
	T4CON = 0x00; //timer 4, auto reload
	RCAP4H = -1;	//timer 4
	RCAP4L = -144;	//timer 4
	DAC0CN = 0x9C;	//Speaker setup

	T2CON = 0x00;	//set up timer 2
	RCAP2H = -2211 >> 8;	//
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
		if(int_cnt==t_cnt) 
		{
			blank_screen();
			draw_paddle(pot_val);
			draw_bricks();
			mov_ball();
			int_cnt = 0;

			//Audio setup for brick breaking
			if(bonk > 0){
				RCAP4H = -2097 >> 8;		// set up for 659Hz
				RCAP4L = -2097;				// set up for 659Hz
	   			duration = 165;				//quarter second
				T4CON = T4CON^0x04;			//enable timer 4
			}
			//Audio for bouncing off paddle
			else if(bonk ==-1)
			{
				RCAP4H = -2642 >> 8;		// set up for 523Hz
				RCAP4L = -2642;				// set up for 523Hz
	   			duration = 131;				//quarter second
				T4CON = T4CON^0x04;			//enable timer 4
			}
			//Audio for bouncing off wall
			else if(bonk == -2)
			{
				RCAP4H = -2354>>8;			// set up for 587Hz
				RCAP4L = -2354;				// set up for 587Hz
	   			duration = 147;				//quarter second
				T4CON = T4CON^0x04;			//enable timer 4
			}
			

		
		}
	}


}