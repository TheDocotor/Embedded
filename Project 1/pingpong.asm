;==========================================================
; Program Name: pingpong.asm
;
;    Authors: Ryan Enslow & Garrett Nadauld
;
; Description:
; This program will allow 2 players to play pingpong using on the boards provided in our
; ECE3710 class. The game will begin by a player serving the ball by pressing a button. 
; The switches will be used to configure the game. Switches 1&2 will decide the window for
; player 1. Switches 7&8 will decide the windwo for player 2. Switches 4&5 will be used to 
; select the speed of the ball. The player serving the ball switches each game. To restart
; the game you need to press the reset button.
;
; Company:
;    Weber State University 
;
; Date            Version        Description
; ----            -------        -----------
; 2/6/2024    V1.0            Initial version
;==========================================================
$include(c8051f020.inc)
        			dseg at 30h
Position: 		ds 1
old_buttons: 	ds 1
l_config:	ds 1
r_config:	ds 1
s_config:	ds 1 
count:		ds 1										
bseg
start_side: 	dbit 1		;1 Start on Right side, 0 Start on left side
direction: 	dbit 1		;1 Moving right, 0 moving left

				cseg

        	mov wdtcn,#0DEh                 ; disable watchdog
        	mov wdtcn,#0ADh
        	mov xbr2,#40h                   ; enable I/O

        	setb 	 P2.7                     ;assign buttons	;Z We no longer need these two lines right????
        	setb 	 P2.6
			
		cpl start_side			;Swap serving side upon reset, 0 left serve 1 right serve
		mov c, start_side		
		cpl c				;Sets direction based upon serving side, 0 moving left, 1 moving right
		mov direction, c
		jnb start_side, left_side	;Jumps to left serve if start side 0, else continues onto right side


right_side:	mov position, #00h		;sets led to far right
		call display				
		mov R1, P1			;Pulls switches and stores them in R1 1/2 left window configuration, 4/5 speed configuration, 7/8 right window configuration, 3/6 do nothing
		mov A, R1
		anl A, #00000011b		;Isolates bits for configuring the left window
		mov l_config, A
		mov A, #10			;10 will be the base window, meaning that l_config must be greater than 0 in order to have a valid window
		subb A, l_config		;10 - l_config sets up the new l_config ranging from 9-7
		mov l_config, A
		mov A, R1
		anl A, #00011000b		;Isolates bits for configuring the speed
		rl A
		swap A				;Moves speed bits to least significant bits
		mov s_config, A
		mov A, R1
		anl A, #11000000b		;Isolates bits for configuring the right window
		swap A
		rr A
		rr A				;Moves right window bits to least significant bits
		mov r_config, A	
		mov A, s_config
		jnz speedy			;If speed configuration is set jump to section to speed the ball up
		mov R0, #25			;Speed is (25 * 10ms = 250ms)
		jmp proceed			;Jump over speedy so as to not configure R0 to 8

speedy:		mov R0, #8 			;Speed is (8 * 10ms = 80ms)				
											
proceed:	mov count, R0			;Count will be used as the number of times delay is called 
		lcall check_buttons 		;See if any buttons have been pressed	
		jb ACC.4, main			;If right button has been pressed move on
		jmp right_side			;If no button press loop back

left_side:	mov position, #09h		;sets led to far left
		call display		
		mov R1, P1			;Pulls switches and stores them in R1 1/2 left window configuration, 4/5 speed configuration, 7/8 right window configuration, 3/6 do nothing
		mov A, R1
		anl A, #00000011b		;Isolates bits for configuring the left window
		mov l_config, A			;10 will be the base window, meaning that l_config must be greater than 0 in order to have a valid window
		mov A, #10			;10 - l_config sets up the new l_config ranging from 9-7
		subb A, l_config
		mov l_config, A
		mov A, R1
		anl A, #00011000b		;Isolates bits for configuring the speed
		rl 	A
		swap A				;Moves speed bits to least significant bits
		mov s_config, A
		mov A, R1
		anl A, #11000000b		;Isolates bits for configuring the right window
		swap A
		rr A
		rr A				;Moves right window bits to least significant bits
		mov r_config, A		
		mov A, s_config
		jnz speedy_l			;If speed configuration is set jump to section to speed the ball up
		mov R0, #25			;speed is (25 * 10ms = 250ms)
		jmp proceed_l			;Jump over speedy so as to not configure R0 to 8

speedy_l:	mov R0, #8 			;Speed is (8 * 10ms = 80ms)					
											
proceed_l:	mov count, R0			;Count will be used as the number of times delay is called 
		lcall check_buttons 		;See if any buttons have been pressed	
		jb ACC.6, main			;If left button has been pressed move on
		jmp left_side			;If no button press loop back

main:		call DELAY			;we decided to jump to next sub from current sub not main ???????
		jmp check_dir			;Check which way ball is moving
		jmp main		

check_dir: 	jnb direction, check_left_window ;If moving left check if ball is in left window
		jmp check_right_window		;Else check if we are in the right window

check_left_window: mov A, position
		CJNE A, l_config, NOTEQUAL_L	;See if position is greater than or less than left window 

NOTEQUAL_L: 	JC GREATER_L			;If carry was set we are not in the window
		CALL check_buttons
		jnb Acc.6, mov_led		;If left button was not pressed continue moving led
		CPL direction			;If left button was pressed change direction of ball
		LJMP mov_led

GREATER_L: 	CALL check_buttons		;Do we need to check buttons here??????
		LJMP mov_led

check_right_window: mov A, position
		CJNE A, r_config, NOTEQUAL	;See if position is greater than or less than right window 

NOTEQUAL: 	JNC GREATER			;If carry was not set we are not in right window 
		CALL check_buttons		:If we are in window see if any buttons were pressed
		jnb Acc.4, mov_led		;If right button was not pressed continue moving ball
		CPL direction			;If right button was pressed reverse direction
		LJMP mov_led

GREATER:	CALL check_buttons		;Do we need to check buttons here??????
		LJMP mov_led

mov_led:	jnb direction, l_led		;If direction is 0 move led to the left
		djnz count, mov_on		;Decrement count until 0 to run full 80 or 250 ms depending on speed configuration
		mov count, R0			;Reset count once it has reached 0
		dec position			;Move led to the right 
		LJMP End_Game			;After led has moved see if game is over

mov_on:		call display		
		LJMP main

l_led:		djnz count, mov_on		;Decrement count until 0 to run full 80 or 250 ms depending on speed configuration
		mov count, R0			;Reset count once it has reached 0
		inc position			;Move led to the left
		LJMP End_Game			;After led has moved see if game is over

; Game over Criteria					
End_Game: 	mov A, position
        	CJNE A, #0FFh, NINE	 	;If position has run over 0 then the game is over else see if it is the other end condition?????? Is this the right way to say this?
		ORL    P3, #0FFh		
         	ORL    P2, #03h
		clr P3.0
        	SJMP    OVER			;End game

NINE:   	mov     A, position
        	CJNE    A, #0Ah, mov_on		;If position = 10 then the game is over
        	SJMP    OVER		

; Time delay of 10ms
DELAY:		mov R2, #67			;Load R2 with 67
outer_loop:   	mov R3, #100 			;Load R3 with 100, 		100 * 67 * 1.5 us = 10ish ms
nested_loop:  	DJNZ R3, nested_loop		;Stay here till R3 = 0
        	DJNZ    R2, outer_loop		;Stay here till R2 = 0
        	RET

;check the buttons
Check_buttons: 	mov A, P2
        	cpl A                           ; CPL inputs since they are active low
        	XCH A, old_buttons              ; takes the input of the new buttons and stores it, passes the old buttons into acc
        	XRL A, old_buttons              ; If the buttons are the same change them to 0's
        	ANL A, old_buttons              ; If buttons are different and pressed they stay
        	RET

Display: 	ORL P3, #0FFh			
         	ORL P2, #03h
         	mov A, position
         	CALL not_one
		RET
				 		
not_zero: 	CJNE A, #00h, not_one		;check if position is 0
        	CLR P3.0
        	RET

not_one: 	CJNE A, #01h, not_two		;check if position is 1
         	CLR P3.1
         	RET

not_two: 	CJNE A, #02h, not_three;	;check if position is 2
         	CLR P3.2
         	RET

not_three: 	CJNE A, #03h, not_four		;check if position is 3
        	CLR P3.3
         	RET

not_four: 	CJNE A, #04h, not_five		;check if position is 4
        	CLR P3.4
        	RET

not_five: 	CJNE  A, #05h, not_six		;check if position is 5
        	CLR P3.5
        	RET

not_six: 	CJNE A, #06h, not_seven		;check if position is 6	
         	CLR P3.6
         	RET

not_seven: 	CJNE A, #07h, not_eight		;check if position is 7
        	CLR P3.7
         	RET

not_eight: 	CJNE A, #08h, not_nine		;check if position is 8
        	CLR P2.0
        	RET	
					
not_nine: 	CJNE A, #09h, not_zero		;check if position is 9
        	CLR P2.1
        	RET				

OVER:			
		end