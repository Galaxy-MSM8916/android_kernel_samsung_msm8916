/*
 *      Common header file for Tima test cases
 *
 *      Author:    Guruprasad Ganesh   <g.ganesh@sta.samsung.com>
 *
 *      Changes:
 *              GG              :      Uploaded the initial Version
 *
 *
 */

#ifndef __KTEST_COMMON_H__
#define __KTEST_COMMON_H__

#define TTS_MAX_L1_PT   4
#define TTS_PAGE_SIZE   0x1000
#define KPGD_INIT_MM    0xc0004000

extern int scan_kern_pgd(void);
extern int tts_verify_rkp(void);
extern unsigned int call_pkm_modify_code(unsigned long arg);
extern unsigned int call_pkm_modify_revert_code(void);
extern unsigned int copy_pkm_func_ptr(unsigned long arg);
extern int tima_check_kern_l2_attributes(void);
extern void lkmauth_restart(void);
extern int tts_verify_arg(unsigned long phys);
extern int tts_verify_cds(void);
extern int tts_verify_rkp_policies(void);
extern void pgd_free(struct mm_struct *mm, pgd_t *pgd_base);
extern int rkp_policy_test_no_kern_page_rwx(void);
extern int rkp_policy_cmp_kernel_l1_pages(void);
extern int rkp_policy_test_section_ro(void);
extern int rkp_policy_verify_user_pxn_bit_set(void);
extern int rkp_policy_free_in_use_l1pgt(void);
extern int rkp_policy_verify_lkm_properties(void);
extern int  rkp_policy_verify_no_double_mapping(void);


unsigned int tts_call_pkm(void);

#endif/*__KTEST_COMMON_H__*/
