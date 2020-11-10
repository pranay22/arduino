                            ; --------------------------------------
                            ; Test LDA and STA. Moving data from addresses to registers and back.
                            ;
                            ; --------------------------------------
                            ; Video demonstrating status lights:
                            ;    https://www.youtube.com/watch?v=3_73NwB6toY
                            ; Program listing: https://altairclone.com/downloads/status_lights.pdf
                            ; 
                            ; 
                            ; --------------------------------------
    Start:
            org 0
            lda 32          ; opcode fetch, mem read x 2, mem write
            sta 33          ; opcode fetch, memory read x 3
            lxi sp,32       ; opcode fetch, mem read x 2
            push a
            pop a
        ;    in      20Q     ;opcode fetch, mem read, I/O input
        ;    out     20Q     ;opcode fetch, mem read, I/O output
        ;    ei              ;interrupts enabled
        ;    di              ;interrupts disabled

            hlt
                            ; --------------------------------------
                            ; Before running load their random data into memory.
                            ; Mem : Data Decimal
                            ; 040 : 353  235
            mvi a,235       ; Octal 353
            out 37
            lda 32          ; Memory location octal 040.
            jmp Start
                            ; --------------------------------------
            end
                            ; To match the video, I moved 235 into address 32.
                            ; Process instruction: lda 32           + Status LEDs ON    OCT Data    Address
                            ; --- lda 32
                            ; First:  Fetch Opcode                  + On: MEMR MI    WO 072         0
                            ; Second, Fetch lb                      + On: MEMR       WO 040         1
                            ; Third,  Fetch hb                      + On: MEMR       WO 000         2
                            ; Forth,  Fetch data from memory        + On: MEMR       WO 377 (353)   2 (Should be memory address: 000 040)
                            ; --- sta 33
                            ; First:  Fetch Opcode                  + On: MEMR MI    WO 062         3
                            ; Second, Fetch lb                      + On: MEMR       WO 041         4
                            ; Third,  Fetch hb                      + On: MEMR       WO 000         5
                            ; Forth,  Write data to memory          + On: MEMR       WO 377 (353)   5 (Should be memory address: 000 041)
                            ;                                         (Status lights should be off)
                            ; --- lxi sp,32
                            ; First:  Fetch Opcode                  + On: MEMR MI    WO 061         6
                            ; Second, Fetch lb                      + On: MEMR       WO 040         7
                            ; Third,  Fetch hb                      + On: MEMR       WO 000         8
                            ; --- push a
                            ; First:  Fetch Opcode                  + On: MEMR MI    WO 365         9
                            ; Second: Stack write                   + On:      STACK    377 (040)   037 (This is missing)
                            ; Third:  Stack write                   + On:      STACK    377 (000)   036 (This is missing)
                            ; --- pop a
                            ; First:  Fetch Opcode                  + On: MEMR MI    WO 361         10
                            ; Second: Stack read                    + On: MEMR STACK WO 377 (040)   036 (This is missing)
                            ; Third:  Stack read                    + On: MEMR STACK WO 377 (000)   037 (This is missing)
                            ; --- hlt
                            ; First:  Fetch Opcode                  + On: MEMR MI WO    166         11
                            ;
++ Address:16-bit bytes       databyte :hex:oct > description
++       0:00000000 00000000: 00111010 : 3A:072 > opcode: lda 32
++       1:00000000 00000001: 00100000 : 20:040 > lb: 32
++       2:00000000 00000010: 00000000 : 00:000 > hb: 0
++       3:00000000 00000011: 00110010 : 32:062 > opcode: sta 33
++       4:00000000 00000100: 00100001 : 21:041 > lb: 33
++       5:00000000 00000101: 00000000 : 00:000 > hb: 0
++       6:00000000 00000110: 00110001 : 31:061 > opcode: lxi sp,32
++       7:00000000 00000111: 00100000 : 20:040 > lb: 32
++       8:00000000 00001000: 00000000 : 00:000 > hb: 0
++       9:00000000 00001001: 11110101 : F5:365 > opcode: push a
++      10:00000000 00001010: 11110001 : F1:361 > opcode: pop a
++      11:00000000 00001011: 01110110 : 76:166 > opcode: hlt
+ End of list.

00: 072 040 000 062 041 000 061 040 
10: 000 365 361 333 020 323 020 373 
20: 363 166P