/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1996   Erich Boleyn  <erich@uruk.org>
 *  Copyright (C) 1999, 2000 Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * Note: These functions defined in this file may be called from C.
 *       Be careful of that you must not modify some registers. Quote
 *       from gcc-2.95.2/gcc/config/i386/i386.h:
	
   1 for registers not available across function calls.
   These must include the FIXED_REGISTERS and also any
   registers that can be used without being saved.
   The latter must include the registers where values are returned
   and the register where structure-value addresses are passed.
   Aside from that, you can include as many other registers as you like.

  ax,dx,cx,bx,si,di,bp,sp,st,st1,st2,st3,st4,st5,st6,st7,arg
{  1, 1, 1, 0, 0, 0, 0, 1, 1,  1,  1,  1,  1,  1,  1,  1,  1 }
 */

#define ASM_FILE

#include "shared.h"

/*# define	ABS(x)	((x) - EXT_C(main) + 0x8200)*/
# define	ABS(x)	((x) - EXT_C(main) + 0x7c00)
	
	.file	"asm.S"

	.text

	/* Tell GAS to generate 16-bit instructions so that this code works
	   in real mode. */
	.code16

	 /*In stage2, do not link start.S with the rest of the source
	 * files directly, so define the start symbols here just to
	 * force ld quiet. These are not referred anyway.
	 */
	.globl	start, _start
start:
_start:
	
ENTRY(main)
	/*
	 *  Guarantee that "main" is loaded at 0x0:0x8200 in stage2 and
	 *  at 0x0:0x2200 in stage1.5.
	 */
	ljmp $0, $ABS(codestart)

/* Modified Etherboot Header */

        .byte 0,0,0
        .long 0x1B031336
        .long 0x00000106
        .long 0x07C00000
        .long 0x07C00000
        .long 0x04000004
        .long 0x00007E00
        .long 9000-512
        .long 9000-512

VARIABLE(basedir)
	.string "/tftpboot/lbl/menu.lst"

	/*
	 *  Compatibility version number
	 *
	 *  These MUST be at byte offset 6 and 7 of the executable
	 *  DO NOT MOVE !!!
	 */
	. = EXT_C(main) + 0x206
/*	.byte	COMPAT_VERSION_MAJOR, COMPAT_VERSION_MINOR */
	.byte   0,1

	/*
	 *  This is a special data area 8 bytes from the beginning.
	 */
	. = EXT_C(main) + 0x208

VARIABLE(install_partition)
	.long	0xFFFFFF
VARIABLE(saved_entryno)
	.long	0
VARIABLE(stage2_id)
	.byte	STAGE2_ID
VARIABLE(force_lba)
	.byte	0
VARIABLE(config_file)
	.string "/tftpboot/lbl/menu.lst"

	/*
	 *  Leave some breathing room for the config file name.
	 */

	. = EXT_C(main) + 0x270

/* the real mode code continues... */
codestart:
	cli

	/* set up %ds, %ss, and %es */
	xorw	%ax, %ax
	movw	%ax, %ds
#ifdef	SUPPORT_NETBOOT
	ADDR32	movw	%es,EXT_C(pxe_pointer_segment)
	ADDR32	movw	%bx,EXT_C(pxe_pointer_offset)
	ADDR32	movw	%ss,EXT_C(pxe_stack_segment)
	ADDR32	movw	%sp,EXT_C(pxe_stack_offset)
#endif
	movw	%ax, %ss
	movw	%ax, %es

#ifndef SUPPORT_DISKLESS
	/*
	 * Save the sector number of the second sector (i.e. this sector)
	 * in INSTALL_SECOND_SECTOR. See also "stage2/start.S".
	 */
	ADDR32	movl	%ebp, EXT_C(install_second_sector)
#endif
	
	/* set up the real mode/BIOS stack */
	movl	$STACKOFF, %ebp
	movl	%ebp, %esp

	sti		/* we're safe again */

#ifndef SUPPORT_DISKLESS
	/* save boot drive reference */
	ADDR32	movw	%dx, EXT_C(boot_drive)

	/* reset disk system (%ah = 0) */
	int	$0x13
#else
#if 0
	pushw	%dx
	movw	$ABS(torito_sp),%si
	movw	$0x4B01,%ax
	int	$0x13
	jc	1f
	popw	%dx
	ADDR32	movw	%dx, EXT_C(boot_drive)
	jmp	2f
1:
	popw	%dx
#endif
	movw 	$0x5650,%ax
	int	$0x1a
	jc	 2f
	cmp	$0x564E,%ax
	jne	 2f
	ADDR32	movw	%es,EXT_C(pxe_int1a_segment)
	ADDR32	movw	%bx,EXT_C(pxe_int1a_offset)
2:
#endif

	/* transition to protected mode */
	DATA32	call EXT_C(real_to_prot)

	/* The ".code32" directive takes GAS out of 16-bit mode. */
	.code32

	/* clean out the bss */

	/* set %edi to the bss starting address */
#if defined(HAVE_USCORE_USCORE_BSS_START_SYMBOL)
	movl	$__bss_start, %edi
#elif defined(HAVE_USCORE_EDATA_SYMBOL)
	movl	$_edata, %edi
#elif defined(HAVE_EDATA_SYMBOL)
	movl	$edata, %edi
#endif

	/* set %ecx to the bss end */	
#if defined(HAVE_END_SYMBOL)
	movl	$end, %ecx
#elif defined(HAVE_USCORE_END_SYMBOL)
	movl	$_end, %ecx
#endif

	/* compute the bss length */
	subl	%edi, %ecx
	
	/* zero %al */
	xorb	%al, %al

	/* set the direction */
	cld
	
	/* clean out */
	rep
	stosb
	
	/*
	 *  Call the start of main body of C code, which does some
	 *  of it's own initialization before transferring to "cmain".
	 */
	call EXT_C(init_bios_info)


/*
 *  This call is special...  it never returns...  in fact it should simply
 *  hang at this point!
 */

ENTRY(stop)
	call	EXT_C(prot_to_real)

	/*
	 * This next part is sort of evil.  It takes advantage of the
	 * byte ordering on the x86 to work in either 16-bit or 32-bit
	 * mode, so think about it before changing it.
	 */

ENTRY(hard_stop)
	hlt
	jmp EXT_C(hard_stop)

/*
 * stop_floppy()
 *
 * Stops the floppy drive from spinning, so that other software is
 * jumped to with a known state.
 */
ENTRY(stop_floppy)
	movw	$0x3F2, %dx
	xorb	%al, %al
	outb	%al, %dx
	ret

/*
 * grub_reboot()
 *
 * Reboot the system. At the moment, rely on BIOS.
 */
ENTRY(grub_reboot)
	call	EXT_C(prot_to_real)
	.code16
	/* cold boot */
	movw	$0x0472, %di
	movw	%ax, (%di)
	ljmp	$0xFFFF, $0x0000
	.code32
	
/*
 * grub_halt(int no_apm)
 *
 * Halt the system, using APM if possible. If NO_APM is true, don't use
 * APM even if it is available.
 */
ENTRY(grub_halt)
	/* get the argument */
	movl	4(%esp), %eax
	
	/* see if zero */
	testl	%eax, %eax
	jnz	EXT_C(stop)

	call	EXT_C(prot_to_real)
	.code16
	
	/* detect APM */
	movw	$0x5300, %ax
	xorw	%bx, %bx
	int	$0x15
	jc	EXT_C(hard_stop)
	/* don't check %bx for buggy BIOSes... */

	/* disconnect APM first */
	movw	$0x5304, %ax
	xorw	%bx, %bx
	int	$0x15

	/* connect APM (Is this really necessary?) */
	movw	$0x5301, %ax
	xorw	%bx, %bx
	int	$0x15
	jc	EXT_C(hard_stop)

  /* From 0.90CVS : set APM protocol level - 1.1 or bust. (this covers APM 1.2 also) */

        movw    $0x530E, %ax
	xorw    %bx, %bx
        movw    $0x0101, %cx
        int     $0x15
        jc      EXT_C(hard_stop)

	/* set the power state to off */
	movw	$0x5307, %ax
	movw	$1, %bx
	movw	$3, %cx
	int	$0x15

	/* shouldn't reach here */
	jmp	EXT_C(hard_stop)
	
	.code32

/*
 * chain_stage1(segment, offset, part_table_addr)
 *
 *  This starts another stage1 loader, at segment:offset.
 */

ENTRY(chain_stage1)
	/* no need to save anything, just use %esp */

	/* store %ESI, presuming %ES is 0 */
	movl	0xc(%esp), %esi

	/* store new offset */
	movl	0x8(%esp), %eax
	movl	%eax, offset

	/* store new segment */
	movw	0x4(%esp), %ax
	movw	%ax, segment

	/* set up to pass boot drive */
	movw	EXT_C(boot_drive), %dx

	call	EXT_C(prot_to_real)
	.code16

#ifdef ABSOLUTE_WITHOUT_ASTERISK
	DATA32	ADDR32	ljmp	(offset)
#else
	DATA32	ADDR32	ljmp	*(offset)
#endif
	.code32


/*
 *  These next two routines, "real_to_prot" and "prot_to_real" are structured
 *  in a very specific way.  Be very careful when changing them.
 *
 *  NOTE:  Use of either one messes up %eax and %ebp.
 */

ENTRY(real_to_prot)
	.code16
	cli

	/* load the GDT register */
	DATA32	ADDR32	lgdt	gdtdesc

	/* turn on protected mode */
	movl	%cr0, %eax
	orl	$CR0_PE_ON, %eax
	movl	%eax, %cr0

	/* jump to relocation, flush prefetch queue, and reload %cs */
	DATA32	ljmp	$PROT_MODE_CSEG, $protcseg

	/*
	 *  The ".code32" directive only works in GAS, the GNU assembler!
	 *  This gets out of "16-bit" mode.
	 */
	.code32

protcseg:
	/* reload other segment registers */
	movw	$PROT_MODE_DSEG, %ax
	movw	%ax, %ds			/* Descriptors -> PROT_MODE_DSEG */
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* put the return address in a known safe location */
	movl	(%esp), %eax
	movl	%eax, STACKOFF

	/* get protected mode stack */
	movl	protstack, %eax
	movl	%eax, %esp
	movl	%eax, %ebp

	/* get return address onto the right stack */
	movl	STACKOFF, %eax
	movl	%eax, (%esp)

	/* zero %eax */
	xorl	%eax, %eax

	/* return on the old (or initialized) stack! */
	ret


ENTRY(prot_to_real)
	/* just in case, set GDT */
	lgdt	gdtdesc

	/* save the protected mode stack */
	movl	%esp, %eax
	movl	%eax, protstack

	/* get the return address */
	movl	(%esp), %eax
	movl	%eax, STACKOFF

	/* set up new stack */
	movl	$STACKOFF, %eax
	movl	%eax, %esp
	movl	%eax, %ebp

	/* set up segment limits */
	movw	$PSEUDO_RM_DSEG, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* this might be an extra step */
	ljmp	$PSEUDO_RM_CSEG, $tmpcseg	/* jump to a 16 bit segment */

tmpcseg:
	.code16

	/* clear the PE bit of CR0 */
	movl	%cr0, %eax
	andl 	$CR0_PE_OFF, %eax
	movl	%eax, %cr0

	/* flush prefetch queue, reload %cs */
	DATA32	ljmp	$0, $realcseg

realcseg:
	/* we are in real mode now
	 * set up the real mode segment registers : DS, SS, ES
	 */
	/* zero %eax */
	xorl	%eax, %eax

	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* restore interrupts */
	sti

	/* return on new stack! */
	DATA32	ret

	.code32


/*
 *   int biosdisk_int13_extensions (int ah, int drive, void *dap)
 *
 *   Call IBM/MS INT13 Extensions (int 13 %ah=AH) for DRIVE. DAP
 *   is passed for disk address packet. If an error occurs, return
 *   non-zero, otherwise zero.
 */

ENTRY(biosdisk_int13_extensions)
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi

	/* compute the address of disk_address_packet */
	movl	0x10(%ebp), %eax
	movw	%ax, %si
	xorw	%ax, %ax
	shrl	$4, %eax
	movw	%ax, %cx	/* save the segment to cx */

	/* drive */
	movb	0xc(%ebp), %dl
	/* ah */
	movb	0x8(%ebp), %dh
	/* enter real mode */
	call	EXT_C(prot_to_real)
	
	.code16
	xorb	%al,%al		/* Write without verify (AL=0) */
	movb	%dh, %ah
	movw	%cx, %ds
	int	$0x13		/* do the operation */
	movb	%ah, %dl	/* save return value */
	/* clear the data segment */
	xorw	%ax, %ax
	movw	%ax, %ds
	/* back to protected mode */
	DATA32	call	EXT_C(real_to_prot)
	.code32

	movb	%dl, %al	/* return value in %eax */

	popl	%esi
	popl	%ebp

	ret
	
#ifdef SUPPORT_NETBOOT
ENTRY(set_pxe_entry)
	pushl	%ebp
	movl	%esp, %ebp
	movw	12(%ebp), %ax
/*	movw	%ax, ABS(pxe_entry_offset)    */
/*	movw	%ax, ABS(oldpxe_entry_offset) */
	movw	%ax, pxe_entry_offset
	movw	%ax, oldpxe_entry_offset
	movw	8(%ebp), %ax
/*	movw	%ax, ABS(pxe_entry_segment)    */
/*	movw	%ax, ABS(oldpxe_entry_segment) */
	movw	%ax, pxe_entry_segment
	movw	%ax, oldpxe_entry_segment
	popl	%ebp
	ret

ENTRY(pxe_call)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi

	/* compute the address of the parameter pointer */
	movw	0x08(%ebp),%dx
	movl	0x0C(%ebp), %eax
	movw	%ax, %si	/* save the offset to si */
	xorw	%ax, %ax
	shrl	$4, %eax
	movw	%ax, %cx	/* save the segment to cx */
	call	EXT_C(prot_to_real)
	.code16
	push	%cx			/* segment */
	push	%si			/* offset */
	push	%dx			/* function */
	.byte	0x9a
pxe_entry_offset:	.word	0	/* offset */
pxe_entry_segment:	.word	0	/* segment */
	add		%sp,6
	movb	%ah,%dl
	xorw	%ax,%ax
	movw	%ax,%ds
	DATA32 call	EXT_C(real_to_prot)
	.code32
	movb	%dl,%al
	
	popl	%esi
	popl	%ebp
	ret

ENTRY(oldpxe)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl   %edi
	pushl	%ebx
	pushl	%ecx

	/* compute the address of the parameter pointer */
	movw	0x08(%ebp),%bx		/*Function*/
	movl	0x0C(%ebp), %eax
	movw	%ax, %di		/* save the offset to di */
	xorw	%ax, %ax
	shrl	$4, %eax
	movw	%ax, %cx		/* save the segment to cx */
	call	EXT_C(prot_to_real)
	.code16
	movw	%cx,%es
	.byte	0x9a
oldpxe_entry_offset:	.word	0x92	/* offset */
oldpxe_entry_segment:	.word	0x9140	/* segment */
	movb	%ah,%dl
	xorw	%ax,%ax
	movw	%ax,%ds
	movw	%ax,%es
	DATA32 call	EXT_C(real_to_prot)
	.code32
	movb	%dl,%al
	
	popl	%ecx
	popl	%ebx
	popl	%edi
	popl	%esi
	popl	%ebp
	ret

ENTRY(pxe_emulation)
	jmp	EXT_C(pxe_oeb)

ENTRY(int10)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	/* compute the address of the parameter pointer */
	movw	0x08(%ebp),%dx
	call	EXT_C(prot_to_real)
	.code16
	movw	%dx,%ax
	int		$0x10
	DATA32 call	EXT_C(real_to_prot)
	.code32
	popl	%esi
	popl	%ebp
	ret

ENTRY(outp)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi

	movw	0x08(%ebp),%dx
	movw	0x0C(%ebp),%ax
	out		%al, %dx

	popl	%esi
	popl	%ebp
	ret

ENTRY(cpuinfo)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi

	pushl	%eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	xorl	%eax,%eax
	cpuid
	movl	%eax,X86_CPUID
	movl	%ebx,X86_VENDOR_ID
	movl	%edx,X86_VENDOR_ID+4
	movl	%ecx,X86_VENDOR_ID+8
	orl	%eax,%eax
	je	1f

	movl	$1,%eax
	cpuid
	movb	%al,%cl
	andb	$0x0F,%ah
	movb	%ah,X86
	andb	$0xF0,%al
	shrb	$4,%al
	movb	%al,X86+1
	andb	$0x0F,%cl
	movb	%cl,X86+2
	movb	%bl,X86+3
	movl	%edx,X86_CAPABILITY
1:
	popl	%edx
	popl	%ecx
	popl	%ebx
	popl	%eax

	popl	%esi
	popl	%ebp
	ret

ENTRY(cpuspeed)
cpuspeed:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	inb 	$0x61,%al
	andl	$0xFE, %eax
	orl	$0x01, %eax
	outb 	%al,$0x61

	movb	$0xB0, %al
	outb 	%al,$0x43

	movb	$0xB1,%al
	outb 	%al,$0x42
	movb	$0xDF,%al
	outb 	%al,$0x42

	rdtsc
	movl	%eax, st_low
	movl	%edx, st_high

	movl	$0, %ecx
	.p2align 4
1:
	incl	%ecx
	inb 	$0x61,%al
	testl	$0x20,%eax
	je	1b

	rdtsc
	subl st_low,%eax
	subl st_high,%edx
	
	movl	%eax, end_low
	movl	%edx, end_high

	cmpl	$3, %ecx
	jle	2f
	cmpl	$49999, %eax
	jg	3f
2:
	movl	$-1, %eax
	jmp	4f

	.p2align 4
3:
	movl	end_low, %eax
	movl	$0x2AAAAAAA, %edx	; / 6
	imull	%edx
	sarl	$3, %edx		; / 8 => /48
	movl	%edx, %eax
4:
	popl	%edx
	popl	%ecx
	popl	%ebx
	ret

ENTRY(set_hw)
	pushl	%ebp
	movl	%esp,%ebp

	movb	8(%ebp),%dl
	movb	0xc(%ebp),%cl

	call	EXT_C(prot_to_real)
	.code16
	mov	%cl,%al
	mov	$0x4E,%ah
	int	$0x13
	jc	1f
	movb	%al,%dl
	movb	%ah,%dh
	jmp	2f
1:
	movb	%ah,%dl
	movb	$0xff,%dh
2:
	xorw	%ax,%ax
	movw	%ax,%ds
	DATA32 call	EXT_C(real_to_prot)
	.code32
	movw	%dx,%ax
	
	popl	%ebp
	ret
#endif

/*
 *   int biosdisk_standard (int ah, int drive, int coff, int hoff, int soff,
 *                          int nsec, int segment)
 *
 *   Call standard and old INT13 (int 13 %ah=AH) for DRIVE. Read/write
 *   NSEC sectors from COFF/HOFF/SOFF into SEGMENT. If an error occurs,
 *   return non-zero, otherwise zero.
 */

ENTRY(biosdisk_standard)
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%edi
	pushl	%esi

	/* set up CHS information */
	movl	0x10(%ebp), %eax
	movb	%al, %ch
	movb	0x18(%ebp), %al
	shlb	$2, %al
	shrw	$2, %ax
	movb	%al, %cl
	movb	0x14(%ebp), %dh
	/* drive */
	movb	0xc(%ebp), %dl
	/* segment */
	movw	0x20(%ebp), %bx
	/* save nsec and ah to %di */
	movb	0x8(%ebp), %ah
	movb	0x1c(%ebp), %al
	movw	%ax, %di
	/* enter real mode */
	call	EXT_C(prot_to_real)

	.code16
	movw	%bx, %es
	xorw	%bx, %bx
	movw	$3, %si		/* attempt at least three times */

1:	
	movw	%di, %ax
	int	$0x13		/* do the operation */
	jnc	2f		/* check if successful */

	movb	%ah, %bl	/* save return value */
	/* if fail, reset the disk system */
	xorw	%ax, %ax
	int	$0x13
	
	decw	%si
	cmpw	$0, %si
	je	2f
	xorb	%bl, %bl
	jmp	1b		/* retry */
2:	
	/* back to protected mode */
	DATA32	call	EXT_C(real_to_prot)
	.code32

	movb	%bl, %al	/* return value in %eax */
	
	popl	%esi
	popl	%edi
	popl	%ebx
	popl	%ebp

	ret


/*
 *   int check_int13_extensions (int drive)
 *
 *   Check if LBA is supported for DRIVE. If it is supported, then return
 *   the major version of extensions, otherwise zero.
 */

ENTRY(check_int13_extensions)
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	/* drive */
	movb	0x8(%ebp), %dl
	/* enter real mode */
	call	EXT_C(prot_to_real)

	.code16
	movb	$0x41, %ah
	movw	$0x55aa, %bx
	int	$0x13		/* do the operation */
	
	/* check the result */
	jc	1f
	cmpw	$0xaa55, %bx
	jne	1f

	movb	%ah, %bl	/* save the major version into %bl */

	/* check if AH=0x42 is supported if FORCE_LBA is zero */
	movb	EXT_C(force_lba), %al
	testb	%al, %al
	jnz	2f
	andw	$1, %cx
	jnz	2f
	
1:
	xorb	%bl, %bl
2:
	/* back to protected mode */
	DATA32	call	EXT_C(real_to_prot)
	.code32

	movb	%bl, %al	/* return value in %eax */

	popl	%ebx
	popl	%ebp

	ret


/*
 *   int get_diskinfo_int13_extensions (int drive, void *drp)
 *
 *   Return the geometry of DRIVE in a drive parameters, DRP. If an error
 *   occurs, then return non-zero, otherwise zero.
 */

ENTRY(get_diskinfo_int13_extensions)
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%esi

	/* compute the address of drive parameters */
	movl	0xc(%ebp), %eax
	movw	%ax, %si
	xorw	%ax, %ax
	shrl	$4, %eax
	movw	%ax, %bx	/* save the segment into %bx */
	/* drive */
	movb	0x8(%ebp), %dl
	/* enter real mode */
	call	EXT_C(prot_to_real)

	.code16
	movb	$0x48, %ah
	movw	%bx, %ds
	int	$0x13		/* do the operation */
	movb	%ah, %bl	/* save return value in %bl */
	/* clear the data segment */
	xorw	%ax, %ax
	movw	%ax, %ds
	/* back to protected mode */
	DATA32	call	EXT_C(real_to_prot)
	.code32

	movb	%bl, %al	/* return value in %eax */

	popl	%esi
	popl	%ebx
	popl	%ebp
	
	ret


/*
 *   int get_diskinfo_standard (int drive, unsigned long *cylinders, 
 *                              unsigned long *heads, unsigned long *sectors)
 *
 *   Return the geometry of DRIVE in CYLINDERS, HEADS and SECTORS. If an
 *   error occurs, then return non-zero, otherwise zero.
 */

ENTRY(get_diskinfo_standard)
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%edi

	/* drive */
	movb	0x8(%ebp), %dl
	/* enter real mode */
	call	EXT_C(prot_to_real)

	.code16
	movb	$0x8, %ah
	int	$0x13		/* do the operation */
	/* check if successful */
	testb	%ah, %ah
	jnz	1f
	/* bogus BIOSes may not return an error number */
	testb	$0x3f, %cl	/* 0 sectors means no disk */
	jnz	1f		/* if non-zero, then succeed */
	/* XXX 0x60 is one of the unused error numbers */
	movb	$0x60, %ah
1:
	movb	%ah, %bl	/* save return value in %bl */
	/* back to protected mode */
	DATA32	call	EXT_C(real_to_prot)
	.code32

	/* restore %ebp */
	leal	0x8(%esp), %ebp
	
	/* heads */
	movb	%dh, %al
	incl	%eax		/* the number of heads is counted from zero */
	movl	0x10(%ebp), %edi
	movl	%eax, (%edi)

	/* sectors */
	xorl	%eax, %eax
	movb	%cl, %al
	andb	$0x3f, %al
	movl	0x14(%ebp), %edi
	movl	%eax, (%edi)

	/* cylinders */
	shrb	$6, %cl
	movb	%cl, %ah
	movb	%ch, %al
	incl	%eax		/* the number of cylinders is 
				   counted from zero */
	movl	0xc(%ebp), %edi
	movl	%eax, (%edi)

	xorl	%eax, %eax
	movb	%bl, %al	/* return value in %eax */

	popl	%edi
	popl	%ebx
	popl	%ebp

	ret

		
/*
 *   int get_diskinfo_floppy (int drive, unsigned long *cylinders, 
 *                            unsigned long *heads, unsigned long *sectors)
 *
 *   Return the geometry of DRIVE in CYLINDERS, HEADS and SECTORS. If an
 *   error occurs, then return non-zero, otherwise zero.
 */

ENTRY(get_diskinfo_floppy)
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%esi

	/* drive */
	movb	0x8(%ebp), %dl
	/* enter real mode */
	call	EXT_C(prot_to_real)

	.code16
	/* init probe value */
	movl	$probe_values-1, %esi
1:
	xorw	%ax, %ax
	int	$0x13		/* reset floppy controller */

	incw	%si
	movb	(%si), %cl
	cmpb	$0, %cl		/* probe failed if zero */
	je	2f

	/* perform read */
	movw	$SCRATCHSEG, %ax
	movw	%ax, %es
	xorw	%bx, %bx
	movw	$0x0201, %ax
	movb	$0, %ch
	movb	$0, %dh
	int	$0x13

	/* FIXME: Read from floppy may fail even if the geometry is correct.
	   So should retry at least three times.  */
	jc	1b		/* next value */
	
	/* succeed */
	jmp	2f
	
probe_values:
	.byte	36, 18, 15, 9, 0
	
2:
	/* back to protected mode */
	DATA32	call	EXT_C(real_to_prot)
	.code32

	/* restore %ebp */
	leal	0x8(%esp), %ebp
	
	/* cylinders */
	movl	0xc(%ebp), %eax
	movl	$80, %ebx
	movl	%ebx, (%eax)
	/* heads */
	movl	0x10(%ebp), %eax
	movl	$2, %ebx
	movl	%ebx, (%eax)
	/* sectors */
	movl	0x14(%ebp), %eax
	movzbl	%cl, %ebx
	movl	%ebx, (%eax)

	/* return value in %eax */
	xorl	%eax, %eax
	cmpb	$0, %cl
	jne	3f
	incl	%eax		/* %eax = 1 (non-zero) */
3:
	popl	%esi
	popl	%ebx
	popl	%ebp

	ret


/* This is stolen from libc/x86/setjmp.S in the OSKit */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * C library -- _setjmp, _longjmp
 *
 *      _longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *      _setjmp(a)
 * by restoring registers from the stack,
 * The previous signal state is NOT restored.
 *
 */
	
ENTRY(grub_setjmp)
	movl	4(%esp), %ecx		/* fetch buffer */
	movl	%ebx, 0(%ecx)
	movl	%esi, 4(%ecx)
	movl	%edi, 8(%ecx)
	movl	%ebp, 12(%ecx)		/* save frame pointer of caller */
	popl	%edx
	movl	%esp, 16(%ecx)		/* save stack pointer of caller */
	movl	%edx, 20(%ecx)		/* save pc of caller */
	xorl	%eax, %eax
        jmp     *%edx

ENTRY(grub_longjmp)
	movl	8(%esp), %eax		/* return(v) */
	movl	4(%esp), %ecx		/* fetch buffer */
	movl	0(%ecx), %ebx
	movl	4(%ecx), %esi
	movl	8(%ecx), %edi
	movl	12(%ecx), %ebp
	movl	16(%ecx), %esp
	orl	%eax, %eax
	jnz	0f
	incl	%eax
0:	jmp	*20(%ecx)		/* done, return.... */

	
/*
 * console_putchar(c)
 *
 * Puts character on the screen, interpreting '\n' as in the UNIX fashion.
 *
 * BIOS call "INT 10H Function 0Eh" to write character to console
 *      Call with       %ah = 0x0e
 *                      %al = character
 *                      %bh = page
 *                      %bl = foreground color ( graphics modes)
 */

ENTRY(console_putchar)
	push	%ebp
	push	%eax
	push	%ebx

	movb	0x10(%esp), %bl
	call	EXT_C(prot_to_real)
	.code16

	movb	%bl, %al
	movb	$0xe, %ah
	movw	$1, %bx
	int	$0x10

	DATA32	call	EXT_C(real_to_prot)
	.code32

	pop	%ebx
	pop	%eax
	pop	%ebp
	ret

/* get_code_end() :  return the address of the end of the code
 * This is here so that it can be replaced by asmstub.c.
 */
ENTRY(get_code_end)
	/* will be the end of the bss */
# if defined(HAVE_END_SYMBOL)
	movl	$end, %eax
# elif defined(HAVE_USCORE_END_SYMBOL)
	movl	$_end, %eax
# endif
	shrl	$2, %eax		/* Round up to the next word. */
	incl	%eax
	shll	$2, %eax
	ret

/*
 *
 * get_memsize(i) :  return the memory size in KB. i == 0 for conventional
 *		memory, i == 1 for extended memory
 *	BIOS call "INT 12H" to get conventional memory size
 *	BIOS call "INT 15H, AH=88H" to get extended memory size
 *		Both have the return value in AX.
 *
 */

ENTRY(get_memsize)
	push	%ebp
	push	%ebx

	mov	0xc(%esp), %ebx

	call	EXT_C(prot_to_real)	/* enter real mode */
	.code16

	cmpb	$0x1, %bl
	DATA32	je	xext

	int	$0x12
	DATA32	jmp	xdone

xext:
	movb	$0x88, %ah
	int	$0x15

xdone:
	movw	%ax, %bx

	DATA32	call	EXT_C(real_to_prot)
	.code32

	movw	%bx, %ax
	pop	%ebx
	pop	%ebp
	ret


/*
 *
 * get_eisamemsize() :  return packed EISA memory map, lower 16 bits is
 *		memory between 1M and 16M in 1K parts, upper 16 bits is
 *		memory above 16M in 64K parts.  If error, return -1.
 *	BIOS call "INT 15H, AH=E801H" to get EISA memory map,
 *		AX = memory between 1M and 16M in 1K parts.
 *		BX = memory above 16M in 64K parts.
 *
 */

ENTRY(get_eisamemsize)
	push	%ebp
	push	%ebx

	call	EXT_C(prot_to_real)	/* enter real mode */
	.code16

	movw	$0xe801, %ax
	int	$0x15

	shll	$16, %ebx
	movw	%ax, %bx

	DATA32	call	EXT_C(real_to_prot)
	.code32

	movl	$0xFFFFFFFF, %eax
	cmpb	$0x86, %bh
	je	xnoteisa

	movl	%ebx, %eax

xnoteisa:
	pop	%ebx
	pop	%ebp
	ret

/*
 *
 * get_mmap_entry(addr, cont) :  address and old continuation value (zero to
 *		start), for the Query System Address Map BIOS call.
 *
 *  Sets the first 4-byte int value of "addr" to the size returned by
 *  the call.  If the call fails, sets it to zero.
 *
 *	Returns:  new (non-zero) continuation value, 0 if done.
 *
 * NOTE: Currently hard-coded for a maximum buffer length of 1024.
 */

ENTRY(get_mmap_entry)
	push	%ebp
	push	%ebx
	push	%edi
	push	%esi

	/* place address (+4) in ES:DI */
	movl	0x14(%esp), %eax
	addl	$4, %eax
	movl	%eax, %edi
	andl	$0xf, %edi
	shrl	$4, %eax
	movl	%eax, %esi

	/* set continuation value */
	movl	0x18(%esp), %ebx

	/* set default maximum buffer size */
	movl	$0x14, %ecx

	/* set EDX to 'SMAP' */
	movl	$0x534d4150, %edx

	call	EXT_C(prot_to_real)	/* enter real mode */
	.code16

	movw	%si, %es
	movl	$0xe820, %eax
	int	$0x15

	DATA32	jc	xnosmap

	cmpl	$0x534d4150, %eax
	DATA32	jne	xnosmap

	cmpl	$0x14, %ecx
	DATA32	jl	xnosmap

	cmpl	$0x400, %ecx
	DATA32	jg	xnosmap

	DATA32	jmp	xsmap

xnosmap:
	movl	$0, %ecx

xsmap:
	DATA32	call	EXT_C(real_to_prot)
	.code32

	/* write length of buffer (zero if error) into "addr" */
	movl	0x14(%esp), %eax
	movl	%ecx, (%eax)

	/* set return value to continuation */
	movl	%ebx, %eax

	pop	%esi
	pop	%edi
	pop	%ebx
	pop	%ebp
	ret

/*
 * gateA20(int linear)
 *
 * Gate address-line 20 for high memory.
 *
 * This routine is probably overconservative in what it does, but so what?
 *
 * It also eats any keystrokes in the keyboard buffer.  :-(
 */

ENTRY(gateA20)
/*	jmp	gateA20_98 */
	
	pushl	%eax

	call    gloop1

	movb	$KC_CMD_WOUT, %al
	outb	$K_CMD

gloopint1:
	inb	$K_STATUS
	andb	$K_IBUF_FUL, %al
	jnz	gloopint1

	movb	$KB_OUTPUT_MASK, %al
	cmpb	$0, 0x8(%esp)
	jz	gdoit

	orb	$KB_A20_ENABLE, %al
gdoit:
	outb	$K_RDWR

	call	gloop1

	popl	%eax
	ret

gloop1:
	inb	$K_STATUS
	andb	$K_IBUF_FUL, %al
	jnz	gloop1

gloop2:
	inb	$K_STATUS
	andb	$K_OBUF_FUL, %al
	jz	gloop2ret
	inb	$K_RDWR
	jmp	gloop2

gloop2ret:
	ret

	
ENTRY(gateA20_98)
	pushl   %eax

	inb     $0x92, %al
	andb    $0xfe, %al
	testb   $2, %al
	jz      goff

	cmpl    $0, 0x8(%esp)
	jnz     gret

	andb    $0xfc, %al
	outb    $0x92
	jmp     gret

goff:
	cmpl    $0, 0x8(%esp)
	jz      gret

	orb     $2, %al
	outb    $0x92

gret:
	popl    %eax
	ret

ENTRY(patch_code)	/* labels start with "pc_" */
	.code16

	mov	%cs, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	ADDR32	movl	$0, 0
pc_stop:
	hlt
	DATA32	jmp	pc_stop
ENTRY(patch_code_end)

	.code32



/*
 * linux_boot()
 *
 * Does some funky things (including on the stack!), then jumps to the
 * entry point of the Linux setup code.
 */

VARIABLE(linux_text_len)
	.long	0
	
VARIABLE(linux_data_tmp_addr)
	.long	0
	
VARIABLE(linux_data_real_addr)
	.long	0
	
ENTRY(linux_boot)
	/* don't worry about saving anything, we're committed at this point */
	cld	/* forward copying */

	/* copy kernel */
	movl	EXT_C(linux_text_len), %ecx
	addl	$3, %ecx
	shrl	$2, %ecx
	movl	$LINUX_BZIMAGE_ADDR, %esi
	movl	$LINUX_ZIMAGE_ADDR, %edi

	rep
	movsl

ENTRY(big_linux_boot)
	movl	EXT_C(linux_data_real_addr), %ebx
	
	/* copy the real mode part */
	movl	EXT_C(linux_data_tmp_addr), %esi
	movl	%ebx, %edi
	movl	$LINUX_SETUP_MOVE_SIZE, %ecx
	cld
	rep
	movsb

	/* change %ebx to the segment address */
	shrl	$4, %ebx
	movl	%ebx, %eax
	addl	$0x20, %eax
	movl	%eax, linux_setup_seg
			
	/* XXX new stack pointer in safe area for calling functions */
	movl	$0x4000, %esp
	call	EXT_C(stop_floppy)

	/* final setup for linux boot */

	call	EXT_C(prot_to_real)
	.code16

	/* final setup for linux boot */
	cli
	movw	%bx, %ss
	movw	$LINUX_SETUP_STACK, %sp
	
	movw	%bx, %ds
	movw	%bx, %es
	movw	%bx, %fs
	movw	%bx, %gs

	/* jump to start */
	/* ljmp */
	.byte	0xea
	.word	0
linux_setup_seg:	
	.word	0
	.code32


/*
 * multi_boot(int start, int mb_info)
 *
 *  This starts a kernel in the manner expected of the multiboot standard.
 */

ENTRY(multi_boot)
	/* no need to save anything */
	call	EXT_C(stop_floppy)

	movl	$0x2BADB002, %eax
	movl	0x8(%esp), %ebx

	/* boot kernel here (absolute address call) */
	call	*0x4(%esp)

	/* error */
	call	EXT_C(stop)


/*
 * console_cls()
 * BIOS call "INT 10H Function 0Fh" to get current video mode
 *	Call with	%ah = 0x0f
 *      Returns         %al = (video mode)
 *                      %bh = (page number)
 * BIOS call "INT 10H Function 00h" to set the video mode (clears screen)
 *	Call with	%ah = 0x00
 *                      %al = (video mode)
 */


ENTRY(console_cls)
	push	%ebp
	push	%ebx                    /* save EBX */

	call	EXT_C(prot_to_real)
	.code16

	movb	$0xf, %ah
	int	$0x10			/* Get Current Video mode */
        xorb	%ah, %ah
        int	$0x10                   /* Set Video mode (clears screen) */

	DATA32	call	EXT_C(real_to_prot)
	.code32

	pop	%ebx
	pop	%ebp
	ret


/*
 * int console_setcursor (int on)
 * BIOS call "INT 10H Function 01h" to set cursor type
 *      Call with       %ah = 0x01
 *                      %ch = cursor starting scanline
 *                      %cl = cursor ending scanline
 */

console_cursor_state:
	.byte	1
console_cursor_shape:
	.word	0
	
ENTRY(console_setcursor)
	push	%ebp
	push	%ebx

	/* check if the standard cursor shape has already been saved */
	movw	console_cursor_shape, %ax
	testw	%ax, %ax
	jne	1f

	call	EXT_C(prot_to_real)
	.code16

	movb	$0x03, %ah
	xorb	%bh, %bh
	int	$0x10

	DATA32	call	EXT_C(real_to_prot)
	.code32

	movw	%cx, console_cursor_shape
1:
	/* set %cx to the designated cursor shape */
	movw	$0x2000, %cx
	movl	0xc(%esp), %ebx
	testl	%ebx, %ebx
	jz	2f
	movw	console_cursor_shape, %cx
2:	
	call	EXT_C(prot_to_real)
	.code16

	movb    $0x1, %ah
	int     $0x10 

	DATA32	call	EXT_C(real_to_prot)
	.code32

	movzbl	console_cursor_state, %eax
	movb	%bl, console_cursor_state
	
	pop	%ebx
	pop	%ebp
	ret

	
/*
 * nocursor()
 * BIOS call "INT 10H Function 01h" to set cursor type
 *      Call with       %ah = 0x01
 *                      %ch = cursor starting scanline
 *                      %cl = cursor ending scanline
 */

ENTRY(nocursor)
	push	%ebp
	push	%ebx                    /* save EBX */

	call	EXT_C(prot_to_real)
	.code16

	movw    $0x2000, %cx
	movb    $0x1, %ah
	int     $0x10 

	DATA32	call	EXT_C(real_to_prot)
	.code32

	pop	%ebx
	pop	%ebp
	ret

/* graphics mode functions */
#ifdef SUPPORT_GRAPHICS
VARIABLE(cursorX)
.word	0
VARIABLE(cursorY)
.word	0
VARIABLE(cursorCount)
.word 0
VARIABLE(cursorBuf)
.byte	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

	
/*
 * int set_videomode(mode)
 * BIOS call "INT 10H Function 0h" to set video mode
 *	Call with	%ah = 0x0
 *			%al = video mode
 *      Returns old videomode.
 */
ENTRY(set_videomode)
	push	%ebp
	push	%ebx
	push	%ecx

	movb	0x10(%esp), %cl

	call	EXT_C(prot_to_real)
	.code16

	xorw	%bx, %bx
	movb	$0xf, %ah
	int	$0x10			/* Get Current Video mode */
	movb	%al, %ch
	xorb	%ah, %ah
	movb	%cl, %al
        int	$0x10			/* Set Video mode */

	DATA32	call	EXT_C(real_to_prot)
	.code32

	xorb	%ah, %ah
	movb	%ch, %al

	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret


/*
 * unsigned char * graphics_get_font()
 * BIOS call "INT 10H Function 11h" to set font
 *      Call with       %ah = 0x11
 */
ENTRY(graphics_get_font)
	push	%ebp
	push	%ebx
	push	%ecx
	push	%edx

	call	EXT_C(prot_to_real)
	.code16

	movw	$0x1130, %ax
	movb	$6, %bh		/* font 8x16 */
	int	$0x10
	movw	%bp, %dx
	movw	%es, %cx

	DATA32	call	EXT_C(real_to_prot)
	.code32

	xorl	%eax, %eax
	movw	%cx, %ax
	shll	$4, %eax
	movw	%dx, %ax

	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret
	

	
/*
 * graphics_set_palette(index, red, green, blue)
 * BIOS call "INT 10H Function 10h" to set individual dac register
 *	Call with	%ah = 0x10
 *			%bx = register number
 *			%ch = new value for green (0-63)
 *			%cl = new value for blue (0-63)
 *			%dh = new value for red (0-63)
 */

ENTRY(graphics_set_palette)
	push	%ebp
	push	%eax
	push	%ebx
	push	%ecx
	push	%edx

	movw	$0x3c8, %bx		/* address write mode register */

	/* wait vertical retrace */

	movw	$0x3da, %dx
l1b:	inb	%dx, %al	/* wait vertical active display */
	test	$8, %al
	jnz	l1b

l2b:	inb	%dx, %al	/* wait vertical retrace */
	test	$8, %al
	jnz	l2b

	mov	%bx, %dx
	movb	0x18(%esp), %al		/* index */
	outb	%al, %dx
	inc	%dx

	movb	0x1c(%esp), %al		/* red */
	outb	%al, %dx

	movb	0x20(%esp), %al		/* green */
	outb	%al, %dx

	movb	0x24(%esp), %al		/* blue */
	outb	%al, %dx

	movw	0x18(%esp), %bx

	call	EXT_C(prot_to_real)
	.code16

	movb	%bl, %bh
	movw	$0x1000, %ax
	int	$0x10

	DATA32	call	EXT_C(real_to_prot)
	.code32	

	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%eax
	pop	%ebp
	ret

#endif /* SUPPORT_GRAPHICS */
		

/*
 * console_getxy()
 * BIOS call "INT 10H Function 03h" to get cursor position
 *	Call with	%ah = 0x03
 *			%bh = page
 *      Returns         %ch = starting scan line
 *                      %cl = ending scan line
 *                      %dh = row (0 is top)
 *                      %dl = column (0 is left)
 */


ENTRY(console_getxy)
	push	%ebp
	push	%ebx                    /* save EBX */

	call	EXT_C(prot_to_real)
	.code16

        xorb	%bh, %bh                /* set page to 0 */
	movb	$0x3, %ah
	int	$0x10			/* get cursor position */

	DATA32	call	EXT_C(real_to_prot)
	.code32

	movb	%dl, %ah
	movb	%dh, %al

	pop	%ebx
	pop	%ebp
	ret


/*
 * console_gotoxy(x,y)
 * BIOS call "INT 10H Function 02h" to set cursor position
 *	Call with	%ah = 0x02
 *			%bh = page
 *                      %dh = row (0 is top)
 *                      %dl = column (0 is left)
 */


ENTRY(console_gotoxy)
	push	%ebp
	push	%ebx                    /* save EBX */

	movb	0xc(%esp), %dl           /* %dl = x */
	movb	0x10(%esp), %dh          /* %dh = y */

	call	EXT_C(prot_to_real)
	.code16

        xorb	%bh, %bh                /* set page to 0 */
	movb	$0x2, %ah
	int	$0x10			/* set cursor position */

	DATA32	call	EXT_C(real_to_prot)
	.code32

	pop	%ebx
	pop	%ebp
	ret


/*
 * set_attrib(attr) :  Sets the character attributes for character at
 *		current cursor position.
 *
 *  Bitfields for character's display attribute:
 *  Bit(s)	Description
 *   7		foreground blink
 *   6-4	background color
 *   3		foreground bright
 *   2-0	foreground color
 *
 *  Values for character color:
 *		Normal		Bright
 *   000b	black		dark gray
 *   001b	blue		light blue
 *   010b	green		light green
 *   011b	cyan		light cyan
 *   100b	red		light red
 *   101b	magenta		light magenta
 *   110b	brown		yellow
 *   111b	light gray	white
 *
 * BIOS call "INT 10H Function 08h" to read character and attribute data
 *	Call with	%ah = 0x08
 *			%bh = page
 *	Returns		%ah = character attribute
 *			%al = character value
 * BIOS call "INT 10H Function 09h" to write character and attribute data
 *	Call with	%ah = 0x09
 *			%al = character value
 *			%bh = page
 *			%bl = character attribute
 *			%cx = count to display (???, possible side-effects!!)
 */

ENTRY(set_attrib)
	push	%ebp
	push	%ebx

	movl	0xc(%esp), %ecx
	xorl	%ebx, %ebx

	call	EXT_C(prot_to_real)
	.code16

	movb	$0x8, %ah
	int	$0x10
	movb	$0x9, %ah
	movb	%cl, %bl
	movw	$1, %cx
	int	$0x10

	DATA32	call	EXT_C(real_to_prot)
	.code32

	pop	%ebx
	pop	%ebp
	ret


/*
 * getrtsecs()
 *	if a seconds value can be read, read it and return it (BCD),
 *      otherwise return 0xFF
 * BIOS call "INT 1AH Function 02H" to check whether a character is pending
 *	Call with	%ah = 0x2
 *	Return:
 *		If RT Clock can give correct values
 *			%ch = hour (BCD)
 *			%cl = minutes (BCD)
 *                      %dh = seconds (BCD)
 *                      %dl = daylight savings time (00h std, 01h daylight)
 *			Carry flag = clear
 *		else
 *			Carry flag = set
 *                         (this indicates that the clock is updating, or
 *                          that it isn't running)
 */
ENTRY(getrtsecs)
	push	%ebp

	call	EXT_C(prot_to_real)	/* enter real mode */
	.code16

	movb	$0x2, %ah
	int	$0x1a

	DATA32	jnc	gottime
	movb	$0xff, %dh

gottime:
	DATA32	call	EXT_C(real_to_prot)
	.code32

	movb	%dh, %al

	pop	%ebp
	ret

	
/*
 * currticks()
 *	return the real time in ticks, of which there are about
 *	18-20 per second
 */
ENTRY(currticks)
	pushl	%ebp

	call	EXT_C(prot_to_real)	/* enter real mode */
	.code16

	/* %ax is already zero */
        int	$0x1a

	DATA32	call	EXT_C(real_to_prot)
	.code32

	movl	%ecx, %eax
	shll	$16, %eax
	movw	%dx, %ax

	popl	%ebp
	ret

	
/*
 * remap_ascii_char remaps the ascii code %bl to another if the code is
 * contained in ASCII_KEY_MAP.
 */
	.code16
	
remap_ascii_char:
	pushw	%si

	/* if key code > 55 is not ascii so don't remap */
	cmpb	$55, %bh
	jg	dont_remap
	
	movw	$ABS(EXT_C(ascii_key_map)), %si
1:
	lodsw
	/* check if this is the end */
	testw	%ax, %ax
	jz	2f
	/* check if this matches the ascii code */
	cmpb	%al, %bl
	jne	1b
	/* if so, perform the mapping */
	movb	%ah, %bl
2:
dont_remap:
	/* restore %si */
	popw	%si

	ret

	.code32

	.align	4
ENTRY(ascii_key_map)
	.space	(KEY_MAP_SIZE + 1) * 2
	

/*
 * console_getkey()
 * BIOS call "INT 16H Function 00H" to read character from keyboard
 *	Call with	%ah = 0x0
 *	Return:		%ah = keyboard scan code
 *			%al = ASCII character
 */

ENTRY(console_getkey)
	push	%ebp
	push	%ebx			/* save %ebx */

	call	EXT_C(prot_to_real)
	.code16

	int	$0x16

	movw	%ax, %bx		/* real_to_prot uses %eax */
	call	remap_ascii_char
	
	DATA32	call	EXT_C(real_to_prot)
	.code32

	movw	%bx, %ax

	pop	%ebx
	pop	%ebp
	ret

ENTRY(console_getkbdstatuskey)
	push	%ebp
	push	%ebx			/* save %ebx */

	call	EXT_C(prot_to_real)
	.code16

	movb	$0x2, %ah
	int	$0x16

	movw	%ax, %bx		/* real_to_prot uses %eax */
	
	DATA32	call	EXT_C(real_to_prot)
	.code32
	movw	%bx, %ax

	pop	%ebx
	pop	%ebp
	ret


/*
 * console_checkkey()
 *	if there is a character pending, return it; otherwise return -1
 * BIOS call "INT 16H Function 01H" to check whether a character is pending
 *	Call with	%ah = 0x1
 *	Return:
 *		If key waiting to be input:
 *			%ah = keyboard scan code
 *			%al = ASCII character
 *			Zero flag = clear
 *		else
 *			Zero flag = set
 */
ENTRY(console_checkkey)
	push	%ebp
	push	%ebx

	xorl	%ebx, %ebx

	call	EXT_C(prot_to_real)	/* enter real mode */
	.code16

	movb	$0x1, %ah
	int	$0x16

	DATA32	jz	notpending
	
	movw	%ax, %bx
	call	remap_ascii_char
	DATA32	jmp	pending

notpending:
	movl	$0xFFFFFFFF, %ebx

pending:
	DATA32	call	EXT_C(real_to_prot)
	.code32

	mov	%ebx, %eax

	pop	%ebx
	pop	%ebp
	ret

/*
 *  This is the area for all of the special variables.
 */

	.p2align	2	/* force 4-byte alignment */

protstack:
	.long	PROTSTACKINIT

VARIABLE(boot_drive)
#ifdef SUPPORT_DISKLESS
	.long	NETWORK_DRIVE
VARIABLE(torito_sp)
	.byte	0x13	/* 13 */
	.byte	0	/* Media Type */
	.byte	0	/* Drive Number */
	.byte	0	/* Controler */
	.long	0	/* LBA */
	.word	0	/* Device Spec */
	.word	0	/* User SEG */
	.word	0	/* Load SEG */
	.word	0	/* Sect Count */
	.byte	0	/* Cyl */
	.byte	0	/* Sect */
	.byte	0	/* Heads */
#else
	.long	0
#endif

VARIABLE(install_second_sector)
	.long	0

#ifdef SUPPORT_NETBOOT
VARIABLE(pxe_pointer_segment)
	.word	0
VARIABLE(pxe_pointer_offset)
	.word	0
VARIABLE(pxe_int1a_segment)
	.word	0
VARIABLE(pxe_int1a_offset)
	.word	0
VARIABLE(pxe_stack_segment)
	.word	0
VARIABLE(pxe_stack_offset)
	.word	0
#endif

	.p2align	4

VARIABLE(X86)
	.long	0
VARIABLE(X86_CPUID)
	.long	0
VARIABLE(X86_CAPABILITY)
	.long	0
VARIABLE(X86_VENDOR_ID)
	.long	0
	.long	0
	.long	0

st_low:
	.long	0
st_high:
	.long	0
end_low:
	.long	0
end_high:
	.long	0

	/* an address can only be long-jumped to if it is in memory, this
	   is used by multiple routines */
offset:
	.long	0x8000
segment:
	.word	0

/*
 * This is the Global Descriptor Table
 *
 *  An entry, a "Segment Descriptor", looks like this:
 *
 * 31          24         19   16                 7           0
 * ------------------------------------------------------------
 * |             | |B| |A|       | |   |1|0|E|W|A|            |
 * | BASE 31..24 |G|/|0|V| LIMIT |P|DPL|  TYPE   | BASE 23:16 |
 * |             | |D| |L| 19..16| |   |1|1|C|R|A|            |
 * ------------------------------------------------------------
 * |                             |                            |
 * |        BASE 15..0           |       LIMIT 15..0          |
 * |                             |                            |
 * ------------------------------------------------------------
 *
 *  Note the ordering of the data items is reversed from the above
 *  description.
 */

	.p2align	2	/* force 4-byte alignment */
gdt:
	.word	0, 0
	.byte	0, 0, 0, 0

	/* code segment */
	.word	0xFFFF, 0
	.byte	0, 0x9A, 0xCF, 0

	/* data segment */
	.word	0xFFFF, 0
	.byte	0, 0x92, 0xCF, 0

	/* 16 bit real mode CS */
	.word	0xFFFF, 0
	.byte	0, 0x9E, 0, 0

	/* 16 bit real mode DS */
	.word	0xFFFF, 0
	.byte	0, 0x92, 0, 0


/* this is the GDT descriptor */
gdtdesc:
	.word	0x27			/* limit */
	.long	gdt			/* addr */

