$include (c8051f020.inc) 
                                        ; P3 & P2.0 & P2.1 are LED outputs
        			dseg at 30h
Position: 		ds 1
old_buttons: 	ds 1

        cseg

        mov wdtcn,#0DEh                 ; disable watchdog
        mov wdtcn,#0ADh
        mov xbr2,#40h                   ; enable I/O

        setb 	 P2.7                      ;assign buttons
        setb 	 P2.6
        mov    Position, #04h	        ;Set the start for LEDs
        CALL   DISPLAY



loop:   CALL   DELAY
        CALL   Check_buttons
        ANL    A, #11000000b						 ; reads the button press
        CJNE   A, #90h, LEFT             ; If both buttons were pushed what to do
        SJMP   loop

LEFT:   CJNE    A, #80h, RIGHT           ; If left button was pressed
        dec     position
        CALL   	Display
        LJMP    Game_over

RIGHT:	CJNE    A, #40h, loop            ; If right button was pressed
        inc     position
        CALL   	Display
        LJMP    Game_over

OVER:   SJMP    OVER			; Ends program



;check the buttons
Check_buttons: mov A, P2
        cpl A                           ; CPL inputs since they are active low
        XCH A, old_buttons              ; takes the input of the new buttons and stores it, passes the old buttons into acc
        XRL A, old_buttons              ; If the buttons are the same change them to 0's
        ANL A, old_buttons              ; If buttons are different and pressed they stay
        RET

; 
Display: ORL    P3, #0FFh
         ORL    P2, #03h
         mov    A, position
         CALL   DISP_LED
         mov    A, position			; Is this code needed, I dont think A will get changed while DISP_LED is ran.XXXXXXXXXX
         inc    A
         CALL  	DISP_LED
         ret

; Game over Criteria
Game_over: mov     A, position
        	 CJNE 		A, #00H, NINE	 	; If position = 0 then the game is over.
        	 SJMP    OVER

NINE:   mov     A, position
        CJNE    A, #08h, loop 		; If position = 8 then the game is over.
        SJMP    OVER

; Update correct LEDs for display
DISP_LED: 	
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
																
; Time delay of 20ms
DELAY:				mov     R2, #67		; Load R2 with 67
outer_loop:   mov     R3, #200 	; Load R3 with 200, 		200 * 67 * 1.5 us = 20.1ms
nested_loop:  DJNZ    R3, nested_loop	; Stay here till R3 = 0
        			DJNZ    R2, outer_loop	; Stay here till R2 = 0
        			RET

        END