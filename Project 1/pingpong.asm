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
l_config:			ds 1
r_config:			ds 1
s_config:			ds 1 
count:				ds 1										
bseg
start_side: dbit 1		;1 Start on Right side, 0 Start on left side
direction: 	dbit 1		;1 Moving right, 0 moving left

				cseg

        mov wdtcn,#0DEh                 ; disable watchdog
        mov wdtcn,#0ADh
        mov xbr2,#40h                   ; enable I/O

        setb 	 P2.7                     ;assign buttons
        setb 	 P2.6
			
				cpl start_side									;change serving side

				mov c, start_side
				cpl c
				mov direction, c

				jnb start_side, left_side				;1 right side serve
																				;0 left side serve


right_side:	mov position, #00h					;sets led to far left
						call display
						
						mov R1, P1									;sets config bytes to lsb of each byte
						mov A, R1
						anl A, #00000011b
						mov l_config, A
						mov A, #10
						subb A, l_config
						mov l_config, A
						mov A, R1
						anl A, #00011000b						;1/2 left window, 4/5 speed, 7/8 right window
						rl 	A
						swap A
						mov s_config, A
						mov A, R1
						anl A, #11000000b
						swap A
						rr A
						rr A
						;add A, #1
						mov r_config, A	
						
						mov A, s_config
						jnz speedy
						mov R0, #25									;speed is (15 * 10ms = 150ms)
						jmp proceed

speedy:		mov R0, #8 						
											
proceed:	mov count, R0
						lcall check_buttons 								;serve ball?
						jb ACC.4, main
						jmp right_side

left_side:	mov position, #09h					;sets led to far left
						call display
						
						mov R1, P1							;sets config bytes to lsb of each byte
						mov A, R1
						anl A, #00000011b
						mov l_config, A
						mov A, #10
						subb A, l_config
						mov l_config, A
						mov A, R1
						anl A, #00011000b				;1/2 left window, 4/5 speed, 7/8 right window
						rl 	A
						swap A
						mov s_config, A
						mov A, R1
						anl A, #11000000b
						swap A
						rr A
						rr A
						;add A, #1
						mov r_config, A	
						
						mov A, s_config
						jnz speedy_l
						mov R0, #25									;speed is (15 * 10ms = 150ms)
						jmp proceed_l

speedy_l:		mov R0, #8 						
											
proceed_l:	mov count, R0
						lcall check_buttons 								;serve ball?
						jb ACC.6, main
						jmp left_side

main:				call DELAY											;we decided to jump to next sub from current sub not main
						jmp check_dir
				
						jmp main
check_dir: 	
						jnb direction, check_left_window
						jmp check_right_window

check_left_window: mov A, position
						CJNE A, l_config, NOTEQUAL_L
NOTEQUAL_L: 	
						JC GREATER_L
						CALL		check_buttons
						jnb    	Acc.6, mov_led
						CPL			direction
						LJMP 		mov_led

GREATER_L: 	CALL check_buttons
						LJMP mov_led


check_right_window: mov A, position
						CJNE A, r_config, NOTEQUAL
NOTEQUAL: 	
						JNC GREATER
						CALL		check_buttons
						jnb 		Acc.4, mov_led
						CPL			direction
						LJMP 		mov_led
						;CJNE    A, #90h, mov_led 			;This is where we left off, check the button since it's less than the window, and change directions and move

GREATER:		CALL		check_buttons
						;CJNE    A, #90h, mov_led
						;CPL			direction
						LJMP 		mov_led

mov_led:	jnb direction, l_led
					djnz	count, mov_on
					mov count, R0
					dec position
					LJMP End_Game

mov_on:		call display
					LJMP main

l_led:		djnz count, mov_on
					mov count, R0
					inc position
					LJMP End_Game

; Game over Criteria					
End_Game: mov     A, position
        	CJNE 		A, #0FFh, NINE	 	;If position = 0 then the game is over.
					ORL    P3, #0FFh
         	ORL    P2, #03h
					clr P3.0
        	SJMP    OVER

NINE:   mov     A, position
        CJNE    A, #0Ah, mov_on		;If position = 10 then the game is over.
        SJMP    OVER	

;CJNE  R0,#00H,NOTEQUAL
; equal code goes here, then branch out
;NOTEQUAL:
;JC GREATER
; less than code goes here, then branch out
;GREATER:
; greater code goes here

;sped:
;				orl	p2, #0Ch
;				call delay
;				call check_buttons
;			;	orl	buttons, A
;				djnz	R0, sped 	

; Time delay of 10ms
DELAY:				mov     R2, #67		; Load R2 with 67
outer_loop:   mov     R3, #100 	; Load R3 with 100, 		100 * 67 * 1.5 us = 10ish ms
nested_loop:  DJNZ    R3, nested_loop	; Stay here till R3 = 0
        			DJNZ    R2, outer_loop	; Stay here till R2 = 0
        			RET
;check the buttons
Check_buttons: mov A, P2
        cpl A                           ; CPL inputs since they are active low
        XCH A, old_buttons              ; takes the input of the new buttons and stores it, passes the old buttons into acc
        XRL A, old_buttons              ; If the buttons are the same change them to 0's
        ANL A, old_buttons              ; If buttons are different and pressed they stay
        RET

Display: ORL    P3, #0FFh
         ORL    P2, #03h
         mov    A, position
         CALL   not_one
				 RET
         ;mov    A, position			; Is this code needed, I dont think A will get changed while DISP_LED is ran.XXXXXXXXXX
         ;inc    A
         ;CALL  	not_one
         ;ret
				 		
not_zero: CJNE  A, #00h, not_one
        	CLR     P3.0
        	RET

not_one: CJNE   A, #01h, not_two
         CLR     P3.1
         RET

not_two: CJNE   A, #02h, not_three;
         CLR     P3.2
         RET

not_three: CJNE A, #03h, not_four
        	 CLR     P3.3
         	 RET

not_four: CJNE  A, #04h, not_five
        	CLR     P3.4
        	RET

not_five: CJNE  A, #05h, not_six
        	CLR     P3.5
        	RET

not_six: CJNE   A, #06h, not_seven;
         CLR     P3.6
         RET

not_seven: CJNE A, #07h, not_eight;
        	 CLR     P3.7
        	 RET

not_eight: CJNE A, #08h, not_nine; 
        	 CLR     P2.0
        	 RET	
					
not_nine: CJNE A, #09h, not_zero
        	CLR     P2.1
        	RET				

OVER:			
					end