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

#include <sys/types.h>
#include <sys/inttypes.h>
#include <sys/systm.h>
#include <sys/elf.h>
#include <sys/elf_notes.h>

#include <util/memcpy.h>

#include "dboot_xboot.h"
#include "dboot_elfload.h"
#include "dboot_printf.h"

static caddr_t elf_file = 0;

#define	PGETBYTES(offset)	((void *)(elf_file + (offset)))

static void *
getehdr(void)
{
	uchar_t *ident;
	void *hdr = NULL;

	ident = PGETBYTES(0);
	if (ident == NULL)
		dboot_panic("Cannot read kernel ELF header");

	if (ident[EI_MAG0] != ELFMAG0 || ident[EI_MAG1] != ELFMAG1 ||
	    ident[EI_MAG2] != ELFMAG2 || ident[EI_MAG3] != ELFMAG3)
		dboot_panic("not an ELF file!");

	if (ident[EI_CLASS] == ELFCLASS32)
		hdr = PGETBYTES(0);
	else if (ident[EI_CLASS] == ELFCLASS64)
		hdr = PGETBYTES(0);
	else
		dboot_panic("Unknown ELF class");

	return (hdr);
}


/*
 * parse the elf file for program information
 */
int
dboot_elfload64(uintptr_t file_image)
{
	Elf64_Ehdr *eh;
	Elf64_Phdr *phdr;
	caddr_t allphdrs;
	int i;
	paddr_t src;
	paddr_t dst;

	elf_file = (caddr_t)file_image;

	allphdrs = NULL;

	eh = getehdr();
	if (eh == NULL)
		dboot_panic("getehdr() failed");

	if (eh->e_type != ET_EXEC)
		dboot_panic("not ET_EXEC, e_type = 0x%x", eh->e_type);

	if (eh->e_phnum == 0 || eh->e_phoff == 0)
		dboot_panic("no program headers");

	/*
	 * Get the program headers.
	 */
	allphdrs = PGETBYTES(eh->e_phoff);
	if (allphdrs == NULL)
		dboot_panic("Failed to get program headers e_phnum = %d",
		    eh->e_phnum);

	/*
	 * Next look for interesting program headers.
	 */
	for (i = 0; i < eh->e_phnum; i++) {
		/*LINTED [ELF program header alignment]*/
		phdr = (Elf64_Phdr *)(allphdrs + eh->e_phentsize * i);

		/*
		 * Dynamically-linked executable.
		 * Complain.
		 */
		if (phdr->p_type == PT_INTERP) {
			dboot_printf("warning: PT_INTERP section\n");
			continue;
		}

		/*
		 * at this point we only care about PT_LOAD segments
		 */
		if (phdr->p_type != PT_LOAD)
			continue;

		if (phdr->p_flags == (PF_R | PF_W) && phdr->p_vaddr == 0) {
			dboot_printf("warning: krtld reloc info?\n");
			continue;
		}

		/*
		 * If memory size is zero just ignore this header.
		 */
		if (phdr->p_memsz == 0)
			continue;

		/*
		 * If load address 1:1 then ignore this header.
		 */
		if (phdr->p_paddr == phdr->p_vaddr) {
			if (prom_debug)
				dboot_printf("Skipping PT_LOAD segment for "
				    "paddr = 0x%lx\n", (ulong_t)phdr->p_paddr);
			continue;
		}

		/*
		 * copy the data to kernel area
		 */
		if (phdr->p_paddr != FOUR_MEG && phdr->p_paddr != 2 * FOUR_MEG)
			dboot_panic("Bad paddr for kernel nucleus segment");
		src = (uintptr_t)PGETBYTES(phdr->p_offset);
		dst = ktext_phys + phdr->p_paddr - FOUR_MEG;
		if (prom_debug)
			dboot_printf("copying %ld bytes from ELF offset 0x%lx "
			    "to physaddr 0x%lx (va=0x%lx)\n",
			    (ulong_t)phdr->p_filesz, (ulong_t)phdr->p_offset,
			    (ulong_t)dst, (ulong_t)phdr->p_vaddr);
		(void) memcpy((void *)(uintptr_t)dst,
		    (void *)(uintptr_t)src, (size_t)phdr->p_filesz);
	}

	/*
	 * Ignore the intepreter (or should we die if there is one??)
	 */
	return (0);
}
