/*
 * Copyright (C) 2015 Samsung Electronics, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/errno.h>	/* cma.h needs this */
#if defined(CONFIG_TZDEV_CMA)
#if defined(CONFIG_DMA_CMA)
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#elif !defined(CONFIG_ARCH_MSM)
#include <linux/cma.h>
#endif
#endif
#include <linux/completion.h>
#include <linux/cpu.h>
#include <linux/dma-mapping.h>
#include <linux/file.h>
#include <linux/highmem.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/list.h>
#include <linux/migrate.h>
#include <linux/miscdevice.h>
#include <linux/mmzone.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/syscore_ops.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>

#include "sysdep.h"
#include "tzdev.h"
#include "tzlog.h"
#if defined(CONFIG_TZDEV_QC_CRYPTO_CLOCKS_MANAGEMENT)
#include "tzpm.h"
#endif
#include "tz_core_migration.h"
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

MODULE_AUTHOR("Jaemin Ryu <jm77.ryu@samsung.com>");
MODULE_AUTHOR("Vasily Leonenko <v.leonenko@samsung.com>");
MODULE_AUTHOR("Alex Matveev <alex.matveev@samsung.com>");
MODULE_DESCRIPTION("TZDEV driver");
MODULE_LICENSE("GPL");

int tzdev_verbosity = 0;

struct hrtimer tzdev_get_event_timer;

module_param(tzdev_verbosity, int, 0644);
MODULE_PARM_DESC(tzdev_verbosity, "0: normal, 1: verbose, 2: debug");

static DEFINE_MUTEX(tzdev_fd_mutex);
static int tzdev_sw_init_done = 0;
static int tzdev_fd_open;

static DEFINE_IDR(tzdev_mem_map);
static DEFINE_MUTEX(tzdev_mem_mutex);
static DECLARE_COMPLETION(tzdev_ow_comp);

#if defined(CONFIG_QCOM_SCM_ARMV8)
DEFINE_MUTEX(tzdev_smc_lock);
#endif

#if defined(CONFIG_TZDEV_QC_CRYPTO_CLOCKS_USR_MNG)
static int tzdev_qc_clk = 0;
static DEFINE_MUTEX(tzdev_qc_clk_mutex);
#endif

struct tzdev_mem_reg {
	unsigned long nr_pages;
	sk_pfn_t *pfns;
	struct list_head list;
};

#if defined(CONFIG_TZDEV_CMA)
#if defined(CONFIG_CMA) && !defined(CONFIG_DMA_CMA)
static struct cma_info tzdev_cma_info;
#elif defined(CONFIG_DMA_CMA)
struct page *tzdev_page = NULL;
#endif
static dma_addr_t tzdev_cma_addr = 0;
#endif

#if defined(CONFIG_TZLOG)
static DEFINE_SPINLOCK(tzdev_log_slock);
static DEFINE_PER_CPU(struct tzio_log_channel *, tzdev_log_channel);
#endif

#ifdef CONFIG_TZDEV_MULTI_CORE_AUX_CHANNELS

static DEFINE_PER_CPU(struct tzio_aux_channel *, tzdev_aux_channel);

#define aux_channel_get(ch)		get_cpu_var(ch)
#define aux_channel_put(ch)		put_cpu_var(ch)
#define aux_channel_init(ch, cpu)	per_cpu(ch, cpu)

#else /* CONFIG_TZDEV_MULTI_CORE_AUX_CHANNELS */

static DEFINE_MUTEX(tzdev_aux_channel_lock);
static struct tzio_aux_channel *tzdev_aux_channel[NR_CPUS];

static struct tzio_aux_channel *aux_channel_get(struct tzio_aux_channel *ch[])
{
	mutex_lock(&tzdev_aux_channel_lock);
	BUG_ON(smp_processor_id() != 0);
	return ch[0];
}

#define aux_channel_put(ch)		mutex_unlock(&tzdev_aux_channel_lock)
#define aux_channel_init(ch, cpu)	ch[cpu]

#endif /* CONFIG_TZDEV_MULTI_CORE_AUX_CHANNELS */

#if defined(CONFIG_TZLOG)

#define TZDEV_OW_LINE_MAX_LEN	256
#define TZDEV_OW_PREFIX		KERN_DEFAULT "SW> "

struct tzdev_log_print_state {
	char line[TZDEV_OW_LINE_MAX_LEN + 1];	/* one byte for \0 */
	unsigned int line_len;
};

DEFINE_PER_CPU(struct tzdev_log_print_state, tzdev_log_print_state);

static int tzio_log_channel_print_debug(const char *buf, unsigned int count)
{
	struct tzdev_log_print_state *ps;
	unsigned int count_out, avail, bytes_in, bytes_out, bytes_printed, tmp, wait_dta;
	char *p;

	ps = &get_cpu_var(tzdev_log_print_state);

	count_out = count;

	wait_dta = 0;
	while (count_out) {
		avail = TZDEV_OW_LINE_MAX_LEN - ps->line_len;

		p = memchr(buf, '\n', count_out);

		if (p) {
			if (p - buf > avail) {
				bytes_in = avail;
				bytes_out = avail;
			} else {
				bytes_in = p - buf + 1;
				bytes_out = p - buf;
			}
		} else {
			if (count_out >= avail) {
				bytes_in = avail;
				bytes_out = avail;
			} else {
				bytes_in = count_out;
				bytes_out = count_out;

				wait_dta = 1;
			}
		}

		memcpy(&ps->line[ps->line_len], buf, bytes_out);
		ps->line_len += bytes_out;

		if (wait_dta)
			break;

		ps->line[ps->line_len] = 0;

		bytes_printed = 0;
		while (bytes_printed < ps->line_len) {
			tmp = printk(TZDEV_OW_PREFIX "%s\n", &ps->line[bytes_printed]);
			if (!tmp)
				break;

			bytes_printed += tmp;
		}

		ps->line_len = 0;

		count_out -= bytes_in;
		buf += bytes_in;
	}

	put_cpu_var(tzdev_log_print_state);

	return count;
}

void tzio_log_channel_read_debug(void)
{
	struct tzio_log_channel *ch;
	unsigned int i, bytes, count;

	spin_lock(&tzdev_log_slock);

	for (i = 0; i < nr_cpu_ids; ++i) {
		ch = per_cpu(tzdev_log_channel, i);
		if (!ch)
			continue;

		if (ch->write_count < ch->read_count) {
			count = TZDEV_LOG_BUF_SIZE - ch->read_count;

			bytes = tzio_log_channel_print_debug(ch->buffer + ch->read_count, count);
			if (bytes < count) {
				ch->read_count += bytes;
				continue;
			}

			ch->read_count = 0;
		}

		count = ch->write_count - ch->read_count;

		bytes = tzio_log_channel_print_debug(ch->buffer + ch->read_count, count);
		ch->read_count += count;
	}

	spin_unlock(&tzdev_log_slock);
}

static void tzdev_alloc_log_channel(int cpu)
{
	struct tzio_log_channel *channel;
	struct page *page[CONFIG_TZLOG_PG_CNT];
	sk_pfn_t pfns[CONFIG_TZLOG_PG_CNT];
	struct tzio_aux_channel *aux_ch;
	int i;

	/* Allocate non-contiguous buffer to reduce page allocator pressure */
	for (i = 0; i < CONFIG_TZLOG_PG_CNT; i++) {
		page[i] = alloc_page(GFP_KERNEL);
		if (!page[i]) {
			tzdev_print(0, "TZDev channel creation failed\n");
			goto free_buffer;
		}
		pfns[i] = (sk_pfn_t)(page_to_phys(page[i]) >> PAGE_SHIFT);
	}

	channel = vmap(page, CONFIG_TZLOG_PG_CNT, VM_MAP, PAGE_KERNEL);

	if (!channel) {
		tzdev_print(0, "TZDev channel mapping failed\n");
		goto free_buffer;
	}

	tzdev_print(0, "IRQ Channel[%d] = 0x%p\n", cpu, channel);

	/* Push PFNs list into aux channel */
	aux_ch = aux_channel_get(tzdev_aux_channel);
	memcpy(aux_ch->buffer, &pfns, CONFIG_TZLOG_PG_CNT * sizeof(sk_pfn_t));

	if (tzdev_smc_connect(TZDEV_CONNECT_LOG, 0, CONFIG_TZLOG_PG_CNT)) {
		aux_channel_put(tzdev_aux_channel);
		vunmap(channel);
		tzdev_print(0, "TZDev log channel registration failed\n");
		goto free_buffer;
	}

	aux_channel_put(tzdev_aux_channel);

	per_cpu(tzdev_log_channel, cpu) = channel;

	tzdev_print(0, ">>>>>>>>>>>>>>>>>>>>. TZDEV CPU : %d\n", cpu);

	return;

free_buffer:
	for (i = 0; (i < CONFIG_TZLOG_PG_CNT) && page[i]; i++)
						__free_page(page[i]);
}
#endif

static void tzdev_alloc_aux_channel(int cpu)
{
	struct tzio_aux_channel *channel;
	struct page *page;

	page = alloc_page(GFP_KERNEL);
	if (!page) {
		tzdev_print(0, "TZDev channel creation failed\n");
		return;
	}

	channel = page_address(page);

	tzdev_print(0, "AUX Channel[%d] = 0x%p\n", cpu, channel);

	if (tzdev_smc_connect(TZDEV_CONNECT_AUX, page_to_phys(page), 1)) {
		__free_page(page);
		tzdev_print(0, "TZDev AUX channel registration failed\n");
		return;
	}

	aux_channel_init(tzdev_aux_channel, cpu) = channel;

	tzdev_print(0, ">>>>>>>>>>>>>>>>>>>>. TZDEV CPU : %d\n", cpu);
}

#ifdef CONFIG_TZLOG_POLLING
static void tzio_log_bh(struct work_struct *work)
{
	tzio_log_channel_read_debug();
	schedule_delayed_work(&tzio_log_work,
			      CONFIG_TZLOG_POLLING_PERIOD * HZ / MSEC_PER_SEC);
}

DECLARE_DELAYED_WORK(tzio_log_work, tzio_log_bh);
#endif

#if CONFIG_TZDEV_IWI_PANIC
static void dump_kernel_panic_bh(struct work_struct *work)
{
       tzio_log_channel_read_debug();
}

static DECLARE_WORK(dump_kernel_panic, dump_kernel_panic_bh);
#endif

static int __tzdev_mem_free(int id, void *p, void *data)
{
	struct tzdev_mem_reg *mem = p;
	unsigned long i;

	for(i = 0; i < mem->nr_pages; i++)
		put_page(pfn_to_page(mem->pfns[i]));

	kfree(mem->pfns);
	kfree(mem);

	return 0;
}

static int tzdev_open(struct inode *inode, struct file *filp)
{
#if defined(CONFIG_TZDEV_CMA)
	struct page *p;
	void *ptr;
	unsigned long pfn;
#endif
	int ret = 0;
	unsigned int i;

	tzdev_migrate();

	mutex_lock(&tzdev_fd_mutex);
	if (tzdev_fd_open != 0) {
		ret = -EBUSY;
		goto out;
	}

	if (!tzdev_sw_init_done) {
		/* check kernel and driver version compatibility with quark */
		ret = tzdev_smc_check_version();
		if (ret) {
			tzdev_print(0, "The version of the Linux kernel or TZDev driver is not compatible with Quark secure kernel\n");
			goto out;
		}

#if defined(CONFIG_TZDEV_CMA)
		if (tzdev_cma_addr != 0) {
			for (pfn = __phys_to_pfn(tzdev_cma_addr);
				pfn < __phys_to_pfn(tzdev_cma_addr +
					CONFIG_TZDEV_MEMRESSZ - CONFIG_TZDEV_MEMRESSZPROT);
				pfn++) {
				p = pfn_to_page(pfn);
				ptr = kmap(p);
				/*
				 * We can just invalidate here, but kernel doesn't
				 * export cache invalidation functions.
				 */
				__flush_dcache_area(ptr, PAGE_SIZE);
				kunmap(p);
			}
			outer_inv_range(tzdev_cma_addr, tzdev_cma_addr +
				CONFIG_TZDEV_MEMRESSZ - CONFIG_TZDEV_MEMRESSZPROT);

			ret = tzdev_smc_mem_reg((unsigned long)tzdev_cma_addr,
				(unsigned long)(get_order(CONFIG_TZDEV_MEMRESSZ) + PAGE_SHIFT));
			if (ret) {
				tzdev_print(0, "Registration of CMA region in SW failed;\n");
				goto out;
			}
		}
#endif

		for (i = 0; i < nr_cpu_ids; ++i)
			tzdev_alloc_aux_channel(i);

#if defined(CONFIG_TZLOG)
		for (i = 0; i < nr_cpu_ids; ++i)
			tzdev_alloc_log_channel(i);
#endif

#if defined(CONFIG_TZDEV_QC_CRYPTO_CLOCKS_MANAGEMENT)
		tzdev_qc_pm_clock_initialize();
#endif
		tzdev_sw_init_done = 1;
	}

	tzdev_fd_open++;
	tzdev_smc_nwd_alive();

out:
	mutex_unlock(&tzdev_fd_mutex);
	return ret;
}

static int tzdev_release(struct inode *inode, struct file *filp)
{
	mutex_lock(&tzdev_fd_mutex);

#if defined(CONFIG_TZDEV_QC_CRYPTO_CLOCKS_USR_MNG)
	if (tzdev_qc_clk == QSEE_CLK_ON) {
		tzdev_qc_pm_clock_disable();
		tzdev_qc_clk = QSEE_CLK_OFF;
	}
#endif

	tzdev_fd_open--;
	BUG_ON(tzdev_fd_open);

	tzdev_smc_nwd_dead();

	mutex_lock(&tzdev_mem_mutex);
	idr_for_each(&tzdev_mem_map, __tzdev_mem_free, NULL);

	IDR_REMOVE_ALL(&tzdev_mem_map);

	idr_destroy(&tzdev_mem_map);
	mutex_unlock(&tzdev_mem_mutex);

	mutex_unlock(&tzdev_fd_mutex);
	return 0;
}

#if defined(CONFIG_TZDEV_PAGE_MIGRATION)
struct page *tzdev_alloc_kernel_page(struct page *page, unsigned long private, int **x)
{
	return alloc_page(GFP_KERNEL);
}

int isolate_lru_page(struct page *page);
#endif

static int get_user_pages_fast_task(struct task_struct *tsk, unsigned long start,
				int nr_pages, int write, struct page **pages)
{
	struct mm_struct *mm;
	int ret;

	mm = get_task_mm(tsk);
	if (!mm)
		return 0;

	down_read(&mm->mmap_sem);
	ret = get_user_pages(tsk, mm, start, nr_pages, write, 0, pages, NULL);
	up_read(&mm->mmap_sem);

	mmput(mm);

	return ret;
}

int tzdev_get_access_info(struct tzio_access_info *s)
{
	struct task_struct *task;
	struct mm_struct *mm;
	struct file *exe_file;

	rcu_read_lock();

	task = find_task_by_vpid(s->pid);
	if (!task) {
		rcu_read_unlock();
		return -ESRCH;
	}

	get_task_struct(task);
	rcu_read_unlock();

	s->gid = task->tgid;

	mm = get_task_mm(task);
	put_task_struct(task);
	if (!mm)
		return -ESRCH;

	exe_file = get_mm_exe_file(mm);
	mmput(mm);
	if (!exe_file)
		return -ESRCH;

	strncpy(s->ca_name, exe_file->f_path.dentry->d_name.name, CA_ID_LEN);
	fput(exe_file);

	return 0;
}

int tzdev_mem_register(struct tzio_mem_register *s)
{
#define TZDEV_PFNS_PER_PAGE	(PAGE_SIZE / sizeof(sk_pfn_t))
	struct task_struct *task;
	struct page **pages;
	struct tzdev_mem_reg *mem;
	sk_pfn_t *pfns;
	struct tzio_aux_channel *ch;
	unsigned long start, end, nr_pages, offset;
	int ret = 0, res, i, id;
#if defined(CONFIG_TZDEV_PAGE_MIGRATION)
	LIST_HEAD(mem_pages_list);
#endif

	start = (unsigned long)s->ptr >> PAGE_SHIFT;
	end = ((unsigned long)s->ptr + s->size + PAGE_SIZE - 1) >> PAGE_SHIFT;
	nr_pages = (s->size ? end - start : 0);
	offset = ((unsigned long)s->ptr) & ~PAGE_MASK;

	rcu_read_lock();

	task = find_task_by_vpid(s->pid);
	if (!task) {
		rcu_read_unlock();
		ret = -EINVAL;
		goto out;
	}

	get_task_struct(task);
	rcu_read_unlock();

	pages = kcalloc(nr_pages, sizeof(struct page *), GFP_KERNEL);
	if (!pages) {
		ret = -ENOMEM;
		goto out_task;
	}

	pfns = kmalloc(nr_pages * sizeof(sk_pfn_t), GFP_KERNEL);
	if (!pfns) {
		ret = -ENOMEM;
		goto out_pages;
	}

	mem = kmalloc(sizeof(struct tzdev_mem_reg), GFP_KERNEL);
	if (!mem) {
		ret = -ENOMEM;
		goto out_pfns;
	}

	res = get_user_pages_fast_task(task, (unsigned long)s->ptr,
			nr_pages, s->write, pages);
	if (res != nr_pages) {
		for (i = 0; i < res; i++)
			put_page(pages[i]);
		ret = -EINVAL;
		goto out_mem;
	}

#if defined(CONFIG_TZDEV_PAGE_MIGRATION)
	/*
	 * In case of enabled migration it is possible that userspace pages
	 * will be migrated from current physical page to some other even they are mlocked
	 * (e.g. during memory compaction)
	 * To avoid fails of CMA migrations we have to move pages to other
	 * region which can not be inside any CMA region. This is done by
	 * allocations with GFP_KERNEL flag to point UNMOVABLE memblock
	 * to be used for such allocations.
	 */
	for (i = 0; i < nr_pages; i++) {
		if (!is_cma_pageblock(pages[i])) {
			put_page(pages[i]);
			continue;
		}

		/* pages should be isolated before migration */
		res = isolate_lru_page(pages[i]);

		/* page was taken in get_user_pages function */
		put_page(pages[i]);

		if (res)
			continue;
		list_add_tail(&pages[i]->lru, &mem_pages_list);
	}

	res = migrate_pages(&mem_pages_list, tzdev_alloc_kernel_page, 0, MIGRATE_SYNC, MR_MEMORY_FAILURE);

	if (res) {
		putback_lru_pages(&mem_pages_list);
		ret = -EFAULT;
		goto out_mem;
	}

	/* get updated page locations after migration */
	res = get_user_pages_fast_task(task, (unsigned long)s->ptr,
			nr_pages, s->write, pages);
	if (res != nr_pages) {
		for (i = 0; i < res; i++)
			put_page(pages[i]);
		ret = -EINVAL;
		goto out_mem;
	}
	for (i = 0; i < nr_pages; i++) {
		if (is_cma_pageblock(pages[i]))
			tzdev_print(0, "Fail to migrate memory from CMA\n");
	}
#endif

	for (i = 0; i < nr_pages; i++)
		pfns[i] = (sk_pfn_t)page_to_pfn(pages[i]);

	mutex_lock(&tzdev_mem_mutex);
	res = sysdep_idr_alloc(&tzdev_mem_map, mem);
	if (res < 0) {
		ret = res;
		mutex_unlock(&tzdev_mem_mutex);
		goto out_mem;
	} else
		id = res;

	ch = aux_channel_get(tzdev_aux_channel);

	for (i = 0; i < nr_pages; i += TZDEV_PFNS_PER_PAGE) {
		memcpy(ch->buffer, &pfns[i], min(nr_pages - i, TZDEV_PFNS_PER_PAGE) * sizeof(sk_pfn_t));
		if (tzdev_smc_shmem_list_reg(id, nr_pages, s->write)) {
			tzdev_smc_shmem_list_rls(id);
			aux_channel_put(tzdev_aux_channel);
			idr_remove(&tzdev_mem_map, id);
			mutex_unlock(&tzdev_mem_mutex);
			ret = -EFAULT;
			goto out_mem;
		}
	}

	aux_channel_put(tzdev_aux_channel);

	mem->nr_pages = nr_pages;
	mem->pfns = pfns;

	mutex_unlock(&tzdev_mem_mutex);

	s->id = id;

	goto out_pages;

out_mem:
	kfree(mem);
out_pfns:
	kfree(pfns);
out_pages:
	kfree(pages);
out_task:
	put_task_struct(task);
out:
	return ret;
}

static int tzdev_mem_release(int id)
{
	struct tzdev_mem_reg *mem;

	mutex_lock(&tzdev_mem_mutex);

	tzdev_smc_shmem_list_rls(id);

	mem = idr_find(&tzdev_mem_map, id);
	if (!mem) {
		mutex_unlock(&tzdev_mem_mutex);
		return -EINVAL;
	}

	idr_remove(&tzdev_mem_map, id);

	mutex_unlock(&tzdev_mem_mutex);

	__tzdev_mem_free(id, mem, NULL);

	return 0;
}

static int tzdev_wait_evt(unsigned long *dta)
{
	int rc;
	struct timespec ts;

retry:
	rc = wait_for_completion_interruptible(&tzdev_ow_comp);
	if (rc == -ERESTARTSYS)
		return rc;

	getnstimeofday(&ts);

	*dta = tzdev_smc_get_event(ts.tv_sec, ts.tv_nsec);
	if (!*dta)
		goto retry;

	return 0;
}

static long tzdev_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	tzdev_migrate();

	switch (cmd) {
	case TZIO_SMC: {
		const struct tzio_smc_cmd __user *argp = (const struct tzio_smc_cmd __user *)arg;
		struct tzio_smc_cmd s;
		if (copy_from_user(&s, argp, sizeof(struct tzio_smc_cmd))) {
			ret = -EFAULT;
			break;
		}
		s.ret = tzdev_smc_command(s.args[0], s.args[1]);
		if (put_user(s.ret, &argp->ret)) {
			ret = -EFAULT;
			break;
		}
		break;
	}
	case TZIO_GET_ACCESS_INFO: {
		struct tzio_access_info __user *argp = (struct tzio_access_info __user *)arg;
		struct tzio_access_info s;

		if (copy_from_user(&s, argp, sizeof(struct tzio_access_info))) {
			ret = -EFAULT;
			break;
		}
		ret = tzdev_get_access_info(&s);
		if (ret)
			break;
		if (copy_to_user(argp, &s, sizeof(struct tzio_access_info))) {
			ret = -EFAULT;
			break;
		}
		break;
	}
	case TZIO_MEM_REGISTER: {
		struct tzio_mem_register __user *argp = (struct tzio_mem_register __user *)arg;
		struct tzio_mem_register s;
		if (copy_from_user(&s, argp, sizeof(struct tzio_mem_register))) {
			ret = -EFAULT;
			break;
		}
		ret = tzdev_mem_register(&s);
		if (ret)
			break;
		if (copy_to_user(argp, &s, sizeof(struct tzio_mem_register))) {
			ret = -EFAULT;
			break;
		}
		break;
	}
	case TZIO_MEM_RELEASE: {
		ret = tzdev_mem_release(arg);
		break;
	}
	case TZIO_WAIT_EVT: {
		unsigned long __user *argp = (unsigned long __user *)arg;
		unsigned long dta;

		ret = tzdev_wait_evt(&dta);
		if (ret)
			break;
		if (put_user(dta, argp)) {
			ret = -EFAULT;
			break;
		}
		break;
	}
	case TZIO_GET_PIPE: {
		struct timespec ts;
		unsigned long dta;
		unsigned long __user *argp = (unsigned long __user *)arg;
		getnstimeofday(&ts);

		dta = tzdev_smc_get_event(ts.tv_sec, ts.tv_nsec);
		if (put_user(dta, argp)) {
			ret = -EFAULT;
			break;
		}
		break;
	}
	case TZIO_MIGRATE: {
		tzdev_print(0, "TZIO_MIGRATE: arg = %lu\n", arg);
		ret = tzdev_migration_request(arg);
		break;
	}
#if defined(CONFIG_TZDEV_QC_CRYPTO_CLOCKS_USR_MNG)
	case TZIO_SET_QC_CLK: {
		unsigned long clk_cmd;
		unsigned long __user *argp = (unsigned long __user *)arg;
		if (copy_from_user(&clk_cmd, argp, sizeof(clk_cmd))) {
			ret = -EFAULT;
			break;
		}

		mutex_lock(&tzdev_qc_clk_mutex);
		if ((clk_cmd == QSEE_CLK_ON) && (tzdev_qc_clk == QSEE_CLK_OFF)) {
			tzdev_qc_pm_clock_enable();
			tzdev_qc_clk = QSEE_CLK_ON;
		} else if ((clk_cmd == QSEE_CLK_OFF) && (tzdev_qc_clk == QSEE_CLK_ON)) {
			tzdev_qc_pm_clock_disable();
			tzdev_qc_clk = QSEE_CLK_OFF;
		}
		mutex_unlock(&tzdev_qc_clk_mutex);
		break;
	}
#endif
	default:
		ret = -EINVAL;
	}

	return ret;
}

static void tzdev_shutdown(void)
{
	if (tzdev_sw_init_done)
		tzdev_smc_shutdown();
}

static const struct file_operations tzdev_fops = {
	.owner = THIS_MODULE,
	.open = tzdev_open,
	.release = tzdev_release,
	.unlocked_ioctl = tzdev_unlocked_ioctl,
};

static struct miscdevice tzdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "tzdev",
	.fops = &tzdev_fops,
};

static struct syscore_ops tzdev_syscore_ops = {
	.shutdown = tzdev_shutdown
};

#if defined(CONFIG_TZDEV_MSM_CRYPTO_WORKAROUND)
#include <linux/msm_ion.h>
#include "../qseecom_kernel.h"

void msm_crypto_workaround(void)
{
	enum qseecom_client_handle_type {
		QSEECOM_CLIENT_APP = 1,
		QSEECOM_LISTENER_SERVICE,
		QSEECOM_SECURE_SERVICE,
		QSEECOM_GENERIC,
		QSEECOM_UNAVAILABLE_CLIENT_APP,
	};
	struct qseecom_client_handle {
		u32  app_id;
		u8 *sb_virt;
		s32 sb_phys;
		uint32_t user_virt_sb_base;
		size_t sb_length;
		struct ion_handle *ihandle;		/* Retrieve phy addr */
	};
	struct qseecom_listener_handle {
		u32               id;
	};
	struct qseecom_dev_handle {
		enum qseecom_client_handle_type type;
		union {
			struct qseecom_client_handle client;
			struct qseecom_listener_handle listener;
		};
		bool released;
		int               abort;
		wait_queue_head_t abort_wq;
		atomic_t          ioctl_count;
		bool  perf_enabled;
		bool  fast_load_enabled;
	};

	/* Bogus handles */
	struct qseecom_dev_handle dev_handle;
	struct qseecom_handle handle = {
		.dev = &dev_handle
	};
	int ret;

	ret = qseecom_set_bandwidth(&handle, 1);
	if (ret)
		tzdev_print(0, "qseecom_set_bandwidth failed ret = %d\n", ret);
}
#endif

static irqreturn_t tzdev_event_handler(int irq, void *ptr)
{
	complete(&tzdev_ow_comp);

	return IRQ_HANDLED;
}

#if CONFIG_TZDEV_IWI_PANIC
static irqreturn_t tzdev_panic_handler(int irq, void *ptr)
{
#ifdef CONFIG_TZLOG
	schedule_work(&dump_kernel_panic);
#endif
	return IRQ_HANDLED;
}
#endif

static enum hrtimer_restart tzdev_get_event_timer_handler(struct hrtimer *timer)
{
	complete(&tzdev_ow_comp);

	return HRTIMER_NORESTART;
}

static int __init init_tzdev(void)
{
	int rc;

#if defined(CONFIG_TZDEV_QC_CRYPTO_CLOCKS_MANAGEMENT)
	tzdev_platform_register();
#endif

	rc = misc_register(&tzdev);
	if (unlikely(rc))
		return rc;

	if (request_irq(CONFIG_TZDEV_IWI_EVENT, tzdev_event_handler, 0,
						"tzdev_iwi_event", NULL))
		tzdev_print(0, "TZDEV_IWI_EVENT registration failed\n");

#if CONFIG_TZDEV_IWI_PANIC
	if (request_irq(CONFIG_TZDEV_IWI_PANIC, tzdev_panic_handler, 0,
						"tzdev_iwi_panic", NULL))
		tzdev_print(0, "TZDEV_IWI_PANIC registration failed\n");
#endif

#if defined(CONFIG_TZDEV_CMA)
	tzdev_cma_addr = 0;
#if defined(CONFIG_DMA_CMA)
	tzdev_page = dma_alloc_from_contiguous(tzdev.this_device,
			(CONFIG_TZDEV_MEMRESSZ - CONFIG_TZDEV_MEMRESSZPROT) >> PAGE_SHIFT,
			get_order(CONFIG_TZDEV_MEMRESSZ));
	if (!tzdev_page) {
		tzdev_print(0, "Allocation CMA region failed;"
		" Memory will not be registered in SWd\n");
		goto out;
	}
	tzdev_cma_addr = page_to_phys(tzdev_page);
#elif defined(CONFIG_CMA)
	if (cma_info(&tzdev_cma_info, tzdev.this_device, NULL) < 0) {
		tzdev_print(0, "Getting CMA info failed;"
		" Memory will not be registered in SWd\n");
		goto out;
	}
	tzdev_cma_addr = cma_alloc(tzdev.this_device, NULL,
		tzdev_cma_info.total_size, 0);
#endif
	if (!tzdev_cma_addr || IS_ERR_VALUE(tzdev_cma_addr))
		tzdev_print(0, "Allocation CMA region failed;"
		" Memory will not be registered in SWd\n");
out:
#endif

	tzdev_init_migration();
	register_syscore_ops(&tzdev_syscore_ops);

	hrtimer_init(&tzdev_get_event_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	tzdev_get_event_timer.function = tzdev_get_event_timer_handler;
#if defined(CONFIG_TZDEV_MSM_CRYPTO_WORKAROUND)
	/* Force crypto engine clocks on */
	msm_crypto_workaround();
#endif

	return rc;
}

static void __exit exit_tzdev(void)
{
	misc_deregister(&tzdev);

	idr_for_each(&tzdev_mem_map, __tzdev_mem_free, NULL);

	IDR_REMOVE_ALL(&tzdev_mem_map);

	idr_destroy(&tzdev_mem_map);

#if defined(CONFIG_TZDEV_CMA)
	if (tzdev_cma_addr) {
#if defined(CONFIG_DMA_CMA)
		dma_release_from_contiguous(tzdev.this_device, tzdev_page,
				(CONFIG_TZDEV_MEMRESSZ - CONFIG_TZDEV_MEMRESSZPROT) >> PAGE_SHIFT);
#elif defined(CONFIG_CMA)
		cma_free(tzdev_cma_addr);
#endif
	}
	tzdev_cma_addr = 0;
#endif

	hrtimer_cancel(&tzdev_get_event_timer);

	unregister_syscore_ops(&tzdev_syscore_ops);
	tzdev_fini_migration();
	tzdev_shutdown();

#if defined(CONFIG_TZDEV_QC_CRYPTO_CLOCKS_MANAGEMENT)
	tzdev_qc_pm_clock_finalize();
	tzdev_platform_unregister();
#endif
}

module_init(init_tzdev);
module_exit(exit_tzdev);
