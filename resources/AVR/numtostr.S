.globl numtostr
numtostr:
  ; args: char * of at least 6 bytes, we use only the first 5, and assume the 6th is null terminated. (24/25)
  ;       word to print (22/23)
  push r22 ; so we don't mess up the word... Do I need to do this? Try without later
  push r23
  push r24
  push r25
  push r28
  push r29
  movw r28,r24
  ldi r30,0x2F
  ldi r31,0x30 ; ascii zero - 1

; REGISTERS
; r24/25: input string *
; r22/23: input word
; r28/29: Y pointer (string *)
; r24/25: remainder
; r30: counter / quotient
; r31: ascii zero

place10000:
  movw r24,r22
  inc  r30
  subi r22,lo8(10000)
  sbci r23,hi8(10000)
  brpl place10000 ; 1/2
  st   Y+,r30
  movw r22,r24
  ldi r30,0x2F
place1000:
  movw r24,r22
  inc  r30
  subi r22,lo8(1000)
  sbci r23,hi8(1000)
  brpl place1000 ; 1/2
  st   Y+,r30
  movw r22,r24
  ldi r30,0x2F
place100:
  movw r24,r22
  inc  r30
  subi r22,lo8(100)
  sbci r23,hi8(100)
  brpl place100 ; 1/2
  st   Y+,r30
  movw r22,r24
  ldi r30,0x2F
place10:
  movw r24,r22
  inc  r30
  subi r22,10
  brpl place10 ; 1/2
  st   Y+,r30
  add  r24,r31
  st   Y+,r24

  pop r29
  pop r28
  pop r25
  pop r24
  pop r23
  pop r22
  ret
