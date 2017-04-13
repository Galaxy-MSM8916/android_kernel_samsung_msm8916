#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/spi/spi.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/regulator/consumer.h>
#include <linux/ioctl.h>
#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#endif
#include <linux/ese_p3.h>

#include <linux/pm_runtime.h>
#include <linux/spi/spidev.h>
#include <linux/clk.h>
#include <linux/wakelock.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/spi/spidev.h>
#include <asm/uaccess.h>

#define P3_SPI_MAJOR                    121
#define P3_SPI_MINORS                    31

#define P3_MAGIC 0xED
/*To prepare clock spi clock */
#define P3_ENABLE_SPI_CLK _IO(P3_MAGIC, 0x05)
/* To unprepare spi clock */
#define P3_DISABLE_SPI_CLK _IO(P3_MAGIC, 0x06)
/* only nonTZ +++++*/
/* Transmit data to the device and retrieve data from it simultaneously.*/
#define P3_RW_SPI_DATA _IOWR(P3_MAGIC, 0x07, unsigned long)
/* only nonTZ -----*/
/* To change SPI clock */
#define P3_SET_SPI_CLK _IOW(P3_MAGIC, 0x08, unsigned long)
/* To enable spi cs pin (make low) */
#define P3_ENABLE_SPI_CS _IO(P3_MAGIC, 0x09)
/* To disable spi cs pin  */
#define P3_DISABLE_SPI_CS _IO(P3_MAGIC, 0x0A)
/* To enable spi clock & cs */
#define P3_ENABLE_CLK_CS _IO(P3_MAGIC, 0x0B)
/* To disable spi clock & cs */
#define P3_DISABLE_CLK_CS _IO(P3_MAGIC, 0x0C)

#define P3_SWING_CS _IOW(P3_MAGIC, 0x0D, unsigned long)
#define SPI_DEFAULT_SPEED 1000000L
#define MAX_SPI_TX_BUF 512

#define SPI_MODE_MASK           ( SPI_CPHA | SPI_CPOL \
                                  )
static struct spip3_data *g_spip3;
struct spip3_data {
	dev_t                 devt;
	spinlock_t            spi_lock;
	struct spi_device     *spi;
	struct list_head      device_entry;
	/* buffer is NULL unless this device is open (users > 0) */
	struct mutex          buf_lock;
	unsigned              users;
	u8                    *tx_buffer;
	u8                    *rx_buffer;
	unsigned int			nfc_ese_pwr_req;
	/* unsigned int			cspin;*/
	struct miscdevice		p3_device;
	struct wake_lock		ese_lock;
	bool					enable_clock;
};

struct spip3_transfer {
	u8                      *rx_buffer;
	u8                      *tx_buffer;
	size_t                  len;
};

static inline ssize_t
spip3_sync_transceive(struct spip3_data *spip3, size_t len,u8 bpw);


static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);
static int bufsiz = 512;
module_param(bufsiz, uint, S_IRUGO);
MODULE_PARM_DESC(bufsiz, "data bytes in biggest supported SPI message");

static struct spip3_transfer p3trnsfr = {
	.rx_buffer  = 0x00,
	.tx_buffer  = 0x00,
	.len        = 1,
};

static void spip3_complete(void *arg)
{
	complete(arg);
}

static ssize_t
spip3_sync(struct spip3_data *spip3, struct spi_message *message)
{
	DECLARE_COMPLETION_ONSTACK(done);
	int status;

	message->complete = spip3_complete;
	message->context = &done;

	spin_lock_irq(&spip3->spi_lock);
	if (spip3->spi == NULL)
		status = -ESHUTDOWN;
	else
		status = spi_async(spip3->spi, message);
	spin_unlock_irq(&spip3->spi_lock);

	if (status == 0) {
		wait_for_completion(&done);
		status = message->status;
		if (status == 0) {
			status = message->actual_length;
		}
	}
	return status;
}

	static inline ssize_t
spip3_sync_transceive(struct spip3_data *spip3, size_t len,u8 bpw)
{
	struct spi_transfer     t = {
		.rx_buf         = spip3->rx_buffer,
		.tx_buf         = spip3->tx_buffer,
		.len            = len,
		.speed_hz		= SPI_DEFAULT_SPEED,
		.bits_per_word = bpw,
	};
	struct spi_message      m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spip3_sync(spip3, &m);
}

/*-------------------------------------------------------------------------*/

static inline ssize_t spip3_readSync(struct spip3_data *spip3,
		size_t len)
{
	int    status = 0;
	struct spi_message m;
	struct spi_transfer t;

	pr_debug("%s\n", __func__);

	spi_message_init(&m);
	memset(&t, 0x0, sizeof(t));

	memset(spip3->tx_buffer, 0x0, len);
	t.tx_buf = spip3->tx_buffer;
	t.rx_buf = spip3->rx_buffer;
	t.len = len;
	t.speed_hz = SPI_DEFAULT_SPEED;

	spi_message_add_tail(&t, &m);

	status = spi_sync(spip3->spi, &m);

	if (status == 0)
		status = len;

	pr_debug("%s spip3,length=%d\n", __func__, (int)len);

	return status;
}

static inline ssize_t spip3_writeSync(struct spip3_data *spip3,
		size_t len)
{
	int    status = 0;
	struct spi_message m;
	struct spi_transfer t;

	pr_debug("%s\n", __func__);

	spi_message_init(&m);
	memset(&t, 0, sizeof(t));

	t.rx_buf = spip3->rx_buffer;
	t.tx_buf = spip3->tx_buffer;
	t.len = len;
	t.speed_hz = SPI_DEFAULT_SPEED;

	spi_message_add_tail(&t, &m);

	status = spi_sync(spip3->spi, &m);

	if (status == 0)
		status = m.actual_length;
	pr_debug("%s spip3,length=%d\n", __func__, m.actual_length);
	return status;
}

	static ssize_t
spip3_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct spip3_data *spip3;
	ssize_t status = 0;

	if (count > MAX_SPI_TX_BUF)
		return -EMSGSIZE;

	spip3 = filp->private_data;

	mutex_lock(&spip3->buf_lock);
	status  = spip3_readSync(spip3, count);
	if (status > 0) {
		unsigned long   missing = 0;
		/* data read. Copy to user buffer.*/
		missing = copy_to_user(buf, spip3->rx_buffer, status);
		if (missing == status)
			status = -EFAULT;
		else
			status = status - missing;
	}
	mutex_unlock(&spip3->buf_lock);
	return status;
}

static ssize_t spip3_write(struct file *filp, const char *buf, size_t count,
		loff_t *f_pos)
{
	struct spip3_data *spip3;
	ssize_t               status = 0;

	pr_debug("spip3_dev_write -Enter count %zu\n", count);

	if (count > MAX_SPI_TX_BUF)
		return -EMSGSIZE;

	spip3 = filp->private_data;

	mutex_lock(&spip3->buf_lock);

	if (spip3->tx_buffer) {
		unsigned long missing = 0;
		missing = copy_from_user(spip3->tx_buffer, buf, count);
		if (missing == 0)
			status = spip3_writeSync(spip3, count);
		else
			status = -EFAULT;
	}
	mutex_unlock(&spip3->buf_lock);
	return status;
}

static int spip3_rdwr_transfer(struct spip3_data *spip3,struct spip3_transfer *u_xfers)
{
	struct spi_message      msg;
	char tempbuf[8];
	struct spip3_transfer   *u_tmp;
	unsigned	n =1, total;
	int  status = -EFAULT;
	unsigned n_xfers =1;

	spi_message_init(&msg);

	total = 0;
	n = n_xfers, u_tmp = u_xfers;
	total += 1;

	if (total > MAX_SPI_TX_BUF) {
		status = -EMSGSIZE;
		return status;
	}

	if (u_tmp->rx_buffer) {
		if (!access_ok(VERIFY_WRITE, (u8 __user *)
			(uintptr_t) u_tmp->rx_buffer,
			u_tmp->len)) {
			return status;
		}
	}

	if (u_tmp->tx_buffer) {
		status = copy_from_user(spip3->tx_buffer, (const u8 __user *)
				(uintptr_t)u_tmp->tx_buffer, u_xfers->len);
		if (status !=0) {
			goto done;
		}
	}

	status = spip3_sync_transceive(spip3,u_xfers->len, 8);
	u_tmp = u_xfers;

	memset(tempbuf, 0, 8);
	memcpy(tempbuf, spip3->rx_buffer, 8);
	printk("spip3_rdwr_transfer 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
	tempbuf[0], tempbuf[1], tempbuf[2], tempbuf[3],
	tempbuf[4], tempbuf[5], tempbuf[6], tempbuf[7]);

	if (u_tmp->rx_buffer) {
		status = __copy_to_user((u8 __user *) (uintptr_t) u_tmp->rx_buffer, spip3->rx_buffer, u_xfers->len);
		if(status!=0) {
			status = -EFAULT;
			goto done;
		}
	}

	done:
	return status;
}


static int p3_enable_clk(struct spip3_data *spip3)
{
	int ret_val = 0;
	struct spi_device *spidev = NULL;

	spidev = spi_dev_get(spip3->spi);


	if (!wake_lock_active(&spip3->ese_lock)) {
		pr_info("%s: [NFC-ESE] wake lock.\n", __func__);
		wake_lock(&spip3->ese_lock);
	}

	/* Qcom spi active pinctrl */
	ret_val = ese_spi_request_gpios(spidev);
	if (ret_val < 0)
		pr_err("%s: couldn't config spi gpio\n", __func__);
	usleep_range(200, 230);

	spidev->max_speed_hz = SPI_DEFAULT_SPEED;
	ret_val = ese_spi_clock_enable(spidev);
	if (ret_val < 0)
		pr_err("%s: Unable to enable spi clk\n",
			__func__);
	else {
		ret_val = ese_spi_clock_set_rate(spidev);
		if (ret_val < 0)
			pr_err("%s: Unable to set spi clk rate\n",
				__func__);
	}

	spip3->enable_clock = true;

	return 0;
}

static int p3_disable_clk(struct spip3_data *spip3)
{
	int ret_val = 0;
	struct spi_device *spidev = NULL;

	spidev = spi_dev_get(spip3->spi);

	if (wake_lock_active(&spip3->ese_lock)) {
		pr_info("%s: [NFC-ESE] wake unlock.\n", __func__);
		wake_unlock(&spip3->ese_lock);
	}

	ret_val = ese_spi_clock_disable(spidev);
	if (ret_val < 0)
		pr_err("%s: couldn't disable spi clks\n", __func__);

	spip3->enable_clock = false;

	return 0;
}


static long
spip3_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0, speed;
	int retval = -EFAULT;
	struct spip3_data *spip3;
	struct spi_device *spi;

	int status = -1;

	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC && _IOC_TYPE(cmd) != P3_MAGIC)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,
						(void __user *)arg, _IOC_SIZE(cmd));

	if (err == 0 && _IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ,
						(void __user *)arg, _IOC_SIZE(cmd));

	if (err)
		return -EFAULT;

	spip3 = filp->private_data;
	spin_lock_irq(&spip3->spi_lock);
	spi = spi_dev_get(spip3->spi);
	spin_unlock_irq(&spip3->spi_lock);

	if (spi == NULL) {
		return -ESHUTDOWN;
	}

	mutex_lock(&spip3->buf_lock);

	switch (cmd) {
		case  P3_ENABLE_SPI_CLK:
			pr_info("** %s P3_ENABLE_SPI_CLK\n", __func__);
			retval = p3_enable_clk(spip3);
			break;

		case P3_DISABLE_SPI_CLK :
			pr_info("** %s P3_DISABLE_SPI_CLK\n", __func__);
			retval = p3_disable_clk(spip3);
			break;

		case P3_RW_SPI_DATA :
			pr_info("** %s P3_RW_SPI_DATA\n", __func__);
			status = __copy_from_user(&p3trnsfr, (void __user *)arg, sizeof(p3trnsfr));
			if( status ==0){
				status = spip3_rdwr_transfer(spip3,&p3trnsfr);
				if (status ==0) {
					retval= 0;
				}
			}
			break;

		case P3_SET_SPI_CLK :
			speed = (int) arg;
			/*speed below 4Mhz doesnt work on APQ8064 - 4000000 value sets the clock to 1Mhz*/
			if(speed < 4000000 )
				speed = 4000000;

			spi->max_speed_hz = speed;
			retval = 0;
			pr_info("** %s P3 P3_SET_SPI_CLK %d\n", __func__, speed);
			break;

		/* To enable spi cs pin (make low) */
		case P3_ENABLE_SPI_CS :
			pr_info("** %s P3_ENABLE_SPI_CS!!!\n", __func__);
			/*gpio_set_value(spip3->cspin, 0);*/
			retval = 0;
			break;

		/* To disable spi cs pin  */
		case P3_DISABLE_SPI_CS :
			pr_info("** %s P3_DISABLE_SPI_CS!!!\n", __func__);
			/*gpio_set_value(spip3->cspin, 1);*/
			retval = 0;
			break;

		/* To enable spi clock & cs */
		case P3_ENABLE_CLK_CS :
			pr_info("** %s P3_ENABLE_CLK_CS\n", __func__);
			retval= 0;
			break;

		/* To disable spi clock & cs */
		case  P3_DISABLE_CLK_CS:
			pr_info("** %s P3 P3_DISABLE_CLK_CS\n", __func__);
			retval= 0;
			break;

		case P3_SWING_CS :
			printk("** %s P3_SWING_CS\n", __func__);
			spip3->tx_buffer[0] = 0;
			status = spip3_sync_transceive(spip3,1,arg);
			retval=0;
			break;

		default:
			break;
	}

	mutex_unlock(&spip3->buf_lock);
	spi_dev_put(spi);

	return retval;
}

static int spip3_open(struct inode *inode, struct file *filp)
{
	int	status = -ENXIO;
	struct spip3_data *spip3 = container_of(filp->private_data, struct spip3_data, p3_device);
	struct spi_device *spi;

	pr_info("%s start.\n", __func__);

	if (!spip3->tx_buffer) {
		spip3->tx_buffer = kmalloc(MAX_SPI_TX_BUF, GFP_KERNEL);
		if (!spip3->tx_buffer) {
			dev_dbg(&spip3->spi->dev, "open/ENOMEM\n");
			status = -ENOMEM;
		}
	}
	if (!spip3->rx_buffer) {
		spip3->rx_buffer = kmalloc(MAX_SPI_TX_BUF, GFP_KERNEL);
		if (!spip3->rx_buffer) {
			dev_dbg(&spip3->spi->dev, "open/ENOMEM\n");
			status = -ENOMEM;
		}
	}
	spip3->users++;
	filp->private_data = spip3;
	nonseekable_open(inode, filp);
	spi = spi_dev_get(g_spip3->spi);

	pr_info("%s : open and ese_pwr=%d\n", __func__,
			gpio_get_value(g_spip3->nfc_ese_pwr_req));

	return 0;
}

static int spip3_release(struct inode *inode, struct file *filp)
{
	struct spip3_data      *spip3;
	int                     status = 0;
	struct spi_device *spidev = NULL;

	mutex_lock(&device_list_lock);
	spip3 = filp->private_data;
	filp->private_data = NULL;

	pr_info("%s: [NFC-ESE]\n", __func__);

	spidev = spi_dev_get(spip3->spi);

	if (spip3->enable_clock) {
		pr_info("%s: [NFC-ESE] disable clock.\n", __func__);
		p3_disable_clk(spip3);
	}

	/* last close? */
	spip3->users--;
	if (!spip3->users) {
		int dofree;

		kfree(spip3->tx_buffer);
		spip3->tx_buffer = NULL;
		kfree(spip3->rx_buffer);
		spip3->rx_buffer = NULL;

		dofree = (spip3->spi == NULL);

		if (dofree)
			kfree(spip3);
	}
	mutex_unlock(&device_list_lock);

	return status;
}

static struct file_operations spip3_fops = {
	.owner =        THIS_MODULE,
	.read =         spip3_read,
	.write =         spip3_write,
	.unlocked_ioctl = spip3_ioctl,
	.open =         spip3_open,
	.release =      spip3_release,
};

#ifdef CONFIG_OF
static int p3_parse_dt(struct device *dev,
	struct spip3_data *data)
{
struct device_node *np = dev->of_node;
enum of_gpio_flags flags;
int ret = -1;

	data->nfc_ese_pwr_req = of_get_named_gpio_flags(np,
		"p3-ese_pwr_req", 0, &flags);

	if (data->nfc_ese_pwr_req < 0) {
		pr_info("%s - fail get nfc_ese_pwr_req\n", __func__);
		return -1;
	}

	ret = gpio_request(data->nfc_ese_pwr_req, "ese_pwr_req");
	if (ret) {
		pr_info("%s - failed to request ese_pwr_req\n", __func__);
	}

	gpio_direction_output(data->nfc_ese_pwr_req, 0);
	return ret;
}
#endif

static int spip3_probe(struct spi_device *spi)
{
	struct spip3_data      *spip3 = NULL;

	int                    ret;
	spip3 = kzalloc(sizeof(*spip3), GFP_KERNEL);
	if (!spip3)
		return -ENOMEM;

	spip3->spi = spi;

	ret = p3_parse_dt(&spi->dev, spip3);
	if (ret) {
		pr_info("%s - Failed to parse DT\n", __func__);
	}

	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0; /*SPI_MODE_3*/
	spi->max_speed_hz = SPI_DEFAULT_SPEED;
	ret = spi_setup(spi);
	if (ret < 0) {
		pr_info("%s failed to do spi_setup()\n",__func__);
	}

	spip3->spi = spi;
	spip3->p3_device.minor = MISC_DYNAMIC_MINOR;
	spip3->p3_device.name = "p3";
	spip3->p3_device.fops = &spip3_fops;
	spip3->p3_device.parent = &spi->dev;

	dev_set_drvdata(&spi->dev, spip3);
	ret = misc_register(&spip3->p3_device);
	if (ret < 0) {
		pr_info("misc_register failed! %d\n", ret);
	}
	spin_lock_init(&spip3->spi_lock);
	mutex_init(&spip3->buf_lock);

	INIT_LIST_HEAD(&spip3->device_entry);

	list_add(&spip3->device_entry, &device_list);
	g_spip3 = dev_get_drvdata(&spi->dev);

	/*wake lock for spi communication*/
	wake_lock_init(&spip3->ese_lock, WAKE_LOCK_SUSPEND, "ese_wake_lock");

	gpio_set_value(spip3->nfc_ese_pwr_req, 1);

	spip3->enable_clock = 0;
	pr_info("spip3_probe success\n");

	return 0;
}

static int spip3_remove(struct spi_device *spi)
{
	struct spip3_data *spip3 = dev_get_drvdata(&spi->dev);

	wake_lock_destroy(&spip3->ese_lock);

	return 0;
}

static struct of_device_id p3_match_table[] = {
	 { .compatible = "p3",},
	 {},
};

struct spi_driver spip3_spi = {
		.driver = {
			.name =         "p3",
			.owner =        THIS_MODULE,
			.bus = &spi_bus_type,
#ifdef CONFIG_OF
			.of_match_table = p3_match_table,
#endif
		},
		.probe =        spip3_probe,
		.remove =        spip3_remove,
};

static int __init spip3_init(void)
{
	pr_info("***spip3_init\n");
	spi_register_driver(&spip3_spi);

	return 1;
}

static void __exit spip3_exit(void)
{
	spi_unregister_driver(&spip3_spi);
}


module_init(spip3_init);
module_exit(spip3_exit);

MODULE_AUTHOR("Sec");
MODULE_DESCRIPTION("User mode SPI device interface");
MODULE_LICENSE("GPL");
