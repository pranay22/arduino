                            ; --------------------------------------
                            ; Test PUSH and POP.
                            ;
                            ; Set registers to values. PUSH the registers.
                            ; Set registers to zero.
                            ; POP registers to values. Check the push values are restored.
                            ;
                            ; --------------------------------------
            lxi sp,24       ; Stack pointer address
    Start:
                            ; --------------------------------------
                            ; Test A:F push and pop.
            call NewTest
            mvi a,'A'
            out 3
            mvi a,':'
            out 3
            mvi a,'F'
            out 3
                            ;
            mvi a,3
            cpi 3           ; Set zero flag on/1, carry bit off/0;
            out 37          ; Print all the register values and the flags.
            out 43          ; Print stack pointer and the flags.
            push f
            mvi a,0
            out 37          ; Register A.
            out 43
            pop f
            out 37
            out 43
                            ;
            mvi a,1
            cpi 3           ; Set zero flag off/0, carry bit on/1;
            out 37
            out 43
            push f
            mvi a,0
            out 37
            out 43
            pop f
            out 37
            out 43
                            ;
            mvi a,6
            cpi 3           ; Set zero flag off/0, carry bit off/0;
            out 37
            out 43
            push f
            mvi a,0
            out 37
            out 43
            pop f
            out 37
            out 43
                            ; --------------------------------------
                            ; Test B:C push and pop.
            call NewTest
            mvi a,'B'
            out 3
            mvi a,':'
            out 3
            mvi a,'C'
            out 3
                            ;
            mvi b,2
            mvi c,3
            push b
            out 30          ; Register B.
            out 31          ; Register C.
            mvi b,0
            mvi c,0
            out 30          ; Register B.
            out 31          ; Register C.
            pop b
            out 30          ; Register B.
            out 31          ; Register C.
            out 43
                            ; 
                            ; --------------------------------------
                            ; Test D:E push and pop.
            call NewTest
            mvi a,'D'
            out 3
            mvi a,':'
            out 3
            mvi a,'E'
            out 3
                            ;
            mvi d,2
            mvi e,3
            push d
            out 32          ; Register D.
            out 33          ; Register E.
            mvi d,0
            mvi e,0
            out 32          ; Register D.
            out 33          ; Register E.
            pop d
            out 32          ; Register D.
            out 33          ; Register E.
            out 43
                            ;
                            ; --------------------------------------
                            ; Test H:L push and pop.
            call NewTest
            mvi a,'H'
            out 3
            mvi a,':'
            out 3
            mvi a,'L'
            out 3
                            ;
            mvi h,2
            mvi l,3
            push h
            out 34          ; Register H.
            out 35          ; Register L.
            mvi h,0
            mvi l,0
            out 34          ; Register H.
            out 35          ; Register L.
            pop h
            out 34          ; Register H.
            out 35          ; Register L.
            out 43
                            ;
                            ; --------------------------------------
            call NewTest
            mvi a,'B'
            out 3
            mvi a,':'
            out 3
            mvi a,'C'
            out 3
            mvi a,' '
            out 3
            mvi a,'*'
            out 3
                            ;
            mvi b,2
            mvi c,3
            push b
            out 30          ; Register B.
            out 31          ; Register C.
            out 43          ; Stack pointer
                            ;
            mvi b,4
            mvi c,5
            push b
            out 30          ; Register B.
            out 31          ; Register C.
            out 43          ; Stack pointer
                            ;
            mvi b,6
            mvi c,7
            push b
            out 30          ; Register B.
            out 31          ; Register C.
            out 43          ; Stack pointer
                            ;
            mvi b,0
            mvi c,0
            out 30          ; Register B.
            out 31          ; Register C.
            out 43          ; Stack pointer
                            ;
            pop b
            out 30          ; Register B.
            out 31          ; Register C.
            out 43          ; Stack pointer
            pop b
            out 30          ; Register B.
            out 31          ; Register C.
            out 43          ; Stack pointer
            pop b
            out 30          ; Register B.
            out 31          ; Register C.
            out 43          ; Stack pointer
                            ; 
                            ; --------------------------------------
            hlt
            jmp Start
                            ; 
                            ; --------------------------------------
                            ; Routines
    NewTest:
            mvi a,'\n'
            out 3
            mvi a,'+'
            out 3
            mvi a,'+'
            out 3
            mvi a,' '
            out 3
            ret
                            ; 
                            ; --------------------------------------
            end
                            ; --------------------------------------
                            ; Successful run:
                            ;
++ A:F
 > Register A =   3 = 003 = 00000011
 > Stack pointer:    24, Zero bit flag: 1, Carry bit flag: 0
 > Register A =   0 = 000 = 00000000
 > Stack pointer:    22, Zero bit flag: 1, Carry bit flag: 0
 > Register A =   3 = 003 = 00000011
 > Stack pointer:    24, Zero bit flag: 1, Carry bit flag: 0
 > Register A =   1 = 001 = 00000001
 > Stack pointer:    24, Zero bit flag: 0, Carry bit flag: 1
 > Register A =   0 = 000 = 00000000
 > Stack pointer:    22, Zero bit flag: 0, Carry bit flag: 1
 > Register A =   1 = 001 = 00000001
 > Stack pointer:    24, Zero bit flag: 0, Carry bit flag: 1
 > Register A =   6 = 006 = 00000110
 > Stack pointer:    24, Zero bit flag: 0, Carry bit flag: 0
 > Register A =   0 = 000 = 00000000
 > Stack pointer:    22, Zero bit flag: 0, Carry bit flag: 0
 > Register A =   6 = 006 = 00000110
 > Stack pointer:    24, Zero bit flag: 0, Carry bit flag: 0
++ B:C
 > Register B =   2 = 002 = 00000010
 > Register C =   3 = 003 = 00000011
 > Register B =   0 = 000 = 00000000
 > Register C =   0 = 000 = 00000000
 > Register B =   2 = 002 = 00000010
 > Register C =   3 = 003 = 00000011
 > Stack pointer:    24, Zero bit flag: 0, Carry bit flag: 0
++ D:E
 > Register D =   2 = 002 = 00000010
 > Register E =   3 = 003 = 00000011
 > Register D =   0 = 000 = 00000000
 > Register E =   0 = 000 = 00000000
 > Register D =   2 = 002 = 00000010
 > Register E =   3 = 003 = 00000011
 > Stack pointer:    24, Zero bit flag: 0, Carry bit flag: 0
++ H:L
 > Register H =   2 = 002 = 00000010
 > Register L =   3 = 003 = 00000011
 > Register H =   0 = 000 = 00000000
 > Register L =   0 = 000 = 00000000
 > Register H =   2 = 002 = 00000010
 > Register L =   3 = 003 = 00000011
 > Stack pointer:    24, Zero bit flag: 0, Carry bit flag: 0
++ B:C *
 > Register B =   2 = 002 = 00000010
 > Register C =   3 = 003 = 00000011
 > Stack pointer:    22, Zero bit flag: 0, Carry bit flag: 0
 > Register B =   4 = 004 = 00000100
 > Register C =   5 = 005 = 00000101
 > Stack pointer:    20, Zero bit flag: 0, Carry bit flag: 0
 > Register B =   6 = 006 = 00000110
 > Register C =   7 = 007 = 00000111
 > Stack pointer:    18, Zero bit flag: 0, Carry bit flag: 0
 > Register B =   0 = 000 = 00000000
 > Register C =   0 = 000 = 00000000
 > Stack pointer:    18, Zero bit flag: 0, Carry bit flag: 0
 > Register B =   6 = 006 = 00000110
 > Register C =   7 = 007 = 00000111
 > Stack pointer:    20, Zero bit flag: 0, Carry bit flag: 0
 > Register B =   4 = 004 = 00000100
 > Register C =   5 = 005 = 00000101
 > Stack pointer:    22, Zero bit flag: 0, Carry bit flag: 0
 > Register B =   2 = 002 = 00000010
 > Register C =   3 = 003 = 00000011
 > Stack pointer:    24, Zero bit flag: 0, Carry bit flag: 0
+ HLT, program halted.
                            ;
                            ; --------------------------------------
