/*
 * This file supplies function epilogues for the .init and .fini sections.
 * It is linked in after all other files.
 */

	.section .init
	ldw .d2t2	*+B15(4), B3
	add .d2		B15, 8, B15
	nop		3
	ret .s2		B3
	nop		5

	.section .fini
	ldw .d2t2	*+B15(4), B3
	add .d2		B15, 8, B15
	nop		3
	ret .s2		B3
	nop		5

