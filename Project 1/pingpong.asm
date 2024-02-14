$include(c8051f020.inc)

        			dseg at 30h
Position: 		ds 1
old_buttons: 	ds 1
l_config:			ds 1
r_config:			ds 1
s_config:			ds 1 
;direction:		ds 1											;0 is travelling right
bseg
start_side: dbit 1		;keeps track of the serve
direction: 	dbit 1

				cseg

        mov wdtcn,#0DEh                 ; disable watchdog
        mov wdtcn,#0ADh
        mov xbr2,#40h                   ; enable I/O

        setb 	 P2.7                      ;assign buttons
        setb 	 P2.6
			
				cpl start_side							;change serving side

				mov c, start_side
				mov direction, c

				jnb start_side, right_side	; 1 right side serve
				jmp left_side								; 0 left side serve

right_side: mov position, #09h					;sets led to far right
						call not_zero
						
						mov R1, P1							;sets config bytes to lsb of each byte
						mov A, R1
						anl A, #00000011b
						mov l_config, A
						mov A, R1
						anl A, #00011000b				;1/2 left window, 4/5 speed, 7/8 right window
						rl 	A
						swap A
						mov s_config, A
						mov A, R1
						anl A, #11000000b
						rlc A
						rlc A
						mov r_config,A
						mov A, #09h
						subb A,r_config
						mov r_config, A

						mov c, P2.7 								;serve ball?
						jnb cy, main
						jmp right_side							;loop until served

left_side:	mov position, #00h					;sets led to far left
						call not_zero
						
						mov R1, P1							;sets config bytes to lsb of each byte
						mov A, R1
						anl A, #00000011b
						mov l_config, A
						mov A, R1
						anl A, #00011000b				;1/2 left window, 4/5 speed, 7/8 right window
						rl 	A
						swap A
						mov s_config, A
						mov A, R1
						anl A, #11000000b
						rlc A
						rlc A
						mov r_config, A		
											
						mov c, P2.6 								;serve ball?
						jnb cy, main
						jmp left_side

main:				call DELAY											;we decided to jump to next sub from current sub not main
						call check_dir
				
						jmp main
check_dir: 	
						jnb direction, check_left_window
						jmp check_right_window

check_left_window: mov A, position
						CJNE A, l_config, NOTEQUAL_L
NOTEQUAL_L: 	
						JC GREATER_L
						CALL		check_buttons
						CJNE    A, #80h, mov_led
						CPL			direction
						LJMP 		mov_led

GREATER_L: 	CALL check_buttons
						LJMP mov_led


check_right_window: mov A, position
						CJNE A, r_config, NOTEQUAL
NOTEQUAL: 	
						JC GREATER
						CALL		check_buttons
						LJMP 		mov_led

GREATER:		CALL		check_buttons
						CJNE    A, #90h, mov_led
						CPL			direction
						LJMP 		mov_led

mov_led:	jnb direction, l_led
					inc position
					call not_zero
					LJMP End_Game

l_led:		dec position
					call not_zero
					LJMP End_Game	

; Game over Criteria					
End_Game: mov     A, position
        	CJNE 		A, #00H, NINE	 	;If position = 0 then the game is over.
        	SJMP    OVER

NINE:   mov     A, position
        CJNE    A, #09h, main 		;If position = 9 then the game is over.
        SJMP    OVER	

;CJNE  R0,#00H,NOTEQUAL
; equal code goes here, then branch out
;NOTEQUAL:
;JC GREATER
; less than code goes here, then branch out
;GREATER:
; greater code goes here


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
					
not_nine: CJNE A, #09h, not_one
        	CLR     P2.1
        	RET				

OVER:			
					end