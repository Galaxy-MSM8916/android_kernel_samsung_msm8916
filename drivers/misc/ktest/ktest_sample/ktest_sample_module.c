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

static int __init tts_sample_mod_init(void)
{
        printk("\n Ktest Sample Module loaded \n");
        return 0;
}

/* Function: tts_sample_mod_exit
 *
 * Cleanup function for Tima Test Suite
 */
static void __exit tts_sample_mod_exit(void)
{
        printk("\n Removing Sample module\n");
}

module_init(tts_sample_mod_init);
module_exit(tts_sample_mod_exit);

MODULE_LICENSE("GPL");              /*Correction : GPL or something else???*/
