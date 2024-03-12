$include (c8051f020.inc)
org 00h
jmp begin

org 23h
serial_isr: jbc ri, read0
						jbc ti, tcnt0
						reti

org 2Bh
T2_isr:	clr TF2
				jmp timer2

read0: ljmp read
tcnt0: ljmp tcnt1

				;increment our count here

        			dseg at 30h
old_buttons: 	ds 1
count:				ds 1
time:					ds 1
tcnt:					ds 1
start 				bit 1

cseg
begin:
mov wdtcn, #0DEh		;disable watchdog
mov wdtcn, #0ADh
;Initalize Variables
mov count, #00h
mov time, #00h
mov tcnt, #00h
clr start
mov xbr2, #40h
mov xbr0, #04h

mov oscxcn, #67h
mov		TMOD, #20h		;Set up timers
			mov		th1, #256-167	; 2MHz clock, 167 counts = 1ms
			setb	tr1

wait_0: 	jnb	tf1, wait_0
			 	CLR TR1						;1ms elapsed, stop timer
				CLR TF1

wait_1: mov a, oscxcn			;wait for clock to warm up
				jnb	acc.7, wait_1
				mov	oscicn, #8		;Now using the 22.1184 MHz
				mov	scon0, #50h 	;8-bit, 1 stop bit, REN
				mov th1, #-6			;Loads value for 9600 buad serial com
				setb	tr1					;Start the serial com timer

mov RCAP2H, #High(-18432)
mov RCAP2L, #Low(-18432)
mov TH2, #High(-18432)
mov TL2, #Low(-18432)
clr t2con.0
setb TR2
mov IE, #0B0h



main:	jmp main


timer2: call display
				jnb start, continue
				inc count
				mov A, count
				cjne a, #10, return
				mov count, #00h
				mov A, time
				add A, #1
				da  A
				mov time, A

continue:call check_buttons
				jb acc.4, reset
				jnb acc.6, return 
				jb start, stop
				cpl start
				;mov time, #00h
				;Increment count
				inc count
				mov A, count
				;cjne a, #10, return
				;mov A, time
				;add A, #1
				;da  A
				;mov time, A

return:	reti

Stop: 	clr start
				reti

reset:	mov time, #00h
				clr start
				reti

read: 	mov A, sbuf0
				cjne a, #72h, notr
				jmp run
notr:				cjne a, #73h, nots
				jmp Stop
nots:				cjne a, #63h, notc
				jmp clear
notc:			cjne a, #74h, return
				jmp report
				reti



run: 	setb start
			reti

clear: 	mov time, #00h
				reti

report: mov r0, time
				mov A, r0
				anl A,#0F0h
				swap A
				add A, #30h
				mov sbuf0, A
				mov tcnt, #01h
				reti

tcnt1: 	mov A, tcnt
				cjne a,#01h, tcnt2
				mov sbuf0, #2Eh;period ascii
				inc tcnt
				reti

tcnt2:	mov A, tcnt
				cjne a, #02h, tcnt3
				mov A, R0 
				anl a, #0Fh
				add A, #30h
				mov sbuf0, A
				inc tcnt
				reti

tcnt3: 	cjne a, #03h, tcnt4
				mov sbuf0, #0Dh;next line
				inc tcnt
				reti

tcnt4: 	cjne a, #04h, tcnt5
				mov sbuf0, #0Ah;next line
				inc tcnt
				reti

tcnt5: reti

Check_buttons: 	mov A, P2
        				cpl A                           ; CPL inputs since they are active low
        				XCH A, old_buttons              ; takes the input of the new buttons and stores it, passes the old buttons into acc
        				XRL A, old_buttons              ; If the buttons are the same change them to 0's
        				ANL A, old_buttons              ; If buttons are different and pressed they stay
        				RET

Display: 	ORL P3, #0FFh			;Turn off All LEDS	
         	ORL P2, #03h
         	mov A, time
					cpl A
					mov P3, A
					RET

					end