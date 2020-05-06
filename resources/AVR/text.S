#define __SP_H__ 0x3e
#define __SP_L__ 0x3d
.globl SPIprint
SPIprint:
	cli ; clear interrupts. Need to set at end if interrupts are needed later
	push r6
	push r7
	push r8
	push r9
	push r10
	push r12
	push r13
	push r14
	push r15
	push r16 ; pushing minX and minY to not clobber them
	push r17
	push r18
	push r19
	push r28
	push r29

	movw r12,r24 ; string pointer
	movw r6,r20 ; colors pointer

	; create 8 byte frame on stack to save coords[] to send it to setWindow() using Y index
	in r28,__SP_L__
	in r29,__SP_H__
	sbiw r28,8
	out __SP_H__,r29
	out __SP_L__,r28

	movw r24,r16 ; copy minY into r24 (to use adiw later, adiw only works on r >= 24)
	clr r17							 ; 1   clear r17 for use as a zero register

;**  word coords[] = {
;**    minX,
;**    minX + (len * 8) - 1,
;**    minY,
;**    minY + 7,
;**  };

	std Y+2,r19 ; store minX (r18:19) in the first word of coords located in Y pointer
	std Y+1,r18
	subi r18,1  ; subtract 1 from r18 (high byte of minX) (doing the math for maxX here)
	sbc r19,r17 ; and carry if needed
	ldi r16,lo8(8) ; and load the lower 8 bits of 8 into r24
	mul r22,r16		 ; and multiply r22 (length) by r24 (8)
	add r18,r0     ; and add the result to r18:19
	adc r19,r1
	std Y+4,r19    ; and store maxX (r18:19)
	std Y+3,r18
	std Y+6,r25    ; and store minY (r16:17)
	std Y+5,r24

	;subi r16,-7		 ; and subtract 249 ? why not add 7...?
	;sbci r17,-1		 ; and subtract 255 ? ...???
	adiw r24,7			 ; add 7 to r24:25
	std Y+8,r25		; and store maxY
 	std Y+7,r24

;**   setWindow(coords);

	movw r24,r28 ; then move r28 into r24 (the pointer to coords[])
	adiw r24,1   ; add 1 (since r28 is really the stack pointer)
	call SPIsetWindow ; call it.

	; reset stack frame since it's no longer in use so we can use r28:29 later

	adiw r28,8
	out __SP_H__,r29
	out __SP_L__,r28

;**   cmd(0x2C);

  ldi r24,0x2C ; load 0x2C into r24 (first param of cmd) and call it
  call cmd ; clobbers 24, 25, 30, 31

;** store the pointer to the font
	ldi r18,lo8(font)
	ldi r19,hi8(font)

;  *******  SEND TEXT DATA  *******  ;

;  REGISTERS
;  r28/29: String pointer (Y)
;  r12/13: Saved String pointer since Y is modified as it traverses a row
;  r10:    Row counter (increments after each row of pixels up to 8)
;  r26/27: Bit / Color pointer (X)
;  r20/21: Input bit/color pointer
;  r30/31: Font pointer (Z)
;  r18/19: Stored font pointer since X is modified directly
;  r16:    95 for multiplying
;  r17:    zero register since r1 needs a zero reg added to it
;  r25:    staging byte to be copied to r24 at the right time
;  r24:    the current byte being pushed (counter available by checking position of the 9th bit)
;  r0:     high byte of multiplication (__tmp_reg__)
;  r1:     low byte of multiplication (__zero_reg__)
;  r8/9:   the current color being sent
;  T:      T flag in SREG set when we're on the last byte
;  0x5,0:    PORTB (Data/Command pin must be disabled when sending data, but not until command finishes sending)

; Prepare the first byte, next bytes will be prepared while waiting in loop
; First time initialization
  set                  ; 1   set T flag (used to signal end of text and the first byte sent)
  ldi r16,95           ; 2   static 95 as multiplicand
  clr r10              ; 1   counter up from 0 to 7 loading bit 4 into T flag to signal completion of all 8 rows
  movw r28,r6          ; 1   move bit pointer to r28/29 (Y) from r20/21 where it is put when this subroutine is called
  							       ;       (3rd parameter, one even register per param from r24/25)
  movw r26,r12	       ; 1   move string pointer into r26/27 from r12/13 since it was moved there earlier when we needed to use r24/25
  ld r25,X+			       ; 2   load the first character and increment X to the next character
	nop
	sbi  0x5,0        	 ; 1   disable DC ; must do this after command finishes sending

	;movw r30,r18
	;adiw r30,2
	;lpm r24,Z
	;sts 198,r0

; **  DESCRIPTION

; main loop: loops from end after pushing 2 bits to prepWord to prepare the next color to send
;   from the next bit of r24 which holds the current byte being pushed.
; The C flag is used with ror to push a 1 into the 9th bit, so when the 1 is pushed out (r24 = 0
;   the zero flag is set and the program is sent to nextByte to retrieve the next byte
;   (calculated and stored while waiting in between byte sends) and update r24 and put the new
;   bit in the carry flag.
; Based on the carry flag (the bit just shifted out of r24), we grab the corresponding word
;   of the colors pointer
; We need to disable DC, but can't until the command byte finishes sending, so that's in the loo
;
; In between bits, I need to prep the next byte from the string since that takes a while
; After the first bit, I grab the next byte from the incremented string pointer in Y
;    and if it's null, I increment a counter which tells if we've finished, otherwise
;    we print the next line (text is printed row by row for all charcters, so each row we have
;    to reset Y in the string to reread the string.) If the counter reached 8 (bit 4 is now set)
;    we need to set the T flag in SREG to exit the loop after sending the last bit.
; After the second bit, I do the math to get the byte from the font which corresponds with
;    the offset given by counter and the character from the string...
;  I subtract a space (0x20) since the font starts at space, multiply the counter by 95 characte
;    to get the offset which mult stores in r0:1. I add the character to r0, reload the font poi
;    add the offset, and load the byte into r25, the staging byte that will be put into r24 when
;   Note: If I need more time, all the nextByte code could be put here... I could write to r24 d
;    after the last bit, watching until r24 = 1 to set r24 to r25...
	rjmp firstByte
prepByte:
  brts endAll   ; 1/2 If T is set, we're done.
firstByte: ; skip check for T flag so T flag can trigger skipping the writing of the last bit
  subi r25,0x20 ; 1   subtract a space since the font starts at space
  mul  r10,r16  ; 1   multiply the counter by 95 characters

  add  r0,r25   ; 1   add character to row offset
  adc  r1,r17   ; 1   add carry bit if it exists to r1
  movw r30,r18  ; 1   reload the font pointer into r30

	add  r30,r0   ; 1   add offset to font pointer (reloaded in bit 2)

	brts skippedWriting
	sts  198,r8   ;     write the second byte of the last color of the last bit of r24
skippedWriting: ; 7

  adc  r31,r1   ; 1
	lpm   r24,Z    ; 3   load the current byte from there into r24 (note: font is stored in program memory)
	sec						; 1   set the carry flag
	clt						; 1   clear T flag
  ror  r24		  ; 1   rotate r24, shifting in the carry flag

;** Prepare the next color to be sent in r8 and r9
prepWord:  ; 7 to endIf
  brcs ifCarry ; branch if carry set
  ld   r9,Y     ; load bit into r8 and r9
  ldd  r8,Y+1
  rjmp endIf   ; jump to end
ifCarry:
  nop
  ldd  r9,Y+2   ; load bit into r8 and r9
  ldd  r8,Y+3
endIf:

;** Send the first byte (r9) of the color
  sts  198,r9   ; 14 clocks in between byte sends...
	nop           ; 1 extra clock if needed somewhere...

;*** Prepare the next character... (13 clocks to endbits)
  ; On the first bit transfer (9th bit is bit 7)...
  sbrs r24,7    ; 2/1 if the 7th bit of r24 is not set (this is not the first bit pushed
  rjmp notbit7  ; 2   then we don't need to deal with the next byte

 	; Then we get the next character and save it in r25 until the next bit
  ld   r25,X+   ; 2   load the next character from the string
  cpi  r25,0    ; 1   compare with 0 (null-terminator)
  brne endif3   ; 1/2 if it's not 0 (Z flag not set), skip to endif3

	; Otherwise, if it is zero and we need to go onto the next row
  inc  r10       ; 1   increment r10
  bst  r10,3     ; 1   Set T if r10 is 8 to abort later
  movw r26,r12  ; 1   go back to the start of the string (reload string pointer)
  ld   r25,X+    ; 2   load the next character into r25 and post increment
  rjmp endbits  ; 2 - total of 14 bits for bit 7 ; And we're done until the next loop

	; If we don't have to go to the next line, wait here so the clock times are the same
endif3:         ; 6 clocks from here to endbits:
  nop
  nop
  nop
  nop
  rjmp endbits  ; 4 + 2 - total of 14 bits for bit 7 along this branch

	; If this is not the first nor second nor last, do nothing.
notbit7:        ; 10 clocks to endbits...
	cpi r24,1     ; 1
	breq prepByte ; 1/2
	nop
	nop
	nop
	nop
	nop
	nop           ; 1   have to rjmp to get to endbits. * check timing
  rjmp endbits  ; 2

	; Where everything goes unless it's the last bit of the byte
	; Here we shift r24 so the next bit is in C, wait, and jump to prepare the next word
endbits:
  sts  198,r8       ; write to SPI first byte of color
  lsr  r24
  nop
  nop
  nop
  nop
  rjmp prepWord     ; 2 c

endAll: ; We're done sending everything, so pop everything we pushed and return
	clr r1
	pop r29
	pop r28
	pop r19
	pop r18
	sts 198,r8 ; send the last bit
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r10
	pop r9
	pop r8
	pop r7
	pop r6
	ret
