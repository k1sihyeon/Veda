    .global     main
    .text
    .align      2

main:
    str     x30, [sp, -16]!
    
    ldr     x0, =fmt            // load address of fmt
    ldr     x1, =q              // load address of q
    ldr     x2, [x1]            // load value at q
    bl      printf              // call printf

    ldr     x0, =fmt            // load address of fmt
    ldr     x1, q               // load value of q
    ldr     x2, [x1]            // CRASH !!
    bl      printf

    ldr     x30, [sp], 16
    mov     w0, wzr
    ret

    .data
q:      .quad   0x1122334455667788
fmt:    .asciz  "address: %p value: %lx\n"

    .end
