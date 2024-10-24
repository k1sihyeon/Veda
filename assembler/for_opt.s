    .data
str: .asciz "i : %d\n"

    .global main
    .text

main:
    stp     x29, x30, [sp, -16]!
    mov     x19, #1
    b       2f

1:
    ldr     x0, =str		// str 문자열의 주소를 x0에 로드
    mov     x1, x19		    // 증가된 숫자 x19를 출력하기 위한 x1에 저장 
    bl      printf		    // printf 함수 호출
    add     x19, x19, #1

2:
    cmp     x19, 10
    bgt     1b

    ldp     x29, x30, [sp], 16
    mov     x0, xzr
    ret

    .end
