	.arch armv8-a
	.file	"DSHOT.c"
	.text
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align	3
.LC0:
	.string	"DSHOT.c"
	.align	3
.LC1:
	.string	"motorMax < DSHOT_NUM_PINS"
	.align	3
.LC2:
	.string	"debug Dshot Frame failure."
	.text
	.align	2
	.p2align 4,,11
	.global	dshotSendFrames
	.type	dshotSendFrames, %function
dshotSendFrames:
.LFB26:
	.cfi_startproc
	stp	x29, x30, [sp, -192]!
	.cfi_def_cfa_offset 192
	.cfi_offset 29, -192
	.cfi_offset 30, -184
	mov	x29, sp
	stp	x19, x20, [sp, 16]
	stp	x21, x22, [sp, 32]
	stp	x23, x24, [sp, 48]
	stp	x25, x26, [sp, 64]
	stp	x27, x28, [sp, 80]
	.cfi_offset 19, -176
	.cfi_offset 20, -168
	.cfi_offset 21, -160
	.cfi_offset 22, -152
	.cfi_offset 23, -144
	.cfi_offset 24, -136
	.cfi_offset 25, -128
	.cfi_offset 26, -120
	.cfi_offset 27, -112
	.cfi_offset 28, -104
	cmp	w1, 26
	bgt	.L2
	mov	w10, w1
	mov	x6, x0
	mov	x4, x2
	mov	w27, 0
	cmp	w1, 0
	ble	.L4
	mov	x1, x0
	add	x3, x0, w10, sxtw 2
	mov	w27, 0
	mov	w2, 1
	.p2align 3,,7
.L5:
	ldr	w0, [x1], 4
	lsl	w0, w2, w0
	orr	w27, w27, w0
	cmp	x3, x1
	bne	.L5
.L4:
	add	x0, sp, 128
	sbfiz	x5, x10, 2, 32
	mov	x9, x0
	mov	w8, 0
	mov	w11, 32768
	mov	w7, 1
	str	x0, [sp, 104]
	.p2align 3,,7
.L9:
	asr	w3, w11, w8
	cmp	w10, 0
	ble	.L18
	mov	x0, 0
	mov	w2, 0
	.p2align 3,,7
.L8:
	ldr	w1, [x4, x0]
	tst	w3, w1
	bne	.L7
	ldr	w1, [x6, x0]
	lsl	w1, w7, w1
	orr	w2, w2, w1
.L7:
	add	x0, x0, 4
	cmp	x5, x0
	bne	.L8
	add	w8, w8, 1
	str	w2, [x9], 4
	cmp	w8, 16
	bne	.L9
.L31:
	adrp	x0, .LANCHOR1
	mov	x19, 51712
	ldr	x1, [sp, 104]
	add	x20, sp, 112
	ldr	x25, [x0, #:lo12:.LANCHOR1]
	add	x28, x1, 64
	movk	x19, 0x3b9a, lsl 16
	mov	w1, 10
	mov	x26, 5300
	str	w1, [sp, 100]
	.p2align 3,,7
.L16:
	mov	x1, x20
	mov	w0, 4
	bl	clock_gettime
	ldp	x24, x21, [sp, 104]
	ldr	x0, [sp, 120]
	madd	x21, x21, x19, x0
	add	x23, x21, 3534
	add	x22, x21, 1768
	.p2align 3,,7
.L10:
	mov	x1, x20
	mov	w0, 4
	bl	clock_gettime
	ldp	x0, x1, [sp, 112]
	madd	x0, x0, x19, x1
	cmp	x22, x0
	bgt	.L10
	sub	x0, x0, x22
	str	w27, [x25, 28]
	cmp	x0, 2000
	bgt	.L11
	.p2align 3,,7
.L12:
	mov	x1, x20
	mov	w0, 4
	bl	clock_gettime
	ldp	x0, x1, [sp, 112]
	madd	x0, x0, x19, x1
	cmp	x23, x0
	bgt	.L12
	ldr	w1, [x24]
	sub	x0, x0, x23
	str	w1, [x25, 40]
	cmp	x0, 2000
	bgt	.L11
	add	x21, x21, x26
	.p2align 3,,7
.L13:
	mov	x1, x20
	mov	w0, 4
	bl	clock_gettime
	ldp	x0, x1, [sp, 112]
	madd	x0, x0, x19, x1
	cmp	x21, x0
	bgt	.L13
	sub	x0, x0, x21
	str	w27, [x25, 40]
	cmp	x0, 2000
	bgt	.L11
	add	x24, x24, 4
	add	x23, x23, x26
	add	x22, x22, x26
	cmp	x28, x24
	bne	.L10
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 192
	.cfi_remember_state
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 27
	.cfi_restore 28
	.cfi_restore 25
	.cfi_restore 26
	.cfi_restore 23
	.cfi_restore 24
	.cfi_restore 21
	.cfi_restore 22
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	ret
	.p2align 2,,3
.L11:
	.cfi_restore_state
	str	w27, [x25, 40]
	mov	w0, 100
	bl	usleep
	ldr	w0, [sp, 100]
	subs	w0, w0, #1
	str	w0, [sp, 100]
	bne	.L16
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	bl	puts
	adrp	x0, :got:stdout
	ldr	x0, [x0, :got_lo12:stdout]
	ldp	x19, x20, [sp, 16]
	ldr	x0, [x0]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 192
	.cfi_remember_state
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 27
	.cfi_restore 28
	.cfi_restore 25
	.cfi_restore 26
	.cfi_restore 23
	.cfi_restore 24
	.cfi_restore 21
	.cfi_restore 22
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	b	fflush
.L18:
	.cfi_restore_state
	mov	w2, 0
	add	w8, w8, 1
	str	w2, [x9], 4
	cmp	w8, 16
	bne	.L9
	b	.L31
.L2:
	adrp	x3, .LANCHOR0
	adrp	x1, .LC0
	adrp	x0, .LC1
	add	x3, x3, :lo12:.LANCHOR0
	add	x1, x1, :lo12:.LC0
	add	x0, x0, :lo12:.LC1
	mov	w2, 294
	bl	__assert_fail
	.cfi_endproc
.LFE26:
	.size	dshotSendFrames, .-dshotSendFrames
	.align	2
	.p2align 4,,11
	.type	dshotRepeatSendCommand, %function
dshotRepeatSendCommand:
.LFB29:
	.cfi_startproc
	stp	x29, x30, [sp, -192]!
	.cfi_def_cfa_offset 192
	.cfi_offset 29, -192
	.cfi_offset 30, -184
	mov	x29, sp
	stp	x19, x20, [sp, 16]
	.cfi_offset 19, -176
	.cfi_offset 20, -168
	mov	w20, w1
	mov	w19, w4
	stp	x21, x22, [sp, 32]
	.cfi_offset 21, -160
	.cfi_offset 22, -152
	mov	x21, x0
	stp	x23, x24, [sp, 48]
	.cfi_offset 23, -144
	.cfi_offset 24, -136
	cmp	w1, 0
	ble	.L33
	orr	w3, w3, w2, lsl 1
	sxtw	x4, w1
	add	x1, sp, 80
	mov	x2, 1
	eor	w0, w3, w3, asr 4
	bfi	w0, w3, 4, 28
	.p2align 3,,7
.L34:
	add	x5, x1, x2, lsl 2
	cmp	x4, x2
	add	x2, x2, 1
	str	w0, [x5, -4]
	bne	.L34
.L33:
	add	x23, sp, 64
	mov	w0, 4
	mov	x1, x23
	bl	clock_gettime
	ldp	x0, x2, [sp, 64]
	mov	x22, 51712
	movk	x22, 0x3b9a, lsl 16
	mov	w1, 16960
	movk	w1, 0xf, lsl 16
	add	x24, sp, 80
	madd	x0, x0, x22, x2
	smaddl	x19, w19, w1, x0
	b	.L35
	.p2align 2,,3
.L36:
	bl	dshotSendFrames
	mov	w0, 100
	bl	usleep
.L35:
	mov	x1, x23
	mov	w0, 4
	bl	clock_gettime
	ldp	x3, x4, [sp, 64]
	mov	x2, x24
	mov	w1, w20
	mov	x0, x21
	madd	x3, x3, x22, x4
	cmp	x19, x3
	bcs	.L36
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x29, x30, [sp], 192
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 23
	.cfi_restore 24
	.cfi_restore 21
	.cfi_restore 22
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE29:
	.size	dshotRepeatSendCommand, .-dshotRepeatSendCommand
	.align	2
	.p2align 4,,11
	.global	motorImplementationSet3dModeAndSpinDirection
	.type	motorImplementationSet3dModeAndSpinDirection, %function
motorImplementationSet3dModeAndSpinDirection:
.LFB30:
	.cfi_startproc
	stp	x29, x30, [sp, -48]!
	.cfi_def_cfa_offset 48
	.cfi_offset 29, -48
	.cfi_offset 30, -40
	adrp	x4, .LANCHOR1+8
	mov	x29, sp
	str	w2, [x4, #:lo12:.LANCHOR1+8]
	mov	w4, 25
	stp	x19, x20, [sp, 16]
	.cfi_offset 19, -32
	.cfi_offset 20, -24
	mov	x19, x0
	mov	w20, w1
	str	x21, [sp, 32]
	.cfi_offset 21, -16
	mov	w21, w3
	mov	w3, 1
	cbz	w2, .L40
	mov	w2, 10
	bl	dshotRepeatSendCommand
.L41:
	mov	w4, 25
	mov	w3, 0
	mov	w1, w20
	mov	x0, x19
	mov	w2, 12
	bl	dshotRepeatSendCommand
	mov	w4, 25
	mov	w3, 1
	cbz	w21, .L42
	mov	w1, w20
	mov	x0, x19
	mov	w2, 21
	bl	dshotRepeatSendCommand
.L43:
	ldr	x21, [sp, 32]
	mov	w1, w20
	mov	x0, x19
	mov	w4, 25
	ldp	x19, x20, [sp, 16]
	mov	w3, 0
	ldp	x29, x30, [sp], 48
	.cfi_remember_state
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 21
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	mov	w2, 12
	b	dshotRepeatSendCommand
	.p2align 2,,3
.L40:
	.cfi_restore_state
	mov	w2, 9
	bl	dshotRepeatSendCommand
	b	.L41
	.p2align 2,,3
.L42:
	mov	w1, w20
	mov	x0, x19
	mov	w2, 20
	bl	dshotRepeatSendCommand
	b	.L43
	.cfi_endproc
.LFE30:
	.size	motorImplementationSet3dModeAndSpinDirection, .-motorImplementationSet3dModeAndSpinDirection
	.section	.rodata.str1.8
	.align	3
.LC3:
	.string	"rb"
	.align	3
.LC4:
	.string	"/proc/device-tree/system/linux,revision"
	.align	3
.LC5:
	.string	"debug Error: Can't open '%s'\n"
	.align	3
.LC6:
	.string	"debug Error: Revision data too short"
	.align	3
.LC7:
	.string	"debug Error: Unrecognised revision code"
	.align	3
.LC8:
	.string	"/dev/gpiomem"
	.align	3
.LC9:
	.string	"debug Error: Can't open /dev/mem "
	.align	3
.LC10:
	.string	"debug Mmap error %p\n"
	.text
	.align	2
	.p2align 4,,11
	.global	motorImplementationInitialize
	.type	motorImplementationInitialize, %function
motorImplementationInitialize:
.LFB31:
	.cfi_startproc
	stp	x29, x30, [sp, -80]!
	.cfi_def_cfa_offset 80
	.cfi_offset 29, -80
	.cfi_offset 30, -72
	mov	x29, sp
	stp	x21, x22, [sp, 32]
	.cfi_offset 21, -48
	.cfi_offset 22, -40
	adrp	x22, .LC4
	add	x22, x22, :lo12:.LC4
	stp	x19, x20, [sp, 16]
	.cfi_offset 19, -64
	.cfi_offset 20, -56
	mov	x19, x0
	mov	w20, w1
	mov	x0, x22
	adrp	x1, .LC3
	add	x1, x1, :lo12:.LC3
	str	x23, [sp, 48]
	.cfi_offset 23, -32
	str	wzr, [sp, 72]
	bl	fopen
	cbz	x0, .L64
	mov	x21, x0
	mov	x3, x0
	mov	x2, 4
	add	x0, sp, 72
	mov	x1, 1
	bl	fread
	cmp	x0, 4
	bne	.L48
	ldrb	w1, [sp, 74]
	mov	x0, x21
	lsr	w21, w1, 4
	bl	fclose
	cmp	w21, 2
	bhi	.L49
	cmp	w21, 0
	mov	w22, 1059061760
	mov	w0, 538968064
	csel	w22, w22, w0, ne
.L50:
	mov	w1, 4098
	adrp	x0, .LC8
	movk	w1, 0x10, lsl 16
	add	x0, x0, :lo12:.LC8
	bl	open
	mov	w21, w0
	tbnz	w0, #31, .L65
	mov	x1, 4096
	sxtw	x5, w22
	mov	w4, w0
	mov	w3, 1
	mov	w2, 3
	adrp	x22, .LANCHOR1
	mov	x0, 0
	add	x23, x22, :lo12:.LANCHOR1
	bl	mmap
	mov	x1, x0
	mov	w0, w21
	str	x1, [x23, 16]
	bl	close
	ldr	x21, [x23, 16]
	cmn	x21, #1
	beq	.L66
	str	x21, [x22, #:lo12:.LANCHOR1]
	mov	x4, x19
	add	x8, x19, w20, sxtw 2
	cmp	w20, 0
	ble	.L56
	mov	w10, 26215
	mov	w9, 7
	movk	w10, 0x6666, lsl 16
	mov	w5, 1
	.p2align 3,,7
.L55:
	ldr	w0, [x4], 4
	lsl	w6, w5, w0
	smull	x2, w0, w10
	asr	x2, x2, 34
	sub	w2, w2, w0, asr 31
	add	w3, w2, w2, lsl 2
	sxtw	x2, w2
	sub	w3, w0, w3, lsl 1
	ldr	w7, [x21, x2, lsl 2]
	add	w3, w3, w3, lsl 1
	lsl	w0, w9, w3
	bic	w0, w7, w0
	str	w0, [x21, x2, lsl 2]
	lsl	w3, w5, w3
	ldr	w0, [x21, x2, lsl 2]
	orr	w3, w3, w0
	str	w3, [x21, x2, lsl 2]
	str	w6, [x21, 40]
	cmp	x8, x4
	bne	.L55
.L56:
	ldp	x21, x22, [sp, 32]
	mov	w1, w20
	ldr	x23, [sp, 48]
	mov	x0, x19
	ldp	x19, x20, [sp, 16]
	mov	w4, 5000
	ldp	x29, x30, [sp], 80
	.cfi_remember_state
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 23
	.cfi_restore 21
	.cfi_restore 22
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	mov	w3, 0
	mov	w2, 0
	b	dshotRepeatSendCommand
	.p2align 2,,3
.L49:
	.cfi_restore_state
	cmp	w21, 3
	bne	.L47
	mov	w22, -31457280
	b	.L50
	.p2align 2,,3
.L48:
	adrp	x0, .LC6
	add	x0, x0, :lo12:.LC6
	bl	puts
	mov	x0, x21
	bl	fclose
.L47:
	adrp	x0, .LC7
	mov	w22, -31457280
	add	x0, x0, :lo12:.LC7
	bl	puts
	b	.L50
.L64:
	mov	x1, x22
	adrp	x0, .LC5
	add	x0, x0, :lo12:.LC5
	bl	printf
	b	.L47
.L66:
	mov	x1, x21
	adrp	x0, .LC10
	add	x0, x0, :lo12:.LC10
	bl	printf
	mov	w0, w21
	bl	exit
.L65:
	adrp	x0, .LC9
	add	x0, x0, :lo12:.LC9
	bl	puts
	mov	w0, -1
	bl	exit
	.cfi_endproc
.LFE31:
	.size	motorImplementationInitialize, .-motorImplementationInitialize
	.align	2
	.p2align 4,,11
	.global	motorImplementationFinalize
	.type	motorImplementationFinalize, %function
motorImplementationFinalize:
.LFB32:
	.cfi_startproc
	adrp	x0, .LANCHOR1+16
	mov	x1, 4096
	ldr	x0, [x0, #:lo12:.LANCHOR1+16]
	b	munmap
	.cfi_endproc
.LFE32:
	.size	motorImplementationFinalize, .-motorImplementationFinalize
	.align	2
	.p2align 4,,11
	.global	motorImplementationSendThrottles
	.type	motorImplementationSendThrottles, %function
motorImplementationSendThrottles:
.LFB33:
	.cfi_startproc
	stp	x29, x30, [sp, -128]!
	.cfi_def_cfa_offset 128
	.cfi_offset 29, -128
	.cfi_offset 30, -120
	mov	x29, sp
	cmp	w1, 26
	bgt	.L69
	cmp	w1, 0
	ble	.L84
	adrp	x3, .LANCHOR1+8
	mov	x5, 65970697666560
	movk	x5, 0x409f, lsl 48
	mov	x4, x2
	ldr	w8, [x3, #:lo12:.LANCHOR1+8]
	mov	x3, 4631952216750555136
	add	x2, sp, 16
	fmov	d2, x5
	fmov	d1, x3
	mov	x5, 105553116266496
	mov	x3, 61572651155456
	movk	x5, 0x4090, lsl 48
	movk	x3, 0x408f, lsl 48
	add	x9, x4, w1, sxtw 3
	mov	x7, x2
	fmov	d3, x3
	fmov	d4, x5
	b	.L76
	.p2align 2,,3
.L85:
	fcmpe	d0, #0.0
	bge	.L79
	ldr	d0, [x4]
	fmsub	d0, d0, d3, d1
	fcvtzs	w6, d0
.L74:
	sub	w5, w6, #48
	mov	w3, 0
	cmp	w5, 1999
	bhi	.L75
	sbfx	x3, x6, 3, 28
	sbfx	x5, x6, 7, 24
	eor	w3, w3, w5
	eor	w3, w3, w6, lsl 1
	and	w3, w3, 15
	orr	w3, w3, w6, lsl 5
.L75:
	add	x4, x4, 8
	str	w3, [x7], 4
	cmp	x9, x4
	beq	.L77
.L76:
	ldr	d0, [x4]
	cbnz	w8, .L85
	fmadd	d0, d0, d2, d1
	fcvtzs	w6, d0
	b	.L74
	.p2align 2,,3
.L79:
	ldr	d0, [x4]
	fmadd	d0, d0, d3, d4
	fcvtzs	w6, d0
	b	.L74
.L84:
	add	x2, sp, 16
.L77:
	bl	dshotSendFrames
	ldp	x29, x30, [sp], 128
	.cfi_remember_state
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa_offset 0
	ret
.L69:
	.cfi_restore_state
	adrp	x3, .LANCHOR0
	add	x3, x3, :lo12:.LANCHOR0
	adrp	x1, .LC0
	adrp	x0, .LC1
	add	x3, x3, 16
	add	x1, x1, :lo12:.LC0
	add	x0, x0, :lo12:.LC1
	mov	w2, 482
	bl	__assert_fail
	.cfi_endproc
.LFE33:
	.size	motorImplementationSendThrottles, .-motorImplementationSendThrottles
	.section	.rodata
	.align	3
	.set	.LANCHOR0,. + 0
	.type	__PRETTY_FUNCTION__.1, %object
	.size	__PRETTY_FUNCTION__.1, 16
__PRETTY_FUNCTION__.1:
	.string	"dshotSendFrames"
	.type	__PRETTY_FUNCTION__.0, %object
	.size	__PRETTY_FUNCTION__.0, 33
__PRETTY_FUNCTION__.0:
	.string	"motorImplementationSendThrottles"
	.bss
	.align	3
	.set	.LANCHOR1,. + 0
	.type	dshotGpio, %object
	.size	dshotGpio, 8
dshotGpio:
	.zero	8
	.type	dshot3dMode, %object
	.size	dshot3dMode, 4
dshot3dMode:
	.zero	4
	.zero	4
	.type	dshotGpioMap, %object
	.size	dshotGpioMap, 8
dshotGpioMap:
	.zero	8
	.ident	"GCC: (Debian 12.2.0-14) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
