
.set mips32r2
.set noreorder

.text

nop
nop

# Generic MIPS insts
addu $4, $5, $6
addu $29, $31, $20
subu $4, $5, $6
subu $29, $31, $20
xor $4, $5, $6
xor $29, $31, $20
and $4, $5, $6
and $29, $31, $20
or $4, $5, $6
or $29, $31, $20
nor $4, $5, $6
nor $29, $31, $20

slt $4, $5, $6
slt $29, $31, $20
sltu $4, $5, $6
sltu $29, $31, $20

sllv $4, $5, $6
sllv $29, $31, $20
srlv $4, $5, $6
srlv $29, $31, $20
srav $4, $5, $6
srav $29, $31, $20
rotrv $4, $5, $6
rotrv $29, $31, $20

sll $4, $5, 0
srl $4, $5, 0
sra $4, $5, 0
rotr $4, $5, 0
sll $4, $5, 1
srl $4, $5, 1
sra $4, $5, 1
rotr $4, $5, 1
sll $4, $5, 30
srl $4, $5, 30
sra $4, $5, 30
rotr $4, $5, 30
sll $4, $5, 31
srl $4, $5, 31
sra $4, $5, 31
rotr $4, $5, 31

lui $4, 0xFFFF
lui $4, 0x8000
lui $4, 0
lui $4, 1

addiu $4, $22, -1
xori  $4, $22, 0xFFFF
ori   $4, $22, 0xFFFF
andi  $4, $22, 0xFFFF
slti  $4, $22, -1
sltiu $4, $22, -1
addiu $4, $22, 0
xori  $4, $22, 0
ori   $4, $22, 0
andi  $4, $22, 0
slti  $4, $22, 0
sltiu $4, $22, 0
addiu $4, $22, 1
xori  $4, $22, 1
ori   $4, $22, 1
andi  $4, $22, 1
slti  $4, $22, 1
sltiu $4, $22, 1
addiu $4, $22, 0x8000
xori  $4, $22, 0x8000
ori   $4, $22, 0x8000
andi  $4, $22, 0x8000
slti  $4, $22, 0x8000
sltiu $4, $22, 0x8000
addiu $4, $22, 0x7FFF
xori  $4, $22, 0x7FFF
ori   $4, $22, 0x7FFF
andi  $4, $22, 0x7FFF
slti  $4, $22, 0x7FFF
sltiu $4, $22, 0x7FFF

mflo $7
mflo $30
mfhi $7
mfhi $30
mtlo $7
mtlo $30
mthi $7
mthi $30

mult $6, $7
mult $18, $20
multu $6, $7
multu $18, $20
div $0, $6, $7
div $0, $18, $20
divu $0, $6, $7
divu $0, $18, $20

jr $5
jr $31
jalr $5
jalr $20

bltzal $4, 1f
bltzal $20, 1f
bgezal $4, 1f
bgezal $20, 1f
bltz $4, 1f
bltz $20, 1f
1:

lb $4,  ($5)
lbu $4, ($5)
lh $4,  ($5)
lhu $4, ($5)
lw $4,  ($5)
lb $4,   1($5)
lbu $4,  1($5)
lh $4,   1($5)
lhu $4,  1($5)
lw $4,   1($5)
lb $4,  -1($5)
lbu $4, -1($5)
lh $4,  -1($5)
lhu $4, -1($5)
lw $4,  -1($5)
lb $4,   0x7FFF($5)
lbu $4,  0x7FFF($5)
lh $4,   0x7FFF($5)
lhu $4,  0x7FFF($5)
lw $4,   0x7FFF($5)
lb $4,  -0x8000($5)
lbu $4, -0x8000($5)
lh $4,  -0x8000($5)
lhu $4, -0x8000($5)
lw $4,  -0x8000($5)

sb $4, ($5)
sh $4, ($5)
sw $4, ($5)
sb $4,  1($5)
sh $4,  1($5)
sw $4,  1($5)
sb $4, -1($5)
sh $4, -1($5)
sw $4, -1($5)
sb $4,  0x7FFF($5)
sh $4,  0x7FFF($5)
sw $4,  0x7FFF($5)
sb $4, -0x8000($5)
sh $4, -0x8000($5)
sw $4, -0x8000($5)

# MIPS32r2/PSP instructions

ext $2, $5, 20, 4
ext $15, $20, 3, 9
ins $2, $5, 20, 4
ins $15, $20, 3, 9

seb $7, $9
seh $7, $9

# PSP specific stuff
# max $4, $28, $15
# min $4, $28, $15
# movz $4, $5, $6
# movn $4, $5, $6


