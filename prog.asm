; @author: Saul Neri
;
; Compiled with VASM, using vasm6502_oldstyle
;
; To compile the program (In my case, in Windows):
;
; vasm6502_oldstyle_win32.exe prog.asm -o prog.bin -Fbin

main:
  ; load x with #10
  ldx #$0a
  txa					; transfer X value to Acumulator
  ldx #00				; clear X register
  sta $0000				; store A value in address $0000 (A = 10)
  ; increment X by 3 times
  inx
  inx
  inx
  txa					; transfer X value to Acumulator
  sta $0001				; store A value in address $0001 (A = 3)
  nop


  


