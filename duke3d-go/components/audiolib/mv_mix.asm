        IDEAL

        p386
        MODEL  flat

        dataseg
        CODESEG

        MASM
        ALIGN 4

EXTRN   _MV_HarshClipTable:DWORD
EXTRN   _MV_MixDestination:DWORD
EXTRN   _MV_MixPosition:DWORD
EXTRN   _MV_LeftVolume:DWORD
EXTRN   _MV_RightVolume:DWORD
EXTRN   _MV_SampleSize:DWORD
EXTRN   _MV_RightChannelOffset:DWORD

;================
;
; MV_Mix8BitMono
;
;================

; eax - position
; edx - rate
; ebx - start
; ecx - number of samples to mix

PROC    MV_Mix8BitMono_
PUBLIC  MV_Mix8BitMono_
; Two at once
        pushad

        mov     ebp, eax

        mov     esi, ebx                        ; Source pointer

        ; Sample size
        mov     ebx, _MV_SampleSize
        mov     eax,OFFSET apatch7+2            ; convice tasm to modify code...
        mov     [eax],bl
        mov     eax,OFFSET apatch8+2            ; convice tasm to modify code...
        mov     [eax],bl
        mov     eax,OFFSET apatch9+3            ; convice tasm to modify code...
        mov     [eax],bl

        ; Volume table ptr
        mov     ebx, _MV_LeftVolume             ; Since we're mono, use left volume
        mov     eax,OFFSET apatch1+4            ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET apatch2+4            ; convice tasm to modify code...
        mov     [eax],ebx

        ; Harsh Clip table ptr
        mov     ebx, _MV_HarshClipTable
        add     ebx, 128
        mov     eax,OFFSET apatch3+2            ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET apatch4+2            ; convice tasm to modify code...
        mov     [eax],ebx

        ; Rate scale ptr
        mov     eax,OFFSET apatch5+2            ; convice tasm to modify code...
        mov     [eax],edx
        mov     eax,OFFSET apatch6+2            ; convice tasm to modify code...
        mov     [eax],edx

        mov     edi, _MV_MixDestination         ; Get the position to write to

        ; Number of samples to mix
        shr     ecx, 1                          ; double sample count
        cmp     ecx, 0
        je      short exit8m

;     eax - scratch
;     ebx - scratch
;     edx - scratch
;     ecx - count
;     edi - destination
;     esi - source
;     ebp - frac pointer
; apatch1 - volume table
; apatch2 - volume table
; apatch3 - harsh clip table
; apatch4 - harsh clip table
; apatch5 - sample rate
; apatch6 - sample rate

        mov     eax,ebp                         ; begin calculating first sample
        add     ebp,edx                         ; advance frac pointer
        shr     eax,16                          ; finish calculation for first sample

        mov     ebx,ebp                         ; begin calculating second sample
        add     ebp,edx                         ; advance frac pointer
        shr     ebx,16                          ; finish calculation for second sample

        movzx   eax, byte ptr [esi+eax]         ; get first sample
        movzx   ebx, byte ptr [esi+ebx]         ; get second sample

        ALIGN 4
mix8Mloop:
        movzx   edx, byte ptr [edi]             ; get current sample from destination
apatch1:
        movsx   eax, byte ptr [2*eax+12345678h] ; volume translate first sample
apatch2:
        movsx   ebx, byte ptr [2*ebx+12345678h] ; volume translate second sample
        add     eax, edx                        ; mix first sample
apatch9:
        movzx   edx, byte ptr [edi + 1]         ; get current sample from destination
apatch3:
        mov     eax, [eax + 12345678h]          ; harsh clip new sample
        add     ebx, edx                        ; mix second sample
        mov     [edi], al                       ; write new sample to destination
        mov     edx, ebp                        ; begin calculating third sample
apatch4:
        mov     ebx, [ebx + 12345678h]          ; harsh clip new sample
apatch5:
        add     ebp,12345678h                   ; advance frac pointer
        shr     edx, 16                         ; finish calculation for third sample
        mov     eax, ebp                        ; begin calculating fourth sample
apatch7:
        add     edi, 1                          ; move destination to second sample
        shr     eax, 16                         ; finish calculation for fourth sample
        mov     [edi], bl                       ; write new sample to destination
apatch6:
        add     ebp,12345678h                   ; advance frac pointer
        movzx   ebx, byte ptr [esi+eax]         ; get fourth sample
        movzx   eax, byte ptr [esi+edx]         ; get third sample
apatch8:
        add     edi, 2                          ; move destination to third sample
        dec     ecx                             ; decrement count
        jnz     mix8Mloop                       ; loop

        mov     _MV_MixDestination, edi         ; Store the current write position
        mov     _MV_MixPosition, ebp            ; return position
exit8m:
        popad
        ret
ENDP    MV_Mix8BitMono_

;================
;
; MV_Mix8BitStereo
;
;================

; eax - position
; edx - rate
; ebx - start
; ecx - number of samples to mix

PROC    MV_Mix8BitStereo_
PUBLIC  MV_Mix8BitStereo_

        pushad
        mov     ebp, eax

        mov     esi, ebx                        ; Source pointer

        ; Sample size
        mov     ebx, _MV_SampleSize
        mov     eax,OFFSET bpatch8+2            ; convice tasm to modify code...
        mov     [eax],bl

        ; Right channel offset
        mov     ebx, _MV_RightChannelOffset
        mov     eax,OFFSET bpatch6+3            ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET bpatch7+2            ; convice tasm to modify code...
        mov     [eax],ebx

        ; Volume table ptr
        mov     ebx, _MV_LeftVolume
        mov     eax,OFFSET bpatch1+4            ; convice tasm to modify code...
        mov     [eax],ebx

        mov     ebx, _MV_RightVolume
        mov     eax,OFFSET bpatch2+4            ; convice tasm to modify code...
        mov     [eax],ebx

        ; Rate scale ptr
        mov     eax,OFFSET bpatch3+2            ; convice tasm to modify code...
        mov     [eax],edx

        ; Harsh Clip table ptr
        mov     ebx, _MV_HarshClipTable
        add     ebx,128
        mov     eax,OFFSET bpatch4+2            ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET bpatch5+2            ; convice tasm to modify code...
        mov     [eax],ebx

        mov     edi, _MV_MixDestination         ; Get the position to write to

        ; Number of samples to mix
        cmp     ecx, 0
        je      short exit8S

;     eax - scratch
;     ebx - scratch
;     edx - scratch
;     ecx - count
;     edi - destination
;     esi - source
;     ebp - frac pointer
; bpatch1 - left volume table
; bpatch2 - right volume table
; bpatch3 - sample rate
; bpatch4 - harsh clip table
; bpatch5 - harsh clip table

        mov     eax,ebp                         ; begin calculating first sample
        shr     eax,16                          ; finish calculation for first sample

        movzx   ebx, byte ptr [esi+eax]         ; get first sample

        ALIGN 4
mix8Sloop:
bpatch1:
        movsx   eax, byte ptr [2*ebx+12345678h] ; volume translate left sample
        movzx   edx, byte ptr [edi]             ; get current sample from destination
bpatch2:
        movsx   ebx, byte ptr [2*ebx+12345678h] ; volume translate right sample
        add     eax, edx                        ; mix left sample
bpatch3:
        add     ebp,12345678h                   ; advance frac pointer
bpatch6:
        movzx   edx, byte ptr [edi+12345678h]   ; get current sample from destination
bpatch4:
        mov     eax, [eax + 12345678h]          ; harsh clip left sample
        add     ebx, edx                        ; mix right sample
        mov     [edi], al                       ; write left sample to destination
bpatch5:
        mov     ebx, [ebx + 12345678h]          ; harsh clip right sample
        mov     edx, ebp                        ; begin calculating second sample
bpatch7:
        mov     [edi+12345678h], bl             ; write right sample to destination
        shr     edx, 16                         ; finish calculation for second sample
bpatch8:
        add     edi, 2                          ; move destination to second sample
        movzx   ebx, byte ptr [esi+edx]         ; get second sample
        dec     ecx                             ; decrement count
        jnz     mix8Sloop                       ; loop

        mov     _MV_MixDestination, edi         ; Store the current write position
        mov     _MV_MixPosition, ebp            ; return position

EXIT8S:
        popad
        ret
ENDP    MV_Mix8BitStereo_

;================
;
; MV_Mix16BitMono
;
;================

; eax - position
; edx - rate
; ebx - start
; ecx - number of samples to mix

PROC    MV_Mix16BitMono_
PUBLIC  MV_Mix16BitMono_
; Two at once
        pushad

        mov     ebp, eax

        mov     esi, ebx                        ; Source pointer

        ; Sample size
        mov     ebx, _MV_SampleSize
        mov     eax,OFFSET cpatch5+3            ; convice tasm to modify code...
        mov     [eax],bl
        mov     eax,OFFSET cpatch6+3            ; convice tasm to modify code...
        mov     [eax],bl
        mov     eax,OFFSET cpatch7+2            ; convice tasm to modify code...
        add     bl,bl
        mov     [eax],bl

        ; Volume table ptr
        mov     ebx, _MV_LeftVolume
        mov     eax,OFFSET cpatch1+4            ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET cpatch2+4            ; convice tasm to modify code...
        mov     [eax],ebx

        ; Rate scale ptr
        mov     eax,OFFSET cpatch3+2            ; convice tasm to modify code...
        mov     [eax],edx
        mov     eax,OFFSET cpatch4+2            ; convice tasm to modify code...
        mov     [eax],edx

        mov     edi, _MV_MixDestination         ; Get the position to write to

        ; Number of samples to mix
        shr     ecx, 1                          ; double sample count
        cmp     ecx, 0
        je      exit16M

;     eax - scratch
;     ebx - scratch
;     edx - scratch
;     ecx - count
;     edi - destination
;     esi - source
;     ebp - frac pointer
; cpatch1 - volume table
; cpatch2 - volume table
; cpatch3 - sample rate
; cpatch4 - sample rate

        mov     eax,ebp                         ; begin calculating first sample
        add     ebp,edx                         ; advance frac pointer
        shr     eax,16                          ; finish calculation for first sample

        mov     ebx,ebp                         ; begin calculating second sample
        add     ebp,edx                         ; advance frac pointer
        shr     ebx,16                          ; finish calculation for second sample

        movzx   eax, byte ptr [esi+eax]         ; get first sample
        movzx   ebx, byte ptr [esi+ebx]         ; get second sample

        ALIGN 4
mix16Mloop:
        movsx   edx, word ptr [edi]             ; get current sample from destination
cpatch1:
        movsx   eax, word ptr [2*eax+12345678h] ; volume translate first sample
cpatch2:
        movsx   ebx, word ptr [2*ebx+12345678h] ; volume translate second sample
        add     eax, edx                        ; mix first sample
cpatch5:
        movsx   edx, word ptr [edi + 2]         ; get current sample from destination

        cmp     eax, -32768                     ; Harsh clip sample
        jge     short m16skip1
        mov     eax, -32768
        jmp     short m16skip2
m16skip1:
        cmp     eax, 32767
        jle     short m16skip2
        mov     eax, 32767
m16skip2:
        add     ebx, edx                        ; mix second sample
        mov     [edi], ax                       ; write new sample to destination
        mov     edx, ebp                        ; begin calculating third sample

        cmp     ebx, -32768                     ; Harsh clip sample
        jge     short m16skip3
        mov     ebx, -32768
        jmp     short m16skip4
m16skip3:
        cmp     ebx, 32767
        jle     short m16skip4
        mov     ebx, 32767
m16skip4:
cpatch3:
        add     ebp,12345678h                   ; advance frac pointer
        shr     edx, 16                         ; finish calculation for third sample
        mov     eax, ebp                        ; begin calculating fourth sample
cpatch6:
        mov     [edi + 2], bx                   ; write new sample to destination
        shr     eax, 16                         ; finish calculation for fourth sample

cpatch4:
        add     ebp,12345678h                   ; advance frac pointer
        movzx   ebx, byte ptr [esi+eax]         ; get fourth sample
cpatch7:
        add     edi, 4                          ; move destination to third sample
        movzx   eax, byte ptr [esi+edx]         ; get third sample
        dec     ecx                             ; decrement count
        jnz     mix16Mloop                      ; loop

        mov     _MV_MixDestination, edi         ; Store the current write position
        mov     _MV_MixPosition, ebp            ; return position
EXIT16M:
        popad
        ret
ENDP    MV_Mix16BitMono_

;================
;
; MV_Mix16BitStereo
;
;================

; eax - position
; edx - rate
; ebx - start
; ecx - number of samples to mix

PROC    MV_Mix16BitStereo_
PUBLIC  MV_Mix16BitStereo_

        pushad
        mov     ebp, eax

        mov     esi, ebx                        ; Source pointer

        ; Sample size
        mov     ebx, _MV_SampleSize
        mov     eax,OFFSET dpatch6+2            ; convice tasm to modify code...
        mov     [eax],bl

        ; Right channel offset
        mov     ebx, _MV_RightChannelOffset
        mov     eax,OFFSET dpatch4+3            ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET dpatch5+3            ; convice tasm to modify code...
        mov     [eax],ebx

        ; Volume table ptr
        mov     ebx, _MV_LeftVolume
        mov     eax,OFFSET dpatch1+4            ; convice tasm to modify code...
        mov     [eax],ebx

        mov     ebx, _MV_RightVolume
        mov     eax,OFFSET dpatch2+4            ; convice tasm to modify code...
        mov     [eax],ebx

        ; Rate scale ptr
        mov     eax,OFFSET dpatch3+2            ; convice tasm to modify code...
        mov     [eax],edx

        mov     edi, _MV_MixDestination         ; Get the position to write to

        ; Number of samples to mix
        cmp     ecx, 0
        je      exit16S

;     eax - scratch
;     ebx - scratch
;     edx - scratch
;     ecx - count
;     edi - destination
;     esi - source
;     ebp - frac pointer
; dpatch1 - left volume table
; dpatch2 - right volume table
; dpatch3 - sample rate

        mov     eax,ebp                         ; begin calculating first sample
        shr     eax,16                          ; finish calculation for first sample

        movzx   ebx, byte ptr [esi+eax]         ; get first sample

        ALIGN 4
mix16Sloop:
dpatch1:
        movsx   eax, word ptr [2*ebx+12345678h] ; volume translate left sample
        movsx   edx, word ptr [edi]             ; get current sample from destination
dpatch2:
        movsx   ebx, word ptr [2*ebx+12345678h] ; volume translate right sample
        add     eax, edx                        ; mix left sample
dpatch3:
        add     ebp,12345678h                   ; advance frac pointer
dpatch4:
        movsx   edx, word ptr [edi+12345678h]   ; get current sample from destination

        cmp     eax, -32768                     ; Harsh clip sample
        jge     short s16skip1
        mov     eax, -32768
        jmp     short s16skip2
s16skip1:
        cmp     eax, 32767
        jle     short s16skip2
        mov     eax, 32767
s16skip2:
        add     ebx, edx                        ; mix right sample
        mov     [edi], ax                       ; write left sample to destination

        cmp     ebx, -32768                     ; Harsh clip sample
        jge     short s16skip3
        mov     ebx, -32768
        jmp     short s16skip4
s16skip3:
        cmp     ebx, 32767
        jle     short s16skip4
        mov     ebx, 32767
s16skip4:

        mov     edx, ebp                        ; begin calculating second sample
dpatch5:
        mov     [edi+12345678h], bx             ; write right sample to destination
        shr     edx, 16                         ; finish calculation for second sample
dpatch6:
        add     edi, 4                          ; move destination to second sample
        movzx   ebx, byte ptr [esi+edx]         ; get second sample
        dec     ecx                             ; decrement count
        jnz     mix16Sloop                      ; loop

        mov     _MV_MixDestination, edi         ; Store the current write position
        mov     _MV_MixPosition, ebp            ; return position
exit16S:
        popad
        ret
ENDP    MV_Mix16BitStereo_

        ENDS

        END
