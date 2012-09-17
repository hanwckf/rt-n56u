
;------------------------------
; Macros I: Faux Instructions
;
; The following "faux instructions" are
; implemented here as macros:
;
; MOVIP register,constant		MOVI with optional PFX & MOVHI, or BGEN
; ADDIP register,constant		PFX and ADDI with optional PFX
; SUBIP register,constant		PFX and SUBI with optional PFX
; CMPIP register,constant		PFX and CMPI with optional PFX
;
; MOVI16 register,constant		PFX and MOVI
; MOVI32 register,constant		PFX, MOVI, PFX, and MOVHI
; MOVIA  register,constant		PFX and MOVHI on Nios32, and PFX and MOVI
;
; ANDIP register,constant		PFX and ANDI
; ANDNIP register,constant		PFX and ANDN
; ORIP register,constant		PFX and ORI
; XORIP register,constant		PFX and XORI
;
; _BSR address						MOVIP address to %g7, and CALL
; _BR address						MOVIP address to %g7, and JMP
;
; BEQ address						SKPS cc_nz and BR, has delay slot
; BNE address						SKPS cc_z and BR, has delay slot
; BLE address						SKPS cc_gt and BR, has delay slot
; BLT address						SKPS cc_ge and BR, has delay slot
; RESTRET							RESTORE and JMP %i7 
;
;-------------------------------
; Macros II: Printing
;
; These macros are guaranteed *not*
; to have branch delay slot after them.
;
; NM_PrintChar char
; NM_Print "string"
; NM_PrintLn "string"			Follows it with a carriage return
; NM_PrintRegister reg			For debugging, prints register name & value
;
;-------------------------------
; Macros III: Inline Debugging
;
; These macros print various information
; using large sections of expanded inline code.
; They each use either few or no registers.
; Thus, they may be safely used in interrupt handlers.
;
; NM_D_TxChar char			print char to UART, affects no registers
; NM_D_TxRegister char,char,register	prints the two characters, and the hex register value

; --------------------------------------


		.macro	_pfx_op	OP,reg,val,pForce=0
		.if		(\pForce) || ((\val) > (31)) || ((\val) < (0))
		PFX		%hi(\val)
		.endif
		\OP		\reg,%lo(\val)
		.endm

		.macro	_bgen reg,val,bit
		.if ((\val)==(1<<\bit))
		BGEN	\reg,\bit
		.equ	_bgenBit,1
		.endif
		.endm

	;------------------------
	; MOVIP %reg,32-bit-value
		.macro	MOVIP reg,val
		; Methodically test every BGEN possibility...
		.equ	_bgenBit,0
.if 1
		_bgen \reg,\val,0
		_bgen \reg,\val,1
		_bgen \reg,\val,2
		_bgen \reg,\val,3
		_bgen \reg,\val,4
		_bgen \reg,\val,5
		_bgen \reg,\val,6
		_bgen \reg,\val,7
		_bgen \reg,\val,8
		_bgen \reg,\val,9
		_bgen \reg,\val,10
		_bgen \reg,\val,11
		_bgen \reg,\val,12
		_bgen \reg,\val,13
		_bgen \reg,\val,14
		_bgen \reg,\val,15
		_bgen \reg,\val,16
		_bgen \reg,\val,17
		_bgen \reg,\val,18
		_bgen \reg,\val,19
		_bgen \reg,\val,20
		_bgen \reg,\val,21
		_bgen \reg,\val,22
		_bgen \reg,\val,23
		_bgen \reg,\val,24
		_bgen \reg,\val,25
		_bgen \reg,\val,26
		_bgen \reg,\val,27
		_bgen \reg,\val,28
		_bgen \reg,\val,29
		_bgen \reg,\val,30
		_bgen \reg,\val,31

		; If no bgen fit...
.endif
		.if !_bgenBit
			.if ((\val) & 0xFFE0)
				PFX %hi(\val)
			.endif
			MOVI \reg,%lo(\val)
			.if __nios32__
				.if ((\val) & 0xffff0000)
					.if ((\val) & 0xFFE00000)
						PFX %xhi(\val)
					.endif
					MOVHI \reg,%xlo(\val)
				.endif
			.endif
		.endif

		.endm

	; ADDIP %reg,16-bit-value
		.macro	ADDIP reg,val
		_pfx_op	ADDI,\reg,\val
		.endm

	; SUBIP %reg,16-bit-value
		.macro	SUBIP reg,val
		_pfx_op	SUBI,\reg,\val
		.endm

	; CMPIP %reg,16-bit-value
		.macro	CMPIP reg,val
		_pfx_op	CMPI,\reg,\val
		.endm

	; ANDIP %reg,16-bit-value
		.macro	ANDIP reg,val
		PFX		%hi(\val)
		AND		\reg,%lo(\val)
		.endm

	; ANDNIP %reg,16-bit-value
		.macro	ANDNIP reg,val
		PFX		%hi(\val)
		ANDN		\reg,%lo(\val)
		.endm

	; ORIP %reg,16-bit-value
		.macro	ORIP reg,val
		PFX		%hi(\val)
		OR			\reg,%lo(\val)
		.endm

	; XORIP %reg,16-bit-value
		.macro	XORIP reg,val
		PFX		%hi(\val)
		XOR		\reg,%lo(\val)
		.endm

	; BEQ addr
		.macro	BEQ addr
		IFS		cc_eq
		BR			\addr
		.endm

	; BNE addr
		.macro	BNE addr
		IFS		cc_ne
		BR			\addr
		.endm

	; BLE addr
		.macro	BLE addr
		SKPS		cc_gt
		BR			\addr
		.endm

	; BLT addr
		.macro	BLT addr
		SKPS		cc_ge
		BR			\addr
		.endm

		.macro	digitToChar reg
		ANDIP	\reg,0x000f
		CMPI	\reg,10
		SKPS	cc_lt
		ADDI	\reg,'A'-'0'-10
		PFX		%hi('0')
		ADDI	\reg,%lo('0')
		.endm

; PUSHRET == dec sp, and stash return addr
	.macro	PUSHRET
	SUBI		%sp,2
	ST			[%sp],%o7
	.endm
; POPRET == pop and jump
	.macro	POPRET
	LD			%o7,[%sp]
	JMP		%o7
	ADDI		%sp,2		; branch delay slot
	.endm

; RESTRET = restore & return
	.macro	RESTRET
	JMP		%i7
	RESTORE
	.endm

	;--------------------
	; MOVI16 %reg,Address
	;
	.macro	MOVI16	reg,val
	PFX	%hi(\val)
	MOVI	\reg,%lo(\val)
	.endm

	;--------------------
	; MOVI32 %reg,Address
	;
	.macro	MOVI32	reg,val
	PFX	%hi(\val)
	MOVI	\reg,%lo(\val)
	PFX	%xhi(\val)
	MOVHI	\reg,%xlo(\val)
	.endm

	;--------------------
	; MOVIA %reg,Address
	;
	.macro	MOVIA		reg,val
	.if __nios32__
		MOVI32 \reg,\val
	.else
		MOVI16 \reg,\val
	.endif
	.endm

	;--------------------
	; _BR

	.macro _BR target,viaRegister=%g7
	MOVIA	\viaRegister,\target@h
	JMP	\viaRegister
	.endm

	;--------------------
	; _BSR

	.macro _BSR target,viaRegister=%g7
	MOVIA	\viaRegister,\target@h
	CALL	\viaRegister
	.endm

	;---------------------
	; NM_Print "Your String Here"
	;
	.macro	NM_Print	string

	BR		pastStringData\@
	NOP

stringData\@:
	.asciz	"\string"
	.align 1		; aligns by 2^n
pastStringData\@:
	MOVIA		%o0,stringData\@
	_BSR		NR_TxString
	NOP
	.endm

	.macro	NM_PrintLn string
	NM_Print	"\string"
	_BSR		NR_TxCR
	NOP
	.endm

	.macro	NM_PrintRegister reg	; affects %g0 & %g1 & %g7, but thrashes the CWP a bit
	SAVE		%sp,-16
	NM_Print	"\reg = "
	RESTORE
	MOV		%g0,\reg
	SAVE		%sp,-16
	MOV		%o0,%g0
	_BSR		NR_TxHex
	NOP
	_BSR		NR_TxCR
	NOP
	RESTORE
	.endm

	.macro	NM_PrintChar char
	MOVIP		%o0,\char
	_BSR		NR_TxChar
	NOP
	.endm

	.macro	NM_Print2Chars char1,char2
	MOVIP		%o0,(\char2<<8)+\char1
	_BSR		NR_TxChar
	NOP
	_BSR		NR_TxChar
	LSRI		%o0,8
	.endm



; ---------------------------
; Completely inline UART sends
; Send the char, or %g7 if not there.
; Trashes %g5 and %g6 and %g7...

	.macro	NM_TxChar char=0
;NM_D_Delay 1000
	MOVIA	%g6,NA_UARTBase
txCharLoop\@:
	PFX	2
.if \char
	LD	%g7,[%g6]
	SKP1	%g7,6
.else
	LD	%g5,[%g6]
	SKP1	%g5,6
.endif
	BR	txCharLoop\@
	NOP
.if \char
	MOVIP	%g7,\char
.endif
	PFX	1
	ST	[%g6],%g7
;NM_D_Delay 4
	.endm

		.macro NM_TxCR
		NM_TxChar 13
		NM_TxChar 10
		.endm

		.macro NM_TxHexDigit,reg,shift
		MOV		%g7,\reg
		LSRI		%g7,\shift
		ANDIP		%g7,0x000f
		CMPI		%g7,10
		SKPS		cc_lt
		ADDIP		%g7,'A'-'0'-10
		ADDIP		%g7,'0'
		NM_TxChar
		.endm

		.macro NM_TxHex

	.if __nios32__
		NM_TxHexDigit %g0,28
		NM_TxHexDigit %g0,24
		NM_TxHexDigit %g0,20
		NM_TxHexDigit %g0,16
	.endif

		NM_TxHexDigit %g0,12
		NM_TxHexDigit %g0,8
		NM_TxHexDigit %g0,4
		NM_TxHexDigit %g0,0
		.endm










; ----------------------
; The following macros are
; rather mighty. They expand
; to large inline code for
; printing various things to
; the serial port. They are
; useful for debugging
; trap handlers, where you
; can't just go and call
; NR_TxChar and such, because,
; well, the CWP might be
; off limits!
;
; They do, however, presume
; that the stack is in good
; working order.


.macro NM_D_PushGRegisters
 	SUBIP %sp,16+69				; oddball number so if we accidentally see it, it looks funny.
	STS	[%sp,16+0],%g0
	STS	[%sp,16+1],%g1
	STS	[%sp,16+2],%g2
	STS	[%sp,16+3],%g3
	STS	[%sp,16+4],%g4
	STS	[%sp,16+5],%g5
	STS	[%sp,16+6],%g6
	STS	[%sp,16+7],%g7
	.endm

.macro NM_D_PopGRegisters
	LDS	%g0,[%sp,16+0]
	LDS	%g1,[%sp,16+1]
	LDS	%g2,[%sp,16+2]
	LDS	%g3,[%sp,16+3]
	LDS	%g4,[%sp,16+4]
	LDS	%g5,[%sp,16+5]
	LDS	%g6,[%sp,16+6]
	LDS	%g7,[%sp,16+7]
	ADDIP	%sp,16+69				; must match the push
	.endm


.macro NM_D_TxChar	c
	SUBI	%sp,16+8		; 32 or 16 bit, that's enough space
	STS	[%sp,16+0],%g6
	STS	[%sp,16+0],%g7
	NM_TxChar \c
	LDS	%g6,[%sp,16+0]
	LDS	%g7,[%sp,16+1]
	ADDI	%sp,16+8
	.endm

.macro NM_D_TxChar3 c1,c2,c3
 NM_D_TxChar '<'
 NM_D_TxChar \c1
 NM_D_TxChar \c2
 NM_D_TxChar \c3
 NM_D_TxChar '>'
.endm

.macro NM_D_TxRegister r,n,reg
 NM_D_PushGRegisters
 NM_TxChar '('
 NM_TxChar \r
 NM_TxChar \n
 NM_TxChar ':'
 MOV		%g0,\reg
 NM_TxHex
 NM_TxChar ')'
 NM_D_PopGRegisters
.endm

.macro NM_D_TxReg r,n,reg
	NM_D_TxRegister \r,\n,\reg
.endm

; Do a delay loop, affects no registers.

.macro NM_D_Delay d
	SUBI	%sp,16+4
	STS	[%sp,16+0],%g0
	MOVIP	%g0,\d
NM_D_DelayLoop\@:
	IFRnz	%g0
	 BR	NM_D_DelayLoop\@
	SUBI	%g0,1
	LDS	%g0,[%sp,16+0]
	ADDI	%sp,16+4
.endm

