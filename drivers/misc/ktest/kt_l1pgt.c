/*
 *      Contains implementations of test cases for L1PGT
 *
 *      Author:    Guruprasad Ganesh   <g.ganesh@sta.samsung.com>
 *
 *      Changes:
 *              GG              :      Uploaded the initial Version
 *              AB              :      Adding Pagewalk functionalities
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/tima/tts_common.h>
#include "ktest_common.h"
#include <linux/mm.h>
#include <asm/proc-fns.h>
#include <asm/pgtable.h>
#include <linux/page-flags.h>
#include <asm/highmem.h>
#include <linux/mm_types.h>

#define TAG "TIMA_RKP "

#define LOWER_12 0xFFF

#define L1_NENTRIES (1 << 12)
#define L1_ENTRY_PAGE_TABLE_PHYSICAL_ADDRESS_MASK ~0x3FF
#define L1_ENTRY_PAGE_TABLE_FLAGS_MASK 0x3FF
#define L2_NENTRIES (1 << 8)
#define L2_ENTRY_SMALL_PAGE_PHYSICAL_ADDRESS_MASK ~0xFFF
#define L2_ENTRY_SMALL_PAGE_FLAGS_MASK 0xFFF

#define TIMA_PHYS_ADDR_ARR_LENGTH 0x100000
#define TIMA_L2_ENTRY_RO 1
#define TIMA_L2_ENTRY_RW 2



#define TTS_PTE_EXT_AP0		(_AT(pteval_t, 3) << 4)
#define PAR_TRANSLATION_FAIL 0x1

#define KERNEL_VADDR_INDEX_START 0xBF0
#define KERNEL_VADDR_INDEX_END   0xFFF
#define USER_VADDR_INDEX_START 0x000
#define USER_VADDR_INDEX_END   0xBEF
#define SECTION_FLAGS_AP2 15
#define SECTION_FLAGS_AP1 10
#define SECTION_FLAGS_AP0 11
#define PAGE_TABLE_PXN_INDEX 0x4
#define SECTION_PXN_INDEX 0x1

#define TIMA_PAGE_RO 1
#define TIMA_PAGE_RW 0
#define TIMA_PAGE_NX 0
#define TIMA_PAGE_X 1

#define RKP_LKM_CODE_SECTION 0
#define RKP_LKM_DATA_SECTION 1


#define PGD_SHIFT   20
#define PTE_SHIFT   12

#define PTE_FLAG_RO         0x210
#define PTE_FLAG_MASK       0x230
#define TTS_SECTION_ENTRY   0x11111111

#define TTS_L1_MASK         0xfff00000
#define TTS_L2_MASK         0x000ff000
#define TTS_OFFSET_MASK     0x00000fff
#define L1_FLAG_MASK        0x000ffc00
#define TTS_L2_OFFSET       0xfffff000
#define TTS_TRANSLATION_FAULT 0x0

#define TTS_L1_SHIFT 20
#define TTS_L2_SHIFT 12
# define debug_align(X) ALIGN(X, PAGE_SIZE)
#define rkp_reset_counters(c1,c2) c1=c2=0


static unsigned int base_ro_count,base_rw_count,hw_lb_fc,sw_lb_fc;
static unsigned int p1_match_count = 0,p1_no_match_count = 0;
static unsigned int rw_section,ro_section;
static unsigned int no_pxn_set = 0, no_pxn_not_set = 0;
static unsigned int code_match_count = 0,code_mismatch_count = 0,data_match_count = 0,data_mismatch_count = 0;
static unsigned int non_single_mapping,single_mapping;
static unsigned int kern_user_l1_entry_match = 0,kern_user_l1_entry_mismatch = 0;

extern unsigned long *l2_mmap_ptr;

extern char tts_err_log[TTS_MAX_RES_LEN];

static long phys_addr_array[TIMA_PHYS_ADDR_ARR_LENGTH];

static int __is_l1_page_table_entry(unsigned long x)
{
	return (((x & 0x3) == 0x1)? 1 : 0);
}
static int __is_l1_section_entry(unsigned long x)
{
	return ((( (x>>1) & 0x1) == 0x1)? 1 : 0);
}


static int is_section_ro(unsigned long flags)
{
        unsigned int ap2,ap1,ap0;

        ap2 = (flags >> SECTION_FLAGS_AP2) & 1;
        ap1 = (flags >> SECTION_FLAGS_AP1) & 1;
        ap0 = (flags >> SECTION_FLAGS_AP0) & 1;

       // printk("flags = %lx ap2 = %x ap1= %x ap0 = %x \n",flags,ap2,ap1,ap0);

        /*Todo: what if ap2 = 1 ap0=0 ap1=0 ? */
        if((ap0 == 0) && (ap1 == 0)){
                return 0;
        }
        if(ap2){
                return 1;
        }

        return 0;

}
#define TTS_TARGET_8974

#ifdef TTS_TARGET_8974
    #define SEC_TO_PGT_MEM_ADDR     0x07b00000
#endif /*TTS_TARGET_8974*/


/*tts_pa_to_va: Converting physical address to virtual */

static unsigned long  tts_pa_to_va(unsigned long pa)
{

        //        if( ( pa >= (unsigned long)SEC_TO_PGT_MEM_ADDR)&&(pa <= (unsigned long)(SEC_TO_PGT_MEM_ADDR+0x200000))) {

        if( ( (pa&0xfff00000) == (unsigned long)SEC_TO_PGT_MEM_ADDR)) {
                return (unsigned long)((unsigned long)l2_mmap_ptr+(pa&0xfffff));
        }
        else {
                return (unsigned long)__va(pa);
        }

}
/** Check if a certain va is made read-only by tima
return: -1 error, 0 writable, 1 readonly
*/


void hw_add_stats(unsigned long va)
{
        unsigned long par;

        __asm__	("mcr	p15, 0, %1, c7, c8, 1\n"
                        "isb\n"
                        "mrc 	p15, 0, %0, c7, c4, 0\n"
                        :"=r"(par):"r"(va));

        if (par & PAR_TRANSLATION_FAIL) {
                base_ro_count++;
        }
        else {
                base_rw_count++;
        }

}
unsigned long  hw_va_to_pa(unsigned long va)
{
	unsigned long  par;/* Physical Address Register*/

	__asm__	("mcr	p15, 0, %1, c7, c8, 0\n"
		"isb\n"
		"mrc 	p15, 0, %0, c7, c4, 0\n"
		:"=r"(par):"r"(va));
    
    /* Translation Fault*/
	if (par & PAR_TRANSLATION_FAIL) {
            return TTS_TRANSLATION_FAULT;
    }

    /*Calculate the physical address*/
    par = ( (par&TTS_L2_OFFSET) + (va&TTS_OFFSET_MASK) );
    hw_add_stats(va);

	return par;
}
/* tts_add_stats: Add values to global stat counters for tracking readonly/readwrite pages */

static void tts_add_stats(unsigned long pte)
{
        if ((pte & 0x230) == 0x210){ 
                base_ro_count++;
        }
        else{
                base_rw_count++;
        }

}
/*
 * tts_get_pte: Get pte , given a virtual address which is not in sec_to_pgt range 
 */
static unsigned long tts_get_pte(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pgd;
    unsigned long pte_val  = 0;

//	printk(KERN_ALERT "pgd = %p\n", mm->pgd);
	pgd = pgd_offset(mm, addr);
//	printk(KERN_ALERT "[%08lx] *pgd=%08llx",
//			addr, (long long)pgd_val(*pgd));

	do {
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		if (pgd_none(*pgd))
			break;

		if (pgd_bad(*pgd)) {
		//	printk("(bad)");
			break;
		}

		pud = pud_offset(pgd, addr);
		if (PTRS_PER_PUD != 1)
			printk(", *pud=%08llx", (long long)pud_val(*pud));

		if (pud_none(*pud))
			break;

		if (pud_bad(*pud)) {
//			printk("(bad)");
			break;
		}

		pmd = pmd_offset(pud, addr);
		if (PTRS_PER_PMD != 1)
			printk(", *pmd=%08llx", (long long)pmd_val(*pmd));

		if (pmd_none(*pmd))
			break;

		if (pmd_bad(*pmd)) {
			printk("(bad)");
			break;
		}

		/* We must not map this if we have highmem enabled */
		if (PageHighMem(pfn_to_page(pmd_val(*pmd) >> PAGE_SHIFT)))
			break;

		pte = pte_offset_map(pmd, addr);
#ifndef CONFIG_ARM_LPAE
//		printk(", *ppte=%08llx",
//		       (long long)pte_val(pte[PTE_HWTABLE_PTRS]));
#endif
        pte_val = (unsigned long)pte_val(pte[PTE_HWTABLE_PTRS]);
		pte_unmap(pte);
    } while(0);

    return(pte_val);
    printk("\n");
}

/*sw_va_to_pa: Converting a virtual address to physical address */
static unsigned long sw_va_to_pa(struct mm_struct *mm,unsigned long va)
{
	unsigned long pgd;
	unsigned long l2_base1;
	unsigned long l2_val;
	unsigned long offset;
	unsigned long l1_index,l2_index,l2_base;
    unsigned long pa;
	
	pgd = (unsigned long)cpu_get_pgd();

	l1_index = (va & TTS_L1_MASK) >> TTS_L1_SHIFT;
    
    /*Calculating the l2base*/
    l2_base = *(unsigned long *)(pgd + l1_index*4);
	
    /* Check if l2_base is a section. Return if true */
    if ((l2_base & 0x3) != 0x1)
    {
           // printk("\npassed2 pa = %lx l2_base= %lx \n",pa_passed,l2_base);
            return TTS_SECTION_ENTRY;
    }
    
    //if( ( l2_base >= (unsigned long)0x88500000)&&(l2_base <= (unsigned long)0x88700000)) {
    if( ( (l2_base&TTS_L1_MASK) == (unsigned long)SEC_TO_PGT_MEM_ADDR)) {
     
            /*Divide the index by 4*/
            offset = ( l2_base & L1_FLAG_MASK ) >> 2;
            l2_base1 = (unsigned long)(l2_mmap_ptr + offset);

            l2_index = (va & TTS_L2_MASK) >> TTS_L2_SHIFT;
            l2_val = *(unsigned long *)(l2_base1 + l2_index*4);

            pa = ((l2_val & TTS_L2_OFFSET) + (va & TTS_OFFSET_MASK));
          //printk("\npassed pa = %lx l2_base = %lx, pte = %lx pte_val = %lx pa = %lx\n",pa_passed,l2_base,l2_base1+offset*4,pte,pa);
    }
    else {
            l2_val = tts_get_pte(mm,va);
            pa = (unsigned long)__pa(va);
    }

    /*Add stats to global counters*/
    tts_add_stats(l2_val);
    
    return pa;
}
/*
 * tts_l1_page_walk: Do the First Level Page walk and find the attributes of pages where L2 page tables lies
 */
static int tts_l1_page_walk(struct mm_struct *mm,unsigned long pgd)
{

        unsigned int i = 0;
        unsigned int l2_tbl_cnt = 0;

        unsigned long l1_entry,l1_flags,l2_tbl_paddr,l2_tbl_vaddr,sw_pa;
        unsigned long hw_pa;

        for (i = 0; i < L1_NENTRIES; i++) {

                l1_entry = *(unsigned long *)(pgd + i*4);
                l1_flags = l1_entry & L1_ENTRY_PAGE_TABLE_FLAGS_MASK;

                if (__is_l1_page_table_entry(l1_flags)) {

                        l2_tbl_cnt++;
                        l2_tbl_paddr    = l1_entry & L1_ENTRY_PAGE_TABLE_PHYSICAL_ADDRESS_MASK;
                        
                        /* find virtual address for the L2 base table*/
                        l2_tbl_vaddr    = (unsigned long)tts_pa_to_va(l2_tbl_paddr);
                        
                        //down_write(&mm->mmap_sem);
                        
                        /*Locking down the pagetable */
                        spin_lock(&mm->page_table_lock);
                        
                        /* Convert virtual address using pagewalk*/
                        hw_pa    = (unsigned long)hw_va_to_pa(l2_tbl_vaddr);
                        sw_pa    = (unsigned long)sw_va_to_pa(mm,l2_tbl_vaddr);

                        /*Unlock the pagetable */
                        spin_unlock(&mm->page_table_lock);
                        //up_write(&mm->mmap_sem);

                        /* if converted physical address is not equivalent of original pa 
                           then something wrong with translation*/
                        if((sw_pa != TTS_SECTION_ENTRY) && (sw_pa != l2_tbl_paddr)) {
                                sw_lb_fc++;
                        }
                        if((hw_pa != l2_tbl_paddr)){
                                hw_lb_fc++;
                               // printk(TAG"pa = %lx,__va = %lx cva = %lx cpa = %lx\n",l2_tbl_paddr,(unsigned long)__va(l2_tbl_paddr),l2_tbl_vaddr,hw_pa);
                        }

                }
        }
        return l2_tbl_cnt;
}


/* 
 * tts_verify_rkp - Main entry point for RKP Test function
 * It does the pagewalk and try to verify whether L2 page tables are placed in ReadOnly pages
 */
int tts_verify_rkp(void)
{
    struct task_struct *task = NULL;
    struct mm_struct *mm = NULL;

	int task_count = 0,invalid_mm = 0,count_l2table = 0;
	
    rkp_reset_counters(base_ro_count,base_rw_count);

    for_each_process(task)
    {
            mm = task->active_mm;
            task_count++;

            if(!(mm) || !(mm->context.id)){
                    invalid_mm++;
                    continue;
            }
            count_l2table = tts_l1_page_walk(mm,(unsigned long)(mm->pgd));

            if(hw_lb_fc ||sw_lb_fc){
                    printk(TAG "Loopback Test failed for process #%s# hw fail_count = %d sw fail_count = %d L2table count = %d\n",task->comm,hw_lb_fc,sw_lb_fc,count_l2table);
                    hw_lb_fc = 0;
                    sw_lb_fc = 0;
            }
    }
    printk(TAG "RO tables = %d RW tables = %d\n",base_ro_count,base_rw_count);
    
    if(base_rw_count)
            return TTS_TC_RET_FAILURE;

    return TTS_TC_RET_SUCCESS;
}



/** Check if a certain va is made read-only by tima
*/
static void is_page_range_ro(unsigned long va)
{
	unsigned long  par;

    printk("\n va= %lx",va);
    /* Translate the page use read-only priv.
	Failing implies a translation fault*/
	__asm__	("mcr	p15, 0, %1, c7, c8, 0\n"
		"isb\n"
		"mrc 	p15, 0, %0, c7, c4, 0\n"
		:"=r"(par):"r"(va));
	if (par & 0x1) {
            printk(" invalid address");
            return;
    }

	/* Translate the page use writable priv.
	Failing means a read-only page 
	(tranlation was confirmed by previous step)*/
	__asm__	("mcr	p15, 0, %1, c7, c8, 1\n"
		"isb\n"
		"mrc 	p15, 0, %0, c7, c4, 0\n"
		:"=r"(par):"r"(va));

	if (par & 0x1) {
            printk(" RO");
            return;
    }
    printk(" RW");
}
void is_page_range_nx(unsigned long lva)
{
        unsigned long pte_val;
        pte_val = tts_get_pte(current->active_mm,lva);
        if(pte_val & PTE_EXT_XN){
                printk(" NX\n");
        }
        else {
                printk(" X\n");
        }

}
static void tima_get_page_attributes(unsigned long va,unsigned int count)
{
        int i;
        unsigned long lva = va;

        for(i = 0;i < count ; i++ )
        {
                lva = va + (PAGE_SIZE*i);
                printk("\nlva = %lx\n",lva);
                is_page_range_ro(lva);
                is_page_range_nx(lva);
        }
}



int tts_verify_cds_attributes(struct module *mod)
{
        unsigned int    *vatext,*vadata;/* base virtual address of text and data regions*/
        unsigned int    text_count,data_count;/* Number of text and data pages present in core section */


     /*Lets first pickup core section */
        vatext      = mod->module_core;
        vadata      = (int *)((char *)(mod->module_core) + mod->core_ro_size);
        text_count  = ((char *)vadata - (char *)vatext);
        data_count  = debug_align(mod->core_size) - text_count;
        text_count  = text_count / PAGE_SIZE;
        data_count  = data_count / PAGE_SIZE;

        /*Should be atleast a page */
        if(!text_count)
                text_count = 1;
        if(!data_count)
                data_count = 1;
        printk("vatext = %x vadata = %x text_count = %d data_count = %d\n",(unsigned int)vatext,(unsigned int)vadata,text_count,data_count);
        printk("\n Text Section\n");
        tima_get_page_attributes((unsigned long)vatext,text_count);
        printk("\n Data Section\n");
        tima_get_page_attributes((unsigned long)vadata,data_count);
        
        return TTS_TC_RET_SUCCESS;

}
/* Verify Code and Data Separatin */
int tts_verify_cds(void)
{

	struct module *mod;
    struct list_head *mod_head=&(((struct module *)THIS_MODULE)->list);

   // printk("\nmod name = ktestmod\n");
    tts_verify_cds_attributes(THIS_MODULE);

    /*For rest of modules*/
    list_for_each_entry(mod,mod_head,list) {
            if(mod && mod->module_core) {
                    //printk("\nmod name =#%s#\n", mod->name);
                    tts_verify_cds_attributes(mod);
            }
	}

    return TTS_TC_RET_SUCCESS;

}
/*
 * tts_l1_page_walk: Do the First Level Page walk and find the attributes of pages where L2 page tables lies
 */
static void __rkp_policy_cmp_kernel_l1_pages(struct mm_struct *mm,unsigned long pgd)
{

        unsigned int i = 0;
        unsigned long kern_l1_entry,proc_l1_entry,proc_l1_flags;
        unsigned long kpgd = (unsigned long)KPGD_INIT_MM;

        pgd = pgd & L1_ENTRY_PAGE_TABLE_PHYSICAL_ADDRESS_MASK;

        //kern_user_l1_entry_match = kern_user_l1_entry_mismatch = 0;

        for (i = KERNEL_VADDR_INDEX_START; i < KERNEL_VADDR_INDEX_END; i++) {
                proc_l1_entry = *(unsigned long *)(pgd + (i<<2));
                proc_l1_flags = proc_l1_entry & L1_ENTRY_PAGE_TABLE_FLAGS_MASK;
 
                if (__is_l1_page_table_entry(proc_l1_flags)) {
                        
                        kern_l1_entry = *(unsigned long *)(kpgd + (i<<2));
                        (proc_l1_entry == kern_l1_entry)? kern_user_l1_entry_match++ : kern_user_l1_entry_mismatch++ ;
                }
        }
}

int rkp_policy_cmp_kernel_l1_pages(void)
{
        struct task_struct *task = NULL;
        struct mm_struct *mm = NULL;

        rkp_reset_counters(kern_user_l1_entry_match,kern_user_l1_entry_mismatch);

        for_each_process(task)
        {
                mm = task->active_mm;

                if(!(mm) || !(mm->context.id)){
                        continue;
                }
                __rkp_policy_cmp_kernel_l1_pages(mm,(unsigned long)(mm->pgd));
                // printk("\nproc_name = %s No of l1 entries matched= %d Not matched = %d\n",task->comm,kern_user_l1_entry_match,kern_user_l1_entry_mismatch);
        }
        printk(TAG"##Policy 2## No l1 entries matched = %d Not matched = %d\n\n",kern_user_l1_entry_match,kern_user_l1_entry_mismatch);

        return kern_user_l1_entry_mismatch?TTS_TC_RET_FAILURE:TTS_TC_RET_SUCCESS;
}
static void __rkp_policy_test_no_kern_page_rwx(unsigned long l2_flags)
{
        unsigned int ro = 0,nx = 0;

        if((l2_flags & PTE_EXT_APX) && (l2_flags & TTS_PTE_EXT_AP0))
                ro = 1;
        if(l2_flags & PTE_EXT_XN)
                nx = 1;
        if(nx == ro){
        /* Policy violations */
                p1_no_match_count++;
        }
        else {
                p1_match_count++;
        }
}


int rkp_policy_test_no_kern_page_rwx(void)
{

        unsigned int i = 0,j = 0;
        unsigned int l2_tbl_cnt = 0;

        unsigned long l1_entry,l2_entry,l1_flags,l2_flags,l2_tbl_paddr,l2_tbl_vaddr;
        unsigned long kpgd = (unsigned long)KPGD_INIT_MM;

        rkp_reset_counters(p1_match_count,p1_no_match_count);
 
        for (i = 0; i < L1_NENTRIES; i++) {

                l1_entry = *(unsigned long *)(kpgd + i*4);
                l1_flags = l1_entry & L1_ENTRY_PAGE_TABLE_FLAGS_MASK;

                if (__is_l1_page_table_entry(l1_flags)) {

                        l2_tbl_cnt++;
                        l2_tbl_paddr    = l1_entry & L1_ENTRY_PAGE_TABLE_PHYSICAL_ADDRESS_MASK;
                        
                        /* find virtual address for the L2 base table*/
                        l2_tbl_vaddr    = (unsigned long)tts_pa_to_va(l2_tbl_paddr);
                        for (j = 0; j < L2_NENTRIES; j++) {
                                l2_entry = *(unsigned long *)(l2_tbl_vaddr + j*4);
                                if(l2_entry){
                                        l2_flags = l2_entry & L2_ENTRY_SMALL_PAGE_FLAGS_MASK;
                                        __rkp_policy_test_no_kern_page_rwx(l2_flags);
                                }
                        }
                }
        }
        printk(TAG"##Policy 1## No of kernel l1 entries  Read Only = %d Read Write = %d\n\n",p1_match_count,p1_no_match_count);

        return p1_no_match_count?TTS_TC_RET_FAILURE:TTS_TC_RET_SUCCESS;
}

int rkp_policy_test_section_ro(void)
{

        unsigned long l1_entry;
        unsigned long kpgd = (unsigned long)KPGD_INIT_MM;
        unsigned int i = 0;
        
        rkp_reset_counters(rw_section,ro_section);

        for (i = 0; i < L1_NENTRIES; i++) {

                l1_entry = *(unsigned long *)(kpgd + i*4);

                if (__is_l1_section_entry(l1_entry)) {
                        if(is_section_ro(l1_entry))
                                ro_section++;
                        else {
                                rw_section++;
                        }

                }
        }
        printk(TAG"##Policy 3## No of RO Sections = %d RW Sections = %d\n\n",ro_section,rw_section);

        return rw_section?TTS_TC_RET_FAILURE:TTS_TC_RET_SUCCESS;
}
static void __rkp_policy_verify_user_pxn_bit_set(struct mm_struct *mm,unsigned long pgd)
{

        unsigned int i = 0;
        unsigned long proc_l1_entry,proc_l1_flags;

        pgd = pgd & L1_ENTRY_PAGE_TABLE_PHYSICAL_ADDRESS_MASK;

        //kern_user_l1_entry_match = kern_user_l1_entry_mismatch = 0;

        for (i = USER_VADDR_INDEX_START; i < USER_VADDR_INDEX_END; i++) {
                proc_l1_entry = *(unsigned long *)(pgd + (i<<2));
                proc_l1_flags = proc_l1_entry & L1_ENTRY_PAGE_TABLE_FLAGS_MASK;

                if (__is_l1_page_table_entry(proc_l1_flags)) {
                        if( (proc_l1_flags & PAGE_TABLE_PXN_INDEX) == PAGE_TABLE_PXN_INDEX) {
                                no_pxn_set++;
                        }
                        else {
                                no_pxn_not_set++;
                        }
                }
                if (__is_l1_section_entry(proc_l1_flags)) {

                        if( (proc_l1_flags & SECTION_PXN_INDEX) == SECTION_PXN_INDEX) {
                                no_pxn_set++;
                        }
                        else {
                                no_pxn_not_set++;
                        }

                }
        }
}


int rkp_policy_verify_user_pxn_bit_set(void)
{
        struct task_struct *task = NULL;
        struct mm_struct *mm = NULL;

        rkp_reset_counters(no_pxn_set,no_pxn_not_set);

        for_each_process(task)
        {
                mm = task->active_mm;

                if(!(mm) || !(mm->context.id)){
                        continue;
                }
                __rkp_policy_verify_user_pxn_bit_set(mm,(unsigned long)(mm->pgd));
                // printk("\nproc_name = %s No of l1 entries matched= %d Not matched = %d\n",task->comm,kern_user_l1_entry_match,kern_user_l1_entry_mismatch);
        }
        printk(TAG"##Policy 4## No of PXN entries matched = %d Not matched = %d\n\n",no_pxn_set,no_pxn_not_set);

        return no_pxn_not_set?TTS_TC_RET_FAILURE:TTS_TC_RET_SUCCESS;
}

int rkp_policy_free_in_use_l1pgt(void)
{
        struct mm_struct *mm;
        pgd_t *pgd;
        mm = current->mm;
        if(!mm)
                return TTS_TC_RET_SUCCESS;

        pgd = mm->pgd;
        
        /*This function should cause*/
        pgd_free(mm,pgd);

        printk(TAG" Run tima_dump_log and see whether it has number of following messages has increased by one\n ");
        printk(TAG" #Clearing an already used TTBR# \n");
        return TTS_TC_RET_SUCCESS;
}

/** Check if a certain va is made read-only by tima
*/

static int rkp_test_page_range_nx(unsigned long lva)
{
        unsigned long pte_val;
        pte_val = tts_get_pte(current->active_mm,lva);
        if(pte_val & PTE_EXT_XN){
                return TIMA_PAGE_NX;
        }
        else {
                return TIMA_PAGE_X;
        }

}
static int rkp_test_page_range_ro(unsigned long va)
{
	unsigned long  par;

    /* Translate the page use read-only priv.
	Failing implies a translation fault*/
	__asm__	("mcr	p15, 0, %1, c7, c8, 0\n"
		"isb\n"
		"mrc 	p15, 0, %0, c7, c4, 0\n"
		:"=r"(par):"r"(va));
	if (par & 0x1) {
            printk(" invalid address");
            return -1;
    }

	/* Translate the page use writable priv.
	Failing means a read-only page 
	(tranlation was confirmed by previous step)*/
	__asm__	("mcr	p15, 0, %1, c7, c8, 1\n"
		"isb\n"
		"mrc 	p15, 0, %0, c7, c4, 0\n"
		:"=r"(par):"r"(va));

	if (par & 0x1) {
            return TIMA_PAGE_RO;
    }
    return TIMA_PAGE_RW;
}

static void rkp_test_lkm_page_properties(unsigned int sect_name,unsigned long va,unsigned int count)
{
        int i,ro,nx,ret;
        unsigned long lva = va;
        ro = 0;
        nx = 0;  
        for(i = 0;i < count ; i++ )
        {
                lva = va + (PAGE_SIZE*i);
                ret = rkp_test_page_range_ro(lva);
                if(ret < 0) 
                        continue;
                if(ret == TIMA_PAGE_RO)
                        ro = 1;

                ret = rkp_test_page_range_nx(lva);
                if(ret == TIMA_PAGE_NX)
                        nx = 1;

                if(sect_name == RKP_LKM_CODE_SECTION){
                        if(ro && (nx == 0)) {
                                code_match_count++;
                        }
                        else {
                                code_mismatch_count++;
                        }

                }
                if(sect_name == RKP_LKM_DATA_SECTION){
                        if( (ro == 0) && nx) {
                                data_match_count++;
                        }
                        else {
                                data_mismatch_count++;
                        }
                }
        }
}

int  __rkp_policy_verify_lkm_properties(struct module *mod)
{
        unsigned int    *vatext,*vadata;/* base virtual address of text and data regions*/
        unsigned int    text_count,data_count;/* Number of text and data pages present in core section */

     /*Lets first pickup core section */
        vatext      = mod->module_core;
        vadata      = (int *)((char *)(mod->module_core) + mod->core_ro_size);
        text_count  = ((char *)vadata - (char *)vatext);
        data_count  = debug_align(mod->core_size) - text_count;
        text_count  = text_count / PAGE_SIZE;
        data_count  = data_count / PAGE_SIZE;

        /*Should be atleast a page */
        if(!text_count)
                text_count = 1;
        if(!data_count)
                data_count = 1;
        
        rkp_test_lkm_page_properties(RKP_LKM_CODE_SECTION,(unsigned long)vatext,text_count);
        rkp_test_lkm_page_properties(RKP_LKM_DATA_SECTION,(unsigned long)vadata,data_count);
    
        return TTS_TC_RET_SUCCESS;

}
/* Verify Code and Data Separatin */
int rkp_policy_verify_lkm_properties(void)
{

	struct module *mod;
    struct list_head *mod_head=&(((struct module *)THIS_MODULE)->list);

    rkp_reset_counters(code_match_count,code_mismatch_count);
    rkp_reset_counters(data_match_count,data_mismatch_count);

    __rkp_policy_verify_lkm_properties(THIS_MODULE);

    /*For rest of modules*/
    list_for_each_entry(mod,mod_head,list) {
            if(mod && mod->module_core) {
                    //printk("\nmod name =#%s#\n", mod->name);
                  __rkp_policy_verify_lkm_properties(mod);
            }
	}

        printk(TAG"##Policy 6## No of Code sections match = %d  code mismatch count = %d data match count = %d data mismatch count = %d\n",code_match_count,code_mismatch_count,data_match_count,data_mismatch_count);
    
        if(code_mismatch_count || data_mismatch_count)
                return TTS_TC_RET_FAILURE;

    return TTS_TC_RET_SUCCESS;

}

static void __rkp_policy_verify_no_double_mapping(unsigned long l2_entry)
{
        unsigned long l2_flags,index,curr_flag = TIMA_L2_ENTRY_RW;
        
        l2_flags = l2_entry & L2_ENTRY_SMALL_PAGE_FLAGS_MASK;
        index = (l2_entry >> 12)&0xfffff;

        if((l2_flags & PTE_EXT_APX) && (l2_flags & TTS_PTE_EXT_AP0))
                curr_flag = TIMA_L2_ENTRY_RO;
    
        if(!(phys_addr_array[index])) {
                phys_addr_array[index] = curr_flag;
                single_mapping++;
        }
        else {
                if(phys_addr_array[index] != curr_flag)
                {
                      //  printk(TAG"physaddr = %lx old_tag = %ld curr_flag = %ld\n",index,phys_addr_array[index],curr_flag);
                        non_single_mapping++;
                }
        }
}

int  rkp_policy_verify_no_double_mapping(void)
{

        unsigned int i = 0,j = 0;
        unsigned int l2_tbl_cnt = 0;

        unsigned long l1_entry,l2_entry,l1_flags,l2_tbl_paddr,l2_tbl_vaddr;
        unsigned long kpgd = (unsigned long)KPGD_INIT_MM;

        rkp_reset_counters(non_single_mapping,single_mapping);
        memset(phys_addr_array,0,TIMA_PHYS_ADDR_ARR_LENGTH);

        for (i = KERNEL_VADDR_INDEX_START; i < KERNEL_VADDR_INDEX_END; i++) {
                l1_entry = *(unsigned long *)(kpgd + i*4);
                l1_flags = l1_entry & L1_ENTRY_PAGE_TABLE_FLAGS_MASK;

                if (__is_l1_page_table_entry(l1_flags)) {

                        l2_tbl_cnt++;
                        l2_tbl_paddr    = l1_entry & L1_ENTRY_PAGE_TABLE_PHYSICAL_ADDRESS_MASK;
                        
                        /* find virtual address for the L2 base table*/
                        l2_tbl_vaddr    = (unsigned long)tts_pa_to_va(l2_tbl_paddr);
                        for (j = 0; j < L2_NENTRIES; j++) {
                                l2_entry = *(unsigned long *)(l2_tbl_vaddr + j*4);
                                if(l2_entry){
                                        __rkp_policy_verify_no_double_mapping(l2_entry);
                                }
                        }
                }
        }
        printk(TAG"##Policy 7## Single mapping = %d Policy violated double mappings= %d\n\n",single_mapping,non_single_mapping);

        return non_single_mapping?TTS_TC_RET_FAILURE:TTS_TC_RET_SUCCESS;
}

static int rkp_test_policy_kern_range(void)
{
        unsigned int ret = TTS_TC_RET_SUCCESS;
        /*Policy 1: Tests whether any kernel page table entries are mapped as read write executable (RWX)*/
        ret |= rkp_policy_test_no_kern_page_rwx();

        /*Policy 2:Cmp kernel entries of each process with initial kernel mapping*/
        ret |= rkp_policy_cmp_kernel_l1_pages();
        
        /*Policy 3:Test whether sections corresponding to kernel are readonly*/
        ret |= rkp_policy_test_section_ro();
        
        /*Policy 4:Test whether any userspace L1 entries have pxn bit NOT set*/
        ret |= rkp_policy_verify_user_pxn_bit_set();

        /*Policy 5: Free a inuse L1 entries of a process and check the output in dumplog*/
        ret |= rkp_policy_free_in_use_l1pgt();

        /*Policy 6: Verify whether LKM Code is RO NX and LKM Data RW NX*/
        ret |= rkp_policy_verify_lkm_properties();

        /*Policy 7: Verify whether physical page has got both readwrite and readonly mapping */
        ret |=  rkp_policy_verify_no_double_mapping();

        return ret?TTS_TC_RET_FAILURE:TTS_TC_RET_SUCCESS;
}
int tts_verify_rkp_policies(void)
{
        unsigned int ret;
        
        ret = rkp_test_policy_kern_range();
        
        return ret ;
}

int tts_verify_arg(unsigned long paddr)
{
        unsigned long vaddr,sw_pa,hw_pa;

        printk(TAG"passed paddr = %lx\n",paddr);
        vaddr = (unsigned long)tts_pa_to_va(paddr);

        hw_pa = (unsigned long)hw_va_to_pa(vaddr);
        /* Convert virtual address using pagewalk*/
        sw_pa = (unsigned long)sw_va_to_pa(current->active_mm,vaddr);

        if(sw_pa == TTS_SECTION_ENTRY) {
                printk(TAG"Software Pagewalk: passed paddr = %lx vaddr = %lx converted pa = SECTION_ENTRY\n",paddr,vaddr);
        }

        if(hw_pa == TTS_TRANSLATION_FAULT) {
                printk(TAG"Hardware Pagewalk: Translation Fault\n");
        }
        else {
                printk(TAG"passed paddr = %lx vaddr = %lx hw pa = %lx sw pa = %lx\n",paddr,vaddr,hw_pa,sw_pa);
        }

        return TTS_TC_RET_SUCCESS;
}
#if 0



static int __is_l2_small_page_entry(unsigned long x)
{
	return (((x & 0x2) == 0x2)? 1 : 0);
}

/*
 * l2_page_walk- walk through each entries of L2 table pointed by pgd
 */
int l2_page_walk(unsigned long L2_virtaddr) 
{
	int i;

	unsigned int L2_entries_count = 0;

	unsigned long L2_entry;
	unsigned long L2_flags;

	unsigned long physaddr;
	unsigned long virtaddr;
			
	for (i = 0; i < L2_NENTRIES; i++) {
		L2_entry = *(unsigned long *)(L2_virtaddr + i*4);
		L2_flags = L2_entry & L2_ENTRY_SMALL_PAGE_FLAGS_MASK;
		if (__is_l2_small_page_entry(L2_flags)) {
			L2_entries_count++;
            physaddr = L2_entry & L2_ENTRY_SMALL_PAGE_PHYSICAL_ADDRESS_MASK;
            virtaddr = (unsigned long)__va(physaddr);
		}
	}

	return L2_entries_count;
}


int l1_page_walk(unsigned long pgd)
{

        unsigned int i = 0;
        unsigned int l2_tbl_cnt = 0;
        unsigned int L2_tables_entries_count = 0;

        unsigned long l1_entry;
        unsigned long l1_flags;
        unsigned long l2_tbl_paddr;
        unsigned long l2_tbl_vaddr;

        TTS_DBG_LOG (TAG "PPP l1_page_walk: Passed pgd = %lx\n", pgd);

        for (i = 0; i < L1_NENTRIES; i++) {
                l1_entry = *(unsigned long *)(pgd + i*4);
                l1_flags = l1_entry & L1_ENTRY_PAGE_TABLE_FLAGS_MASK;
                if (__is_l1_page_table_entry(l1_flags)) {
                        l2_tbl_cnt++;
                        l2_tbl_paddr = l1_entry & l1_entry_PAGE_TABLE_PHYSICAL_ADDRESS_MASK;
                        l2_tbl_vaddr = (unsigned long)__va(l2_tbl_paddr);

                        L2_tables_entries_count += l2_page_walk(l2_tbl_vaddr);
                }
        }

        TTS_DBG_LOG (TAG "\n");
        TTS_DBG_LOG (TAG "l2_tbl_cnt =         %u\n", l2_tbl_cnt);
        TTS_DBG_LOG (TAG "L2_tables_entries_count = %u\n", L2_tables_entries_count);

        return TTS_TC_RET_SUCCESS;
}


int scan_kern_pgd(void)
{
    unsigned long kpgd;

/*    kpgd = (unsigned long)(init_mm.pgd); */
    kpgd = (unsigned long)KPGD_INIT_MM;

    return l1_page_walk(kpgd);
}
#endif
