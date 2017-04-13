/* 2015-05-04 woosik.youm@samsung.com */

#include <linux/linkage.h>
#include <linux/const.h>
#include <asm/assembler.h>
#include <asm/page.h>

#define CACHE_LINE_SIZE 64

#define result		x0
#define src		x1
#define data1		x2
#define data2		x3
#define data3		x4
#define data4		x5
#define data5		x6
#define data6		x7
#define data7		x8
#define data8		x9
#define temp          x9

ENTRY(calc_pagesum)

	prfm	pldl1strm, [src, #(CACHE_LINE_SIZE * 1)]
	prfm	pldl1strm, [src, #(CACHE_LINE_SIZE * 2)]
        mov         src, x0
        ldp     data1, data2, [src], #16
        ldp     data3, data4, [src], #16
        ldp     data5, data6, [src], #16
        ldp     data7, data8, [src], #16

        adds    result, data1, data2
        adcs    result, result, data3
        adcs    result, result, data4
        adcs    result, result, data5
        adcs    result, result, data6
        adcs    result, result, data7
        adcs    result, result, data8

.loop_sum: // cache line size align
	prfm  pldl1strm, [src, #(CACHE_LINE_SIZE * 2)]

        ldp     data1, data2, [src], #16
        ldp     data3, data4, [src], #16
        ldp     data5, data6, [src], #16
        ldp     data7, data8, [src], #16

        adcs    result, data1, data2
        adcs    result, result, data3
        adcs    result, result, data4
        adcs    result, result, data5
        adcs    result, result, data6
        adcs    result, result, data7
        adcs    result, result, data8
        and     temp, src, #(PAGE_SIZE-1)
        cbnz    temp, .loop_sum
        ret

END(calc_pagesum)





