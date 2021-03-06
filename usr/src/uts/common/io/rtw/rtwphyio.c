/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
/*
 * Copyright (c) 2004, 2005 David Young.  All rights reserved.
 *
 * Programmed for NetBSD by David Young.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of David Young may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY David Young ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL David
 * Young BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
/*
 * Control input/output with the Philips SA2400 RF front-end and
 * the baseband processor built into the Realtek RTL8180.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"


#include <sys/param.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stream.h>
#include <sys/termio.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/cmn_err.h>
#include <sys/stropts.h>
#include <sys/strtty.h>
#include <sys/kbio.h>
#include <sys/cred.h>
#include <sys/stat.h>
#include <sys/consdev.h>
#include <sys/kmem.h>
#include <sys/modctl.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
#include <sys/pci.h>
#include <sys/errno.h>
#include <sys/gld.h>
#include <sys/dlpi.h>
#include <sys/ethernet.h>
#include <sys/list.h>
#include <sys/byteorder.h>
#include <sys/strsun.h>
#include <inet/common.h>
#include <inet/nd.h>
#include <inet/mi.h>


#include "rtwreg.h"
#include "max2820reg.h"
#include "sa2400reg.h"
#include "si4136reg.h"
#include "rtwvar.h"
#include "rtwphyio.h"
#include "rtwphy.h"

static int rtw_macbangbits_timeout = 100;

uint8_t
rtw_bbp_read(struct rtw_regs *regs, uint_t addr)
{
	RTW_WRITE(regs, RTW_BB,
	    LSHIFT(addr, RTW_BB_ADDR_MASK) | RTW_BB_RD_MASK | RTW_BB_WR_MASK);
	DELAY(10);
	RTW_WBR(regs, RTW_BB, RTW_BB);
	return (MASK_AND_RSHIFT(RTW_READ(regs, RTW_BB), RTW_BB_RD_MASK));
}

int
rtw_bbp_write(struct rtw_regs *regs, uint_t addr, uint_t val)
{
#define	BBP_WRITE_ITERS	50
#define	BBP_WRITE_DELAY	1
	int i;
	uint32_t wrbbp, rdbbp;

	RTW_DPRINTF(RTW_DEBUG_PHYIO,
	    "%s: bbp[%u] <- %u\n", __func__, addr, val);

	wrbbp = LSHIFT(addr, RTW_BB_ADDR_MASK) | RTW_BB_WREN |
	    LSHIFT(val, RTW_BB_WR_MASK) | RTW_BB_RD_MASK,
	    rdbbp = LSHIFT(addr, RTW_BB_ADDR_MASK) |
	    RTW_BB_WR_MASK | RTW_BB_RD_MASK;

	RTW_DPRINTF(RTW_DEBUG_PHYIO,
	    "%s: rdbbp = %08x, wrbbp = %08x\n", __func__, rdbbp, wrbbp);

	for (i = BBP_WRITE_ITERS; --i >= 0; ) {
		RTW_RBW(regs, RTW_BB, RTW_BB);
		RTW_WRITE(regs, RTW_BB, wrbbp);
		RTW_SYNC(regs, RTW_BB, RTW_BB);
		RTW_WRITE(regs, RTW_BB, rdbbp);
		RTW_SYNC(regs, RTW_BB, RTW_BB);
		DELAY(BBP_WRITE_DELAY);	/* 1 microsecond */
		if (MASK_AND_RSHIFT(RTW_READ(regs, RTW_BB),
		    RTW_BB_RD_MASK) == val) {
			RTW_DPRINTF(RTW_DEBUG_PHYIO,
			    "%s: finished in %dus\n", __func__,
			    BBP_WRITE_DELAY * (BBP_WRITE_ITERS - i));
			return (0);
		}
		DELAY(BBP_WRITE_DELAY);	/* again */
	}
	cmn_err(CE_NOTE, "%s: timeout\n", __func__);
	return (-1);
}

/*
 * Help rtw_rf_hostwrite bang bits to RF over 3-wire interface.
 */
static void
rtw_rf_hostbangbits(struct rtw_regs *regs, uint32_t bits, int lo_to_hi,
    uint_t nbits)
{
	int i;
	uint32_t mask, reg;

	RTW_DPRINTF(RTW_DEBUG_PHYIO,
	    "%s: %u bits, %08x, %s\n", __func__, nbits, bits,
	    (lo_to_hi) ? "lo to hi" : "hi to lo");

	reg = RTW_PHYCFG_HST;
	RTW_WRITE(regs, RTW_PHYCFG, reg);
	RTW_SYNC(regs, RTW_PHYCFG, RTW_PHYCFG);

	if (lo_to_hi)
		mask = 0x1;
	else
		mask = 1 << (nbits - 1);

	for (i = 0; i < nbits; i++) {
		RTW_DPRINTF(RTW_DEBUG_PHYBITIO,
		    "%s: bits %08x mask %08x -> bit %08x\n",
		    __func__, bits, mask, bits & mask);

		if ((bits & mask) != 0)
			reg |= RTW_PHYCFG_HST_DATA;
		else
			reg &= ~RTW_PHYCFG_HST_DATA;

		reg |= RTW_PHYCFG_HST_CLK;
		RTW_WRITE(regs, RTW_PHYCFG, reg);
		RTW_SYNC(regs, RTW_PHYCFG, RTW_PHYCFG);

		DELAY(2);	/* arbitrary delay */

		reg &= ~RTW_PHYCFG_HST_CLK;
		RTW_WRITE(regs, RTW_PHYCFG, reg);
		RTW_SYNC(regs, RTW_PHYCFG, RTW_PHYCFG);

		if (lo_to_hi)
			mask <<= 1;
		else
			mask >>= 1;
	}

	reg |= RTW_PHYCFG_HST_EN;
	RTW_WRITE(regs, RTW_PHYCFG, reg);
	RTW_SYNC(regs, RTW_PHYCFG, RTW_PHYCFG);
}

/*
 * Help rtw_rf_macwrite: tell MAC to bang bits to RF over the 3-wire
 * interface.
 */
static int
rtw_rf_macbangbits(struct rtw_regs *regs, uint32_t reg)
{
	int i;

	RTW_DPRINTF(RTW_DEBUG_PHY, "%s: %08x\n", __func__, reg);

	RTW_WRITE(regs, RTW_PHYCFG, RTW_PHYCFG_MAC_POLL | reg);

	RTW_WBR(regs, RTW_PHYCFG, RTW_PHYCFG);

	for (i = rtw_macbangbits_timeout; --i >= 0; DELAY(1)) {
		if ((RTW_READ(regs, RTW_PHYCFG) & RTW_PHYCFG_MAC_POLL) == 0) {
			RTW_DPRINTF(RTW_DEBUG_PHY,
			    "%s: finished in %dus\n", __func__,
			    rtw_macbangbits_timeout - i);
			return (0);
		}
		RTW_RBR(regs, RTW_PHYCFG, RTW_PHYCFG);	/* paranoia? */
	}

	cmn_err(CE_NOTE, "%s: RTW_PHYCFG_MAC_POLL still set.\n", __func__);
	return (-1);
}

/*ARGSUSED*/
static uint32_t
rtw_grf5101_host_crypt(uint_t addr, uint32_t val)
{
	/* TBD */
	return (0);
}

static uint32_t
rtw_grf5101_mac_crypt(uint_t addr, uint32_t val)
{
	uint32_t data_and_addr;
#define	EXTRACT_NIBBLE(d, which) (((d) >> (4 * (which))) & 0xf)
	static uint8_t caesar[16] =
	{
		0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
		0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
	};

	data_and_addr =  caesar[EXTRACT_NIBBLE(val, 2)] |
	    (caesar[EXTRACT_NIBBLE(val, 1)] <<  4) |
	    (caesar[EXTRACT_NIBBLE(val, 0)] <<  8) |
	    (caesar[(addr >> 1) & 0xf] << 12) |
	    ((addr & 0x1) << 16) |
	    (caesar[EXTRACT_NIBBLE(val, 3)] << 24);
	return (LSHIFT(data_and_addr, RTW_PHYCFG_MAC_PHILIPS_ADDR_MASK |
	    RTW_PHYCFG_MAC_PHILIPS_DATA_MASK));
#undef EXTRACT_NIBBLE
}

static const char *
rtw_rfchipid_string(enum rtw_rfchipid rfchipid)
{
	switch (rfchipid) {
	case RTW_RFCHIPID_MAXIM:
		return ("Maxim");
	case RTW_RFCHIPID_PHILIPS:
		return ("Philips");
	case RTW_RFCHIPID_GCT:
		return ("GCT");
	case RTW_RFCHIPID_RFMD:
		return ("RFMD");
	case RTW_RFCHIPID_INTERSIL:
		return ("Intersil");
	default:
		return ("unknown");
	}
}

/*
 * Bang bits over the 3-wire interface.
 */
int
rtw_rf_hostwrite(struct rtw_regs *regs, enum rtw_rfchipid rfchipid,
    uint_t addr, uint32_t val)
{
	uint_t nbits;
	int lo_to_hi;
	uint32_t bits;

	RTW_DPRINTF(RTW_DEBUG_PHYIO, "%s: %s[%u] <- %08x\n", __func__,
	    rtw_rfchipid_string(rfchipid), addr, val);

	switch (rfchipid) {
	case RTW_RFCHIPID_MAXIM:
		nbits = 16;
		lo_to_hi = 0;
		bits = LSHIFT(val, MAX2820_TWI_DATA_MASK) |
		    LSHIFT(addr, MAX2820_TWI_ADDR_MASK);
		break;
	case RTW_RFCHIPID_PHILIPS:
		bits = LSHIFT(val, SA2400_TWI_DATA_MASK) |
		    LSHIFT(addr, SA2400_TWI_ADDR_MASK) | SA2400_TWI_WREN;
		nbits = 32;
		lo_to_hi = 1;
		break;
	case RTW_RFCHIPID_GCT:
	case RTW_RFCHIPID_RFMD:
		if (rfchipid == RTW_RFCHIPID_GCT)
			bits = rtw_grf5101_host_crypt(addr, val);
		else {
			bits = LSHIFT(val, SI4126_TWI_DATA_MASK) |
			    LSHIFT(addr, SI4126_TWI_ADDR_MASK);
		}
		nbits = 22;
		lo_to_hi = 0;
		break;
	case RTW_RFCHIPID_INTERSIL:
	default:
		cmn_err(CE_WARN, "%s: unknown rfchipid %d\n",
		    __func__, rfchipid);
		return (-1);
	}

	rtw_rf_hostbangbits(regs, bits, lo_to_hi, nbits);

	return (0);
}

static uint32_t
rtw_maxim_swizzle(uint_t addr, uint32_t val)
{
	uint32_t hidata, lodata;

	lodata = MASK_AND_RSHIFT(val, RTW_MAXIM_LODATA_MASK);
	hidata = MASK_AND_RSHIFT(val, RTW_MAXIM_HIDATA_MASK);
	return (LSHIFT(lodata, RTW_PHYCFG_MAC_MAXIM_LODATA_MASK) |
	    LSHIFT(hidata, RTW_PHYCFG_MAC_MAXIM_HIDATA_MASK) |
	    LSHIFT(addr, RTW_PHYCFG_MAC_MAXIM_ADDR_MASK));
}

/*
 * Tell the MAC what to bang over the 3-wire interface.
 */
int
rtw_rf_macwrite(struct rtw_regs *regs, enum rtw_rfchipid rfchipid,
    uint_t addr, uint32_t val)
{
	uint32_t reg;

	RTW_DPRINTF(RTW_DEBUG_PHYIO, "%s: %s[%u] <- %08x\n", __func__,
	    rtw_rfchipid_string(rfchipid), addr, val);

	switch (rfchipid) {
	case RTW_RFCHIPID_GCT:
		reg = rtw_grf5101_mac_crypt(addr, val);
		break;
	case RTW_RFCHIPID_MAXIM:
		reg = rtw_maxim_swizzle(addr, val);
		break;
	default:
	case RTW_RFCHIPID_PHILIPS:

		reg = LSHIFT(addr, RTW_PHYCFG_MAC_PHILIPS_ADDR_MASK) |
		    LSHIFT(val, RTW_PHYCFG_MAC_PHILIPS_DATA_MASK);
	}

	switch (rfchipid) {
	case RTW_RFCHIPID_GCT:
	case RTW_RFCHIPID_MAXIM:
	case RTW_RFCHIPID_RFMD:
		reg |= RTW_PHYCFG_MAC_RFTYPE_RFMD;
		break;
	case RTW_RFCHIPID_INTERSIL:
		reg |= RTW_PHYCFG_MAC_RFTYPE_INTERSIL;
		break;
	case RTW_RFCHIPID_PHILIPS:
		reg |= RTW_PHYCFG_MAC_RFTYPE_PHILIPS;
		break;
	default:
		cmn_err(CE_WARN, "%s: unknown rfchipid %d\n",
		    __func__, rfchipid);
		return (-1);
	}

	return (rtw_rf_macbangbits(regs, reg));
}
