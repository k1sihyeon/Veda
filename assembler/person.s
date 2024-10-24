    .global FindOldestPerson
    .text   
    .align  2

// x0 has struct Person * people
// will be used for oldest_ptr as this is the return value
// w1 has int length
// w2 used for oldest_age
// x3 used for Person*
// x4 used for end_ptr
// w5 used for scratch 

FindOldestPerson:
    cbz     x0, 99f
    mov     w2, wzr
    mov     x3, x0
    mov     x0, xzr
    mov     w5, 24
    smaddl  x4, w1, w5, x3
    b       10f

1:
    ldr     w5, [x3, p.age]
    cmp     w2, w5
    csel    w2, w2, w5, gt
    csel    x0, x0, x3, gt
    add     x3, x3, 24

10:
    cmp     x3, x4
    blt     1b

99:
    ret

    .data
    .struct 0

p.fn:   .skip   8
p.ln:   .skip   8
p.age:  .skip   4
p.pad:  .skip   4

    .end
