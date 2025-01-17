                                    ; --------------------------------------
                                    ; Test SHLD.
                                    ; 
                                    ; Load the H:L register pair bytes into a memory address and the memory address plus one.
                                    ; SHLD adr : L -> (adr); H -> (adr+1)
                                    ; Store register L to address: adr(hb:lb).
                                    ; Store register H to address: adr+1.
                                    ;
                                    ; --------------------------------------
                lxi sp,512          ; Set stack pointer which is used with CALL and RET.
    Start:
                call Initialize
                                    ; --------------------------------------
                mvi h,0             ; Set address register pair H:L, to 0:0.
                mvi l,0
                out 36              ; Print the register values for H:L, and the content at that address.
                                    ;
                sta byteL           ; Initialize variables to 0. Set the address values to zero (register A value).
                sta byteH
                lda byteL           ; Confirm set to 0. Load and print the address values.
                out 37
                lda byteH
                out 37
                                    ;
                                    ; --------------------------------------
                call NewTest
                lxi h,Addr3         ; Load the address value of Addr3 into H:L.
                                    ; Register H:L = 3:18, address# 786.
                out 36              ; Print the register values for H:L, and the content at that address.
                                    ;
                shld byteL          ; Given byteL address(hb:lb),
                                    ;   Store register L to memory location of byteL (byteL address).
                                    ;   Store register H to memory location of byteL + 1 (byteH address).
                                    ;
                lda byteH           ; Load and print the address values.
                out 37
                lda byteL
                out 37
                                    ; --------------------------------------
                lxi sp,512          ; Set stack pointer which is used with CALL and RET.
                call NewTest
                                    ; --------------------------------------
                call println
                hlt
                jmp Start
                                    ; ------------------------------------------
                                    ; ------------------------------------------
                                    ; Subroutines
                                    ;
                                    ; -------------------
        InitializeStr db    '\r\n++ Initialize setting.'
    Initialize:
                lxi h,InitializeStr
                call PrintStr
                mvi a,0             ; Initialize the test counter.
                sta testCounter
                ret
                                    ; -------------------
        SeparatorStr    db  '\r\n---'
    Separator:
                push h
                lxi h,SeparatorStr
                call PrintStr
                pop h
                ret
                                    ; -------------------
        NewTestA    ds      1
        TestStr     db      '\r\n++ '
        testCounter db  0           ; Initialize test counter.
    NewTest:
                push h
                sta NewTestA
                sta PrintDigitA     ; Retain register A value.
                lxi h,TestStr
                call PrintStr
                lda testCounter     ; Increment and print the counter
                inr a
                sta testCounter
                call PrintDigit
                mvi a,':'
                out 3
                mvi a,' '
                out 3
                lda NewTestA
                pop h
                ret
                                    ; -------------------
        PrintDigitA    db 0         ; A variable for storing register A's value.
    PrintDigit:                 ; Print the digit (0-9) in register A.
                sta PrintDigitA     ; Retain register A value.
                mvi b,'0'
                add b
                out 3
                lda PrintDigitA     ; Restore register A value.
                ret
                                    ; -------------------
                                    ; Routines to print a DB strings.
                                    ;
        PrintStrA ds    1
        STRTERM equ     0ffh        ; String terminator. ASM appends 0ffh at the end of db strings.
    PrintStr:
                sta PrintStrA       ; Retain register A value.
    PrintStrContinue:
                mov a,m             ; Move the data from H:L address to register A. (HL) -> A. 
                cpi STRTERM         ; Compare to see if it's the string terminate byte.
                jz PrintStrDone
                out 3               ; Out register A to the serial terminal port.
                inx h               ; Increment H:L register pair.
                jmp PrintStrContinue
        PrintStrDone:
                lda PrintStrA       ; Restore register A value.
                ret
                                    ;
    println:
                mvi a,'\r'
                out 3
                mvi a,'\n'
                out 3
                ret
                                    ; --------------------------------------
                                    ; Variables
                                    ;
                                    ;         H         L
    Addr1       equ     128         ; 0000 0000 1000 0000 H:L = 0:128
    Addr2       equ     512         ; 0000 0010 0000 0000 H:L = 2:000
    Addr3       equ     786         ; 0000 0011 0001 0010 H:L = 3:018
    byteL       ds      1
    byteH       ds      1
                                    ; --------------------------------------
                end
                                    ; --------------------------------------
                                    ; Successful run:
                                    ;
 > Register H:L =   0:  0, Data:  62 = 076 = 00111110
 > Register A =   0 = 000 = 00000000
 > Register A =   0 = 000 = 00000000
++ 1: 
 > Register H:L =   3: 18, Data:   0 = 000 = 00000000
 > Register A =  18 = 022 = 00010010
 > Register A =   3 = 003 = 00000011
+ Download complete.
+ r, RUN.
+ runProcessor()

 > Register H:L =   0:  0, Data:  49 = 061 = 00110001
 > Register A =   0 = 000 = 00000000
 > Register A =   0 = 000 = 00000000
                                    ++ 1: 
 > Register H:L =   3: 18, Data:   0 = 000 = 00000000
 > Register A =  18 = 022 = 00010010
 > Register A =   3 = 003 = 00000011
++ HALT, host_read_status_led_WAIT() = 0
                                    ;
++      31:00000000 00011111: 00100001 : 21:041 > opcode: lxi h,Addr3
++      32:00000000 00100000: 00010010 : 12:022 > lb: 18
++      33:00000000 00100001: 00000011 : 03:003 > hb: 3
                                    ;
++ 2: 
 > Register SP =   512 = 00000010:00000000
 > Register H:L =   0:  0 = 00000000:00000000, Data:  49 = 061 = 00110001
 > Register A =   0 = 000 = 00000000
 > Register A =   0 = 000 = 00000000
 > Register SP =     0 = 00000000:00000000
                                    ; --------------------------------------

...
                                    ; --------------------------------------
                                    ;  + Store SP to memory and retrieve SP from memory .
                lxi sp,512          ; Set stack pointer which is used with CALL and RET.
                call NewTest
                                    ;  + Store SP to a memory address and retrieve it.
                out 45              ; Print the SP register address.
                xthl                ;  ++ XTHL       L <-> (SP); H <-> (SP+1) ... Set H:L to same value as SP
                out 36              ; Print the register values for H:L.
                shld byteL          ;  ++ SHLD adr   (adr) <-L; (adr+1)<-H :
                lda byteL           ; Confirm the address values.
                out 37
                lda byteH
                out 37
                                    ;  + Restore SP from a memory address:
                lxi sp,255          ; Set stack pointer test value.
                lhld byteL          ;  ++ LHLD adr   SP=HL                    ... Set SP to same value as H:L
                sphl                ;  ++ SPHL       SP=HL Set SP to the same address value as H:L.
                out 45              ; Print the SP register address.
                                    ;