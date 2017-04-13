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
#include <linux/qseecom.h>
#include <linux/tima/tts_common.h>

struct tts_qseecom_handle {
    void *dev; /* in/out */
    unsigned char *sbuf; /* in/out */
    uint32_t sbuf_len; /* in/out */
};

extern int qseecom_start_app(struct tts_qseecom_handle **handle, char *app_name, unsigned int size);
extern int qseecom_shutdown_app(struct tts_qseecom_handle **handle);

void lkmauth_restart(void)
{
        struct tts_qseecom_handle *qhandle = NULL;

        qseecom_start_app(&qhandle, "lkmauth", 1024);
        qseecom_shutdown_app(&qhandle);
        qseecom_start_app(&qhandle, "lkmauth", 1024);

}
