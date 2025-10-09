.data
var1:   .quad 3
var2:   .quad 4

    .text
    .global _start

_start:
    adrp x0, var1@page
    add  x0, x0, var1@pageoff

    adrp x1, var2@page
    add  x1, x1, var2@pageoff

    ldr x2, [x0]

    str x2, [x1, #8]

    str x2, [x1, #8]!

    ldr x3, [x1], #8

    mov x0, #0
    mov x16, #1
    svc #0
