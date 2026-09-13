.extern handleTrap

.section .text
.align 4

.global trapVector
.type trapVector, @function
trapVector:
    addi sp, sp, -272

    sd x1, 1 * 8(sp)
    addi x1, sp, 272
    sd x1, 2 * 8(sp)
    .irp index, 3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index * 8(sp)
    .endr


    csrr t0, sepc
    sd t0, 32 * 8(sp)
    csrr t0, sstatus
    sd t0, 33 * 8(sp)

    mv a0, sp
    call handleTrap

    ld t0, 33 * 8(sp)
    csrw sstatus, t0
    ld t0, 32 * 8(sp)
    csrw sepc, t0

    ld x1, 1 * 8(sp)
    .irp index,  3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\index, \index * 8(sp)
    .endr

    addi sp, sp, 272
    sret
