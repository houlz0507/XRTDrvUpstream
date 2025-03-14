/* SPDX-License-Identifier: GPL-2.0 */

/*
 * Copyright (C) 2016-2018 Xilinx, Inc. All rights reserved.
 *
 * Authors: Lizhi.Hou@Xilinx.com
 */

#ifndef	_XOCL_DRV_H_
#define	_XOCL_DRV_H_

#include <linux/version.h>
#include <drm/drmP.h>
#include <drm/drm_gem.h>
#include <drm/drm_mm.h>
#include "xclbin.h"
#include "devices.h"
#include <drm/xocl_drm.h>
#include <drm/xmgmt_drm.h>

static inline void xocl_memcpy_fromio(void *buf, void *iomem, u32 size)
{
	int i;

	BUG_ON(size & 0x3);

	for (i = 0; i < size / 4; i++)
		((u32 *)buf)[i] = ioread32((char *)(iomem) + sizeof(u32) * i);
}

static inline void xocl_memcpy_toio(void *iomem, void *buf, u32 size)
{
	int i;

	BUG_ON(size & 0x3);

	for (i = 0; i < size / 4; i++)
		iowrite32(((u32 *)buf)[i], ((char *)(iomem) + sizeof(u32) * i));
}

#define	XOCL_MODULE_NAME	"xocl"
#define	XCLMGMT_MODULE_NAME	"xclmgmt"
#define	ICAP_XCLBIN_V2			"xclbin2"

#define XOCL_MAX_DEVICES	16
#define XOCL_EBUF_LEN           512
#define xocl_sysfs_error(xdev, fmt, args...)	 \
	snprintf(((struct xocl_dev_core *)xdev)->ebuf, XOCL_EBUF_LEN,	\
		 fmt, ##args)
#define MAX_M_COUNT      64

#define	XDEV2DEV(xdev)		(&XDEV(xdev)->pdev->dev)

#define xocl_err(dev, fmt, args...)			\
	dev_err(dev, "%s: "fmt, __func__, ##args)
#define xocl_info(dev, fmt, args...)			\
	dev_info(dev, "%s: "fmt, __func__, ##args)
#define xocl_dbg(dev, fmt, args...)			\
	dev_dbg(dev, "%s: "fmt, __func__, ##args)

#define xocl_xdev_info(xdev, fmt, args...)		\
	xocl_info(XDEV2DEV(xdev), fmt, ##args)
#define xocl_xdev_err(xdev, fmt, args...)		\
	xocl_err(XDEV2DEV(xdev), fmt, ##args)
#define xocl_xdev_dbg(xdev, fmt, args...)		\
	xocl_dbg(XDEV2DEV(xdev), fmt, ##args)

#define	XOCL_DRV_VER_NUM(ma, mi, p)		\
	((ma) * 1000 + (mi) * 100 + (p))

#define	XOCL_READ_REG32(addr)		\
	ioread32(addr)
#define	XOCL_WRITE_REG32(val, addr)	\
	iowrite32(val, addr)

/* xclbin helpers */
#define sizeof_sect(sect, data) \
({ \
	size_t ret; \
	size_t data_size; \
	data_size = (sect) ? sect->m_count * sizeof(typeof(sect->data)) : 0; \
	ret = (sect) ? offsetof(typeof(*sect), data) + data_size : 0; \
	(ret); \
})

#define	XOCL_PL_TO_PCI_DEV(pldev)		\
	to_pci_dev(pldev->dev.parent)

#define XOCL_PL_DEV_TO_XDEV(pldev) \
	pci_get_drvdata(XOCL_PL_TO_PCI_DEV(pldev))

#define	XOCL_QDMA_USER_BAR	2
#define	XOCL_DSA_VERSION(xdev)			\
	(XDEV(xdev)->priv.dsa_ver)

#define XOCL_DSA_IS_MPSOC(xdev)                \
	(XDEV(xdev)->priv.mpsoc)

#define	XOCL_DEV_ID(pdev)			\
	((pci_domain_nr(pdev->bus) << 16) |	\
	PCI_DEVID(pdev->bus->number, pdev->devfn))

#define XOCL_ARE_HOP 0x400000000ull

#define	XOCL_XILINX_VEN		0x10EE
#define	XOCL_CHARDEV_REG_COUNT	16

#ifdef RHEL_RELEASE_VERSION

#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7,6)
#define RHEL_P2P_SUPPORT_74  0
#define RHEL_P2P_SUPPORT_76  1
#elif RHEL_RELEASE_CODE > RHEL_RELEASE_VERSION(7,3) && RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(7,6)
#define RHEL_P2P_SUPPORT_74  1
#define RHEL_P2P_SUPPORT_76  0
#endif
#else
#define RHEL_P2P_SUPPORT_74  0
#define RHEL_P2P_SUPPORT_76  0
#endif


#define RHEL_P2P_SUPPORT (RHEL_P2P_SUPPORT_74 | RHEL_P2P_SUPPORT_76)

#define INVALID_SUBDEVICE ~0U

#define XOCL_INVALID_MINOR -1

extern struct class *xrt_class;

struct drm_xocl_bo;
struct client_ctx;

struct xocl_subdev {
	struct platform_device *pldev;
	void                   *ops;
};

struct xocl_subdev_private {
	int		id;
	bool		is_multi;
	char		priv_data[1];
};

#define	XOCL_GET_SUBDEV_PRIV(dev)				\
	(((struct xocl_subdev_private *)dev_get_platdata(dev))->priv_data)

typedef	void *xdev_handle_t;

struct xocl_pci_funcs {
	int (*intr_config)(xdev_handle_t xdev, u32 intr, bool enable);
	int (*intr_register)(xdev_handle_t xdev, u32 intr,
		irq_handler_t handler, void *arg);
	int (*reset)(xdev_handle_t xdev);
};

#define	XDEV(dev)	((struct xocl_dev_core *)(dev))
#define	XDEV_PCIOPS(xdev)	(XDEV(xdev)->pci_ops)

#define	xocl_user_interrupt_config(xdev, intr, en)	\
	XDEV_PCIOPS(xdev)->intr_config(xdev, intr, en)
#define	xocl_user_interrupt_reg(xdev, intr, handler, arg)	\
	XDEV_PCIOPS(xdev)->intr_register(xdev, intr, handler, arg)
#define xocl_reset(xdev)			\
	(XDEV_PCIOPS(xdev)->reset ? XDEV_PCIOPS(xdev)->reset(xdev) : \
	-ENODEV)

struct xocl_health_thread_arg {
	int (*health_cb)(void *arg);
	void		*arg;
	u32		interval;    /* ms */
	struct device	*dev;
};

struct xocl_drvinst_proc {
	struct list_head	link;
	u32			pid;
	u32			count;
};

struct xocl_drvinst {
	struct device		*dev;
	u32			size;
	atomic_t		ref;
	struct completion	comp;
	struct list_head	open_procs;
	void			*file_dev;
	char			data[1];
};

struct xocl_dev_core {
	struct pci_dev         *pdev;
	int			dev_minor;
	struct xocl_subdev	subdevs[XOCL_SUBDEV_NUM];
	u32			subdev_num;
	struct xocl_pci_funcs  *pci_ops;

	u32			bar_idx;
	void *__iomem           bar_addr;
	resource_size_t		bar_size;
	resource_size_t		feature_rom_offset;

	u32			intr_bar_idx;
	void *__iomem	        intr_bar_addr;
	resource_size_t		intr_bar_size;

	struct task_struct     *health_thread;
	struct xocl_health_thread_arg thread_arg;

	struct xocl_board_private priv;

	char			ebuf[XOCL_EBUF_LEN + 1];

	bool			offline;
};

enum data_kind {
	MIG_CALIB,
	DIMM0_TEMP,
	DIMM1_TEMP,
	DIMM2_TEMP,
	DIMM3_TEMP,
	FPGA_TEMP,
	VCC_BRAM,
	CLOCK_FREQ_0,
	CLOCK_FREQ_1,
	FREQ_COUNTER_0,
	FREQ_COUNTER_1,
	VOL_12V_PEX,
	VOL_12V_AUX,
	CUR_12V_PEX,
	CUR_12V_AUX,
	SE98_TEMP0,
	SE98_TEMP1,
	SE98_TEMP2,
	FAN_TEMP,
	FAN_RPM,
	VOL_3V3_PEX,
	VOL_3V3_AUX,
	VPP_BTM,
	VPP_TOP,
	VOL_5V5_SYS,
	VOL_1V2_TOP,
	VOL_1V2_BTM,
	VOL_1V8,
	VCC_0V9A,
	VOL_12V_SW,
	VTT_MGTA,
	VOL_VCC_INT,
	CUR_VCC_INT,
	IDCODE,
	IPLAYOUT_AXLF,
	MEMTOPO_AXLF,
	CONNECTIVITY_AXLF,
	DEBUG_IPLAYOUT_AXLF,
	PEER_CONN,
	XCLBIN_UUID,
};


#define	XOCL_DSA_PCI_RESET_OFF(xdev_hdl)			\
	(((struct xocl_dev_core *)xdev_hdl)->priv.flags &	\
	XOCL_DSAFLAG_PCI_RESET_OFF)
#define	XOCL_DSA_MB_SCHE_OFF(xdev_hdl)			\
	(((struct xocl_dev_core *)xdev_hdl)->priv.flags &	\
	XOCL_DSAFLAG_MB_SCHE_OFF)
#define	XOCL_DSA_AXILITE_FLUSH_REQUIRED(xdev_hdl)			\
	(((struct xocl_dev_core *)xdev_hdl)->priv.flags &	\
	XOCL_DSAFLAG_AXILITE_FLUSH)

#define	XOCL_DSA_XPR_ON(xdev_hdl)		\
	(((struct xocl_dev_core *)xdev_hdl)->priv.xpr)


#define	SUBDEV(xdev, id)	\
	(XDEV(xdev)->subdevs[id])

/* rom callbacks */
struct xocl_rom_funcs {
	unsigned int (*dsa_version)(struct platform_device *pdev);
	bool (*is_unified)(struct platform_device *pdev);
	bool (*mb_mgmt_on)(struct platform_device *pdev);
	bool (*mb_sched_on)(struct platform_device *pdev);
	uint32_t* (*cdma_addr)(struct platform_device *pdev);
	u16 (*get_ddr_channel_count)(struct platform_device *pdev);
	u64 (*get_ddr_channel_size)(struct platform_device *pdev);
	bool (*is_are)(struct platform_device *pdev);
	bool (*is_aws)(struct platform_device *pdev);
	bool (*verify_timestamp)(struct platform_device *pdev, u64 timestamp);
	u64 (*get_timestamp)(struct platform_device *pdev);
	void (*get_raw_header)(struct platform_device *pdev, void *header);
};
#define ROM_DEV(xdev)	\
	SUBDEV(xdev, XOCL_SUBDEV_FEATURE_ROM).pldev
#define	ROM_OPS(xdev)	\
	((struct xocl_rom_funcs *)SUBDEV(xdev, XOCL_SUBDEV_FEATURE_ROM).ops)
#define	xocl_dsa_version(xdev)		\
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->dsa_version(ROM_DEV(xdev)) : 0)
#define	xocl_is_unified(xdev)		\
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->is_unified(ROM_DEV(xdev)) : true)
#define	xocl_mb_mgmt_on(xdev)		\
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->mb_mgmt_on(ROM_DEV(xdev)) : false)
#define	xocl_mb_sched_on(xdev)		\
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->mb_sched_on(ROM_DEV(xdev)) : false)
#define	xocl_cdma_addr(xdev)		\
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->cdma_addr(ROM_DEV(xdev)) : 0)
#define	xocl_get_ddr_channel_count(xdev) \
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->get_ddr_channel_count(ROM_DEV(xdev)) :\
	0)
#define	xocl_get_ddr_channel_size(xdev) \
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->get_ddr_channel_size(ROM_DEV(xdev)) : 0)
#define	xocl_is_are(xdev)		\
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->is_are(ROM_DEV(xdev)) : false)
#define	xocl_is_aws(xdev)		\
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->is_aws(ROM_DEV(xdev)) : false)
#define	xocl_verify_timestamp(xdev, ts)	\
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->verify_timestamp(ROM_DEV(xdev), ts) : \
	false)
#define	xocl_get_timestamp(xdev) \
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->get_timestamp(ROM_DEV(xdev)) : 0)
#define	xocl_get_raw_header(xdev, header) \
	(ROM_DEV(xdev) ? ROM_OPS(xdev)->get_raw_header(ROM_DEV(xdev), header) :\
	NULL)

/* dma callbacks */
struct xocl_dma_funcs {
	ssize_t (*migrate_bo)(struct platform_device *pdev,
			      struct sg_table *sgt, u32 dir, u64 paddr, u32 channel, u64 sz);
	int (*ac_chan)(struct platform_device *pdev, u32 dir);
	void (*rel_chan)(struct platform_device *pdev, u32 dir, u32 channel);
	u32 (*get_chan_count)(struct platform_device *pdev);
	u64 (*get_chan_stat)(struct platform_device *pdev, u32 channel,
			     u32 write);
	u64 (*get_str_stat)(struct platform_device *pdev, u32 q_idx);
	int (*user_intr_config)(struct platform_device *pdev, u32 intr, bool en);
	int (*user_intr_register)(struct platform_device *pdev, u32 intr,
				  irq_handler_t handler, void *arg, int event_fd);
	int (*user_intr_unreg)(struct platform_device *pdev, u32 intr);
	void *(*get_drm_handle)(struct platform_device *pdev);
};

#define DMA_DEV(xdev)	\
	SUBDEV(xdev, XOCL_SUBDEV_DMA).pldev
#define	DMA_OPS(xdev)	\
	((struct xocl_dma_funcs *)SUBDEV(xdev, XOCL_SUBDEV_DMA).ops)
#define	xocl_migrate_bo(xdev, sgt, write, paddr, chan, len)	\
	(DMA_DEV(xdev) ? DMA_OPS(xdev)->migrate_bo(DMA_DEV(xdev), \
	sgt, write, paddr, chan, len) : 0)
#define	xocl_acquire_channel(xdev, dir)		\
	(DMA_DEV(xdev) ? DMA_OPS(xdev)->ac_chan(DMA_DEV(xdev), dir) : \
	-ENODEV)
#define	xocl_release_channel(xdev, dir, chan)	\
	(DMA_DEV(xdev) ? DMA_OPS(xdev)->rel_chan(DMA_DEV(xdev), dir, \
	chan) : NULL)
#define	xocl_get_chan_count(xdev)		\
	(DMA_DEV(xdev) ? DMA_OPS(xdev)->get_chan_count(DMA_DEV(xdev)) \
	: 0)
#define	xocl_get_chan_stat(xdev, chan, write)		\
	(DMA_DEV(xdev) ? DMA_OPS(xdev)->get_chan_stat(DMA_DEV(xdev), \
	chan, write) : 0)
#define xocl_dma_intr_config(xdev, irq, en)			\
	(DMA_DEV(xdev) ? DMA_OPS(xdev)->user_intr_config(DMA_DEV(xdev), \
	irq, en) : -ENODEV)
#define xocl_dma_intr_register(xdev, irq, handler, arg, event_fd)	\
	(DMA_DEV(xdev) ? DMA_OPS(xdev)->user_intr_register(DMA_DEV(xdev), \
	irq, handler, arg, event_fd) : -ENODEV)
#define xocl_dma_intr_unreg(xdev, irq)				\
	(DMA_DEV(xdev) ? DMA_OPS(xdev)->user_intr_unreg(DMA_DEV(xdev),	\
	irq) : -ENODEV)
#define	xocl_dma_get_drm_handle(xdev)				\
	(DMA_DEV(xdev) ? DMA_OPS(xdev)->get_drm_handle(DMA_DEV(xdev)) : \
	NULL)

/* mb_scheduler callbacks */
struct xocl_mb_scheduler_funcs {
	int (*create_client)(struct platform_device *pdev, void **priv);
	void (*destroy_client)(struct platform_device *pdev, void **priv);
	uint (*poll_client)(struct platform_device *pdev, struct file *filp,
		poll_table *wait, void *priv);
	int (*client_ioctl)(struct platform_device *pdev, int op,
		void *data, void *drm_filp);
	int (*stop)(struct platform_device *pdev);
	int (*reset)(struct platform_device *pdev);
};
#define	MB_SCHEDULER_DEV(xdev)	\
	SUBDEV(xdev, XOCL_SUBDEV_MB_SCHEDULER).pldev
#define	MB_SCHEDULER_OPS(xdev)	\
	((struct xocl_mb_scheduler_funcs *)SUBDEV(xdev, 	\
		XOCL_SUBDEV_MB_SCHEDULER).ops)
#define	xocl_exec_create_client(xdev, priv)		\
	(MB_SCHEDULER_DEV(xdev) ?			\
	MB_SCHEDULER_OPS(xdev)->create_client(MB_SCHEDULER_DEV(xdev), priv) : \
	-ENODEV)
#define	xocl_exec_destroy_client(xdev, priv)		\
	(MB_SCHEDULER_DEV(xdev) ?			\
	MB_SCHEDULER_OPS(xdev)->destroy_client(MB_SCHEDULER_DEV(xdev), priv) : \
	NULL)
#define	xocl_exec_poll_client(xdev, filp, wait, priv)		\
	(MB_SCHEDULER_DEV(xdev) ? 				\
	MB_SCHEDULER_OPS(xdev)->poll_client(MB_SCHEDULER_DEV(xdev), filp, \
	wait, priv) : 0)
#define	xocl_exec_client_ioctl(xdev, op, data, drm_filp)		\
	(MB_SCHEDULER_DEV(xdev) ?				\
	MB_SCHEDULER_OPS(xdev)->client_ioctl(MB_SCHEDULER_DEV(xdev),	\
	op, data, drm_filp) : -ENODEV)
#define	xocl_exec_stop(xdev)		\
	(MB_SCHEDULER_DEV(xdev) ? 				\
	 MB_SCHEDULER_OPS(xdev)->stop(MB_SCHEDULER_DEV(xdev)) : \
	 -ENODEV)
#define	xocl_exec_reset(xdev)		\
	(MB_SCHEDULER_DEV(xdev) ? 				\
	 MB_SCHEDULER_OPS(xdev)->reset(MB_SCHEDULER_DEV(xdev)) : \
	 -ENODEV)

#define XOCL_MEM_TOPOLOGY(xdev)						\
	((struct mem_topology *)					\
	 xocl_icap_get_data(xdev, MEMTOPO_AXLF))
#define XOCL_IP_LAYOUT(xdev)						\
	((struct ip_layout *)						\
	 xocl_icap_get_data(xdev, IPLAYOUT_AXLF))

#define	XOCL_IS_DDR_USED(xdev, ddr)					\
	(XOCL_MEM_TOPOLOGY(xdev)->m_mem_data[ddr].m_used == 1)
#define	XOCL_DDR_COUNT_UNIFIED(xdev)		\
	(XOCL_MEM_TOPOLOGY(xdev) ? XOCL_MEM_TOPOLOGY(xdev)->m_count : 0)
#define	XOCL_DDR_COUNT(xdev)			\
	((xocl_is_unified(xdev) ? XOCL_DDR_COUNT_UNIFIED(xdev) :	\
	xocl_get_ddr_channel_count(xdev)))

/* sysmon callbacks */
enum {
	XOCL_SYSMON_PROP_TEMP,
	XOCL_SYSMON_PROP_TEMP_MAX,
	XOCL_SYSMON_PROP_TEMP_MIN,
	XOCL_SYSMON_PROP_VCC_INT,
	XOCL_SYSMON_PROP_VCC_INT_MAX,
	XOCL_SYSMON_PROP_VCC_INT_MIN,
	XOCL_SYSMON_PROP_VCC_AUX,
	XOCL_SYSMON_PROP_VCC_AUX_MAX,
	XOCL_SYSMON_PROP_VCC_AUX_MIN,
	XOCL_SYSMON_PROP_VCC_BRAM,
	XOCL_SYSMON_PROP_VCC_BRAM_MAX,
	XOCL_SYSMON_PROP_VCC_BRAM_MIN,
};
struct xocl_sysmon_funcs {
	int (*get_prop)(struct platform_device *pdev, u32 prop, void *val);
};
#define	SYSMON_DEV(xdev)	\
	SUBDEV(xdev, XOCL_SUBDEV_SYSMON).pldev
#define	SYSMON_OPS(xdev)	\
	((struct xocl_sysmon_funcs *)SUBDEV(xdev, 	\
		XOCL_SUBDEV_SYSMON).ops)
#define	xocl_sysmon_get_prop(xdev, prop, val)		\
	(SYSMON_DEV(xdev) ? SYSMON_OPS(xdev)->get_prop(SYSMON_DEV(xdev), \
	prop, val) : -ENODEV)

/* firewall callbacks */
enum {
	XOCL_AF_PROP_TOTAL_LEVEL,
	XOCL_AF_PROP_STATUS,
	XOCL_AF_PROP_LEVEL,
	XOCL_AF_PROP_DETECTED_STATUS,
	XOCL_AF_PROP_DETECTED_LEVEL,
	XOCL_AF_PROP_DETECTED_TIME,
};
struct xocl_firewall_funcs {
	int (*get_prop)(struct platform_device *pdev, u32 prop, void *val);
	int (*clear_firewall)(struct platform_device *pdev);
	u32 (*check_firewall)(struct platform_device *pdev, int *level);
};
#define AF_DEV(xdev)	\
	SUBDEV(xdev, XOCL_SUBDEV_AF).pldev
#define	AF_OPS(xdev)	\
	((struct xocl_firewall_funcs *)SUBDEV(xdev,	\
	XOCL_SUBDEV_AF).ops)
#define	xocl_af_get_prop(xdev, prop, val)		\
	(AF_DEV(xdev) ? AF_OPS(xdev)->get_prop(AF_DEV(xdev), prop, val) : \
	-ENODEV)
#define	xocl_af_check(xdev, level)			\
	(AF_DEV(xdev) ? AF_OPS(xdev)->check_firewall(AF_DEV(xdev), level) : 0)
#define	xocl_af_clear(xdev)				\
	(AF_DEV(xdev) ? AF_OPS(xdev)->clear_firewall(AF_DEV(xdev)) : -ENODEV)

/* microblaze callbacks */
struct xocl_mb_funcs {
	void (*reset)(struct platform_device *pdev);
	int (*stop)(struct platform_device *pdev);
	int (*load_mgmt_image)(struct platform_device *pdev, const char *buf,
		u32 len);
	int (*load_sche_image)(struct platform_device *pdev, const char *buf,
		u32 len);
	int (*get_data)(struct platform_device *pdev, enum data_kind kind);
};

struct xocl_dna_funcs {
	u32 (*status)(struct platform_device *pdev);
	u32 (*capability)(struct platform_device *pdev);
	void (*write_cert)(struct platform_device *pdev, const uint32_t *buf, u32 len);
};

#define	XMC_DEV(xdev)		\
	SUBDEV(xdev, XOCL_SUBDEV_XMC).pldev
#define	XMC_OPS(xdev)		\
	((struct xocl_mb_funcs *)SUBDEV(xdev,	\
	XOCL_SUBDEV_XMC).ops)

#define	DNA_DEV(xdev)		\
	SUBDEV(xdev, XOCL_SUBDEV_DNA).pldev
#define	DNA_OPS(xdev)		\
	((struct xocl_dna_funcs *)SUBDEV(xdev,	\
	XOCL_SUBDEV_DNA).ops)
#define	xocl_dna_status(xdev)			\
	(DNA_DEV(xdev) ? DNA_OPS(xdev)->status(DNA_DEV(xdev)) : 0)
#define	xocl_dna_capability(xdev)			\
	(DNA_DEV(xdev) ? DNA_OPS(xdev)->capability(DNA_DEV(xdev)) : 2)
#define xocl_dna_write_cert(xdev, data, len)  \
	(DNA_DEV(xdev) ? DNA_OPS(xdev)->write_cert(DNA_DEV(xdev), data, len) : 0)

#define	MB_DEV(xdev)		\
	SUBDEV(xdev, XOCL_SUBDEV_MB).pldev
#define	MB_OPS(xdev)		\
	((struct xocl_mb_funcs *)SUBDEV(xdev,	\
	XOCL_SUBDEV_MB).ops)
#define	xocl_mb_reset(xdev)			\
	(XMC_DEV(xdev) ? XMC_OPS(xdev)->reset(XMC_DEV(xdev)) : \
	(MB_DEV(xdev) ? MB_OPS(xdev)->reset(MB_DEV(xdev)) : NULL))

#define	xocl_mb_stop(xdev)			\
	(XMC_DEV(xdev) ? XMC_OPS(xdev)->stop(XMC_DEV(xdev)) : \
	(MB_DEV(xdev) ? MB_OPS(xdev)->stop(MB_DEV(xdev)) : -ENODEV))

#define xocl_mb_load_mgmt_image(xdev, buf, len)		\
	(XMC_DEV(xdev) ? XMC_OPS(xdev)->load_mgmt_image(XMC_DEV(xdev), buf, len) :\
	(MB_DEV(xdev) ? MB_OPS(xdev)->load_mgmt_image(MB_DEV(xdev), buf, len) :\
	-ENODEV))
#define xocl_mb_load_sche_image(xdev, buf, len)		\
	(XMC_DEV(xdev) ? XMC_OPS(xdev)->load_sche_image(XMC_DEV(xdev), buf, len) :\
	(MB_DEV(xdev) ? MB_OPS(xdev)->load_sche_image(MB_DEV(xdev), buf, len) :\
	-ENODEV))

#define xocl_xmc_get_data(xdev, cmd)			\
	(XMC_DEV(xdev) ? XMC_OPS(xdev)->get_data(XMC_DEV(xdev), cmd) : -ENODEV)

/*
 * mailbox callbacks
 */
enum mailbox_request {
	MAILBOX_REQ_UNKNOWN = 0,
	MAILBOX_REQ_TEST_READY,
	MAILBOX_REQ_TEST_READ,
	MAILBOX_REQ_LOCK_BITSTREAM,
	MAILBOX_REQ_UNLOCK_BITSTREAM,
	MAILBOX_REQ_HOT_RESET,
	MAILBOX_REQ_FIREWALL,
	MAILBOX_REQ_GPCTL,
	MAILBOX_REQ_LOAD_XCLBIN_KADDR,
	MAILBOX_REQ_LOAD_XCLBIN,
	MAILBOX_REQ_RECLOCK,
	MAILBOX_REQ_PEER_DATA,
	MAILBOX_REQ_CONN_EXPL,
};

enum mb_cmd_type {
	MB_CMD_DEFAULT = 0,
	MB_CMD_LOAD_XCLBIN,
	MB_CMD_RECLOCK,
	MB_CMD_CONN_EXPL,
	MB_CMD_LOAD_XCLBIN_KADDR,
	MB_CMD_READ_FROM_PEER,
};
struct mailbox_req_bitstream_lock {
	pid_t pid;
	uuid_t uuid;
};

struct mailbox_subdev_peer {
	enum data_kind kind;
};

struct mailbox_bitstream_kaddr {
	uint64_t addr;
};

struct mailbox_gpctl {
	enum mb_cmd_type cmd_type;
	uint32_t data_total_len;
	uint64_t priv_data;
	void *data_ptr;
};


struct mailbox_req {
	enum mailbox_request req;
	uint32_t data_total_len;
	uint64_t flags;
	char data[0];
};

#define MB_PROT_VER_MAJOR 0
#define MB_PROT_VER_MINOR 5
#define MB_PROTOCOL_VER   ((MB_PROT_VER_MAJOR<<8) + MB_PROT_VER_MINOR)

#define MB_PEER_CONNECTED 0x1
#define MB_PEER_SAME_DOM  0x2
#define MB_PEER_SAMEDOM_CONNECTED (MB_PEER_CONNECTED | MB_PEER_SAME_DOM)

typedef	void (*mailbox_msg_cb_t)(void *arg, void *data, size_t len,
	u64 msgid, int err);
struct xocl_mailbox_funcs {
	int (*request)(struct platform_device *pdev, void *req,
		size_t reqlen, void *resp, size_t *resplen,
		mailbox_msg_cb_t cb, void *cbarg);
	int (*post)(struct platform_device *pdev, u64 req_id,
		void *resp, size_t len);
	int (*listen)(struct platform_device *pdev,
		mailbox_msg_cb_t cb, void *cbarg);
	int (*reset)(struct platform_device *pdev, bool end_of_reset);
	int (*get_data)(struct platform_device *pdev, enum data_kind kind);
};
#define	MAILBOX_DEV(xdev)	SUBDEV(xdev, XOCL_SUBDEV_MAILBOX).pldev
#define	MAILBOX_OPS(xdev)	\
	((struct xocl_mailbox_funcs *)SUBDEV(xdev, XOCL_SUBDEV_MAILBOX).ops)
#define MAILBOX_READY(xdev)	(MAILBOX_DEV(xdev) && MAILBOX_OPS(xdev))
#define	xocl_peer_request(xdev, req, reqlen, resp, resplen, cb, cbarg)		\
	(MAILBOX_READY(xdev) ? MAILBOX_OPS(xdev)->request(MAILBOX_DEV(xdev), \
	req, reqlen, resp, resplen, cb, cbarg) : -ENODEV)
#define	xocl_peer_response(xdev, reqid, buf, len)			\
	(MAILBOX_READY(xdev) ? MAILBOX_OPS(xdev)->post(MAILBOX_DEV(xdev), \
	reqid, buf, len) : -ENODEV)
#define	xocl_peer_notify(xdev, req, reqlen)					\
	(MAILBOX_READY(xdev) ? MAILBOX_OPS(xdev)->post(MAILBOX_DEV(xdev), 0, \
	req, reqlen) : -ENODEV)
#define	xocl_peer_listen(xdev, cb, cbarg)				\
	(MAILBOX_READY(xdev) ? MAILBOX_OPS(xdev)->listen(MAILBOX_DEV(xdev), \
	cb, cbarg) : -ENODEV)
#define	xocl_mailbox_reset(xdev, end)				\
	(MAILBOX_READY(xdev) ? MAILBOX_OPS(xdev)->reset(MAILBOX_DEV(xdev), \
	end) : -ENODEV)
#define	xocl_mailbox_get_data(xdev, kind)				\
	(MAILBOX_READY(xdev) ? MAILBOX_OPS(xdev)->get_data(MAILBOX_DEV(xdev), kind) \
		: -ENODEV)

struct xocl_icap_funcs {
	void (*reset_axi_gate)(struct platform_device *pdev);
	int (*reset_bitstream)(struct platform_device *pdev);
	int (*download_bitstream_axlf)(struct platform_device *pdev,
		const void __user *arg);
	int (*download_boot_firmware)(struct platform_device *pdev);
	int (*ocl_set_freq)(struct platform_device *pdev,
		unsigned int region, unsigned short *freqs, int num_freqs);
	int (*ocl_get_freq)(struct platform_device *pdev,
		unsigned int region, unsigned short *freqs, int num_freqs);
	int (*ocl_update_clock_freq_topology)(struct platform_device *pdev, struct xclmgmt_ioc_freqscaling *freqs);
	int (*ocl_lock_bitstream)(struct platform_device *pdev,
		const uuid_t *uuid, pid_t pid);
	int (*ocl_unlock_bitstream)(struct platform_device *pdev,
		const uuid_t *uuid, pid_t pid);
	uint64_t (*get_data)(struct platform_device *pdev,
		enum data_kind kind);
};
#define	ICAP_DEV(xdev)	SUBDEV(xdev, XOCL_SUBDEV_ICAP).pldev
#define	ICAP_OPS(xdev)							\
	((struct xocl_icap_funcs *)SUBDEV(xdev, XOCL_SUBDEV_ICAP).ops)
#define	xocl_icap_reset_axi_gate(xdev)					\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->reset_axi_gate(ICAP_DEV(xdev)) :		\
	NULL)
#define	xocl_icap_reset_bitstream(xdev)					\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->reset_bitstream(ICAP_DEV(xdev)) :		\
	-ENODEV)
#define	xocl_icap_download_axlf(xdev, xclbin)				\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->download_bitstream_axlf(ICAP_DEV(xdev), xclbin) : \
	-ENODEV)
#define	xocl_icap_download_boot_firmware(xdev)				\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->download_boot_firmware(ICAP_DEV(xdev)) :	\
	-ENODEV)
#define	xocl_icap_ocl_get_freq(xdev, region, freqs, num)		\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->ocl_get_freq(ICAP_DEV(xdev), region, freqs, num) : \
	-ENODEV)
#define	xocl_icap_ocl_update_clock_freq_topology(xdev, freqs)		\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->ocl_update_clock_freq_topology(ICAP_DEV(xdev), freqs) : \
	-ENODEV)
#define	xocl_icap_ocl_set_freq(xdev, region, freqs, num)		\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->ocl_set_freq(ICAP_DEV(xdev), region, freqs, num) : \
	-ENODEV)
#define	xocl_icap_lock_bitstream(xdev, uuid, pid)			\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->ocl_lock_bitstream(ICAP_DEV(xdev), uuid, pid) :	\
	-ENODEV)
#define	xocl_icap_unlock_bitstream(xdev, uuid, pid)			\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->ocl_unlock_bitstream(ICAP_DEV(xdev), uuid, pid) : \
	-ENODEV)
#define	xocl_icap_get_data(xdev, kind)				\
	(ICAP_OPS(xdev) ? 						\
	ICAP_OPS(xdev)->get_data(ICAP_DEV(xdev), kind) : \
	0)

/* helper functions */
xdev_handle_t xocl_get_xdev(struct platform_device *pdev);
void xocl_init_dsa_priv(xdev_handle_t xdev_hdl);

/* subdev functions */
int xocl_subdev_create_multi_inst(xdev_handle_t xdev_hdl,
	struct xocl_subdev_info *sdev_info);
int xocl_subdev_create_one(xdev_handle_t xdev_hdl,
	struct xocl_subdev_info *sdev_info);
int xocl_subdev_create_by_id(xdev_handle_t xdev_hdl, int id);
int xocl_subdev_create_all(xdev_handle_t xdev_hdl,
			   struct xocl_subdev_info *sdev_info, u32 subdev_num);
void xocl_subdev_destroy_one(xdev_handle_t xdev_hdl, u32 subdev_id);
void xocl_subdev_destroy_all(xdev_handle_t xdev_hdl);
void xocl_subdev_destroy_by_id(xdev_handle_t xdev_hdl, int id);

int xocl_subdev_create_by_name(xdev_handle_t xdev_hdl, char *name);
int xocl_subdev_destroy_by_name(xdev_handle_t xdev_hdl, char *name);

int xocl_subdev_get_devinfo(uint32_t subdev_id,
			    struct xocl_subdev_info *subdev_info, struct resource *res);

void xocl_subdev_register(struct platform_device *pldev, u32 id,
			  void *cb_funcs);
void xocl_fill_dsa_priv(xdev_handle_t xdev_hdl, struct xocl_board_private *in);
int xocl_xrt_version_check(xdev_handle_t xdev_hdl,
			   struct axlf *bin_obj, bool major_only);
int xocl_alloc_dev_minor(xdev_handle_t xdev_hdl);
void xocl_free_dev_minor(xdev_handle_t xdev_hdl);

/* context helpers */
extern struct mutex xocl_drvinst_mutex;
extern struct xocl_drvinst *xocl_drvinst_array[XOCL_MAX_DEVICES * 10];

void *xocl_drvinst_alloc(struct device *dev, u32 size);
void xocl_drvinst_free(void *data);
void *xocl_drvinst_open(void *file_dev);
void xocl_drvinst_close(void *data);
void xocl_drvinst_set_filedev(void *data, void *file_dev);

/* health thread functions */
int health_thread_start(xdev_handle_t xdev);
int health_thread_stop(xdev_handle_t xdev);

/* init functions */
int __init xocl_init_userpf(void);
void xocl_fini_fini_userpf(void);

int __init xocl_init_drv_user_qdma(void);
void xocl_fini_drv_user_qdma(void);

int __init xocl_init_feature_rom(void);
void xocl_fini_feature_rom(void);

int __init xocl_init_xdma(void);
void xocl_fini_xdma(void);

int __init xocl_init_qdma(void);
void xocl_fini_qdma(void);

int __init xocl_init_mb_scheduler(void);
void xocl_fini_mb_scheduler(void);

int __init xocl_init_xvc(void);
void xocl_fini_xvc(void);

int __init xocl_init_firewall(void);
void xocl_fini_firewall(void);

int __init xocl_init_sysmon(void);
void xocl_fini_sysmon(void);

int __init xocl_init_mb(void);
void xocl_fini_mb(void);

int __init xocl_init_xiic(void);
void xocl_fini_xiic(void);

int __init xocl_init_mailbox(void);
void xocl_fini_mailbox(void);

int __init xocl_init_icap(void);
void xocl_fini_icap(void);

int __init xocl_init_mig(void);
void xocl_fini_mig(void);

int __init xocl_init_xmc(void);
void xocl_fini_xmc(void);

int __init xocl_init_dna(void);
void xocl_fini_dna(void);

int __init xocl_init_fmgr(void);
void xocl_fini_fmgr(void);
#endif
