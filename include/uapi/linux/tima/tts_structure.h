/*
 *      This Header file contains important datastructures , which must be included
 *      only by infrastructure Code
 *
 *      Author:    Guruprasad Ganesh   <g.ganesh@sta.samsung.com>
 *
 *      Changes:
 *              GG              :      Uploaded the initial Version
 *
 *
 */

#ifndef __TTS_STRUCTURES_H__
#define __TTS_STRUCTURES_H__

#include"tts_common.h"

/*
   Each test case would be recognized as a combination of above mentioned components

   TC = 32 bit  FEATURE_ID + TC_TYPE + TC ID
*/
#define TTS_GETLIST     0xffffffff
#define TTS_GET_LOG     0xfffffffe

#define FN_SEC_OFFSET       0x100
#define TTS_TC_DESCR_MAX    1024


#define TTS_COMM_DEV        "/dev/timatest"
#define TTS_MKNOD_FLAGS     S_IFCHR|S_IRWXU|S_IRWXG|S_IRWXO
#define TTS_COMM_MAJOR      100
#define TTS_COMM_MINOR      0


#define L1PGT_FN_BASE       0x00000000
#define L1PGT_SEC_BASE      0x00100000+FN_SEC_OFFSET 
#define LKMAUTH_FN_BASE     0x01000000
#define LKMAUTH_SEC_BASE    0x01100000+FN_SEC_OFFSET
#define PKM_FN_BASE         0x02000000
#define PKM_SEC_BASE        0x02100000+FN_SEC_OFFSET

#define TTS_FEATURE_BASE    0x00
#define TTS_TYPE_BASE       0x0


#define TTS_FEATURE_SHIFT   24
#define TTS_TYPE_SHIFT      20
#define TTS_TC_SHIFT        0

#define TTS_PRINT_GETLIST(y) printf(#y":%d\n",y)
#define TTS_GET_FID(cmd) (cmd >> TTS_FEATURE_SHIFT)
#define TTS_GET_TYPE_ID(cmd) (cmd >> TTS_TYPE_SHIFT)&0xf
#define TTS_GET_CASE_ID(cmd) (cmd & 0xfffff)


/*
   Each test case id must follow this format
   TTS_<FEATURE_ID>_<TC_TYPE>_<TC_ID>

   FEATURE_ID = 8 bit , starts from 0x00   - 0xff
   TC_TYPE    = 4 bit , starts from 0x0    - 0xf
   TC ID      = 20 bit, starts from 0x00000 - 0xfffff

*/
typedef enum
{
    TTS_L1PGT=TTS_FEATURE_BASE,
    TTS_LKMAUTH,
    TTS_PKM,
    TTS_FEATURE_MAX,
}E_TTS_FEATURE_ID;

typedef enum
{   
    TTS_FUNCTIONAL=TTS_TYPE_BASE,
    TTS_SECURITY,
    TTS_TYPE_MAX,
}E_TTS_TC_TYPE;

typedef enum
{

/*Feature :L1PGT */
/*Functional TCs*/
   TTS_L1PGT_FN_00 = L1PGT_FN_BASE,
   TTS_L1PGT_FN_01,
   TTS_L1PGT_FN_02,
   TTS_L1PGT_FN_03,
   TTS_L1PGT_FN_04,
   TTS_L1PGT_FN_05,
   TTS_L1PGT_FN_06,
   TTS_L1PGT_FN_07,
   TTS_L1PGT_FN_08,
   TTS_L1PGT_FN_09,
   TTS_L1PGT_FN_10,
   TTS_L1PGT_FN_11,
   TTS_L1PGT_FN_12,
   TTS_L1PGT_FN_13,
   TTS_L1PGT_FN_14,
   TTS_L1PGT_FN_15,
   TTS_L1PGT_FN_16,
   TTS_L1PGT_FN_MAX,
/*Security TCs*/
   TTS_L1PGT_SEC_00 = L1PGT_SEC_BASE,
   TTS_L1PGT_SEC_01,
   TTS_L1PGT_SEC_02,
   TTS_L1PGT_SEC_03,
   TTS_L1PGT_SEC_MAX,


/*Feature :LKMAUTH*/
/*Functional TCs*/
   TTS_LKMAUTH_FN_00 = LKMAUTH_FN_BASE,
   TTS_LKMAUTH_FN_01,
   TTS_LKMAUTH_FN_02,
   TTS_LKMAUTH_FN_03,
   TTS_LKMAUTH_FN_04,
   TTS_LKMAUTH_FN_05,
   TTS_LKMAUTH_FN_06,
   TTS_LKMAUTH_FN_07,
   TTS_LKMAUTH_FN_08,
   TTS_LKMAUTH_FN_09,
   TTS_LKMAUTH_FN_10,
   TTS_LKMAUTH_FN_11,
   TTS_LKMAUTH_FN_MAX,
/*Security TCs*/
   TTS_LKMAUTH_SEC_00 = LKMAUTH_SEC_BASE,
   TTS_LKMAUTH_SEC_01,
   TTS_LKMAUTH_SEC_MAX,


/*Feature :PKM*/
/*Functional TCs*/
   TTS_PKM_FN_00 = PKM_FN_BASE,
   TTS_PKM_FN_01,
   TTS_PKM_FN_02,
   TTS_PKM_FN_03,
   TTS_PKM_FN_04,
   TTS_PKM_FN_05,
   TTS_PKM_FN_06,
   TTS_PKM_FN_07,
   TTS_PKM_FN_08,
   TTS_PKM_FN_09,
   TTS_PKM_FN_10,
   TTS_PKM_FN_11,
   TTS_PKM_FN_MAX,
/*Security TCs*/
   TTS_PKM_SEC_00 = PKM_SEC_BASE,
   TTS_PKM_SEC_01,
   TTS_PKM_SEC_02,
   TTS_PKM_SEC_03,
   TTS_PKM_SEC_04,
   TTS_PKM_SEC_05,
   TTS_PKM_SEC_06,
   TTS_PKM_SEC_07,
   TTS_PKM_SEC_08,
   TTS_PKM_SEC_09,
   TTS_PKM_SEC_10,
   TTS_PKM_SEC_11,
   TTS_PKM_SEC_12,
   TTS_PKM_SEC_MAX,
}E_TTS_TC;

typedef struct tts_data_pkt
{
        unsigned int cmd;
        unsigned long arg;
}tts_pkt;

struct s_tts_tc_list
{
        E_TTS_TC    tc;
        char tc_descr[TTS_TC_DESCR_MAX];
}tts_tc_list[] = \
{
    /*Feature :L1PGT */
    {TTS_L1PGT_FN_00,"RKP Behaviour test: It loops through all L1 entries of each process and tests whether L2 tables are in read only pages"},
    {TTS_L1PGT_FN_01,"RKP Code and Data Separation: Verify whether Code are RO-X and Data Pages are RW-NX"},
    {TTS_L1PGT_FN_02,"RKP Policy Test 1:Tests whether any kernel page table entries are mapped as read write executable (RWX)"},
    {TTS_L1PGT_FN_03,"RKP Policy Test 2:Compares kernel entries of each process with initial kernel mapping"},
    {TTS_L1PGT_FN_04,"RKP Policy Test 3:Tests whether any kernel sections are readwrite"},
    {TTS_L1PGT_FN_05,"RKP Policy Test 4:Tests whether any userspace L1 entries have pxn bit NOT set"},
    {TTS_L1PGT_FN_06,"RKP Policy Test 5:Free a inuse L1 entries of a process.Please check the output in tima_dumplog"},
    {TTS_L1PGT_FN_07,"RKP Policy Test 6:Verify whether LKM Code is RO NX and LKM Data RW NX"},
    {TTS_L1PGT_FN_08,"RKP Policy Test 7: Verify whether physical page has got both readwrite and readonly mapping"},
    {TTS_L1PGT_FN_09,"RKP Policy All in one Test: Run all the above tests at once"},

    /*Feature :LKMAUTH*/
    {TTS_LKMAUTH_FN_00,"Sanity test case for LKM Authorization"},
    {TTS_LKMAUTH_FN_01,"Try to load all signed modules in /system/lib/modules"},
    {TTS_LKMAUTH_FN_02,"LKMAUTH stress test: Call lkmauth in loops "},
    {TTS_LKMAUTH_SEC_00,"Modify existing presigned LKM and try to load it"},
    {TTS_LKMAUTH_SEC_01,"Try to disable lkmauth app and insmod unsigned module"},

    /*Feature :PKM*/
    {TTS_PKM_FN_00, "Sanity test case for Periodic Kernel Measurement"},
    {TTS_PKM_FN_01, "PKM stress test:Call pkm in loops"},
    {TTS_PKM_SEC_00,"Move PKM hash file and restart kernel measurement"},
    {TTS_PKM_SEC_01,"Remount /system file system as read write and move PKM hash file and restart kernel measurement"},
    {TTS_PKM_SEC_02,"Change permissions of PKM hash file and restart kernel measurement"},
    {TTS_PKM_SEC_03,"Remount /system file system as read write and Change permissions of PKM hash file and restart kernel measurement"},
    {TTS_PKM_SEC_04,"Start an attack on timaservice and try to shut it down "},
    {TTS_PKM_SEC_05,"Modify code in the kernel and kickstart PKM measurement"},
    {TTS_PKM_SEC_06,"Modify code, revert it back and kickstart the measurement"},
    {TTS_PKM_SEC_07,"Modify code by writing into /dev/kmem and kickstart PKM measurement"}, 
};

#endif/*__TTS_STRUCTURES_H__*/
