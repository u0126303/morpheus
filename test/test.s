	.attribute	4, 16
	.attribute	5, "rv32i2p1_m2p0_a2p1_c2p0_zmmul1p0_zaamo1p0_zalrsc1p0"
	.file	"test.c"
	.option	push
	.option	arch, +a, +c, +m, +zaamo, +zalrsc, +zmmul
	.text
	.globl	test                            # -- Begin function test
	.p2align	1
	.type	test,@function
test:                                   # @test
# %bb.0:                                # %entry
	addi	sp, sp, -32
	sw	ra, 28(sp)                      # 4-byte Folded Spill
	sw	s0, 24(sp)                      # 4-byte Folded Spill
	addi	s0, sp, 32
	sw	a0, -12(s0)
	sw	a1, -16(s0)
	lw	a0, -12(s0)
	lw	a1, -16(s0)
	bge	a1, a0, .LBB0_2
	j	.LBB0_1
.LBB0_1:                                # %if.then
	lw	a0, -12(s0)
	addi	a0, a0, 7
	sw	a0, -20(s0)
	j	.LBB0_3
.LBB0_2:                                # %if.else
	lw	a0, -12(s0)
	addi	a0, a0, -9
	sw	a0, -20(s0)
	j	.LBB0_3
.LBB0_3:                                # %if.end
	lw	a0, -20(s0)
	lw	ra, 28(sp)                      # 4-byte Folded Reload
	lw	s0, 24(sp)                      # 4-byte Folded Reload
	addi	sp, sp, 32
	ret
.Lfunc_end0:
	.size	test, .Lfunc_end0-test
                                        # -- End function
	.option	pop
	.ident	"clang version 20.0.0git (git@github.com:u0126303/morpheus.git ed2b4b854f4a472a299e3e4e3acc2ad7389e88e3)"
	.section	".note.GNU-stack","",@progbits
