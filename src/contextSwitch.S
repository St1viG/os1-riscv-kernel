.section .text
.align 4

.global contextSwitch
.type contextSwitch, @function
contextSwitch:
    sd ra, 0 * 8(a0)          # Context::ra
    sd sp, 1 * 8(a0)          # Context::sp
    sd s0,  2 * 8(a0)
    sd s1,  3 * 8(a0)
    sd s2,  4 * 8(a0)
                              # Context = {ra,sp}
    ld ra, 0 * 8(a1)
    ld sp, 1 * 8(a1)
    ld s0, 2 * 8(a1)
    ld s1, 3 * 8(a1)
    ld s2, 4 * 8(a1)

    ret