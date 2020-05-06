#include "SPI.h"
/*
movw
ld
movw
ror  (1)
brcs (1/2)
ld   (2)
ldd  (2)
rjmp (2)
nop  (1)
ldd  (2)
ldd  (2)
*/
/*void SPI_C::print(const char * string, byte len, const word colors[2], word minX, word minY) {
  word coords[] = {
    minX,
    minX + (len * 8) - 1,
    minY,
    minY + 7,
  };
  setWindow(coords);
  cmd(0x2C);
  asm volatile (
    // Prep references (get first byte)
    "\n\t  push r28"
    "\n\t  push r29"
    "\n\t  clt"           // clear T flag (used to signal end of text)
    "\n\t  ldi r16,95" // static 95 as multiplicand
    "\n\t  clr r10"   // counter down to 0 from 7, less clocks to check for 0 after decrement
    "\n\t  movw r30,%1"   // move bit pointer to r30/31 (Z)

    "\n\t  movw r28,%2"
    "\n\t  ld r25,Y+"
    "\n\t  subi r25,0x20" // 1   subtract a space since the font starts at space
    "\n\t  mul r10,r16"  // 1   multiply the counter by 95 characters

    "\n\t  add  r0,r25"   // 1   add character to row offset
    "\n\t  adc  r1,__zero_reg__" // 1   add carry bit if it exists to r1
    "\n\t  movw r26,%0"   // 1   reload the font pointer into r26
    "\n\t  add  r26,r0"   // 1   add index to font pointer
    "\n\t  adc  r27,r1"   // 1
    "\n\t  ld   r24,X"    // 2   load the current byte from there into r25
    "\n\t  sec"
    "\n\t  ror  r24"
    "\n\t  sbi  0x5,0"        // disable DC // 1 clock // must do this after command finishes sending, but it's long done by now

  // main loop: loops from end after pushing 2 bits to prepWord to prepare the next color to send
  //   from the next bit of r24 which holds the current byte being pushed.
  // The C flag is used with ror to push a 1 into the 9th bit, so when the 1 is pushed out (r24 = 0)
  //   the zero flag is set and the program is sent to nextByte to retrieve the next byte
  //   (calculated and stored while waiting in between byte sends) and update r24 and put the new
  //   bit in the carry flag.
  // Based on the carry flag (the bit just shifted out of r24), we grab the corresponding word
  //   of the colors pointer
  // We need to disable DC, but can't until the command byte finishes sending, so that's in the loop
  //
  // In between bits, I need to prep the next byte from the string since that takes a while
  // After the first bit, I grab the next byte from the incremented string pointer in Y
  //    and if it's null, I increment a counter which tells if we've finished, otherwise
  //    we print the next line (text is printed row by row for all charcters, so each row we have
  //    to reset Y in the string to reread the string.) If the counter reached 8 (bit 4 is now set),
  //    we need to set the T flag in SREG to exit the loop after sending the last bit.
  // After the second bit, I do the math to get the byte from the font which corresponds with
  //    the offset given by counter and the character from the string...
  //  I subtract a space (0x20) since the font starts at space, multiply the counter by 95 characters
  //    to get the offset which mult stores in r0:1. I add the character to r0, reload the font pointer into X,
  //    add the offset, and load the byte into r25, the staging byte that will be put into r24 when necessary.
  //   Note: If I need more time, all the nextByte code could be put here... I could write to r24 directly and only
  //    after the last bit, watching until r24 = 1 to set r24 to r25...

  "\n  prepWord:" // 7 clocks to endIf
    "\n\t  brcs ifCarry" // branch if carry set
    "\n\t  ld   r9,Z"     // load bit into r8 and r9
    "\n\t  ldd  r8,Z+1"
    "\n\t  rjmp endIf"   // jump to end
  "\n  ifCarry:"
    "\n\t  nop"
    "\n\t  ldd  r9,Z+2"   // load bit into r8 and r9
    "\n\t  ldd  r8,Z+3"

  "\n  endIf:"
    // Send Word
    "\n\t  sts  198,r9"       // write to SPI first byte of color // 14 clocks to next one
    "\n\t  nop"



   // process next character...
    "\n\t  sbrs r24,7"    // 2/1 if the 7th bit of r24 is not set (this is not the first bit pushed)
    "\n\t  rjmp notbit7"   // 2   then we don't need to deal with the next byte

   // get the next character
    "\n\t  ld   r25,Y+"   // 2   load the next character in the string
    "\n\t  cpi  r25,0"    // 1   compare with 0 (null-terminator)
    "\n\t  brne endif3"   // 1/2 if it's not null, skip to endif3

    "\n\t  inc r10"       // 1   Otherwise, decrement r10
    "\n\t  bld r10,4"     // 1   Set T if r10 counter finished for abort later
    "\n\t  movw r28,%2"   // 1   go back to the start of the string
    "\n\t  ld  r25,Y+"    // 2   load the next character into r25
    "\n\t  rjmp endbits"  // 2 - total of 14 bits for bit 7
  "\n\t  endif3:"         // 6 clocks from here to endbits:
    "\n\t  nop"           //   We've gotten the next character in r25. Didn't need to increment to the next row
    "\n\t  nop"
    "\n\t  nop"
    "\n\t  nop"
    "\n\t  rjmp endbits"  // 4 + 2 - total of 14 bits for bit 7

  "\n\t  notbit7:"       // 10 clocks from here to endbits: continue math, prep to load into r24 during the last bit
    "\n\t  brts tset"     // 1/2 If t is set, the math here is invalid, so we'll jump to somewhere else while this last byte sends
    "\n\t  cpi r24,0x40"  // 1
    "\n\t  brlt notbit6"  // 1/2 check if bit 6 is on (we know bit 7 is off at this point)
    "\n\t  subi r25,0x20" // 1   subtract a space since the font starts at space
    "\n\t  mul r10,r16"  // 1   multiply the counter by 95 characters

    "\n\t  add  r0,r25"   // 1   add character to row offset
    "\n\t  adc  r1,__zero_reg__" // 1   add carry bit if it exists to r1
    "\n\t  movw r26,%0"   // 1   reload the font pointer into r26

    "\n\t  rjmp endbits"  // 2 - total of 14 bits for bit 6
  "\n\t  notbit1:"        // 4 clocks from here to endbits... Reordered to give clocks to the last bit so it does not
    "\n\t  nop"           //    have to rjmp to get to endbits.
    "\n\t  rjmp endbits"
  "\n  tset:" // (8)
    "\n\t  cpi r24,1"   // 1
    "\n\t  breq endAll" // 1/2
    "\n\t  nop"
    "\n\t  nop"
    "\n\t  nop"
    "\n\t  nop"
    "\n\t  rjmp endbits"
  "\n\t  notbit6:"        // 7 clocks from here to endbits:
    "\n\t  cpi  r24,1"    // 1   Check if we're on the last bit (only the added carry bit is left)
    "\n\t  brne notbit1"  // 1/2 If not, we jump to notbit1 to twiddle our thumbs

    "\n\t  add  r26,r0"   // 1   add index to font pointer
    "\n\t  adc  r27,r1"   // 1
    "\n\t  ld   r24,X"    // 2   load the current byte from there into r25
    "\n\t  sts  198,r8"       // write to SPI first byte of color
    "\n\t  sec"
    "\n\t  ror  r24"
    "\n\t  rjmp waitererer"
  "\n  endbits:"
    "\n\t  sts  198,r8"       // write to SPI first byte of color
    "\n\t  lsr  r24"
    "\n\t  nop"
  "\n  waitererer:"
    "\n\t  nop"
    "\n\t  nop"
    "\n\t  nop"
    "\n\t  rjmp prepWord"     // 2 c

  "\n endAll:"
    "\n\t  pop r29"
    "\n\t  pop r28"
    :
    : "r" (font), "r" (colors), "r" (string)
    : "r30", "r31", "r24", "r25", "r26", "r27", "r10", "r16", "r8", "r9", "r0", "r1"

  );




    /*"\n\t  sts  198,r8"    // UDR0 = first byte
    "\n\t  sbi  0x5,0"      // PORTB |= DC

    "\n\t  lds  r18,numPixels" // initialize loop counter
    "\n\t  lds  r19,numPixels+1"
    "\n\t  lds  r17,numPixels+2" // numPixels cannot be more than 3 bytes long
    "\n\t  clr  r16" // counter to increment picture pointer
  "\n  loop001:"
    "\n\t  subi r17,1"
    "\n\t  sbci r19,1"
    "\n\t  sbci r18,1" // decrement loop
    "\n\t  inc r16"
    "\n\t  ror  r24" // rotate right, puts 0th bit into C

    "\n\t  sts  198,r8" // push out second byte // ignores C

    "\n\t  brcs carry002" // branch if carry set
    "\n\t  ld   r9,Z"
    "\n\t  ldd  r8,Z+1"
    "\n\t  rjmp end002"

  "\n  carry002:"
    "\n\t  ldd  r9,Z+2"
    "\n\t  ldd  r8,Z+3"

  "\n  end002:"

    "\n\t  cp   r18,__zero_reg__"
    "\n\t  cpc  r19,__zero_reg__"
    "\n\t  cpc  r17,__zero_reg__"
    "\n\t  breq done002"

    "\n\t  sts 198,r9"

    "\n\t  andi r16,7" // increment every 8 pixels sent, update picture pointer
    "\n\t  brne loop001" // breq (r16 & 7 == 0)

    "\n\t  adiw r26,1" // increment the picture pointer
    "\n\t  ld r24,X"
    "\n\t  rjmp loop001"
  "\n  done002:"
    :
    : "r" (picture), "r" (bit)
    : "r30", "r31", "r24", "r26", "r27", "r17", "r18", "r19", "r8", "r9", "r16"
  );*/
//}
