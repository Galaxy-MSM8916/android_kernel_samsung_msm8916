/*
 * Copyright (C) 2013 ARM Ltd.
 * Copyright (C) 2013 Linaro.
 *
 * This code is based on glibc cortex strings work originally authored by Linaro
 * and re-licensed under GPLv2 for the Linux kernel. The original code can
 * be found @
 *
 * http://bazaar.launchpad.net/~linaro-toolchain-dev/cortex-strings/trunk/
 * files/head:/src/aarch64/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

/*
* compare memory areas(when two memory areas' offset are different,
* alignment handled by the hardware)
*
* Parameters:
*  x0 - const memory area 1 pointer
*  x1 - const memory area 2 pointer
*  x2 - the maximal compare byte length
* Returns:
*  x0 - a compare result, maybe less than, equal to, or greater than ZERO
*/

/* woosik.youm@samsung.com : modified for page compare */

/* Parameters and result.  */
src1	               .req   x0
src2	               .req   x1
limit	               .req   x2
result              .req   x0

/* Internal variables.  */
data1               .req   x3
data2               .req   x4
data3               .req   x5
data4               .req   x6
diff                   .req   x7
endloop           .req   x8
pos                  .req   x9

#define CACHE_LINE_SIZE 64

ENTRY(page_memcmp)

.Lloop_aligned:
	prfm	pldl1strm, [src1, #(CACHE_LINE_SIZE * 1)]
	prfm	pldl1strm, [src2, #(CACHE_LINE_SIZE * 1)]
	prfm	pldl1strm, [src1, #(CACHE_LINE_SIZE * 2)]
	prfm	pldl1strm, [src2, #(CACHE_LINE_SIZE * 2)]

        ldp     data1, data2, [src1], #16
        ldp     data3, data4, [src2], #16

        eor   diff, data1, data3
        cbnz diff, .Ldiff1
        eor   diff, data2, data4
        cbnz diff, .Ldiff2


        ldp     data1, data2, [src1], #16
        ldp     data3, data4, [src2], #16

        eor   diff, data1, data3
        cbnz diff, .Ldiff1
        eor   diff, data2, data4
        cbnz diff, .Ldiff2

        ldp     data1, data2, [src1], #16
        ldp     data3, data4, [src2], #16

        eor   diff, data1, data3
        cbnz diff, .Ldiff1
        eor   diff, data2, data4
        cbnz diff, .Ldiff2


        ldp     data1, data2, [src1], #16
        ldp     data3, data4, [src2], #16

        eor   diff, data1, data3
        cbnz diff, .Ldiff1
        eor   diff, data2, data4
        cbnz diff, .Ldiff2

        sub limit, limit, #64
	cbnz limit, .Lloop_aligned

	mov	result, #0
	ret

.Ldiff1:
CPU_LE( rev	diff, diff )
CPU_LE( rev	data1, data1 )
CPU_LE( rev	data3, data3 )
	clz	pos, diff
	lsl	data1, data1, pos
	lsl	data3, data3, pos
	lsr	data1, data1, #56
	sub	result, data1, data3, lsr #56
	ret

.Ldiff2:
CPU_LE( rev	diff, diff )
CPU_LE( rev	data2, data2 )
CPU_LE( rev	data4, data4 )
	clz	pos, diff
	lsl	data2, data2, pos
	lsl	data4, data4, pos
	lsr	data2, data2, #56
	sub	result, data2, data4, lsr #56
	ret

ENDPROC(page_memcmp)
