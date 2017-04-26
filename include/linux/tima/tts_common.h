/*
 *      This Header file contains common definitions used by both ktest and utest 
 *    
 *      Dont add anything here  unless both utest and ktest requires the defines
 *    
 *      Author:    Guruprasad Ganesh   <g.ganesh@sta.samsung.com> 
 *
 *      Changes:
 *              GG              :      Uploaded the initial Version 
 *
 *
 */
#ifndef __TTS_COMMON_H__
#define __TTS_COMMON_H__


#define	TTS_ERR_LOG(...)	snprintf(tts_err_log,TTS_MAX_RES_LEN,__VA_ARGS__)
#define DEBUG_TIMATEST
#ifdef  DEBUG_TIMATEST 
#define	TTS_DBG_LOG(...)	printk(__VA_ARGS__)
#else
#define	TTS_DBG_LOG(...)
#endif

#define TTS_TC_RET_SUCCESS  0x0
#define TTS_TC_RET_FAILURE  0x1

#define UTEST_TC_KERNEL       0xabcd
#define TTS_MAX_RES_LEN       1024
#endif/*__TTS_COMMON_H__*/
