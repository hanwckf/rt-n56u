

	.include "NM_Macros.S"

	.file	"okmul.c"
gcc2_compiled.:
	.text
	.p2align 1
	.globl __mulsi3
	.type	 __mulsi3,@function
__mulsi3:

	;SKP0	%o0,31
	;NEG	%o1
	;ABS	%o0

		.MACRO ZSTEP bit
		SKP0	%o0,\bit
		ADD	%g0,%o1
		LSLI	%o1,1
		.ENDM

	MOVI	%g0,0
	ZSTEP	0
	ZSTEP	1
	ZSTEP	2
	ZSTEP	3
	ZSTEP	4
	ZSTEP	5
	ZSTEP	6
	ZSTEP	7
	ZSTEP	8
	ZSTEP	9
	ZSTEP	10
	ZSTEP	11
	ZSTEP	12
	ZSTEP	13
	ZSTEP	14
	ZSTEP	15
	ZSTEP	16
	ZSTEP	17
	ZSTEP	18
	ZSTEP	19
	ZSTEP	20
	ZSTEP	21
	ZSTEP	22
	ZSTEP	23
	ZSTEP	24
	ZSTEP	25
	ZSTEP	26
	ZSTEP	27
	ZSTEP	28
	ZSTEP	29
	ZSTEP	30
	ZSTEP	31
	; No bit 31: we already set %o0 to positive

	JMP	%o7
	MOV	%o0,%g0

.Lfe1:
	.size	 __mulsi3,.Lfe1-__mulsi3

