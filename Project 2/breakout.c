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

////////////////////////////////////////////////////////////////////////////////////
//						Wait Screen
//	This function displays a screen in between players turns.
//	Prints "Player *current player* Ready" 
// 	Resets the speed and starts the ball in the middle of the screen.
//	Waits for the player to press the start button.
////////////////////////////////////////////////////////////////////////////////////
void wait_screen(){
	
	TR2 = 0;	//stops the clock
	xpos = 40;	//middle of screen
	ypos = 40;	//one pixel below the bricks
	xangle = 1;	//resets xangle 
	yangle = 1;	//resets yangle
	blank_screen();
	
	//Display "Player (current player) Ready"
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

	display();	//Display boarders and scores
	
	//Checks if both players lives are at 3 if so it's the start of the game
	if(ball_1 == 3 && ball_2 == 3){ 
		while (button == 1){
		pad_w = P1&3;	//set player1 paddle size with switches (1 and 2)
		pad_w2 = P1&96;	//set player2 paddle size with switches (6 and 7)
		pad_w2 = pad_w2 >>5; //shift player2 so that it is between 0 to 3

		//sets the initial speed of the ball 
		t_cnt_init = P1&24; //switches 4 and 5
		t_cnt_init = t_cnt_init >>3; //shift so it's between 0 to 3
		
		num_player = player_sw; //One or two players based on switch 8;
		
		}

		//sets the paddle width for 24, 16, 12, and 8 bits
		if(pad_w == 3){
			pad_w = 24;
		}
		else{
			pad_w = pad_w2*4 +8; //(0 to 2)*4 +8 = 16-8 
		}
		if(pad_w2 == 3){
			pad_w2 = 24;
		}
		else{
			pad_w2 = pad_w2*4 +8; //(0 to 2)*4 +8 = 16-8
		}
		
		//initialize speed of the ball
		t_cnt_init = t_cnt_init*10 + 30; //(0 t0 3)*10 + 30 = 30 - 60
		t_cnt = t_cnt_init;				//set current count to the inital
		if(num_player == 0){
		ball_2 = 0; //no two player remove their lives
		}
	}
	else{
		while (button == 1){}; //wait till button is pressed
	}
	TR2 = 1;	//turn timer back on
}

////////////////////////////////////////////////////////////////////////////////////
//						Game Over Screen
//	This function displays a screen at the end of the game
//	Prints "GAME OVER" 
// 	Waits for the reset button to be pressed
////////////////////////////////////////////////////////////////////////////////////
void game_over()
{
	TR2 = 0; //turn timer off

	blank_screen(); //clear screen
	//print Game Over
	disp_char(2, 30, 'G');
	disp_char(2, 36, 'A');
	disp_char(2, 42, 'M');
	disp_char(2, 48, 'E');
	disp_char(3, 30, 'O');
	disp_char(3, 36, 'V');
	disp_char(3, 42, 'E');
	disp_char(3, 48, 'R');

	display(); //display boarders and scores
	while(1){} //Stays until board is reset
	
}

////////////////////////////////////////////////////////////////////////////////////
//						Turn Over Screen
//	This function displays a screen at the end of one players turn
//	Switches variables for the next player and checks the end game condition 
// 	Sends player to the wait screen or game over screen
////////////////////////////////////////////////////////////////////////////////////
void turn_end()
{
	//Audio for ball touching the bottom
	RCAP4H = -1763>>8;			// set up for 784Hz
	RCAP4L = -1763;				// set up for 784 Hz
	duration = 196;				// quarter second
	T4CON = T4CON^0x04;			//enable timer 4
	
	//speed reset
	su = 0;					//reset speed up for 3rd brick			
	t_cnt = t_cnt_init;		//reset speed to original speed
	
	//Check if it's a two player game
	if(num_player == 1){
		//if it was the first players turn and they lost
		if(player == 0){
			ball_1 = ball_1 -1;	//decrease lives by one
			player = 1;			//set player to second player
			score_1 = score;	//Save player One's current score
			score = score_2;	//Set the score to player two's
		}

		//if it was the second players turn and they lost
		else{
			//check game over condition no more lives left
			if(ball_2 == 1){
				ball_2 -= 1;
				game_over();}
			
			else{
				ball_2 = ball_2 -1;	//decrease lives by one
				player = 0;			//set player to first player
				score_2 = score;	//Save player Two's current score
				score = score_1;	//Set the score to player One's
			}
		}
	}
	//If it's a single player game
	else{
		if(ball_1 == 1){ //Check for a game over
			game_over();}
		else{
			ball_1 = ball_1 -1; //Decrease lives
			}
	}
	wait_screen(); //Send to wait screen
}

////////////////////////////////////////////////////////////////////////////////////
//						Draw the Bricks
//	This function takes the arrays of bricks and draws them accordingly on the
//	screen. A one represents a brick to be drawn and a zero represents a brick to 
// 	be deleted. Bricks are 11x5
////////////////////////////////////////////////////////////////////////////////////
void draw_bricks(){
	int i;
	int j;
	int k;
	int zero_cnt = 0;
	xdata unsigned char blocks[11][5] = {0};
	
	//Sets the bricks to be drawn dependent on the player
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
	
	//Loops through the bricks and draws them or deletes them from the screen
	for(i= 0; i < 11; i ++)
	{
		for(j = 0; j < 5; j ++)
		{
			if(blocks[i][j] == 1) //If there is a block at this location
			{
				if(j%2 == 1) //If y value is even draw on the first half of the page
				{
					for(k = 0; k < 6; k++) //draws the brick 6 wide
					{
						screen[((j-1)/2)*128 + i*7 +k +131] |= 0x70; //Three bits at the top of the page with one space on top
						//((j-1)/2)*128 = row to print, i*7 +131 = column, +k does it 6 times for the brick
					}
				}
				else //If y value is odd draw on the second half of the page
				{
					for(k = 0; k < 6; k++) //draws the brick 6 wide
					{
						screen[(j/2)*128 + i*7 + k +131] |= 0x07; //Three bits on the bottom of the page with one top space
						//((j/2)*128 = row to print, i*7 +131 = column, +k does it 6 times for the brick
					}
				}
			}
			else
			{
				zero_cnt ++; //keeps track of the cleared bricks allowing the screen to repopulate at 55
				if(j%2 == 1)
				{
					for(k = 0; k < 6; k++)
					{
						screen[(j-1)/2*128 + i*7 + k + 131] &= 0x07; //ands the top with zero to clear desired brick
						//see above for math of drawing blocks
						
					}
				}
				else
				{
					for(k = 0; k < 6; k++)
					{
						screen[j/2*128 + i*7 + k + 131] &= 0x70;	//ands the bottom with zero to clear desired brick
						//see above for math of drawing blocks
					}
				}
			}
		}
	}
	if(zero_cnt == 55 && ypos > 40){	//Check if all the blocks have been cleared
		if(player == 0) //reset bricks of player 1
		{
			for(i= 0; i < 11; i ++)
			{
				for(j = 0; j < 5; j ++)
				{
				blocks_1[i][j] = 1; //loops and sets each value to one
				}
			}		
		}
		else //reset bricks of player 2
		{
			for(i= 0; i < 11; i ++)
			{
				for(j = 0; j < 5; j ++)
				{
				blocks_2[i][j] = 1;	//loops and sets each value to one
				}
			}	
		}
	} 
}

////////////////////////////////////////////////////////////////////////////////////
//						Draw the Paddle
//	This function takes the current pot value passed in and draws the paddle
//	according to the paddle width selected. 
// 	
////////////////////////////////////////////////////////////////////////////////////
void draw_paddle(char x)
{
	int i;
	int padd_w;

	//Sets padd_w depending on who's turn it is
	if(player == 0){
		padd_w = (int)pad_w;
		
	}
	else{
		padd_w = (int)pad_w2;
	}

	//Draw the paddle
	for(i = 0; i < padd_w; i ++)
		{
			screen[898+x+i] |= 0xc0; //898 is the bottom left corner of the play area 0xc0 is a paddle 2 bits thick
			//x offsets the paddle to the knob position, i then draws the paddle padd_w wide.
		}
}

////////////////////////////////////////////////////////////////////////////////////
//						Draw the Ball
//	This function does a lot:
//		-Draws the ball
//		-Bounces the ball off walls
//		-Bounces the ball off the paddle
//			-changes angle based on bounce off paddle
//		-Detects hits on bricks and breaks the corressponding brick
//		-Returns a hit code that determines which audio is played
// 	
////////////////////////////////////////////////////////////////////////////////////
unsigned char draw_ball(int x, int y)
{	unsigned char row, col, shift, j, hit;
	int i;
	
	col = x-2; //correct for center of ball
	row = y - 2; //correct for center of ball
	shift = row%8;
	row = row/8;
	hit = 0; //initialize hit to zero
	
	//draw the ball and return a value greater than zero if a hit occurs
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

	//Checks if the ball is gonna hit the corner of the screen and inverts both angles
	if((x<5 || x > 78) && y < 3)
	{
		yangle = -1*yangle;
		xangle = -1*xangle;
		return -2;//audio code
	}
	//checks the left and right edges of the screen and inverts xangle if hit
	else if(x<5 || x > 78)
	{
		xangle = -1*xangle;
		return -2;//audio code
	}
	//checks the top of the play area and inverts y if hit
	else if (y < 3)
	{
		yangle = -1*yangle;
		return -2;//audio code
	}
	//checks the bottom of the screen and ends the game if hit
	else if(y > 61)
	{
		turn_end();
		return -3;
	}

	//Paddle hit detection and subsequent angle configs for different parts of the paddle.
	//This code devides the paddle into 4 sections and returns the angle as needed
	if( y == 60 && hit > 0){	//The only place the ball can hit the paddle is at y = 60 and if the ball detected a hit that means it hit the paddle

		char col = xpos - pot_val -2; //takes xposition of the ball subtracts the pot val and -2 to tell where the ball is in relation to the paddle
		int div = 0;
		//Calculates the paddle division based on player and the size of their paddle.
		if(player == 0){
			div = pad_w/4;	//paddle separated into four sections
			}
		else{
			div = pad_w2/4;	//paddle separated into four sections
			}
		if( col < div || col > 3*div) //did the ball hit the outside quarters of the paddle?
		{
			xangle = 2; //If so return a steeper angle
			yangle = -1;
		}
		else
		{
			xangle = 1; //If it hit the center return a shallower angle
			yangle = -2;
		}
		if(col < div *2) //did the ball hit the left side of the paddle?
		{
			xangle = -1*xangle; //If so send the ball to the left
		}
		return -1;//audio code
	}
	
	//All brick breaking logic, if it didn't hit the paddle or the sides, you hit a brick
	else if( hit > 0)
	{
		
		int x_b, y_b;
		score += 1; //hit a brick increase the score!

		//Normalize the y_position to return a value inside of brick matrix = (ypos -8)/4
		//Normalize the x_position to return a value inside of brick matrix = (xpos -3)/7
		if(yangle < 0)
		{
			y_b = (ypos -10)/4; //ball is moving up so normalize to the top edge of the ball with extra -2
		}
		else{
			y_b = (ypos -6)/4;	//ball is moving down so normalize to the bottom edge of the ball with extra +2
		}
		if(xangle < 0)
		{
			x_b = (xpos -5)/7; //ball is moving left so normalize to the left edge of the ball with extra -2
		}
		else
		{
			x_b = (xpos -1)/7;	//ball is moving right so normalize to the right edge of the ball with extra +2
		}

		if(y_b > 4 || x_b > 10){ //If the x_b and y_b positions aren't in the bounds of the matrix ignore it
		}
		
		//Update players bricks according to the brick that was hit	
		else if(player == 0)
		{
			if(blocks_1[x_b][y_b] == 0) //Check if the current index of the ball has a brick in it, if not it's probably on one of the sides
				{
					if(xangle < 0) //ball is moving left so delete the ball to the left
						{
							y_b = (ypos -8)/4;	//Normalize position to the center ball 
							blocks_1[x_b-1][y_b] = 0; //delete the ball to the left
						}
					else
						{
							y_b = (ypos -8)/4;	//Normalize position to the center ball
							blocks_1[x_b+1][y_b] = 0;	//delete the ball to the left
						}
					xangle = -1*xangle;	//bounce the ball to the side
				}
			else
			{
					blocks_1[x_b][y_b] = 0;//brick hit the top, delete that brick
					yangle = -1*yangle; //bounce the ball up or down
			}
		}
		//Same logic described above is used for player 2's bricks
		else if(player == 1)
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

		//checks if the ball is hitting the third row of bricks for the first time
		if(hit > 0 && ypos <= 21 && su ==0) //21 is the first row in which the ball could hit the 3rd row of bricks
		{
			t_cnt = t_cnt*0.6; //speed up the ball movement by a factor of .4
			su = 1;				//set the flag so it doesn't continuously speed up
		}
	}
	return hit; //return hit value mainly used for audio que
}

////////////////////////////////////////////////////////////////////////////////////
//						Move the Ball
//	This function updates the balls postition based on the current x and y angle
//	and then calls draw ball and stores the audio que in a variable called bonk
// 	
////////////////////////////////////////////////////////////////////////////////////
void mov_ball() {
	xpos += xangle; //update xpos by xangle
	ypos += yangle;	//update ypos by yangle
	bonk = draw_ball(xpos, ypos);	//saves audio code in bonk and draws the ball
	display();	//update the display
}

////////////////////////////////////////////////////////////////////////////////////
//						Timer2 Interrupt
//	This interrupt updates the int_cnt which will determine how often the ball 
//	is updated and drawn which in turn effects the speed of the ball.
// 	
////////////////////////////////////////////////////////////////////////////////////
void timer2(void) interrupt 5
{
	
	TF2 = 0;	//reset the timer flag 
	int_cnt ++;	//increase the timer count 
		
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
	RCAP2H = -2211 >> 8;	
	RCAP2L = -2211;

	ADC0CN = 0x8c;	//configure the ADC for the potentiometer
	REF0CN = 0x07;
	ADC0CF = 0x40;	//Configure the ADC for the audio
	AMX0SL = 0x0;
	CKCON = 0x40;	//Turn off divide by 12 for timer 4

	TR2 = 1;			//Turn on Timer 2
	T4CON = T4CON^0x04; //Turn on Timer 4
	
	init_lcd();	//initalize the LCD
	xpos = 40;//middle of screen
	ypos = 40;//one pixel below the bricks


	display(); //update display
	wait_screen(); //send game to wait for start
	
	
	
	
	while(1)
	{
		if(int_cnt==t_cnt) //check the clock's count with the desired update count
		{
			//Main gameplay loop
			blank_screen();			//blank screen
			draw_paddle(pot_val);	//update the paddle
			draw_bricks();			//update bricks
			mov_ball();				//move the ball
			int_cnt = 0;			//reset the count

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