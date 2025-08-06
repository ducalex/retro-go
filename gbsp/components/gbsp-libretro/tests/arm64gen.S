
b 16*4
bl 16*4

b.eq 16*4
b.ne 16*4
b.hs 16*4
b.lo 16*4
b.mi 16*4
b.pl 16*4
b.vs 16*4
b.vc 16*4
b.hi 16*4
b.ls 16*4
b.ge 16*4
b.lt 16*4
b.gt 16*4
b.le 16*4
b.al 16*4
b.nv 16*4

ldr w1,  [x2,  #64]
ldr w29, [x30, #64]
str w1,  [x2,  #64]
str w29, [x30, #64]

mov w0,  #0x1234
mov w12, #0x5656
mov w12, #0xFFFF

movk w13, #0x9876, lsl #16
movk w13, #0xFFFF, lsl #16

movz w13, #0xabcd, lsl #16

mov w14, #0xffff5555

add w11, w12, w13, lsl #0
add w11, w12, w13, lsl #19
add w11, w12, w13, lsl #31

add w1, w29, #0x123
add w1, w29, #0xFFF
sub w1, w29, #0x123
sub w1, w29, #0xFFF

add w3, w30, #0x123000
add w3, w30, #0xFFF000
sub w3, w30, #0x123000
sub w3, w30, #0xFFF000

adds w29, w30, #0x123
adds w29, w30, #0xFFF
subs w29, w30, #0x123
subs w29, w30, #0xFFF

madd w2, w3, w4, w5
madd w25, w26, w27, w28
msub w2, w3, w4, w5
msub w25, w26, w27, w28

smaddl x2, w3, w4, x5
smaddl x25, w26, w27, x28
umaddl x2, w3, w4, x5
umaddl x25, w26, w27, x28

mul w1, w2, w3
mul w27, w28, w29

ror w1, w2, #1
ror w1, w2, #31
ror w30, w29, #1
ror w30, w29, #31

lsr w1, w2, #1
lsr w1, w2, #31
lsr w30, w29, #1
lsr w30, w29, #31

lsl w1, w2, #1
lsl w1, w2, #31
lsl w30, w29, #1
lsl w30, w29, #31

asr w1, w2, #1
asr w1, w2, #31
asr w30, w29, #1
asr w30, w29, #31

lsr x1, x2, #1
lsr x1, x2, #2
lsr x1, x2, #62
lsr x1, x2, #63
lsr x30, x29, #1
lsr x30, x29, #62

eor w3, w4, #1
eor w3, w4, #(~1)
orr w3, w4, #1
orr w3, w4, #(~1)
and w3, w4, #1
and w3, w4, #(~3)

and x3, x4, #0xffffffff
and x3, x4, #0x1
and x1, x2, #1
and x1, x2, #(~1)
and x1, x2, #0xffffffff

mov w1, w2
mov w30, wzr

orr w1, w2, w3
orr w29, w30, wzr
eor w1, w2, w3
eor w29, w30, wzr
orn w1, w2, w3
orn w29, w30, wzr
and w1, w2, w3
and w29, w30, wzr
bic w1, w2, w3
bic w29, w30, wzr
ands w1, w2, w3
ands w29, w30, wzr

tst w1, w2
tst w25, wzr
cmp w1, #0
cmp w30, #0
cmp w1, #32
cmp w30, #32
cmp w1, #200
cmp w30, #200

add w1, w2, w3
add w29, w30, w28
sub w1, w2, w3
sub w29, w30, w28
adc w1, w2, w3
adc w29, w30, w28
sbc w1, w2, w3
sbc w29, w30, w28
adds w1, w2, w3
adds w29, w30, w28
subs w1, w2, w3
subs w29, w30, w28
adcs w1, w2, w3
adcs w29, w30, w28
sbcs w1, w2, w3
sbcs w29, w30, w28

tbz w20, #1, 63*4
tbnz w20, #1, 63*4
tbz w20, #0, 2*4
tbnz w20, #7, 2*4

cbz w20, 63*4
cbnz w20, 63*4
cbz w20, 2*4
cbnz w20, 2*4

csel w20, w24, w25, ne
csel w1, w2, w3, eq
csel w1, w20, wzr, lt
csel w1, wzr, wzr, gt

csinc w20, w24, w25, ne
csinc w1, w2, w3, eq
csinc w1, w20, wzr, lt
csinc w1, wzr, wzr, gt

csinv w20, w24, w25, ne
csinv w1, w2, w3, eq
csinv w1, w20, wzr, lt
csinv w1, wzr, wzr, gt

csneg w20, w24, w25, ne
csneg w1, w2, w3, eq
csneg w1, w20, wzr, lt
csneg w1, wzr, wzr, gt

cset w1, eq
cset w1, hs
cset w20, lo
csetm w1, hs
csetm w20, lo

ubfx w1, w2, #8, #8
ubfx w1, w2, #16, #16
ubfx w1, wzr, #8, #24
ubfx w1, wzr, #16, #16

rorv w1, w2, w3
rorv w28, w29, w30
lslv w1, w2, w3
lslv w28, w29, w30
lsrv w1, w2, w3
lsrv w28, w29, w30
asrv w1, w2, w3
asrv w28, w29, w30

orr	x1, x2, x3, lsl #32
orr	x25, x26, x27, lsl #32

sdiv	w1, w2, w3
sdiv	w28, w29, w30


