    .global main
    .text

main:
    stp     x29, x30, [sp, -16]!
    mov     x20, #0

loop_start:
    cmp     x20, #100
    bgt     loop_end
    
    // %3, %5 == 0 check
    mov     x21, #3
    udiv    x22, x20, x21       // x22 = x20 / x21(3)
    msub    x23, x22, x21, x20  // x20 - (x22 * x21(3))
    cmp     x23, #0             // x23(remain) != 0
    bne     check_mod_3

    mov     x21, #5
    udiv    x22, x20, x21
    msub    x23, x22, x21, x20
    cmp     x23, #0
    bne     check_mod_3
    
    // satisfy %3, %5
    // printf
    ldr     x0, =FB
    bl      printf

    // next loop
    b       loop_increment

check_mod_3:
    mov     x21, #3
    udiv    x22, x20, x21
    msub    x23, x22, x21, x20
    cmp     x23, #0
    bne     check_mod_5

    //printf
    ldr     x0, =Fizz
    bl      printf

    // next
    B       loop_increment

check_mod_5:
    mov     x21, #5
    udiv    x22, x20, x21
    msub    x23, x22, x21, x20
    cmp     x23, #0
    bne     default

    //printf
    ldr     x0, =Buzz
    bl      printf

    B       loop_increment

default:
    //printf
    ldr     x0, =Num
    mov     x1, x20
    bl      printf

    b       loop_increment

loop_increment:
    add     x20, x20, #1
    b       loop_start

loop_end:
    ldp     x29, x30, [sp], 16

    ret


    .data
Fizz:   .asciz  "Fizz\n"
Buzz:   .asciz  "Buzz\n"
FB:     .asciz  "FizzBuzz\n"
Num:    .asciz  "%d\n"

    .end
