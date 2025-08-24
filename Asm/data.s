.data
	dw a
	dw b
	dw c
	dw d
	dw e
	dw f
.stack
	db dup(0x1000)
.code
proc play:
	sub $sp 0
	loadw $0 a
	loadb $7 1
	sub $0 $7 $8
	store a $8
	loadw $1 b
	loadb $9 1
	sub $1 $9 $10
	store b $10
	loadw $2 c
	loadb $11 1
	sub $2 $11 $12
	store c $12
endp
proc draw:
	;x @2
	;y @4
	;a1 @0
	;b1 @2
	;c1 @4
	;d1 @6
	sub $sp 8
	loadw $14 x
	store a1 $14
	loadw $15 y
	store b1 $15
	loadw $16 a1
	store c1 $16
	loadw $17 b1
	store d1 $17
	loadw $18 c1
	store e1 $18
	loadw $19 d1
	store f1 $19
	call play 0
endp
proc main:
	;a @0
	sub $sp 2
	loadb $26 10
	loadb $27 11
	param $27
	param $26
	call draw 2
endp
