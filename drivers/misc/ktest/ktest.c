/*
 *      Tima Test Suite infrastructure implementation on kernel side
 *
 *      Author:    Guruprasad Ganesh   <g.ganesh@sta.samsung.com>
 *
 *      Changes:
 *              GG              :      Uploaded the initial Version
 *
 *
 */

/* PLEASE DONT ADD TEST CASE IMPLEMENTATIONS IN THIS FILE*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/highmem.h>
#include <linux/proc_fs.h>
#include <linux/tima/tts_structure.h>
#include "ktest_common.h"

#define DRIVER_DESC         "Tima Test Suite Infrastructure"

char tts_err_log[TTS_MAX_RES_LEN];
EXPORT_SYMBOL(tts_err_log);
int tts_walk_proc_pgd(void);

int ktest_parse_l1pgt(unsigned int cmd,unsigned long arg)
{
        unsigned int rett = TTS_TC_RET_FAILURE;
        switch(cmd)
        {
                case TTS_L1PGT_FN_00:
                        {
                               rett = tts_verify_rkp();
                        }
                        break;
                case TTS_L1PGT_FN_01:
                        {
                                rett = rkp_policy_verify_lkm_properties();
                                //rett = tts_verify_cds();
                        }
                        break;
                case TTS_L1PGT_FN_02:
                        {
                                rett = rkp_policy_test_no_kern_page_rwx();
                        }
                        break;
                case TTS_L1PGT_FN_03:
                        {
                                rett = rkp_policy_cmp_kernel_l1_pages();
                        }
                        break;
                case TTS_L1PGT_FN_04:
                        {
                                rett = rkp_policy_test_section_ro();
                        }
                        break;
                case TTS_L1PGT_FN_05:
                        {
                                rett = rkp_policy_verify_user_pxn_bit_set();
                        }
                        break;
                case TTS_L1PGT_FN_06:
                        {
                                rett = rkp_policy_free_in_use_l1pgt();
                        }
                        break;
                case TTS_L1PGT_FN_07:
                        {
                                rett = rkp_policy_verify_lkm_properties();
                        }
                        break;
                case TTS_L1PGT_FN_08:
                        {
                                rett = rkp_policy_verify_no_double_mapping();
                        }
                        break;
                case TTS_L1PGT_FN_09:
                        {
                                rett = tts_verify_rkp_policies();
                        }
                        break;
                case TTS_L1PGT_FN_10:
                        {
                                rett = tts_verify_arg(arg);
                        }
                        break;
                case TTS_L1PGT_SEC_00:
                        {
                        }
                        break;
                case TTS_L1PGT_SEC_01:
                        {
                        }
                        break;

                default:
                        rett = TTS_TC_RET_FAILURE;
        }
        return rett;
}
int ktest_parse_lkmauth(unsigned int cmd,unsigned long arg)
{
        unsigned int rett = TTS_TC_RET_FAILURE;
        switch(cmd)
        {
                case TTS_LKMAUTH_FN_01:
                        {
                        }
                        break;
                case TTS_LKMAUTH_SEC_01:
                        {
                                lkmauth_restart();
                        }
                        break;
                default:
                        rett = TTS_TC_RET_FAILURE;

        }
        return rett;
}

int ktest_parse_pkm(unsigned int cmd,unsigned long arg)
{
        unsigned int rett = TTS_TC_RET_FAILURE;
        switch(cmd)
        {
                case TTS_PKM_FN_02:
                        {
                        }
                        break;
                case TTS_PKM_SEC_05:
                        {
                            rett = call_pkm_modify_code(arg);
                        }
                        break;
                case TTS_PKM_SEC_06:
                        {
                            rett = call_pkm_modify_revert_code();
                        }
                        break;
               case TTS_PKM_SEC_07:
                        {
                           rett = copy_pkm_func_ptr(arg);
                        }
                        break;
                default:
                        rett = TTS_TC_RET_FAILURE;

        }
        return rett;
}

static inline unsigned int ktest_get_fid(unsigned int cmd)
{
            return (cmd >> TTS_FEATURE_SHIFT);
}

unsigned int ktest_depktize(const char __user *buf,unsigned int *cmd,unsigned long *arg)
{

        tts_pkt kpkt;
        long ret;

        ret = copy_from_user((void *)&kpkt,buf,sizeof(struct tts_data_pkt));
        if( ret < 0 )
                return TTS_TC_RET_FAILURE;

        *cmd = kpkt.cmd;
        *arg = kpkt.arg;

        return TTS_TC_RET_SUCCESS;
}

/* Function: ktest_main_write
 *
 * Initial  main/ioctl Framework
 */

static ssize_t ktest_main_write(struct file *file, const char __user *buf,
                               size_t count, loff_t *ppos)
{
        unsigned int fId,cmd;
        unsigned long arg;
        unsigned int rett;

        rett = ktest_depktize(buf,&cmd,&arg);
        if( rett == TTS_TC_RET_FAILURE)
                return rett;

        TTS_DBG_LOG( "KTEST MODULE cmd = #%x# arg = #%lx#\n",cmd,arg);
        if(cmd == TTS_GET_LOG)
        {
                int ncopy;
                ncopy = copy_to_user((char __user  *)arg,tts_err_log,TTS_MAX_RES_LEN);
                memset(tts_err_log,0,TTS_MAX_RES_LEN);

                return(ncopy?TTS_TC_RET_FAILURE:TTS_TC_RET_SUCCESS);
        }

        fId = TTS_GET_FID(cmd);
        switch( fId)
        {
                case TTS_L1PGT:
                        rett = ktest_parse_l1pgt(cmd,arg);

                        break;
                case TTS_LKMAUTH:
                        rett = ktest_parse_lkmauth(cmd,arg);

                        break;
                case TTS_PKM:
                        rett = ktest_parse_pkm(cmd,arg);

                        break;
                default:
                        rett = -EINVAL;
        }
        return rett;
}

static int ktest_main_open(struct inode * inode, struct file * file)
{
        return 0;
}

struct file_operations ktest_main_fops={
      write: ktest_main_write,
      open:  ktest_main_open,
};

/* Function: ktest_main_init
 *
 * Entry point for Tima Test Suite
 */
static int __init ktest_main_init(void)
{
        TTS_DBG_LOG("\n Initializing Tima Test kernel infrastructure\n");
         proc_create("tts_test", 0644,NULL,&ktest_main_fops);

        return 0;
}

/* Function: ktest_main_exit
 *
 * Cleanup function for Tima Test Suite
 */
static void __exit ktest_main_exit(void)
{
       remove_proc_entry("tts_test", NULL);

}

module_init(ktest_main_init);
module_exit(ktest_main_exit);

MODULE_LICENSE("GPL");              /*Correction : GPL or something else???*/
MODULE_DESCRIPTION(DRIVER_DESC);
