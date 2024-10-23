    .data
string: .asciz "Hello World!\n"

    .text
    .global main
    .extern printf

main:
    stp     x29, x30, [sp, -16]!
    ldr     x0, =string
    bl      printf
    ldp     x29, x30, [sp], 16
    ret
