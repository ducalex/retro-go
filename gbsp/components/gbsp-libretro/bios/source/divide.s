
@---------------------------------------------------------------------------------
@ Setup some nicer names
@---------------------------------------------------------------------------------
numerator .req r0
denominator .req r1
accumulator .req r2
current_bit .req r3

numerator_signed .req r12
denominator_signed .req r3

sign_flip .req r12

result .req r0
remainder .req r1
result_abs .req r3

temp .req r3

@---------------------------------------------------------------------------------
.global swi_DivARM
.type swi_DivARM STT_FUNC
swi_DivARM:
@---------------------------------------------------------------------------------
	mov     temp, numerator
	mov     numerator, denominator
	mov     denominator, temp
	b       swi_Div

@---------------------------------------------------------------------------------
.global swi_Div
.type swi_Div STT_FUNC
swi_Div:
@ See http://www.tofla.iconbar.com/tofla/arm/arm02/index.htm for more information
@---------------------------------------------------------------------------------	
	@ Set if numerator is signed, and abs numerator
	ands numerator_signed, numerator, #0x80000000
	rsbmi numerator, numerator, #0

	// Same with denominator
	ands denominator_signed, denominator, #0x80000000
	rsbmi denominator, denominator, #0

	// Gets set if sign(numerator) != sign(denominator)
	eor sign_flip, numerator_signed, denominator_signed

	mov accumulator, #0
	mov current_bit, #1

	// This moves out the current bit to the MSB of the denominator,
	// and aligns the denominator up to the same bit-length as the
	// numerator
 0:
	cmp denominator, numerator
	movls denominator, denominator, lsl #1
	movls current_bit, current_bit, lsl #1
	bls 0b

	// Basically the grade-school algorithm, for unsigned integers in binary
 1:
	cmp numerator, denominator
	subcs numerator, numerator, denominator
	orrcs accumulator, accumulator, current_bit
	movs current_bit, current_bit, lsr #1
	movcc denominator, denominator, lsr #1
	bcc 1b

	mov remainder, numerator
	mov result_abs, accumulator
	mov result, accumulator

	tst sign_flip, #0x80000000
	rsbmi result, result, #0
	
	bx lr
