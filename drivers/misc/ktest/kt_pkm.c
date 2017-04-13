/*
 *      Contains implementations of test cases for PKM
 *
 *      Author:    Guruprasad Ganesh   <g.ganesh@sta.samsung.com>
 *
 *      Changes:
 *              GG              :      Uploaded the initial Version
 *
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/highmem.h>
#include <linux/sched.h>
#include <linux/tima/tts_common.h>

extern void tts_debug_func_mod(void);
#if 0
unsigned int call_pkm_modify_rodata(unsigned long arg)
{
      //  char *p = (char *)&tts_debug_string[0];
      //void *write_addr=(void *)tts_debug_string;
      void *write_addr;
      //write_addr = (void*)0xc02a4718;
      write_addr = (void*)0xc09033dc;
      printk("\n write_add = %p\n",write_addr);
      *((uint32_t*)write_addr) = 0xaabbccdd;
       // arg?(*p='d'):(*p='t');
        return TTS_TC_RET_SUCCESS;
}
#endif
unsigned int call_pkm_modify_code(unsigned long arg)
{
        unsigned long *func;
        static unsigned long oldfunc;

        func = (unsigned long*)tts_debug_func_mod;
        if(!func)
                return TTS_TC_RET_FAILURE;
        if(arg)
        {
                oldfunc = *func;
                *func = 'r';
        }
        else
        {
                *func =  oldfunc;
        }
        return TTS_TC_RET_SUCCESS;
}

#if 0
unsigned int call_pkm_modify_revert_rodata(void);
unsigned int call_pkm_modify_revert_rodata(void)
{
#if 0
        char *p = (char *)tts_debug_string;
        if(!p)
                return TTS_TC_RET_FAILURE;

        *p = 'd';
        *p = 't';
     #endif
        return TTS_TC_RET_SUCCESS;
}
#endif
unsigned int call_pkm_modify_revert_code(void);
unsigned int call_pkm_modify_revert_code(void)
{
        unsigned long *func;
        static unsigned long oldfunc;

        func = (unsigned long*)tts_debug_func_mod;
        if(!func)
                return TTS_TC_RET_FAILURE;

        oldfunc = *func;
        *func = 'r';
        *func =  oldfunc;

        return TTS_TC_RET_SUCCESS;
}

unsigned int copy_pkm_func_ptr(unsigned long arg)
{
        int ncopy;
        unsigned long val = (unsigned long)tts_debug_func_mod;
        ncopy = copy_to_user((char __user  *)arg,(unsigned long*)&val,4);

        return(ncopy?TTS_TC_RET_FAILURE:TTS_TC_RET_SUCCESS);

}

