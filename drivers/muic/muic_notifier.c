#include <linux/device.h>

#include <linux/notifier.h>
#include <linux/muic/muic.h>
#include <linux/muic/muic_notifier.h>
#include <linux/sec_class.h>

#define SET_MUIC_NOTIFIER_BLOCK(nb, fn, dev) do {	\
		(nb)->notifier_call = (fn);		\
		(nb)->priority = (dev);			\
	} while (0)

#define DESTROY_MUIC_NOTIFIER_BLOCK(nb)			\
		SET_MUIC_NOTIFIER_BLOCK(nb, NULL, -1)
static struct muic_notifier_struct muic_notifier;

struct device *switch_device;

int muic_notifier_register(struct notifier_block *nb, notifier_fn_t notifier,
			muic_notifier_device_t listener)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s: listener=%d\n", __func__, listener);

	SET_MUIC_NOTIFIER_BLOCK(nb, notifier, listener);
	ret = blocking_notifier_chain_register(&(muic_notifier.notifier_call_chain), nb);
	if (ret < 0)
		printk(KERN_ERR "[muic] notifier_chain_register error(%d)\n", ret);

	/* current muic's attached_device status notify */
	nb->notifier_call(nb, muic_notifier.cmd,
			&(muic_notifier.attached_dev));

	return ret;
}

int muic_notifier_unregister(struct notifier_block *nb)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s: listener=%d unregister\n", __func__, nb->priority);

	ret = blocking_notifier_chain_unregister(&(muic_notifier.notifier_call_chain), nb);
	if (ret < 0)
		printk(KERN_ERR "[muic] notifier_chain_unregister error(%d)\n", ret);
	DESTROY_MUIC_NOTIFIER_BLOCK(nb);

	return ret;
}

static int muic_notifier_notify(void)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s: CMD=%d, DATA=%d\n", __func__, muic_notifier.cmd,
			muic_notifier.attached_dev);

	ret = blocking_notifier_call_chain(&(muic_notifier.notifier_call_chain),
			muic_notifier.cmd, &(muic_notifier.attached_dev));

	switch (ret) {
	case NOTIFY_STOP_MASK:
	case NOTIFY_BAD:
		printk(KERN_ERR "[muic] %s: notify error occur(0x%x)\n", __func__, ret);
		break;
	case NOTIFY_DONE:
	case NOTIFY_OK:
		printk(KERN_DEBUG "[muic] %s: notify done(0x%x)\n", __func__, ret);
		break;
	default:
		printk(KERN_DEBUG "[muic] %s: notify status unknown(0x%x)\n", __func__, ret);
		break;
	}

	return ret;
}

void muic_notifier_attach_attached_dev(muic_attached_dev_t new_dev)
{
	printk(KERN_DEBUG "[muic] %s: (%d)\n", __func__, new_dev);

	muic_notifier.cmd = MUIC_NOTIFY_CMD_ATTACH;
	muic_notifier.attached_dev = new_dev;

	/* muic's attached_device attach broadcast */
	muic_notifier_notify();
}

void muic_notifier_detach_attached_dev(muic_attached_dev_t cur_dev)
{
	printk(KERN_DEBUG "[muic] %s: (%d)\n", __func__, cur_dev);

	muic_notifier.cmd = MUIC_NOTIFY_CMD_DETACH;

	if (muic_notifier.attached_dev != cur_dev)
		printk(KERN_DEBUG "[muic] %s:  muic_notifier(%d) != muic_data(%d)\n",
				__func__, muic_notifier.attached_dev, cur_dev);

	if (muic_notifier.attached_dev != ATTACHED_DEV_NONE_MUIC) {
		/* muic's attached_device detach broadcast */
		muic_notifier_notify();
	}

	muic_notifier.attached_dev = ATTACHED_DEV_NONE_MUIC;
}

void muic_notifier_logically_attach_attached_dev(muic_attached_dev_t new_dev)
{
	printk(KERN_DEBUG "[muic] %s: (%d)\n", __func__, new_dev);

	muic_notifier.cmd = MUIC_NOTIFY_CMD_LOGICALLY_ATTACH;
	muic_notifier.attached_dev = new_dev;

	/* muic's attached_device attach broadcast */
	muic_notifier_notify();
}

void muic_notifier_logically_detach_attached_dev(muic_attached_dev_t cur_dev)
{
	printk(KERN_DEBUG "[muic] %s: (%d)\n", __func__, cur_dev);

	muic_notifier.cmd = MUIC_NOTIFY_CMD_LOGICALLY_DETACH;
	muic_notifier.attached_dev = cur_dev;

	/* muic's attached_device detach broadcast */
	muic_notifier_notify();

	muic_notifier.attached_dev = ATTACHED_DEV_NONE_MUIC;
}

static int __init muic_notifier_init(void)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	switch_device = device_create(sec_class, NULL, 0, NULL, "switch");
	if (IS_ERR(switch_device)) {
		printk(KERN_ERR "[muic] Failed to create device(switch)!\n");
		ret = -ENODEV;
		goto out;
	}

	BLOCKING_INIT_NOTIFIER_HEAD(&(muic_notifier.notifier_call_chain));
	muic_notifier.cmd = MUIC_NOTIFY_CMD_DETACH;
	muic_notifier.attached_dev = ATTACHED_DEV_UNKNOWN_MUIC;

out:
	return ret;
}
device_initcall(muic_notifier_init);

