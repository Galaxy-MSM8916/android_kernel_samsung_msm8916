/*
 */
			
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include <soc/qcom/scm.h>

unsigned int kap_on_reboot = 0;
/* 1: turn on kap after reboot; 0: no pending ON action */
unsigned int kap_off_reboot = 0;
/* 1: turn off kap after reboot; 0: no pending OFF action */

static void turn_off_kap(void) {
	kap_on_reboot = 0;
	kap_off_reboot = 1;
	printk(KERN_ALERT " %s -> Turn off kap mode\n", __FUNCTION__);
}

static void turn_on_kap(void) {
	kap_off_reboot = 0;
	kap_on_reboot = 1;
	printk(KERN_ALERT " %s -> Turn on kap mode\n", __FUNCTION__);
}

ssize_t knox_kap_write(struct file *file, const char __user *buffer, size_t size, loff_t *offset)
{

	unsigned long mode;
	char *string;

	printk(KERN_ALERT " %s\n", __FUNCTION__);

	string = kmalloc(size + sizeof(char), GFP_KERNEL);

	memcpy(string, buffer, size);
	string[size] = '\0';

	if(kstrtoul(string, 0, &mode)) {
		kfree(string);
		return size;
	}

	kfree(string);

	printk(KERN_ALERT "id: %d\n", (int)mode);

	switch(mode) {
		case 0:
			turn_off_kap();
			break;
		case 1:
			turn_on_kap();
			break;
		default:
			printk(KERN_ERR " %s -> Invalid kap mode operations, %d\n", __FUNCTION__, (int)mode);
			break;
	}

	*offset += size;

	return size;
}

extern int boot_mode_security;

#define KAP_RET_SIZE	5
#define KAP_MAGIC	0x5afe0000
#define KAP_MAGIC_MASK	0xffff0000

static int knox_kap_read(struct seq_file *m, void *v)
{
	unsigned long tz_ret = 0;
	unsigned char ret_buffer[KAP_RET_SIZE];
	unsigned volatile int ret_val;

	tz_ret = kap_status_scm_call();

	if (tz_ret == 0)
		printk(KERN_ERR "Failure : KAP Read STATUS %lx val = %lx\n", __pa(&tz_ret), tz_ret);
	else
		printk(KERN_ERR "Success : KAP Read STATUS %lx val = %lx\n", __pa(&tz_ret), tz_ret);

	if (tz_ret == (KAP_MAGIC | 3)) {
		ret_val = 0x03;	//RKP and/or DMVerity says device is tampered
	} else if (tz_ret == (KAP_MAGIC | 2)) {
		ret_val = 0x2;	//The device is tampered through trusted boot
	} else if (tz_ret == (KAP_MAGIC | 1)) {
		/* KAP is ON*/
		/* Check if there is any pending On/Off action */
		if (kap_off_reboot == 1){
			ret_val = 0x10;	//KAP is ON and will turn OFF upon next reboot
		} else {
			ret_val = 0x11;	//KAP is ON and and no change so far
		}
	} else if (tz_ret == (KAP_MAGIC)) {
		/* KAP is OFF*/
		/* Check if there is any pending On/Off action */
		if (kap_on_reboot == 1){
			ret_val = 0x01;	//KAP is OFF but will turn on upon next reboot
		} else {
			ret_val = 0;	//KAP is OFF and no change so far
		}
	} else {
		ret_val = 0x04;	//The magic string is not there. KAP mode not implemented
	}

	printk(KERN_ERR "knox_kap_read ret_val = %0x\n", ret_val);
	memset(ret_buffer,0,KAP_RET_SIZE);
	snprintf(ret_buffer, sizeof(ret_buffer), "%02x\n", ret_val);
	seq_write(m, ret_buffer, sizeof(ret_buffer));

	return 0;
}

static int knox_kap_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, knox_kap_read, NULL);
}

long knox_kap_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	/* 
	 * Switch according to the ioctl called 
	 */
	switch (cmd) {
		case 0:
			turn_off_kap();
			break;
		case 1:
			turn_on_kap();
			break;
		default:
			printk(KERN_ERR " %s -> Invalid kap mode operations, %d\n", __FUNCTION__, cmd);
			return -1;
			break;
	}

	return 0;
}

const struct file_operations knox_kap_fops = {
	.open	= knox_kap_open,
	.read	= seq_read,
	.write	= knox_kap_write,
	.unlocked_ioctl  = knox_kap_ioctl,
};
