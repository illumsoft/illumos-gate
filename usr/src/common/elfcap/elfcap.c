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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/* LINTLIBRARY */

/*
 * String conversion routine for hardware capabilities types.
 */
#include	<strings.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<sys/machelf.h>
#include	<sys/elf.h>
#include	<sys/auxv_SPARC.h>
#include	<sys/auxv_386.h>
#include	<elfcap.h>

/*
 * Given a literal string, generate an initialization for an
 * elfcap_str_t value.
 */
#define	STRDESC(_str) { _str, sizeof (_str) - 1 }

/*
 * The items in the elfcap_desc_t arrays are required to be
 * ordered so that the array index is related to the
 * c_val field as:
 *
 *	array[ndx].c_val = 2^ndx
 *
 * meaning that
 *
 *	array[0].c_val = 2^0 = 1
 *	array[1].c_val = 2^1 = 2
 *	array[2].c_val = 2^2 = 4
 *	.
 *	.
 *	.
 *
 * Since 0 is not a valid value for the c_val field, we use it to
 * mark an array entry that is a placeholder. This can happen if there
 * is a hole in the assigned bits.
 *
 * The RESERVED_ELFCAP_DESC macro is used to reserve such holes.
 */
#define	RESERVED_ELFCAP_DESC { 0, { NULL, 0 }, { NULL, 0 }, { NULL, 0 } }

/*
 * Define separators for output string processing. This must be kept in
 * sync with the elfcap_fmt_t values in elfcap.h.
 */
static const elfcap_str_t format[] = {
	STRDESC(" "),			/* ELFCAP_FMT_SNGSPACE */
	STRDESC("  "),			/* ELFCAP_FMT_DBLSPACE */
	STRDESC(" | ")			/* ELFCAP_FMT_PIPSPACE */
};
#define	FORMAT_NELTS	(sizeof (format) / sizeof (format[0]))



/*
 * Define all known software capabilities in all the supported styles.
 * Order the capabilities by their numeric value. See SF1_SUNW_
 * values in sys/elf.h.
 */
static const elfcap_desc_t sf1[ELFCAP_NUM_SF1] = {
	{						/* 0x00000001 */
		SF1_SUNW_FPKNWN, STRDESC("SF1_SUNW_FPKNWN"),
		STRDESC("FPKNWN"), STRDESC("fpknwn")
	},
	{						/* 0x00000002 */
		SF1_SUNW_FPUSED, STRDESC("SF1_SUNW_FPUSED"),
		STRDESC("FPUSED"), STRDESC("fpused"),
	}
};



/*
 * Order the SPARC hardware capabilities to match their numeric value.  See
 * AV_SPARC_ values in sys/auxv_SPARC.h.
 */
static const elfcap_desc_t hw1_sparc[ELFCAP_NUM_HW1_SPARC] = {
	{						/* 0x00000001 */
		AV_SPARC_MUL32, STRDESC("AV_SPARC_MUL32"),
		STRDESC("MUL32"), STRDESC("mul32"),
	},
	{						/* 0x00000002 */
		AV_SPARC_DIV32, STRDESC("AV_SPARC_DIV32"),
		STRDESC("DIV32"), STRDESC("div32"),
	},
	{						/* 0x00000004 */
		AV_SPARC_FSMULD, STRDESC("AV_SPARC_FSMULD"),
		STRDESC("FSMULD"), STRDESC("fsmuld"),
	},
	{						/* 0x00000008 */
		AV_SPARC_V8PLUS, STRDESC("AV_SPARC_V8PLUS"),
		STRDESC("V8PLUS"), STRDESC("v8plus"),
	},
	{						/* 0x00000010 */
		AV_SPARC_POPC, STRDESC("AV_SPARC_POPC"),
		STRDESC("POPC"), STRDESC("popc"),
	},
	{						/* 0x00000020 */
		AV_SPARC_VIS, STRDESC("AV_SPARC_VIS"),
		STRDESC("VIS"), STRDESC("vis"),
	},
	{						/* 0x00000040 */
		AV_SPARC_VIS2, STRDESC("AV_SPARC_VIS2"),
		STRDESC("VIS2"), STRDESC("vis2"),
	},
	{						/* 0x00000080 */
		AV_SPARC_ASI_BLK_INIT, STRDESC("AV_SPARC_ASI_BLK_INIT"),
		STRDESC("ASI_BLK_INIT"), STRDESC("asi_blk_init"),
	},
	{						/* 0x00000100 */
		AV_SPARC_FMAF, STRDESC("AV_SPARC_FMAF"),
		STRDESC("FMAF"), STRDESC("fmaf"),
	},
	RESERVED_ELFCAP_DESC,				/* 0x00000200 */
	RESERVED_ELFCAP_DESC,				/* 0x00000400 */
	RESERVED_ELFCAP_DESC,				/* 0x00000800 */
	RESERVED_ELFCAP_DESC,				/* 0x00001000 */
	RESERVED_ELFCAP_DESC,				/* 0x00002000 */
	{						/* 0x00004000 */
		AV_SPARC_FJFMAU, STRDESC("AV_SPARC_FJFMAU"),
		STRDESC("FJFMAU"), STRDESC("fjfmau"),
	},
	{						/* 0x00008000 */
		AV_SPARC_IMA, STRDESC("AV_SPARC_IMA"),
		STRDESC("IMA"), STRDESC("ima"),
	}
};



/*
 * Order the Intel hardware capabilities to match their numeric value.  See
 * AV_386_ values in sys/auxv_386.h.
 */
static const elfcap_desc_t hw1_386[ELFCAP_NUM_HW1_386] = {
	{						/* 0x00000001 */
		AV_386_FPU, STRDESC("AV_386_FPU"),
		STRDESC("FPU"), STRDESC("fpu"),
	},
	{						/* 0x00000002 */
		AV_386_TSC, STRDESC("AV_386_TSC"),
		STRDESC("TSC"), STRDESC("tsc"),
	},
	{						/* 0x00000004 */
		AV_386_CX8, STRDESC("AV_386_CX8"),
		STRDESC("CX8"), STRDESC("cx8"),
	},
	{						/* 0x00000008 */
		AV_386_SEP, STRDESC("AV_386_SEP"),
		STRDESC("SEP"), STRDESC("sep"),
	},
	{						/* 0x00000010 */
		AV_386_AMD_SYSC, STRDESC("AV_386_AMD_SYSC"),
		STRDESC("AMD_SYSC"), STRDESC("amd_sysc"),
	},
	{						/* 0x00000020 */
		AV_386_CMOV, STRDESC("AV_386_CMOV"),
		STRDESC("CMOV"), STRDESC("cmov"),
	},
	{						/* 0x00000040 */
		AV_386_MMX, STRDESC("AV_386_MMX"),
		STRDESC("MMX"), STRDESC("mmx"),
	},
	{						/* 0x00000080 */
		AV_386_AMD_MMX, STRDESC("AV_386_AMD_MMX"),
		STRDESC("AMD_MMX"), STRDESC("amd_mmx"),
	},
	{						/* 0x00000100 */
		AV_386_AMD_3DNow, STRDESC("AV_386_AMD_3DNow"),
		STRDESC("AMD_3DNow"), STRDESC("amd_3dnow"),
	},
	{						/* 0x00000200 */
		AV_386_AMD_3DNowx, STRDESC("AV_386_AMD_3DNowx"),
		STRDESC("AMD_3DNowx"), STRDESC("amd_3dnowx"),
	},
	{						/* 0x00000400 */
		AV_386_FXSR, STRDESC("AV_386_FXSR"),
		STRDESC("FXSR"), STRDESC("fxsr"),
	},
	{						/* 0x00000800 */
		AV_386_SSE, STRDESC("AV_386_SSE"),
		STRDESC("SSE"), STRDESC("sse"),
	},
	{						/* 0x00001000 */
		AV_386_SSE2, STRDESC("AV_386_SSE2"),
		STRDESC("SSE2"), STRDESC("sse2"),
	},
	{						/* 0x00002000 */
		AV_386_PAUSE, STRDESC("AV_386_PAUSE"),
		STRDESC("PAUSE"), STRDESC("pause"),
	},
	{						/* 0x00004000 */
		AV_386_SSE3, STRDESC("AV_386_SSE3"),
		STRDESC("SSE3"), STRDESC("sse3"),
	},
	{						/* 0x00008000 */
		AV_386_MON, STRDESC("AV_386_MON"),
		STRDESC("MON"), STRDESC("mon"),
	},
	{						/* 0x00010000 */
		AV_386_CX16, STRDESC("AV_386_CX16"),
		STRDESC("CX16"), STRDESC("cx16"),
	},
	{						/* 0x00020000 */
		AV_386_AHF, STRDESC("AV_386_AHF"),
		STRDESC("AHF"), STRDESC("ahf"),
	},
	{						/* 0x00040000 */
		AV_386_TSCP, STRDESC("AV_386_TSCP"),
		STRDESC("TSCP"), STRDESC("tscp"),
	},
	{						/* 0x00080000 */
		AV_386_AMD_SSE4A, STRDESC("AV_386_AMD_SSE4A"),
		STRDESC("AMD_SSE4A"), STRDESC("amd_sse4a"),
	},
	{						/* 0x00100000 */
		AV_386_POPCNT, STRDESC("AV_386_POPCNT"),
		STRDESC("POPCNT"), STRDESC("popcnt"),
	},
	{						/* 0x00200000 */
		AV_386_AMD_LZCNT, STRDESC("AV_386_AMD_LZCNT"),
		STRDESC("AMD_LZCNT"), STRDESC("amd_lzcnt"),
	},
	{						/* 0x00400000 */
		AV_386_SSSE3, STRDESC("AV_386_SSSE3"),
		STRDESC("SSSE3"), STRDESC("ssse3"),
	},
	{						/* 0x00800000 */
		AV_386_SSE4_1, STRDESC("AV_386_SSE4_1"),
		STRDESC("SSE4.1"), STRDESC("sse4.1"),
	},
	{						/* 0x01000000 */
		AV_386_SSE4_2, STRDESC("AV_386_SSE4_2"),
		STRDESC("SSE4.2"), STRDESC("sse4.2"),
	}
};

/*
 * Concatenate a token to the string buffer.  This can be a capabilities token
 * or a separator token.
 */
static elfcap_err_t
token(char **ostr, size_t *olen, const elfcap_str_t *nstr)
{
	if (*olen < nstr->s_len)
		return (ELFCAP_ERR_BUFOVFL);

	(void) strcat(*ostr, nstr->s_str);
	*ostr += nstr->s_len;
	*olen -= nstr->s_len;

	return (ELFCAP_ERR_NONE);
}

static elfcap_err_t
get_str_desc(elfcap_style_t style, const elfcap_desc_t *cdp,
    const elfcap_str_t **ret_str)
{
	switch (style) {
	case ELFCAP_STYLE_FULL:
		*ret_str = &cdp->c_full;
		break;
	case ELFCAP_STYLE_UC:
		*ret_str = &cdp->c_uc;
		break;
	case ELFCAP_STYLE_LC:
		*ret_str = &cdp->c_lc;
		break;
	default:
		return (ELFCAP_ERR_INVSTYLE);
	}

	return (ELFCAP_ERR_NONE);
}


/*
 * Expand a capabilities value into the strings defined in the associated
 * capabilities descriptor.
 */
static elfcap_err_t
expand(elfcap_style_t style, uint64_t val, const elfcap_desc_t *cdp,
    uint_t cnum, char *str, size_t slen, elfcap_fmt_t fmt)
{
	uint_t			cnt;
	int			follow = 0, err;
	const elfcap_str_t	*nstr;

	if (val == 0)
		return (ELFCAP_ERR_NONE);

	for (cnt = cnum; cnt > 0; cnt--) {
		uint_t mask = cdp[cnt - 1].c_val;

		if ((val & mask) != 0) {
			if (follow++ && ((err = token(&str, &slen,
			    &format[fmt])) != ELFCAP_ERR_NONE))
				return (err);

			err = get_str_desc(style, &cdp[cnt - 1], &nstr);
			if (err != ELFCAP_ERR_NONE)
				return (err);
			if ((err = token(&str, &slen, nstr)) != ELFCAP_ERR_NONE)
				return (err);

			val = val & ~mask;
		}
	}

	/*
	 * If there are any unknown bits remaining display the numeric value.
	 */
	if (val) {
		if (follow && ((err = token(&str, &slen, &format[fmt])) !=
		    ELFCAP_ERR_NONE))
			return (err);

		(void) snprintf(str, slen, "0x%llx", val);
	}
	return (ELFCAP_ERR_NONE);
}

/*
 * Expand a CA_SUNW_HW_1 value.
 */
elfcap_err_t
elfcap_hw1_to_str(elfcap_style_t style, uint64_t val, char *str,
    size_t len, elfcap_fmt_t fmt, ushort_t mach)
{
	/*
	 * Initialize the string buffer, and validate the format request.
	 */
	*str = '\0';
	if ((fmt < 0) || (fmt >= FORMAT_NELTS))
		return (ELFCAP_ERR_INVFMT);

	if ((mach == EM_386) || (mach == EM_IA_64) || (mach == EM_AMD64))
		return (expand(style, val, &hw1_386[0], ELFCAP_NUM_HW1_386,
		    str, len, fmt));

	if ((mach == EM_SPARC) || (mach == EM_SPARC32PLUS) ||
	    (mach == EM_SPARCV9))
		return (expand(style, val, hw1_sparc, ELFCAP_NUM_HW1_SPARC,
		    str, len, fmt));

	return (ELFCAP_ERR_UNKMACH);
}

/*
 * Expand a CA_SUNW_SF_1 value.  Note, that at present these capabilities are
 * common across all platforms.  The use of "mach" is therefore redundant, but
 * is retained for compatibility with the interface of elfcap_hw1_to_str(), and
 * possible future expansion.
 */
elfcap_err_t
/* ARGSUSED4 */
elfcap_sf1_to_str(elfcap_style_t style, uint64_t val, char *str,
    size_t len, elfcap_fmt_t fmt, ushort_t mach)
{
	/*
	 * Initialize the string buffer, and validate the format request.
	 */
	*str = '\0';
	if ((fmt < 0) || (fmt >= FORMAT_NELTS))
		return (ELFCAP_ERR_INVFMT);

	return (expand(style, val, &sf1[0], ELFCAP_NUM_SF1, str, len, fmt));
}

/*
 * Given a capability tag type and value, map it to a string representation.
 */
elfcap_err_t
elfcap_tag_to_str(elfcap_style_t style, uint64_t tag, uint64_t val,
    char *str, size_t len, elfcap_fmt_t fmt, ushort_t mach)
{
	if (tag == CA_SUNW_HW_1)
		return (elfcap_hw1_to_str(style, val, str, len, fmt, mach));
	if (tag == CA_SUNW_SF_1)
		return (elfcap_sf1_to_str(style, val, str, len, fmt, mach));

	return (ELFCAP_ERR_UNKTAG);
}

/*
 * Determine a capabilities value from a capabilities string.
 */
static uint64_t
value(elfcap_style_t style, const char *str, const elfcap_desc_t *cdp,
    uint_t cnum)
{
	const elfcap_str_t	*nstr;
	uint_t	num;
	int	err;

	for (num = 0; num < cnum; num++) {
		/*
		 * Skip "reserved" bits. These are unassigned bits in the
		 * middle of the assigned range.
		 */
		if (cdp[num].c_val == 0)
			continue;

		if ((err = get_str_desc(style, &cdp[num], &nstr)) != 0)
			return (err);
		if (strcmp(str, nstr->s_str) == 0)
			return (cdp[num].c_val);
	}
	return (0);
}

uint64_t
elfcap_sf1_from_str(elfcap_style_t style, const char *str, ushort_t mach)
{
	return (value(style, str, &sf1[0], ELFCAP_NUM_SF1));
}

uint64_t
elfcap_hw1_from_str(elfcap_style_t style, const char *str, ushort_t mach)
{
	if ((mach == EM_386) || (mach == EM_IA_64) || (mach == EM_AMD64))
		return (value(style, str, &hw1_386[0], ELFCAP_NUM_HW1_386));

	if ((mach == EM_SPARC) || (mach == EM_SPARC32PLUS) ||
	    (mach == EM_SPARCV9))
		return (value(style, str, hw1_sparc, ELFCAP_NUM_HW1_SPARC));

	return (0);
}

/*
 * These functions allow the caller to get direct access to the
 * cap descriptors.
 */
const elfcap_desc_t *
elfcap_getdesc_hw1_sparc(void)
{
	return (hw1_sparc);
}

const elfcap_desc_t *
elfcap_getdesc_hw1_386(void)
{
	return (hw1_386);
}

const elfcap_desc_t *
elfcap_getdesc_sf1(void)
{
	return (sf1);
}
