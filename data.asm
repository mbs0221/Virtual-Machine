data 6 data
load $3 #0
store $3 *0
load $4 #24
store $4 *2
load $5 #48
store $5 *4
load $6 #10
store $6 *0
label L0:
load $0 *0
load $7 #0
> $0 $7 $8
jg L2
jmp L1
label L2:
load $0 *0
load $9 #1
- $0 $9 $10
store $10 *0
load $1 *2
load $11 #1
+ $1 $11 $12
store $12 *2
jmp L0
label L1:
label L3:
load $0 *0
load $13 #10
< $0 $13 $14
jb L5
jmp L4
label L5:
load $0 *0
load $15 #1
+ $0 $15 $16
store $16 *0
jmp L3
label L4:
label L6:
load $1 *2
load $17 #1
- $1 $17 $18
store $18 *2
load $2 *4
load $19 #1
+ $2 $19 $20
store $20 *4
load $1 *2
load $21 #12
> $1 $21 $22
jg L6
jmp L7
label L7:
load $1 *2
load $0 *0
! $1 $0 $23
jne L9
jmp L8
label L9:
load $0 *0
load $24 #64
+ $0 $24 $25
store $25 *0
load $1 *2
load $26 #32
+ $1 $26 $27
store $27 *2
load $2 *4
load $28 #16
+ $2 $28 $29
store $29 *4
label L8:
halt
#