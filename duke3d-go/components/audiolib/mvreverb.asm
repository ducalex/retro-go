        IDEAL

        p386
        MODEL  flat

        dataseg
        CODESEG

        MASM
        ALIGN 4

;================
;
; MV_16BitReverb
;
;================

; eax - source position
; edx - destination position
; ebx - Volume table
; ecx - number of samples

PROC    MV_16BitReverb_
PUBLIC  MV_16BitReverb_

        mov     esi, eax
        lea     edi, [edx - 2]

        ALIGN 4
rev16loop:
        movzx   eax, word ptr [esi]             ; get sample
        add     edi, 2

        movzx   edx, ah
        sub     ah, ah

        movsx   eax, byte ptr [2*eax+ebx+1]     ; volume translate low byte of sample
        xor     edx, 80h

        movsx   edx, word ptr [2*edx+ebx]       ; volume translate high byte of sample
        add     esi, 2

        lea     eax, [ eax + edx + 80h ]        ; mix high byte of sample
        dec     ecx                             ; decrement count

        mov     [edi], ax                       ; write new sample to destination
        jnz     rev16loop                       ; loop

        ret
ENDP    MV_16BitReverb_

;================
;
; MV_8BitReverb
;
;================

; eax - source position
; edx - destination position
; ebx - Volume table
; ecx - number of samples

PROC    MV_8BitReverb_
PUBLIC  MV_8BitReverb_

        mov     esi, eax
        lea     edi, [edx - 1]

        xor     eax, eax

        ALIGN 4
rev8loop:
;        movzx   eax, byte ptr [esi]             ; get sample
        mov     al, byte ptr [esi]              ; get sample
        inc     edi

;        movsx   eax, byte ptr [2*eax+ebx]       ; volume translate sample
        mov     al, byte ptr [2*eax+ebx]        ; volume translate sample
        inc     esi

;        add     eax, 80h
        add     al, 80h
        dec     ecx                             ; decrement count

        mov     [edi], al                       ; write new sample to destination
        jnz     rev8loop                        ; loop

        ret
ENDP    MV_8BitReverb_

;================
;
; MV_16BitReverbFast
;
;================

; eax - source position
; edx - destination position
; ebx - number of samples
; ecx - shift

PROC    MV_16BitReverbFast_
PUBLIC  MV_16BitReverbFast_

        mov     esi, eax
        mov     eax,OFFSET rpatch16+3

        mov     [eax],cl
        lea     edi, [edx - 2]

        ALIGN 4
frev16loop:
        mov     ax, word ptr [esi]             ; get sample
        add     edi, 2

rpatch16:
        sar     ax, 5    ;;;;Add 1 before shift
        add     esi, 2

        mov     [edi], ax                       ; write new sample to destination
        dec     ebx                             ; decrement count

        jnz     frev16loop                      ; loop

        ret
ENDP    MV_16BITREVERBFAST_

;================
;
; MV_8BitReverbFast
;
;================

; eax - source position
; edx - destination position
; ebx - number of samples
; ecx - shift

PROC    MV_8BitReverbFast_
PUBLIC  MV_8BitReverbFast_
        mov     esi, eax
        mov     eax,OFFSET rpatch8+2

        mov     edi, edx
        mov     edx, 80h

        mov     [eax],cl
        mov     eax, 80h

        shr     eax, cl

        dec     edi
        sub     edx, eax

        ALIGN 4
frev8loop:
        mov     al, byte ptr [esi]             ; get sample
        inc     esi

        mov     ecx, eax
        inc     edi

rpatch8:
        shr     eax, 3
        xor     ecx, 80h                        ; flip the sign bit

        shr     ecx, 7                          ; shift the sign down to 1
        add     eax, edx

        add     eax, ecx                        ; add sign bit to round to 0
        dec     ebx                             ; decrement count

        mov     [edi], al                       ; write new sample to destination
        jnz     frev8loop                       ; loop

        ret
ENDP    MV_8BITREVERBFAST_

        ENDS

        END
