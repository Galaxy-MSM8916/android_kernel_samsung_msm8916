/* Copyright (c) 2002,2007-2017 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/export.h>
#include <linux/vmalloc.h>
#include <asm/cacheflush.h>
#include <linux/slab.h>
#include <linux/kmemleak.h>
#include <linux/highmem.h>
#include <soc/qcom/scm.h>

#include "kgsl.h"
#include "kgsl_sharedmem.h"
#include "kgsl_cffdump.h"
#include "kgsl_device.h"
#include "kgsl_log.h"

static DEFINE_MUTEX(kernel_map_global_lock);

struct cp2_mem_chunks {
	unsigned int chunk_list;
	unsigned int chunk_list_size;
	unsigned int chunk_size;
} __attribute__ ((__packed__));

struct cp2_lock_req {
	struct cp2_mem_chunks chunks;
	unsigned int mem_usage;
	unsigned int lock;
} __attribute__ ((__packed__));

#define MEM_PROTECT_LOCK_ID2		0x0A
#define MEM_PROTECT_LOCK_ID2_FLAT	0x11

/* An attribute for showing per-process memory statistics */
struct kgsl_mem_entry_attribute {
	struct attribute attr;
	int memtype;
	ssize_t (*show)(struct kgsl_process_private *priv,
		int type, char *buf);
};

#define to_mem_entry_attr(a) \
container_of(a, struct kgsl_mem_entry_attribute, attr)

#define __MEM_ENTRY_ATTR(_type, _name, _show) \
{ \
	.attr = { .name = __stringify(_name), .mode = 0444 }, \
	.memtype = _type, \
	.show = _show, \
}

/*
 * A structure to hold the attributes for a particular memory type.
 * For each memory type in each process we store the current and maximum
 * memory usage and display the counts in sysfs.  This structure and
 * the following macro allow us to simplify the definition for those
 * adding new memory types
 */

struct mem_entry_stats {
	int memtype;
	struct kgsl_mem_entry_attribute attr;
	struct kgsl_mem_entry_attribute max_attr;
};


#define MEM_ENTRY_STAT(_type, _name) \
{ \
	.memtype = _type, \
	.attr = __MEM_ENTRY_ATTR(_type, _name, mem_entry_show), \
	.max_attr = __MEM_ENTRY_ATTR(_type, _name##_max, \
		mem_entry_max_show), \
}

static int kgsl_cma_unlock_secure(struct kgsl_device *device,
			struct kgsl_memdesc *memdesc);

/**
 * Show the current amount of memory allocated for the given memtype
 */

static ssize_t
mem_entry_show(struct kgsl_process_private *priv, int type, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", priv->stats[type].cur);
}

/**
 * Show the maximum memory allocated for the given memtype through the life of
 * the process
 */

static ssize_t
mem_entry_max_show(struct kgsl_process_private *priv, int type, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", priv->stats[type].max);
}


static void mem_entry_sysfs_release(struct kobject *kobj)
{
}

static ssize_t mem_entry_sysfs_show(struct kobject *kobj,
	struct attribute *attr, char *buf)
{
	struct kgsl_mem_entry_attribute *pattr = to_mem_entry_attr(attr);
	struct kgsl_process_private *priv;
	ssize_t ret;

	/*
	 * 1. sysfs_remove_file waits for reads to complete before the node
	 *    is deleted.
	 * 2. kgsl_process_init_sysfs takes a refcount to the process_private,
	 *    which is put at the end of kgsl_process_uninit_sysfs.
	 * These two conditions imply that priv will not be freed until this
	 * function completes, and no further locking is needed.
	 */
	priv = kobj ? container_of(kobj, struct kgsl_process_private, kobj) :
			NULL;

	if (priv && pattr->show)
		ret = pattr->show(priv, pattr->memtype, buf);
	else
		ret = -EIO;

	return ret;
}

static const struct sysfs_ops mem_entry_sysfs_ops = {
	.show = mem_entry_sysfs_show,
};

static struct kobj_type ktype_mem_entry = {
	.sysfs_ops = &mem_entry_sysfs_ops,
	.default_attrs = NULL,
	.release = mem_entry_sysfs_release
};

static struct mem_entry_stats mem_stats[] = {
	MEM_ENTRY_STAT(KGSL_MEM_ENTRY_KERNEL, kernel),
	MEM_ENTRY_STAT(KGSL_MEM_ENTRY_PMEM, pmem),
#ifdef CONFIG_ASHMEM
	MEM_ENTRY_STAT(KGSL_MEM_ENTRY_ASHMEM, ashmem),
#endif
	MEM_ENTRY_STAT(KGSL_MEM_ENTRY_USER, user),
#ifdef CONFIG_ION
	MEM_ENTRY_STAT(KGSL_MEM_ENTRY_ION, ion),
#endif
};

void
kgsl_process_uninit_sysfs(struct kgsl_process_private *private)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mem_stats); i++) {
		sysfs_remove_file(&private->kobj, &mem_stats[i].attr.attr);
		sysfs_remove_file(&private->kobj,
			&mem_stats[i].max_attr.attr);
	}

	kobject_put(&private->kobj);
	/* Put the refcount we got in kgsl_process_init_sysfs */
	kgsl_process_private_put(private);
}

/**
 * kgsl_process_init_sysfs() - Initialize and create sysfs files for a process
 *
 * @device: Pointer to kgsl device struct
 * @private: Pointer to the structure for the process
 *
 * @returns: 0 on success, error code otherwise
 *
 * kgsl_process_init_sysfs() is called at the time of creating the
 * process struct when a process opens the kgsl device for the first time.
 * This function creates the sysfs files for the process.
 */
int
kgsl_process_init_sysfs(struct kgsl_device *device,
		struct kgsl_process_private *private)
{
	unsigned char name[16];
	int i, ret = 0;

	snprintf(name, sizeof(name), "%d", private->pid);

	ret = kobject_init_and_add(&private->kobj, &ktype_mem_entry,
		kgsl_driver.prockobj, name);

	if (ret)
		return ret;

	for (i = 0; i < ARRAY_SIZE(mem_stats); i++) {
		/* We need to check the value of sysfs_create_file, but we
		 * don't really care if it passed or not */

		ret = sysfs_create_file(&private->kobj,
			&mem_stats[i].attr.attr);
		ret = sysfs_create_file(&private->kobj,
			&mem_stats[i].max_attr.attr);
	}

	/* Keep private valid until the sysfs enries are removed. */
	if (!ret)
		kgsl_process_private_get(private);

	return ret;
}

static ssize_t kgsl_drv_memstat_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	unsigned int val = 0;

	if (!strcmp(attr->attr.name, "vmalloc"))
		val = kgsl_driver.stats.vmalloc;
	else if (!strcmp(attr->attr.name, "vmalloc_max"))
		val = kgsl_driver.stats.vmalloc_max;
	else if (!strcmp(attr->attr.name, "page_alloc"))
		val = kgsl_driver.stats.page_alloc;
	else if (!strcmp(attr->attr.name, "page_alloc_max"))
		val = kgsl_driver.stats.page_alloc_max;
	else if (!strcmp(attr->attr.name, "coherent"))
		val = kgsl_driver.stats.coherent;
	else if (!strcmp(attr->attr.name, "coherent_max"))
		val = kgsl_driver.stats.coherent_max;
	else if (!strcmp(attr->attr.name, "secure"))
		val = kgsl_driver.stats.secure;
	else if (!strcmp(attr->attr.name, "secure_max"))
		val = kgsl_driver.stats.secure_max;
	else if (!strcmp(attr->attr.name, "mapped"))
		val = kgsl_driver.stats.mapped;
	else if (!strcmp(attr->attr.name, "mapped_max"))
		val = kgsl_driver.stats.mapped_max;

	return snprintf(buf, PAGE_SIZE, "%u\n", val);
}

static ssize_t kgsl_drv_full_cache_threshold_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	int ret;
	unsigned int thresh = 0;

	ret = kgsl_sysfs_store(buf, &thresh);
	if (ret)
		return ret;

	kgsl_driver.full_cache_threshold = thresh;
	return count;
}

static ssize_t kgsl_drv_full_cache_threshold_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n",
			kgsl_driver.full_cache_threshold);
}

static DEVICE_ATTR(vmalloc, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(vmalloc_max, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(page_alloc, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(page_alloc_max, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(coherent, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(coherent_max, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(secure, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(secure_max, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(mapped, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(mapped_max, 0444, kgsl_drv_memstat_show, NULL);
static DEVICE_ATTR(full_cache_threshold, 0644,
		kgsl_drv_full_cache_threshold_show,
		kgsl_drv_full_cache_threshold_store);

static const struct device_attribute *drv_attr_list[] = {
	&dev_attr_vmalloc,
	&dev_attr_vmalloc_max,
	&dev_attr_page_alloc,
	&dev_attr_page_alloc_max,
	&dev_attr_coherent,
	&dev_attr_coherent_max,
	&dev_attr_secure,
	&dev_attr_secure_max,
	&dev_attr_mapped,
	&dev_attr_mapped_max,
	&dev_attr_full_cache_threshold,
	NULL
};

void
kgsl_sharedmem_uninit_sysfs(void)
{
	kgsl_remove_device_sysfs_files(&kgsl_driver.virtdev, drv_attr_list);
}

int
kgsl_sharedmem_init_sysfs(void)
{
	return kgsl_create_device_sysfs_files(&kgsl_driver.virtdev,
		drv_attr_list);
}

static int kgsl_page_alloc_vmfault(struct kgsl_memdesc *memdesc,
				struct vm_area_struct *vma,
				struct vm_fault *vmf)
{
	int i, pgoff;
	struct scatterlist *s = memdesc->sg;
	unsigned int offset;

	offset = ((unsigned long) vmf->virtual_address - vma->vm_start);

	if (offset >= memdesc->size)
		return VM_FAULT_SIGBUS;

	pgoff = offset >> PAGE_SHIFT;

	/*
	 * The sglist might be comprised of mixed blocks of memory depending
	 * on how many 64K pages were allocated.  This means we have to do math
	 * to find the actual 4K page to map in user space
	 */

	for (i = 0; i < memdesc->sglen; i++) {
		int npages = s->length >> PAGE_SHIFT;

		if (pgoff < npages) {
			struct page *page = sg_page(s);

			page = nth_page(page, pgoff);

			get_page(page);
			vmf->page = page;

			return 0;
		}

		pgoff -= npages;
		s = sg_next(s);
	}

	return VM_FAULT_SIGBUS;
}

/*
 * kgsl_page_alloc_unmap_kernel() - Unmap the memory in memdesc
 *
 * @memdesc: The memory descriptor which contains information about the memory
 *
 * Unmaps the memory mapped into kernel address space
 */
static void kgsl_page_alloc_unmap_kernel(struct kgsl_memdesc *memdesc)
{
	mutex_lock(&kernel_map_global_lock);
	if (!memdesc->hostptr) {
		BUG_ON(memdesc->hostptr_count);
		goto done;
	}
	memdesc->hostptr_count--;
	if (memdesc->hostptr_count)
		goto done;
	vunmap(memdesc->hostptr);
	kgsl_driver.stats.vmalloc -= memdesc->size;
	memdesc->hostptr = NULL;
done:
	mutex_unlock(&kernel_map_global_lock);
}

static void kgsl_page_alloc_free(struct kgsl_memdesc *memdesc)
{
	int i = 0;
	struct scatterlist *sg;
	int sglen = memdesc->sglen;

	kgsl_driver.stats.page_alloc -= memdesc->size;

	kgsl_page_alloc_unmap_kernel(memdesc);
	/* we certainly do not expect the hostptr to still be mapped */
	BUG_ON(memdesc->hostptr);

	if (sglen && memdesc->sg)
		for_each_sg(memdesc->sg, sg, sglen, i)
			__free_pages(sg_page(sg), get_order(sg->length));
}

/*
 * kgsl_page_alloc_map_kernel - Map the memory in memdesc to kernel address
 * space
 *
 * @memdesc - The memory descriptor which contains information about the memory
 *
 * Return: 0 on success else error code
 */
static int kgsl_page_alloc_map_kernel(struct kgsl_memdesc *memdesc)
{
	int ret = 0;

	mutex_lock(&kernel_map_global_lock);
	if (!memdesc->hostptr) {
		pgprot_t page_prot = pgprot_writecombine(PAGE_KERNEL);
		struct page **pages = NULL;
		struct scatterlist *sg;
		int npages = PAGE_ALIGN(memdesc->size) >> PAGE_SHIFT;
		int sglen = memdesc->sglen;
		int i, count = 0;

		/* create a list of pages to call vmap */
		pages = kgsl_malloc(npages * sizeof(struct page *));
		if (pages == NULL) {
			ret = -ENOMEM;
			goto done;
		}

		for_each_sg(memdesc->sg, sg, sglen, i) {
			struct page *page = sg_page(sg);
			int j;

			for (j = 0; j < sg->length >> PAGE_SHIFT; j++)
				pages[count++] = page++;
		}


		memdesc->hostptr = vmap(pages, count,
					VM_IOREMAP, page_prot);
		if (memdesc->hostptr)
			KGSL_STATS_ADD(memdesc->size, kgsl_driver.stats.vmalloc,
				kgsl_driver.stats.vmalloc_max);
		else
			ret = -ENOMEM;
		kgsl_free(pages);
	}
	if (memdesc->hostptr)
		memdesc->hostptr_count++;
done:
	mutex_unlock(&kernel_map_global_lock);

	return ret;
}

static int kgsl_contiguous_vmfault(struct kgsl_memdesc *memdesc,
				struct vm_area_struct *vma,
				struct vm_fault *vmf)
{
	unsigned long offset, pfn;
	int ret;

	offset = ((unsigned long) vmf->virtual_address - vma->vm_start) >>
		PAGE_SHIFT;

	pfn = (memdesc->physaddr >> PAGE_SHIFT) + offset;
	ret = vm_insert_pfn(vma, (unsigned long) vmf->virtual_address, pfn);

	if (ret == -ENOMEM || ret == -EAGAIN)
		return VM_FAULT_OOM;
	else if (ret == -EFAULT)
		return VM_FAULT_SIGBUS;

	return VM_FAULT_NOPAGE;
}

static void kgsl_cma_coherent_free(struct kgsl_memdesc *memdesc)
{
	if (memdesc->hostptr) {
		if (memdesc->priv & KGSL_MEMDESC_SECURE) {
			kgsl_driver.stats.secure -= memdesc->size;
			if (memdesc->priv & KGSL_MEMDESC_TZ_LOCKED)
				kgsl_cma_unlock_secure(
				memdesc->pagetable->mmu->device, memdesc);
		} else
			kgsl_driver.stats.coherent -= memdesc->size;

		dma_free_coherent(memdesc->dev, memdesc->size,
				memdesc->hostptr, memdesc->physaddr);
	}
}

/* Global - also used by kgsl_drm.c */
static struct kgsl_memdesc_ops kgsl_page_alloc_ops = {
	.free = kgsl_page_alloc_free,
	.vmflags = VM_DONTDUMP | VM_DONTEXPAND | VM_DONTCOPY,
	.vmfault = kgsl_page_alloc_vmfault,
	.map_kernel = kgsl_page_alloc_map_kernel,
	.unmap_kernel = kgsl_page_alloc_unmap_kernel,
};

/* CMA ops - used during NOMMU mode */
static struct kgsl_memdesc_ops kgsl_cma_ops = {
	.free = kgsl_cma_coherent_free,
	.vmflags = VM_DONTDUMP | VM_PFNMAP | VM_DONTEXPAND | VM_DONTCOPY,
	.vmfault = kgsl_contiguous_vmfault,
};

#ifdef CONFIG_ARM64
/*
 * For security reasons, ARMv8 doesn't allow invalidate only on read-only
 * mapping. It would be performance prohibitive to read the permissions on
 * the buffer before the operation. Every use case that we have found does not
 * assume that an invalidate operation is invalidate only, so we feel
 * comfortable turning invalidates into flushes for these targets
 */
static inline unsigned int _fixup_cache_range_op(unsigned int op)
{
	if (op == KGSL_CACHE_OP_INV)
		return KGSL_CACHE_OP_FLUSH;
	return op;
}
#else
static inline unsigned int _fixup_cache_range_op(unsigned int op)
{
	return op;
}
#endif

static int kgsl_do_cache_op(struct page *page, void *addr,
		uint64_t offset, uint64_t size, unsigned int op)
{
	void (*cache_op)(const void *, const void *);

	/*
	 * The dmac_xxx_range functions handle addresses and sizes that
	 * are not aligned to the cacheline size correctly.
	 */
	switch (_fixup_cache_range_op(op)) {
	case KGSL_CACHE_OP_FLUSH:
		cache_op = dmac_flush_range;
		break;
	case KGSL_CACHE_OP_CLEAN:
		cache_op = dmac_clean_range;
		break;
	case KGSL_CACHE_OP_INV:
		cache_op = dmac_inv_range;
		break;
	default:
		return -EINVAL;
	}

	if (page != NULL) {
		unsigned long pfn = page_to_pfn(page) + offset / PAGE_SIZE;
		/*
		 *  page_address() returns the kernel virtual address of page.
		 *  For high memory kernel virtual address exists only if page
		 *  has been mapped. So use a version of kmap rather than
		 *  page_address() for high memory.
		 */
		if (PageHighMem(page)) {
			offset &= ~PAGE_MASK;

			do {
				unsigned int len = size;

				if (len + offset > PAGE_SIZE)
					len = PAGE_SIZE - offset;

				page = pfn_to_page(pfn++);
				addr = kmap_atomic(page);
				cache_op(addr + offset, addr + offset + len);
				kunmap_atomic(addr);

				size -= len;
				offset = 0;
			} while (size);

			return 0;
		}

		addr = page_address(page);
	}

	cache_op(addr + offset, addr + offset + (size_t) size);
	return 0;
}

int kgsl_cache_range_op(struct kgsl_memdesc *memdesc, size_t offset,
		size_t size, unsigned int op)
{
	void *addr = NULL;
	int ret = 0;

	if (size == 0 || size > UINT_MAX)
		return -EINVAL;

	/* Make sure that the offset + size does not overflow */
	if ((offset + size < offset) || (offset + size < size))
		return -ERANGE;

	/* Check that offset+length does not exceed memdesc->size */
	if ((offset + size) > memdesc->size)
		return -ERANGE;

	if (memdesc->hostptr) {
		addr = memdesc->hostptr;
		/* Make sure the offset + size do not overflow the address */
		if (addr + ((size_t) offset + (size_t) size) < addr)
			return -ERANGE;

		ret = kgsl_do_cache_op(NULL, addr, offset, size, op);
		return ret;
	}

	/*
	 * If the buffer is not to mapped to kernel, perform cache
	 * operations after mapping to kernel.
	 */
	if (memdesc->sg) {
		struct scatterlist *sg;
		unsigned int i, pos = 0;

		for_each_sg(memdesc->sg, sg, memdesc->sglen, i) {
			uint64_t sg_offset, sg_left;

			if (offset >= (pos + sg->length)) {
				pos += sg->length;
				continue;
			}
			sg_offset = offset > pos ? offset - pos : 0;
			sg_left = (sg->length - sg_offset > size) ? size :
						sg->length - sg_offset;
			ret = kgsl_do_cache_op(sg_page(sg), NULL, sg_offset,
								sg_left, op);
			size -= sg_left;
			if (size == 0)
				break;
			pos += sg->length;
		}
	}
	return ret;
}
EXPORT_SYMBOL(kgsl_cache_range_op);

#ifndef CONFIG_ALLOC_BUFFERS_IN_4K_CHUNKS
static inline int get_page_size(size_t size, unsigned int align)
{
	return (align >= ilog2(SZ_64K) && size >= SZ_64K)
					? SZ_64K : PAGE_SIZE;
}
#else
static inline int get_page_size(size_t size, unsigned int align)
{
	return PAGE_SIZE;
}
#endif

static int
_kgsl_sharedmem_page_alloc(struct kgsl_memdesc *memdesc,
			struct kgsl_pagetable *pagetable,
			size_t size)
{
	int pcount = 0, ret = 0;
	int j, page_size, sglen_alloc, sglen = 0;
	size_t len;
	struct page **pages = NULL;
	pgprot_t page_prot = pgprot_writecombine(PAGE_KERNEL);
	void *ptr;
	unsigned int align;
	int step = ((VMALLOC_END - VMALLOC_START)/8) >> PAGE_SHIFT;

	size = PAGE_ALIGN(size);
	if (size == 0 || size > UINT_MAX)
		return -EINVAL;

	align = (memdesc->flags & KGSL_MEMALIGN_MASK) >> KGSL_MEMALIGN_SHIFT;

	page_size = get_page_size(size, align);

	/*
	 * The alignment cannot be less than the intended page size - it can be
	 * larger however to accomodate hardware quirks
	 */

	if (ilog2(align) < page_size)
		kgsl_memdesc_set_align(memdesc, ilog2(page_size));

	/*
	 * There needs to be enough room in the sg structure to be able to
	 * service the allocation entirely with PAGE_SIZE sized chunks
	 */

	sglen_alloc = PAGE_ALIGN(size) >> PAGE_SHIFT;

	memdesc->pagetable = pagetable;
	memdesc->ops = &kgsl_page_alloc_ops;

	/* Check for integer overflow */
	if (sglen_alloc && (sizeof(struct scatterlist) > INT_MAX / sglen_alloc))
		return -EINVAL;
	memdesc->sg = kgsl_malloc(sglen_alloc * sizeof(struct scatterlist));

	if (memdesc->sg == NULL) {
		ret = -ENOMEM;
		goto done;
	}

	/*
	 * Allocate space to store the list of pages to send to vmap. This is an
	 * array of pointers so we can track 1024 pages per page of allocation
	 */

	pages = kgsl_malloc(sglen_alloc * sizeof(struct page *));

	if (pages == NULL) {
		ret = -ENOMEM;
		goto done;
	}

	if (!is_vmalloc_addr(memdesc->sg))
		kmemleak_not_leak(memdesc->sg);

	sg_init_table(memdesc->sg, sglen_alloc);

	len = size;

	while (len > 0) {
		struct page *page;
		unsigned int gfp_mask = __GFP_HIGHMEM;
		int j;

		/* don't waste space at the end of the allocation*/
		if (len < page_size)
			page_size = PAGE_SIZE;

		/*
		 * Don't do some of the more aggressive memory recovery
		 * techniques for large order allocations
		 */
		if (page_size != PAGE_SIZE)
			gfp_mask |= __GFP_COMP | __GFP_NORETRY |
				__GFP_NO_KSWAPD | __GFP_NOWARN;
		else
			gfp_mask |= GFP_KERNEL;

		page = alloc_pages(gfp_mask, get_order(page_size));

		if (page == NULL) {
			if (page_size != PAGE_SIZE) {
				page_size = PAGE_SIZE;
				continue;
			}

			/*
			 * Update sglen and memdesc size,as requested allocation
			 * not served fully. So that they can be correctly freed
			 * in kgsl_sharedmem_free().
			 */
			memdesc->sglen = sglen;
			memdesc->size = (size - len);

			if (sglen > 0)
				sg_mark_end(&memdesc->sg[sglen - 1]);

			KGSL_CORE_ERR(
				"Out of memory: only allocated %zuKB of %zuKB requested\n",
				(size - len) >> 10, size >> 10);

			ret = -ENOMEM;
			goto done;
		}

		for (j = 0; j < page_size >> PAGE_SHIFT; j++)
			pages[pcount++] = nth_page(page, j);

		sg_set_page(&memdesc->sg[sglen++], page, page_size, 0);
		len -= page_size;
	}

	memdesc->sglen = sglen;
	memdesc->size = size;

	if (sglen > 0)
		sg_mark_end(&memdesc->sg[sglen - 1]);

	/*
	 * All memory that goes to the user has to be zeroed out before it gets
	 * exposed to userspace. This means that the memory has to be mapped in
	 * the kernel, zeroed (memset) and then unmapped.  This also means that
	 * the dcache has to be flushed to ensure coherency between the kernel
	 * and user pages. We used to pass __GFP_ZERO to alloc_page which mapped
	 * zeroed and unmaped each individual page, and then we had to turn
	 * around and call flush_dcache_page() on that page to clear the caches.
	 * This was killing us for performance. Instead, we found it is much
	 * faster to allocate the pages without GFP_ZERO, map a chunk of the
	 * range ('step' pages), memset it, flush it and then unmap
	 * - this results in a factor of 4 improvement for speed for large
	 * buffers. There is a small decrease in speed for small buffers,
	 * but only on the order of a few microseconds at best. The 'step'
	 * size is based on a guess at the amount of free vmalloc space, but
	 * will scale down if there's not enough free space.
	 */
	for (j = 0; j < pcount; j += step) {
		step = min(step, pcount - j);

		ptr = vmap(&pages[j], step, VM_IOREMAP, page_prot);

		if (ptr != NULL) {
			memset(ptr, 0, step * PAGE_SIZE);
			dmac_flush_range(ptr, ptr + step * PAGE_SIZE);
			vunmap(ptr);
		} else {
			int k;
			/* Very, very, very slow path */

			for (k = j; k < j + step; k++) {
				ptr = kmap_atomic(pages[k]);
				memset(ptr, 0, PAGE_SIZE);
				dmac_flush_range(ptr, ptr + PAGE_SIZE);
				kunmap_atomic(ptr);
			}
			/* scale down the step size to avoid this path */
			if (step > 1)
				step >>= 1;
		}
	}

done:
	KGSL_STATS_ADD(memdesc->size, kgsl_driver.stats.page_alloc,
		kgsl_driver.stats.page_alloc_max);

	kgsl_free(pages);

	if (ret)
		kgsl_sharedmem_free(memdesc);

	return ret;
}

int
kgsl_sharedmem_page_alloc_user(struct kgsl_memdesc *memdesc,
			    struct kgsl_pagetable *pagetable,
			    size_t size)
{
	size = PAGE_ALIGN(size);
	if (size == 0)
		return -EINVAL;

	return _kgsl_sharedmem_page_alloc(memdesc, pagetable, size);
}
EXPORT_SYMBOL(kgsl_sharedmem_page_alloc_user);

void kgsl_sharedmem_free(struct kgsl_memdesc *memdesc)
{
	if (memdesc == NULL || memdesc->size == 0)
		return;

	if (memdesc->gpuaddr) {
		kgsl_mmu_unmap(memdesc->pagetable, memdesc);
		kgsl_mmu_put_gpuaddr(memdesc->pagetable, memdesc);
	}

	if (memdesc->ops && memdesc->ops->free)
		memdesc->ops->free(memdesc);

	kgsl_free(memdesc->sg);

	memset(memdesc, 0, sizeof(*memdesc));
}
EXPORT_SYMBOL(kgsl_sharedmem_free);

int
kgsl_sharedmem_readl(const struct kgsl_memdesc *memdesc,
			uint32_t *dst,
			unsigned int offsetbytes)
{
	uint32_t *src;
	BUG_ON(memdesc == NULL || memdesc->hostptr == NULL || dst == NULL);
	WARN_ON(offsetbytes % sizeof(uint32_t) != 0);
	if (offsetbytes % sizeof(uint32_t) != 0)
		return -EINVAL;

	WARN_ON(offsetbytes + sizeof(uint32_t) > memdesc->size);
	if (offsetbytes + sizeof(uint32_t) > memdesc->size)
		return -ERANGE;

	rmb();
	src = (uint32_t *)(memdesc->hostptr + offsetbytes);
	*dst = *src;
	return 0;
}
EXPORT_SYMBOL(kgsl_sharedmem_readl);

int
kgsl_sharedmem_writel(struct kgsl_device *device,
			const struct kgsl_memdesc *memdesc,
			unsigned int offsetbytes,
			uint32_t src)
{
	uint32_t *dst;
	BUG_ON(memdesc == NULL || memdesc->hostptr == NULL);
	WARN_ON(offsetbytes % sizeof(uint32_t) != 0);
	if (offsetbytes % sizeof(uint32_t) != 0)
		return -EINVAL;

	WARN_ON(offsetbytes + sizeof(uint32_t) > memdesc->size);
	if (offsetbytes + sizeof(uint32_t) > memdesc->size)
		return -ERANGE;
	kgsl_cffdump_write(device,
		memdesc->gpuaddr + offsetbytes,
		src);
	dst = (uint32_t *)(memdesc->hostptr + offsetbytes);
	*dst = src;

	wmb();

	return 0;
}
EXPORT_SYMBOL(kgsl_sharedmem_writel);

int
kgsl_sharedmem_set(struct kgsl_device *device,
		const struct kgsl_memdesc *memdesc, unsigned int offsetbytes,
		unsigned int value, unsigned int sizebytes)
{
	BUG_ON(memdesc == NULL || memdesc->hostptr == NULL);
	BUG_ON(offsetbytes + sizebytes > memdesc->size);

	kgsl_cffdump_memset(device,
		memdesc->gpuaddr + offsetbytes, value,
		sizebytes);
	memset(memdesc->hostptr + offsetbytes, value, sizebytes);
	return 0;
}
EXPORT_SYMBOL(kgsl_sharedmem_set);

static const char * const memtype_str[] = {
	[KGSL_MEMTYPE_OBJECTANY] = "any(0)",
	[KGSL_MEMTYPE_FRAMEBUFFER] = "framebuffer",
	[KGSL_MEMTYPE_RENDERBUFFER] = "renderbuffer",
	[KGSL_MEMTYPE_ARRAYBUFFER] = "arraybuffer",
	[KGSL_MEMTYPE_ELEMENTARRAYBUFFER] = "elementarraybuffer",
	[KGSL_MEMTYPE_VERTEXARRAYBUFFER] = "vertexarraybuffer",
	[KGSL_MEMTYPE_TEXTURE] = "texture",
	[KGSL_MEMTYPE_SURFACE] = "surface",
	[KGSL_MEMTYPE_EGL_SURFACE] = "egl_surface",
	[KGSL_MEMTYPE_GL] = "gl",
	[KGSL_MEMTYPE_CL] = "cl",
	[KGSL_MEMTYPE_CL_BUFFER_MAP] = "cl_buffer_map",
	[KGSL_MEMTYPE_CL_BUFFER_NOMAP] = "cl_buffer_nomap",
	[KGSL_MEMTYPE_CL_IMAGE_MAP] = "cl_image_map",
	[KGSL_MEMTYPE_CL_IMAGE_NOMAP] = "cl_image_nomap",
	[KGSL_MEMTYPE_CL_KERNEL_STACK] = "cl_kernel_stack",
	[KGSL_MEMTYPE_COMMAND] = "command",
	[KGSL_MEMTYPE_2D] = "2d",
	[KGSL_MEMTYPE_EGL_IMAGE] = "egl_image",
	[KGSL_MEMTYPE_EGL_SHADOW] = "egl_shadow",
	[KGSL_MEMTYPE_MULTISAMPLE] = "egl_multisample",
	/* KGSL_MEMTYPE_KERNEL handled below, to avoid huge array */
};

void kgsl_get_memory_usage(char *name, size_t name_size, unsigned int memflags)
{
	unsigned char type;

	type = (memflags & KGSL_MEMTYPE_MASK) >> KGSL_MEMTYPE_SHIFT;
	if (type == KGSL_MEMTYPE_KERNEL)
		strlcpy(name, "kernel", name_size);
	else if (type < ARRAY_SIZE(memtype_str) && memtype_str[type] != NULL)
		strlcpy(name, memtype_str[type], name_size);
	else
		snprintf(name, name_size, "unknown(%3d)", type);
}
EXPORT_SYMBOL(kgsl_get_memory_usage);

int kgsl_cma_alloc_coherent(struct kgsl_device *device,
			struct kgsl_memdesc *memdesc,
			struct kgsl_pagetable *pagetable, size_t size)
{
	int result = 0;

	if (size == 0)
		return -EINVAL;

	memdesc->size = size;
	memdesc->pagetable = pagetable;
	memdesc->ops = &kgsl_cma_ops;
	memdesc->dev = device->dev->parent;

	memdesc->hostptr = dma_alloc_coherent(memdesc->dev, size,
					&memdesc->physaddr, GFP_KERNEL);

	if (memdesc->hostptr == NULL) {
		result = -ENOMEM;
		goto err;
	}

	result = memdesc_sg_phys(memdesc, memdesc->physaddr, size);
	if (result)
		goto err;

	/* Record statistics */

	KGSL_STATS_ADD(size, kgsl_driver.stats.coherent,
		       kgsl_driver.stats.coherent_max);

err:
	if (result)
		kgsl_sharedmem_free(memdesc);

	return result;
}
EXPORT_SYMBOL(kgsl_cma_alloc_coherent);

int kgsl_cma_alloc_secure(struct kgsl_device *device,
			struct kgsl_memdesc *memdesc, size_t size)
{
	int result = 0;
	struct cp2_lock_req request;
	unsigned int resp;
	unsigned int *chunk_list = NULL;
	struct kgsl_pagetable *pagetable = device->mmu.securepagetable;
	struct scm_desc desc = {0};

	if (size == 0)
		return -EINVAL;

	/* Align size to 1M boundaries */
	size = ALIGN(size, SZ_1M);

	memdesc->size = size;
	memdesc->pagetable = pagetable;
	memdesc->ops = &kgsl_cma_ops;
	memdesc->dev = device->dev->parent;

	memdesc->hostptr = dma_alloc_coherent(memdesc->dev, size,
					&memdesc->physaddr, GFP_KERNEL);

	if (memdesc->hostptr == NULL) {
		result = -ENOMEM;
		goto err;
	}

	result = memdesc_sg_phys(memdesc, memdesc->physaddr, size);
	if (result)
		goto err;

	/*
	 * Flush the virt addr range before sending the memory to the
	 * secure environment to ensure the data is actually present
	 * in RAM
	 *
	 * Chunk_list holds the physical address of secure memory.
	 * Pass in the virtual address of chunk_list to flush.
	 * Chunk_list size is 1 because secure memory is physically
	 * contiguous.
	 */
	chunk_list = kzalloc(sizeof(unsigned int), GFP_KERNEL);
	if (!chunk_list) {
		result = -ENOMEM;
		goto err;
	}
	chunk_list[0] = memdesc->physaddr;
	dmac_flush_range((void *)chunk_list, (void *)chunk_list + 1);

	desc.args[0] = request.chunks.chunk_list = virt_to_phys(chunk_list);
	desc.args[1] = request.chunks.chunk_list_size = 1;
	desc.args[2] = request.chunks.chunk_size = memdesc->size;
	desc.args[3] = request.mem_usage = 0;
	desc.args[4] = request.lock = 1;
	desc.args[5] = 0;
	desc.arginfo = SCM_ARGS(6, SCM_RW, SCM_VAL, SCM_VAL, SCM_VAL, SCM_VAL,
				SCM_VAL);
	kmap_flush_unused();
	kmap_atomic_flush_unused();

	if (result == 0)
		memdesc->priv |= KGSL_MEMDESC_TZ_LOCKED;
	else {
		KGSL_DRV_ERR(device, "Secure buffer size %zx failed pt %d\n",
					 memdesc->size, pagetable->name);
		goto err;
	}

	if (!is_scm_armv8()) {
		result = scm_call(SCM_SVC_MP, MEM_PROTECT_LOCK_ID2,
				&request, sizeof(request), &resp, sizeof(resp));
	} else {
		result = scm_call2(SCM_SIP_FNID(SCM_SVC_MP,
				   MEM_PROTECT_LOCK_ID2_FLAT), &desc);
		resp = desc.ret[0];
	}
	if (result) {
		KGSL_DRV_ERR(device, "Secure buffer allocation failed\n");
		goto err;
	}

	/* Record statistics */
	KGSL_STATS_ADD(size, kgsl_driver.stats.secure,
		       kgsl_driver.stats.secure_max);

err:
	kfree(chunk_list);

	if (result)
		kgsl_sharedmem_free(memdesc);

	return result;
}
EXPORT_SYMBOL(kgsl_cma_alloc_secure);

/**
 * kgsl_cma_unlock_secure() - Unlock secure memory by calling TZ
 * @device: kgsl device pointer
 * @memdesc: memory descriptor
 */
static int kgsl_cma_unlock_secure(struct kgsl_device *device,
			struct kgsl_memdesc *memdesc)
{
	int result = 0;
	struct cp2_lock_req request;
	unsigned int resp;
	unsigned int *chunk_list;
	struct kgsl_pagetable *pagetable = device->mmu.securepagetable;
	struct scm_desc desc;

	if (!memdesc->size) {
		KGSL_DRV_ERR(device, "Secure buffer invalid size 0\n");
		return -EINVAL;
	}

	if (!IS_ALIGNED(memdesc->size, SZ_1M)) {
		KGSL_DRV_ERR(device,
			 "Secure buffer size %zx must be %x aligned",
			 memdesc->size, SZ_1M);
			return -EINVAL;
	}

	/*
	 * Flush the phys addr range before sending the memory to the
	 * secure environment to ensure the data is actually present
	 * in RAM
	 */
	chunk_list = kzalloc(sizeof(unsigned int), GFP_KERNEL);
	if (!chunk_list)
		return -ENOMEM;
	chunk_list[0] = memdesc->physaddr;
	dmac_flush_range((void *)chunk_list, (void *)chunk_list + 1);

	desc.args[0] = request.chunks.chunk_list = virt_to_phys(chunk_list);
	desc.args[1] = request.chunks.chunk_list_size = 1;
	desc.args[2] = request.chunks.chunk_size = memdesc->size;
	desc.args[3] = request.mem_usage = 0;
	desc.args[4] = request.lock = 0;
	desc.args[5] = 0;
	desc.arginfo = SCM_ARGS(6, SCM_RW, SCM_VAL, SCM_VAL, SCM_VAL, SCM_VAL,
				SCM_VAL);
	kmap_flush_unused();
	kmap_atomic_flush_unused();

	if (!is_scm_armv8()) {
		result = scm_call(SCM_SVC_MP, MEM_PROTECT_LOCK_ID2,
				&request, sizeof(request), &resp, sizeof(resp));
	} else {
		result = scm_call2(SCM_SIP_FNID(SCM_SVC_MP,
				   MEM_PROTECT_LOCK_ID2_FLAT), &desc);
		resp = desc.ret[0];
	}
	kfree(chunk_list);

	if (result)
		KGSL_DRV_ERR(device,
		"Secure buffer unlock size %zx failed pt %d\n",
		memdesc->size, pagetable->name);

	return result;
}
