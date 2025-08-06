
.arm
@---------------------------------------------------------------------------------
.global swi_SoftReset
.type swi_SoftReset STT_FUNC
swi_SoftReset:
@---------------------------------------------------------------------------------
	mov     r3, #0x04000000
	@ Read flag from 0x3007FFA
	@ 00h=8000000h (ROM), or 01h-FFh=2000000h (RAM)
	@ This must be done before init because it gets cleared in init
	ldrb    r2, [r3,#-6]
	bl      init
	cmp     r2, #0
	ldmdb   r3, {r0-r12}
	movne   lr, #0x02000000
	moveq   lr, #0x08000000
	mov     r0, #0x1F
	msr     cpsr_cf, r0
	mov     r0, #0
	bx      lr

@---------------------------------------------------------------------------------
.global init
init:
@ Requires r3 to be set to 0x04000000 already.
@---------------------------------------------------------------------------------
	@ Reset the stack locations
    mov     r0, #0xD3
    msr     cpsr_cf, r0
    ldr     sp, Cpu_Stack_SVC
    mov     lr, #0
    msr     spsr_cf, lr
    mov     r0, #0xD2
    msr     cpsr_cf, r0
    ldr     sp, Cpu_Stack_IRQ
    mov     lr, #0
    msr     spsr_cf, lr
    mov     r0, #0x5F
    msr     cpsr_cf, r0
    ldr     sp, Cpu_Stack_USR
	
	movs    r0, #0
    subs    r1, r0, #0x200
	
	@ Clear top 0x200 bytes of IWRAM 03007E00 -> 03008000

init_loop:
    str     r0, [r3,r1]
    adds    r1, r1, #4
    blt     init_loop
	
    bx      lr

@---------------------------------------------------------------------------------
@---------------------------------------------------------------------------------

Cpu_Stack_USR: .word 0x03007F00
Cpu_Stack_IRQ: .word 0x03007FA0
Cpu_Stack_SVC: .word 0x03007FE0

@---------------------------------------------------------------------------------