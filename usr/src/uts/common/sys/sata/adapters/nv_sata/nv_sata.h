/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _NV_SATA_H
#define	_NV_SATA_H

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#ifdef	__cplusplus
extern "C" {
#endif

#define	NV_MAX_PORTS(nvc) nvc->nvc_sata_hba_tran.sata_tran_hba_num_cports

typedef struct nv_port nv_port_t;

typedef struct nv_ctl {
	/*
	 * Each of these are specific to the chipset in use.
	 */
	uint_t		(*nvc_interrupt)(caddr_t arg1, caddr_t arg2);
	void		(*nvc_reg_init)(struct nv_ctl *nvc,
			    ddi_acc_handle_t pci_conf_handle);

	dev_info_t	*nvc_dip; /* devinfo pointer of controller */

	struct nv_port	*nvc_port; /* array of pointers to port struct */

	/*
	 * handle and base address to register space.
	 *
	 * 0: port 0 task file
	 * 1: port 0 status
	 * 2: port 1 task file
	 * 3: port 1 status
	 * 4: bus master for both ports
	 * 5: extended registers for SATA features
	 */
	ddi_acc_handle_t nvc_bar_hdl[6];
	uchar_t		*nvc_bar_addr[6];

	/*
	 * sata registers in bar 5 which are shared on all devices
	 * on the channel.
	 */
	uint32_t	*nvc_mcp55_ctl;
	uint32_t	*nvc_mcp55_ncq; /* NCQ status control bits */

	kmutex_t	nvc_mutex; /* ctrl level lock */

	ddi_intr_handle_t *nvc_htable;	/* For array of interrupts */
	int		 nvc_intr_type;	/* What type of interrupt */
	int		nvc_intr_cnt;	/* # of intrs count returned */
	size_t		nvc_intr_size;	/* Size of intr array to */
	uint_t		nvc_intr_pri;   /* Interrupt priority */
	int		nvc_intr_cap;	/* Interrupt capabilities */
	uint8_t		*nvc_mcp04_int_status; /* interrupt status mcp04 */

	sata_hba_tran_t	nvc_sata_hba_tran; /* sata_hba_tran for ctrl */

	/*
	 * enable/disable interrupts, controller specific
	 */
	void		(*nvc_set_intr)(nv_port_t *nvp, int flag);
	int		nvc_state;	/* state flags of ctrl see below */
	uint8_t		nvc_revid;	/* PCI revid of device */
} nv_ctl_t;


struct nv_port {

	struct nv_ctl	*nvp_ctlp; /* back pointer to controller */

	uint8_t		nvp_port_num; /* port number, ie 1 or 2 */

	uint8_t		nvp_type;	/* SATA_DTYPE_{NONE,ATADISK,UNKNOWN} */
	uint32_t	nvp_signature;	/* sig acquired from task file regs */
	uchar_t		*nvp_cmd_addr;	/* base addr for cmd regs for port */
	uchar_t		*nvp_bm_addr;	/* base addr for bus master for port */
	uchar_t		*nvp_ctl_addr;	/* base addr for ctrl regs for port */

	ddi_acc_handle_t nvp_cmd_hdl;
	uchar_t		*nvp_data;	/* data register */
	uchar_t		*nvp_error;	/* error register (read) */
	uchar_t		*nvp_feature;	/* features (write) */
	uchar_t		*nvp_count;	/* sector count */
	uchar_t		*nvp_sect;	/* sector number */
	uchar_t		*nvp_lcyl;	/* cylinder low byte */
	uchar_t		*nvp_hcyl;	/* cylinder high byte */
	uchar_t		*nvp_drvhd;	/* drive/head register */
	uchar_t		*nvp_status;	/* status/command register */
	uchar_t		*nvp_cmd;	/* status/command register */

	ddi_acc_handle_t nvp_ctl_hdl;
	uchar_t		*nvp_altstatus; /* alternate status (read) */
	uchar_t		*nvp_devctl;	/* device control (write) */

	ddi_acc_handle_t nvp_bm_hdl;
	uchar_t		*nvp_bmisx;
	uint32_t	*nvp_bmidtpx;
	uchar_t		*nvp_bmicx;

	ddi_dma_handle_t *nvp_sg_dma_hdl; /* dma handle to prd table */
	caddr_t		 *nvp_sg_addr;	  /* virtual addr of prd table */
	uint32_t	 *nvp_sg_paddr;   /* physical address of prd table */
	ddi_acc_handle_t *nvp_sg_acc_hdl; /* mem acc handle to the prd table */

	uint32_t	*nvp_sstatus;
	uint32_t	*nvp_serror;
	uint32_t	*nvp_sctrl;
	uint32_t	*nvp_sactive;

	kmutex_t	nvp_mutex;	/* main per port mutex */
	kcondvar_t	nvp_poll_cv;	/* handshake cv between poll & isr */

	/*
	 * nvp_slot is a pointer to an array of nv_slot
	 */
	struct nv_slot	*nvp_slot;
	uint32_t	nvp_sactive_cache; /* cache of SACTIVE */
	uint8_t		nvp_queue_depth;

	/*
	 * NCQ flow control.  During NCQ operation, no other commands
	 * allowed.  The following are used to enforce this.
	 */
	int		nvp_ncq_run;
	int		nvp_non_ncq_run;

	timeout_id_t	nvp_timeout_id;

	clock_t		nvp_reset_time;	/* time of last reset */
	clock_t		nvp_probe_time;	/* time when probe began */
	clock_t		nvp_link_lost_time; /* time link lost was noticed */

	int		nvp_state; /* state of port. flags defined below */

	uint16_t	*nvp_mcp55_int_status;
	uint16_t	*nvp_mcp55_int_ctl;
};


typedef struct nv_device_table {
	ushort_t vendor_id;	/* vendor id */
	ushort_t device_id;	/* device id */
	ushort_t type;		/* chipset type, mcp04 or mcp55 */
} nv_device_table_t;


typedef struct nv_slot {
	caddr_t		nvslot_v_addr;	/* I/O buffer address */
	size_t		nvslot_byte_count; /* # bytes left to read/write */
	sata_pkt_t	*nvslot_spkt;
	uint8_t		nvslot_rqsense_buff[SATA_ATAPI_RQSENSE_LEN];
	clock_t		nvslot_stime;
	int		(*nvslot_start)(nv_port_t *nvp, int queue);
	void		(*nvslot_intr)(nv_port_t *nvp,
			    struct nv_slot *nv_slotp);
	uint32_t	nvslot_flags;
} nv_slot_t;


/*
 * nvslot_flags
 */
#define	NVSLOT_COMPLETE 0x01
#define	NVSLOT_NCQ	0x02	/* NCQ is active */
#define	NVSLOT_RQSENSE	0x04	/* processing request sense */

/*
 * state values for nv_attach
 */
#define	ATTACH_PROGRESS_NONE			(1 << 0)
#define	ATTACH_PROGRESS_STATEP_ALLOC		(1 << 1)
#define	ATTACH_PROGRESS_PCI_HANDLE		(1 << 2)
#define	ATTACH_PROGRESS_BARS			(1 << 3)
#define	ATTACH_PROGRESS_INTR_ADDED		(1 << 4)
#define	ATTACH_PROGRESS_MUTEX_INIT		(1 << 5)
#define	ATTACH_PROGRESS_CTL_SETUP		(1 << 6)
#define	ATTACH_PROGRESS_TRAN_SETUP		(1 << 7)
#define	ATTACH_PROGRESS_COUNT			(1 << 8)
#define	ATTACH_PROGRESS_CONF_HANDLE		(1 << 9)
#define	ATTACH_PROGRESS_SATA_MODULE		(1 << 10)

#ifdef DEBUG

#define	NV_DEBUG		1

#endif /* DEBUG */


/*
 * nv_debug_flags
 */
#define	NVDBG_ALWAYS	0x0001
#define	NVDBG_INIT	0x0002
#define	NVDBG_ENTRY	0x0004
#define	NVDBG_DELIVER	0x0008
#define	NVDBG_EVENT	0x0010
#define	NVDBG_SYNC	0x0020
#define	NVDBG_PKTCOMP	0x0040
#define	NVDBG_TIMEOUT	0x0080
#define	NVDBG_INFO	0x0100
#define	NVDBG_VERBOSE	0x0200
#define	NVDBG_INTR	0x0400
#define	NVDBG_ERRS	0x0800
#define	NVDBG_COOKIES	0x1000
#define	NVDBG_HOT	0x2000
#define	NVDBG_PROBE	0x4000
#define	NVDBG_ATAPI	0x8000

#ifdef DEBUG
#define	NVLOG(a) nv_log a
#else
#define	NVLOG(a)
#endif

#define	NV_SUCCESS	0
#define	NV_FAILURE	-1

/*
 * indicates whether nv_wait functions can sleep or not.
 */
#define	NV_SLEEP	1
#define	NV_NOSLEEP	2

/*
 * port offsets from base address ioaddr1
 */
#define	NV_DATA		0x00	/* data register 			*/
#define	NV_ERROR	0x01	/* error register (read)		*/
#define	NV_FEATURE	0x01	/* features (write)			*/
#define	NV_COUNT	0x02    /* sector count 			*/
#define	NV_SECT		0x03	/* sector number 			*/
#define	NV_LCYL		0x04	/* cylinder low byte 			*/
#define	NV_HCYL		0x05	/* cylinder high byte 			*/
#define	NV_DRVHD	0x06    /* drive/head register 			*/
#define	NV_STATUS	0x07	/* status/command register 		*/
#define	NV_CMD		0x07	/* status/command register 		*/

/*
 * port offsets from base address ioaddr2
 */
#define	NV_ALTSTATUS	0x02	/* alternate status (read)		*/
#define	NV_DEVCTL	0x02	/* device control (write)		*/

/*
 * device control register
 */
#define	ATDC_NIEN    	0x02    /* disable interrupts */
#define	ATDC_SRST	0x04	/* controller reset */
#define	ATDC_D3		0x08	/* mysterious bit */
#define	ATDC_HOB	0x80	/* high order byte to read 48-bit values */


#define	MCP55_CTL		0x400 /* queuing control */
#define	MCP55_INT_STATUS	0x440 /* status bits for interrupt */
#define	MCP55_INT_CTL		0x444 /* enable bits for interrupt */
#define	MCP55_NCQ		0x448 /* NCQ status and ctrl bits */

/*
 * if either of these bits are set, when using NCQ, if no other commands are
 * active while a new command is started, DMA engine can be programmed ahead
 * of time to save extra interrupt.  Presumably pre-programming is discarded
 * if a subsequent command ends up finishing first.
 */
#define	MCP_SATA_AE_NCQ_PDEV_FIRST_CMD	(1 << 7)
#define	MCP_SATA_AE_NCQ_SDEV_FIRST_CMD	(1 << 23)

/*
 * bit definitions to indicate which NCQ command requires
 * DMA setup.
 */
#define	MCP_SATA_AE_NCQ_PDEV_DMA_SETUP_TAG_SHIFT	2
#define	MCP_SATA_AE_NCQ_SDEV_DMA_SETUP_TAG_SHIFT	18
#define	MCP_SATA_AE_NCQ_DMA_SETUP_TAG_MASK		0x1f


/*
 * Bits for NV_MCP55_INT_CTL and NV_MCP55_INT_STATUS
 */
#define	MCP55_INT_SNOTIFY	0x200	/* snotification set */
#define	MCP55_INT_SERROR	0x100	/* serror set */
#define	MCP55_INT_DMA_SETUP	0x80	/* DMA to be programmed */
#define	MCP55_INT_DH_REGFIS	0x40	/* REGFIS received */
#define	MCP55_INT_SDB_FIS	0x20	/* SDB FIS */
#define	MCP55_INT_TX_BACKOUT	0x10	/* TX backout */
#define	MCP55_INT_REM		0x08	/* device removed */
#define	MCP55_INT_ADD		0x04	/* device added */
#define	MCP55_INT_PM		0x02	/* power changed */
#define	MCP55_INT_COMPLETE	0x01	/* device interrupt */

/*
 * Bits above that are not used for now.
 */
#define	MCP55_INT_IGNORE (MCP55_INT_DMA_SETUP|MCP55_INT_DH_REGFIS|\
	MCP55_INT_SDB_FIS|MCP55_INT_TX_BACKOUT|MCP55_INT_PM|\
	MCP55_INT_SNOTIFY|MCP55_INT_SERROR)

/*
 * Bits for MCP_SATA_AE_CTL
 */
#define	MCP_SATA_AE_CTL_PRI_SWNCQ	(1 << 1) /* software NCQ chan 0 */
#define	MCP_SATA_AE_CTL_SEC_SWNCQ	(1 << 2) /* software NCQ chan 1 */

#define	NV_DELAY_NSEC(wait_ns) \
{ \
	hrtime_t start, end; \
	start = end =  gethrtime(); \
	while ((end - start) < wait_ns) \
		end = gethrtime(); \
}

/*
 * signatures in task file registers after device reset
 */
#define	NV_SIG_DISK	0x00000101
#define	NV_SIG_ATAPI	0xeb140101
#define	NV_SIG_PM	0x96690101
#define	NV_SIG_NOTREADY	0x00000000

/*
 * These bar5 offsets are common to mcp55/mcp04 and thus
 * prefixed with NV.
 */
#define	NV_SSTATUS	0x00
#define	NV_SERROR	0x04
#define	NV_SCTRL	0x08
#define	NV_SACTIVE	0x0c
#define	NV_SNOTIFICATION 0x10

#define	CH0_SREG_OFFSET	0x0
#define	CH1_SREG_OFFSET	0x40


/*
 * The following config space offsets are needed to enable
 * bar 5 register access in mcp04/mcp55
 */
#define	NV_SATA_CFG_20		0x50
#define	NV_BAR5_SPACE_EN	0x04
#define	NV_40BIT_PRD		0x20

/*
 * mcp04 interrupt status register
 */

/*
 * offsets to bar 5 registers
 */
#define	MCP04_SATA_INT_STATUS	0x440
#define	MCP04_SATA_INT_EN	0x441


/*
 * bit fields for int status and int enable
 * registers
 */
#define	MCP04_INT_PDEV_INT	0x01 /* completion interrupt */
#define	MCP04_INT_PDEV_PM	0x02 /* power change */
#define	MCP04_INT_PDEV_ADD	0x04 /* hot plug */
#define	MCP04_INT_PDEV_REM	0x08 /* hot remove */
#define	MCP04_INT_PDEV_HOT	MCP04_INT_PDEV_ADD|MCP04_INT_PDEV_REM

#define	MCP04_INT_SDEV_INT	0x10 /* completion interrupt */
#define	MCP04_INT_SDEV_PM	0x20 /* power change */
#define	MCP04_INT_SDEV_ADD	0x40 /* hot plug */
#define	MCP04_INT_SDEV_REM	0x80 /* hot remove */
#define	MCP04_INT_SDEV_HOT	MCP04_INT_SDEV_ADD|MCP04_INT_SDEV_REM

#define	MCP04_INT_PDEV_ALL	MCP04_INT_PDEV_INT|MCP04_INT_PDEV_HOT|\
				MCP04_INT_PDEV_PM
#define	MCP04_INT_SDEV_ALL	MCP04_INT_SDEV_INT|MCP04_INT_SDEV_HOT|\
				MCP04_INT_SDEV_PM

/*
 * config space offset 42
 */
#define	NV_SATA_CFG_42			0xac

/*
 * bit in CFG_42 which delays hotplug interrupt until
 * PHY ready
 */
#define	MCP04_CFG_DELAY_HOTPLUG_INTR	(0x1 << 12)


/*
 * bar 5 offsets for SATA registers in ck804
 */
#define	MCP04_CH1_SSTATUS	0x00
#define	MCP04_CH1_SERROR	0x04
#define	MCP04_CH1_SCTRL		0x08
#define	MCP04_CH1_SACTIVE	0x0c
#define	MCP04_CH1_SNOTIFICATION	0x10

#define	MCP04_CH2_SSTATUS	0x40
#define	MCP04_CH2_SERROR	0x44
#define	MCP04_CH2_SCTRL		0x48
#define	MCP04_CH2_SACTIVE	0x4c
#define	MCP04_CH2_SNOTIFICATION	0x50


/*
 * bar 5 offsets for ADMACTL settings for both mcp04/mcp55
 */
#define	NV_ADMACTL_X	0x4C0
#define	NV_ADMACTL_Y	0x5C0

/*
 * Bits for NV_ADMACTL_X and NV_ADMACTL_Y
 */
#define	NV_HIRQ_EN	0x01 /* hot plug/unplug interrupt enable */
#define	NV_CH_RST	0x04 /* reset channel */


/*
 * bar 5 offset for ADMASTAT regs for mcp04
 */
#define	MCP04_ADMASTAT_X	0x4C4
#define	MCP04_ADMASTAT_Y	0x5C4

/*
 * Bits for MCP04_ADMASTAT_X and MCP04_ADMASTAT_Y
 */
#define	MCP04_HPIRQ	0x4
#define	MCP05_HUIRQ	0x2


/*
 * bar 4 offset to bus master command registers
 */
#define	BMICX_REG	0

/*
 * bit definitions for BMICX_REG
 */
#define	BMICX_SSBM	0x01	/* Start/Stop Bus Master */
				/* 1=Start (Enable) */
				/* 0=Start (Disable) */

/*
 * NOTE: "read" and "write" are the actions of the DMA engine
 * on the PCI bus, not the SATA bus.  Therefore for a ATA READ
 * command, program the DMA engine to "write to memory" mode
 * (and vice versa).
 */
#define	BMICX_RWCON			0x08 /* Read/Write Control */
#define	BMICX_RWCON_WRITE_TO_MEMORY	0x08 /* 1=Write (dev to host) */
#define	BMICX_RWCON_READ_FROM_MEMORY	0x00 /* 0=Read  (host to dev) */

/*
 * BMICX bits to preserve during updates
 */
#define	BMICX_MASK	(~(BMICX_SSBM | BMICX_RWCON))

/*
 * bar 4 offset to bus master status register
 */
#define	BMISX_REG	2

/*
 * bit fields for bus master status register
 */
#define	BMISX_BMIDEA	0x01	/* Bus Master IDE Active */
#define	BMISX_IDERR	0x02	/* IDE DMA Error */
#define	BMISX_IDEINTS	0x04	/* IDE Interrupt Status */

/*
 * bus master status register bits to preserve
 */
#define	BMISX_MASK	0xf8

/*
 * bar4 offset to bus master PRD descriptor table
 */
#define	BMIDTPX_REG	4


/*
 * structure for a single entry in the PRD table
 * (physical region descriptor table)
 */
typedef struct prde {
	uint32_t p_address; /* physical address */
	uint32_t p_count;   /* byte count, EOT in high order bit */
} prde_t;


#define	PRDE_EOT	((uint_t)0x80000000)

#define	NV_DMA_NSEGS	256  /* XXX DEBUG TEST change back to 257 */

/*
 * ck804 and mcp55 both have 2 ports per controller
 */
#define	NV_NUM_CPORTS	2

/*
 * Number of slots to allocate in data nv_sata structures to handle
 * multiple commands at once.  This does not reflect the capability of
 * the drive or the hardware, and in many cases will not match.
 * 1 or 32 slots are allocated, so in cases where the driver has NCQ
 * enabled but the drive doesn't support it, or supports fewer than
 * 32 slots, here may be an over allocation of memory.
 */
#ifdef NCQ
#define	NV_QUEUE_SLOTS	32
#else
#define	NV_QUEUE_SLOTS	1
#endif

/*
 * wait 30 seconds for signature
 */
#define	NV_SIG_TIMEOUT		45

#define	NV_BM_64K_BOUNDARY	0x10000ull

/*
 * every 1 second
 */
#define	NV_ONE_SEC	1000000


/*
 * the amount of time link can be down during
 * reset without taking action.
 */
#define	NV_LINK_LOST_OK	2

/*
 * nv_reset() flags
 */
#define	NV_RESET_SEND_EVENT	0x1 /* send reset event to sata module */
#define	NV_RESET_WAIT		0x2 /* OK to block waiting for reset */



#define	NV_RESET_ATTEMPTS 3

/*
 * nvp_state flags
 */
#define	NV_PORT_INACTIVE	0x001
#define	NV_PORT_ABORTING	0x002
#define	NV_PORT_HOTREMOVED	0x004
#define	NV_PORT_INIT		0x008
#define	NV_PORT_FAILED		0x010
#define	NV_PORT_RESET		0x020
#define	NV_PORT_RESET_PROBE	0x040
#define	NV_PORT_RESTORE		0x080

/*
 * nvc_state flags
 */
#define	NV_CTRL_SUSPEND		0x1


/*
 * flags for mcp04_set_intr/mcp55_set_intr
 */
#define	NV_INTR_DISABLE		0x1
#define	NV_INTR_ENABLE		0x2
#define	NV_INTR_CLEAR_ALL	0x4

/*
 * sizes of strings to allocate
 */
#define	NV_STRING_10	10
#define	NV_STRING_512	512

#define	NV_BYTES_PER_SEC 512

#define	NV_WAIT_REG_CHECK	10	/* 10 microseconds */
#define	NV_ATA_NUM_CMDS		256	/* max num ATA cmds possible, 8 bits */
#define	NV_PRINT_INTERVAL	40	/* throttle debug msg from flooding */
#define	MCP55_INT_CLEAR		0xffff	/* clear all interrupts */

/*
 * definition labels for the BAR registers
 */
#define	NV_BAR_0 0 /* chan 0 task file regs */
#define	NV_BAR_1 1 /* chan 0 status reg */
#define	NV_BAR_2 2 /* chan 1 task file regs */
#define	NV_BAR_3 3 /* chan 1 status reg */
#define	NV_BAR_4 4 /* bus master regs */
#define	NV_BAR_5 5 /* extra regs mostly SATA related */

/*
 * transform seconds to microseconds
 */
#define	NV_SEC2USEC(x) x * MICROSEC


/*
 * ck804 maps in task file regs into bar 5.  These are
 * only used to identify ck804, therefore only this reg is
 * listed here.
 */
#define	NV_BAR5_TRAN_LEN_CH_X	0x518

/*
 * if after this many iterations through the interrupt
 * processing loop, declare the interrupt wedged and
 * disable.
 */
#define	NV_MAX_INTR_LOOP 10

/*
 * flag values for nv_copy_regs_out
 */
#define	NV_COPY_COMPLETE 0x01	/* normal command completion */
#define	NV_COPY_ERROR    0x02	/* error, did not complete ok */
#define	NV_COPY_SSREGS   0x04	/* SS port registers */


#ifdef	__cplusplus
}
#endif

#endif /* _NV_SATA_H */
