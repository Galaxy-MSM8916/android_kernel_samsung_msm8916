#include <linux/linkage.h>
#include <asm/assembler.h>

.text
.arm
.arch	armv7-a

#define CACHE_LINE_SIZE 64

ENTRY(page_memcmp)
memcmp_ksm:
	pld	[r0, #(CACHE_LINE_SIZE * 0)]
	pld	[r1, #(CACHE_LINE_SIZE * 0)]
	pld	[r0, #(CACHE_LINE_SIZE * 1)]
	pld	[r1, #(CACHE_LINE_SIZE * 1)]

	stmfd	sp!, {r4, lr}
	mov	r3, r0

	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
	subs	r0, r4, r0
	popne	{r4, pc}
	subs	r2, r2, #64
loop:
	pld	[r3, #(CACHE_LINE_SIZE * 2)]
	pld	[r1, #(CACHE_LINE_SIZE * 2)]

	ldr	r4, [r3], #4
	ldr	r0, [r1], #4
        eors        r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4
        ldreq       r4, [r3], #4
        ldreq       r0, [r1], #4
        eoreqs     r0, r0, r4

        bne         end_1
        subs        r2, r2, #64
        bne         loop

	mov        r0, #0
        ldmfd	sp!, {r4, lr}
	bx	lr

end_1:
	sub         r3, r3, #4
        sub         r0, r0, #4
	sub        r0, r0, r3
        ldmfd	sp!, {r4, lr}
	bx	lr

END(page_memcmp)
