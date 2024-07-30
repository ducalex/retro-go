@---------------------------------------------------------------------------------
	.section ".init"
@---------------------------------------------------------------------------------
	.global _start
	.type _start STT_FUNC
	.align	4
	.arm

@---------------------------------------------------------------------------------
_start:
@---------------------------------------------------------------------------------
	b reset_vector 		@ 0x00 Reset
	b reserved_vector		@ 0x04 Undefined
	b swi_vector			@ 0x08 SWI
	b reserved_vector 	@ 0x0C Abort Prefetch
	b reserved_vector 	@ 0x10 Abort Data
	b reserved_vector		@ 0x14 Reserved
	b irq_vector			@ 0x18 IRQ
	b irq_vector			@ 0x1C FIQ
	
@---------------------------------------------------------------------------------
irq_vector:
@---------------------------------------------------------------------------------	
	@ Save these registers, IRQ functions will be allowed to modify them w/o
	@ saving.
	stmdb sp!, { r0 - r3, r12, lr }
	
	@ Pointer to IRQ handler is at 0x03FFFFFC (mirrored WRAM)
	mov r0, #0x04000000
	
	@ Store return address and branch to handler
	mov lr, pc
	ldr pc, [ r0, #-4 ]

	@ Return from IRQ
	ldmia sp!, { r0 - r3, r12, lr }
	subs pc, lr, #4

@---------------------------------------------------------------------------------
reset_vector: @This isn't required if not booting from bios
@---------------------------------------------------------------------------------	
	mov     r0, #0xDF
	msr     cpsr_cf, r0
	
	@Disable Interrupts IME=0
	mov     r3, #0x04000000
	strb    r3, [r3,#0x208]
	
	@Setup stacks
	bl      init
	
	mov r2, #1
	strb    r2, [r3,#0x208]
	
	ldr   r0, =DrawLogo
	ldr   lr, =swi_SoftReset
	bx r0
	
@---------------------------------------------------------------------------------
reserved_vector: @Lets just infinite loop for now
@---------------------------------------------------------------------------------	
	b reserved_vector
	
@ SWI calling convention:
@ Parameters are passed in via r0 - r3
@ Called SWI can modify r0 - r3 (and return things here), r12, and r14.
@ They can't modify anything else.
@---------------------------------------------------------------------------------
swi_vector:
@---------------------------------------------------------------------------------	
	@ Save these as temporaries
	stmdb sp!, { r11, r12, lr }
	
	@ Load comment from SWI instruction, which indicates which SWI
	@ to use.
	ldrb r12, [lr,#-2]
	adr r11, swi_branch_table
	ldr r12, [r11,r12,lsl#2]
	
	@ get SPSR and enter system mode, interrupts on
	MRS R11, SPSR
	@ This must be stacked and not just saved, because otherwise SWI won't
	@ be reentrant, which can happen if you're waiting for interrupts and the
	@ interrupt handler triggers the SWI.
	stmfd sp!, {r11}
	
	@ Set up new CPSR value
	and r11, r11, #0x80
	orr r11, r11, #0x1f
	msr cpsr_cf, r11
	
	@ We have to now save system-mode lr register as well
	stmfd sp!, {r2, r3, lr}
	
	@ Set return address
	adr lr, swi_complete
	@ Branch to SWI handler
	bx r12
	
swi_complete:
	@ Restore system mode lr
	ldmfd sp!, {r2, r3, lr}
	
	@ Go back to supervisor mode to get back to that stack
	mov r12, #0xD3
	msr cpsr_cf, r12
	
	@ SPSR has to be restored because the transition to system mode broke it
	ldmfd sp!, {r11}
	msr spsr_cf, r11
	
	@ Restore stuff we saved
	ldmfd sp!, {r11,r12,lr}
	
	@ Return from exception handler
	movs pc, lr

@---------------------------------------------------------------------------------
swi_branch_table:
@---------------------------------------------------------------------------------	
	.word swi_SoftReset					@ 0x00_SoftReset
	.word swi_RegisterRamReset			@ 0x01_RegisterRAMReset
	.word swi_Halt						@ 0x02_Halt
	.word swi_Stop						@ 0x03_Stop
	.word swi_IntrWait					@ 0x04_IntrWait
	.word swi_VBlankIntrWait				@ 0x05_VBlankIntrWait
	.word swi_Div						@ 0x06_Div
	.word swi_DivARM					@ 0x07_DivARM
	.word swi_Sqrt						@ 0x08_Sqrt
	.word swi_ArcTan						@ 0x09_ArcTan
	.word swi_ArcTan2					@ 0x0A_ArcTan2
	.word swi_CpuSet					@ 0x0B_CPUSet
	.word swi_CpuFastSet					@ 0x0C_CPUFastSet
	.word swi_GetBiosChecksum			@ 0x0D_GetBiosChecksum
	.word swi_BgAffineSet					@ 0x0E_BgAffineSet
	.word swi_ObjAffineSet				@ 0x0F_ObjAffineSet
	.word swi_BitUnPack					@ 0x10_BitUnPack
	.word swi_LZ77UnCompWram			@ 0x11_LZ77UnCompWram
	.word swi_LZ77UnCompVram			@ 0x12_LZ77UnCompVram
	.word swi_HuffUnComp				@ 0x13_HuffUnComp
	.word swi_RLUnCompWram			@ 0x14_RLUnCompWram
	.word swi_RLUnCompVram			@ 0x15_RLUnCompVram
	.word swi_Diff8bitUnFilterWram			@ 0x16_Diff8bitUnFilterWram
	.word swi_Diff8bitUnFilterVram			@ 0x17_Diff8bitUnFilterVram
	.word swi_Diff16bitUnFilter				@ 0x18_Diff16bitUnFilter
	.word swi_Invalid						@ 0x19_SoundBiasChange
	.word swi_Invalid						@ 0x1A_SoundDriverInit
	.word swi_Invalid						@ 0x1B_SoundDriverMode
	.word swi_Invalid						@ 0x1C_SoundDriverMain
@	.word swi_SoundDriverMain				@ 0x1C_SoundDriverMain
	.word swi_Invalid						@ 0x1D_SoundDriverVSync
	.word swi_Invalid						@ 0x1E_SoundChannelClear
	.word swi_MidiKey2Freq				@ 0x1F_MidiKey2Freq
	.word swi_Invalid						@ 0x20_MusicPlayerOpen
	.word swi_Invalid						@ 0x21_MusicPlayerStart
	.word swi_Invalid						@ 0x22_MusicPlayerStop
	.word swi_MusicPlayerContinue			@ 0x23_MusicPlayerContinue
	.word swi_MusicPlayerFadeOut			@ 0x24_MusicPlayerFadeOut
	.word swi_Invalid						@ 0x25_MultiBoot
	.word swi_Invalid						@ 0x26_HardReset
	.word swi_CustomHalt					@ 0x27_CustomHalt
	.word swi_Invalid						@ 0x28_SoundDriverVSyncOff
	.word swi_Invalid						@ 0x29_SoundDriverVSyncOn
	.word swi_SoundGetJumpList			@ 0x2A_SoundGetJumpList
	
@---------------------------------------------------------------------------------
.global swi_Invalid
.type swi_Invalid STT_FUNC
swi_Invalid:
@---------------------------------------------------------------------------------
	@ Infinite loop for now
	@b swi_Invalid
	
	@ Do nothing for release builds
	bx lr
@---------------------------------------------------------------------------------