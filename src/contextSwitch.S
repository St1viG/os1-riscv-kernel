.section .text
.align 4

.global contextSwitch
.type contextSwitch, @function
contextSwitch:
    sd ra, 0 * 8(a0)          # Context::ra
    sd sp, 1 * 8(a0)          # Context::sp
                              # Context = {ra,sp}
    ld ra, 0 * 8(a1)
    ld sp, 1 * 8(a1)

    ret