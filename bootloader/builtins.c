/* builtins.c - the GRUB builtin commands */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1996  Erich Boleyn  <erich@uruk.org>
 *  Copyright (C) 1999, 2000  Free Software Foundation, Inc.
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

/* Include stdio.h before shared.h, because we can't define
   WITHOUT_LIBC_STUBS here.  */

#include <shared.h>
#include <filesys.h>
#include <term.h>
#include "pci.h"
#include "builtins_lbs.h"

#ifdef SUPPORT_NETBOOT
#include <etherboot.h>
#include "pxe.h"
#endif

//#include "mbr.h"
#include "pc_slice.h"

/* Terminal types.  */
int terminal = TERMINAL_CONSOLE;
/* The type of kernel loaded.  */
kernel_t kernel_type;
/* The boot device.  */
static int bootdev;
/* True when the debug mode is turned on, and false
   when it is turned off.  */
int debug = 0;
/* The default entry.  */
int default_entry = 0;
/* The fallback entry.  */
int fallback_entry = -1;
/* The number of current entry.  */
int current_entryno;
/* The address for Multiboot command-line buffer.  */
static char *mb_cmdline;
/* Color settings.  */
int normal_color;
int highlight_color;
/* The timeout.  */
int grub_timeout = -1;
/* Whether to show the menu or not.  */
int show_menu = 1;
/* The BIOS drive map.  */
static unsigned short bios_drive_map[DRIVE_MAP_SIZE + 1];


/* Initialize the data for builtins.  */
void
init_builtins (void)
{
  kernel_type = KERNEL_TYPE_NONE;
  /* BSD and chainloading evil hacks!  */
  bootdev = set_bootdev (0);
  mb_cmdline = (char *) MB_CMDLINE_BUF;
}

/* Initialize the data for the configuration file.  */
void
init_config (void)
{
  default_entry = 0;
  normal_color = A_NORMAL;
  highlight_color = A_REVERSE;
  fallback_entry = -1;
  grub_timeout = -1;
}

/* Print which sector is read when loading a file.  */
static void
disk_read_print_func (int sector, int offset, int length)
{
  grub_printf ("[%d,%d,%d]", sector, offset, length);
}

/* boot */
static int
boot_func (char *arg, int flags)
{
  /* Clear the int15 handler if we can boot the kernel successfully.
     This assumes that the boot code never fails only if KERNEL_TYPE is
     not KERNEL_TYPE_NONE. Is this assumption is bad?  */
//  if (kernel_type != KERNEL_TYPE_NONE)
//    unset_int15_handler ();

  /* Linbox: go back to console before booting */
  if (current_term->shutdown)                                                 
    {                                                                         
       (*current_term->shutdown)();                                            
       current_term = term_table; /* assumption: console is first */           
     }
  

  printf ("Kernel type : %d\n", kernel_type);

  switch (kernel_type)
    {
    case KERNEL_TYPE_FREEBSD:
    case KERNEL_TYPE_NETBSD:
      /* *BSD */
      bsd_boot (kernel_type, bootdev, (char *) mbi.cmdline);
      break;

    case KERNEL_TYPE_LINUX:
      /* Linux */
      pxe_unload ();
      linux_boot ();
      break;

    case KERNEL_TYPE_BIG_LINUX:
      /* Big Linux */
      pxe_unload ();
      big_linux_boot ();
      break;

    case KERNEL_TYPE_CHAINLOADER:
      /* Chainloader */

#ifdef INT13HANDLER
      /* Check if we should set the int13 handler.  */
      if (bios_drive_map[0] != 0)
	{
	  int i;

	  /* Search for SAVED_DRIVE.  */
	  for (i = 0; i < DRIVE_MAP_SIZE; i++)
	    {
	      if (!bios_drive_map[i])
		break;
	      else if ((bios_drive_map[i] & 0xFF) == saved_drive)
		{
		  /* Exchage SAVED_DRIVE with the mapped drive.  */
		  saved_drive = (bios_drive_map[i] >> 8) & 0xFF;
		  break;
		}
	    }

	  /* Set the handler. This is somewhat dangerous.  */
	  set_int13_handler (bios_drive_map);
	}
#endif

      gateA20 (0);
      boot_drive = saved_drive;

      /* Copy the boot partition information to 0x7be-0x7fd, if
         BOOT_DRIVE is a hard disk drive.  */
      if (boot_drive & 0x80)
	{
	  char *dst, *src;
	  int i;
	  
	  /* Read the MBR here, because it might be modified
	     after opening the partition.  */
	  if (!rawread (boot_drive, boot_part_offset,
			0, SECTOR_SIZE, (char *) SCRATCHADDR))
	    {
	      /* This should never happen.  */
	      errnum = ERR_READ;
	      return 0;
	    }

	  /* Need only the partition table.
	     XXX: We cannot use grub_memmove because BOOT_PART_TABLE
	     (0x07be) is less than 0x1000.  */
	  dst = (char *) BOOT_PART_TABLE;
	  src = (char *) SCRATCHADDR + BOOTSEC_PART_OFFSET;
	  while (dst < (char *) BOOT_PART_TABLE + BOOTSEC_PART_LENGTH)
	    *dst++ = *src++;

	/* Set the active flag of the booted partition.  */
	    for (i = 0; i < 4; i++)
        	PC_SLICE_FLAG (BOOT_PART_TABLE, i) = 0;
	    *((unsigned char *) boot_part_addr) = PC_SLICE_FLAG_BOOTABLE;
	}

      pxe_unload ();
      chain_stage1 (0, BOOTSEC_LOCATION, boot_part_addr);
      break;

    case KERNEL_TYPE_MULTIBOOT:
      /* Multiboot */
      multi_boot ((int) entry_addr, (int) &mbi);
      break;

    default:
      errnum = ERR_BOOT_COMMAND;
      return 1;
    }

  return 0;
}

static struct builtin builtin_boot = {
  "boot",
  boot_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "boot",
  "Boot the OS/chain-loader which has been loaded."
#endif
};


#ifdef SUPPORT_NETBOOT_NO
/* bootp */
static int
bootp_func (char *arg, int flags)
{
  if (!bootp ())
    {
      if (errnum == ERR_NONE)
	errnum = ERR_DEV_VALUES;

      return 1;
    }

  /* Notify the configuration.  */
  print_network_configuration ();
  return 0;
}

static struct builtin builtin_bootp = {
  "bootp",
  bootp_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "bootp",
  "Initialize a network device via BOOTP."
#endif
};
#endif /* SUPPORT_NETBOOT */


/* cat */

int set_hw (int, int);

static int
cat_func (char *arg, int flags)
{
  char c;

  if (!grub_open (arg))
    return 1;

  while (grub_read (&c, 1))
    grub_putchar (c);

  grub_close ();
  return 0;
}

static struct builtin builtin_cat = {
  "cat",
  cat_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "cat FILE",
  "Print the contents of the file FILE."
#endif
};


/* chainloader */
static int
chainloader_func (char *arg, int flags)
{
  int force = 0;
  char *file = arg;

  /* If the option `--force' is specified?  */
  if (substring ("--force", arg) <= 0)
    {
      force = 1;
      file = skip_to (0, arg);
    }

  /* Open the file.  */
  if (!grub_open (file))
    {
      kernel_type = KERNEL_TYPE_NONE;
      return 1;
    }

  /* Read the first block.  */
  if (grub_read ((char *) BOOTSEC_LOCATION, SECTOR_SIZE) != SECTOR_SIZE)
    {
      grub_close ();
      kernel_type = KERNEL_TYPE_NONE;
      return 1;
    }

  /* If not loading it forcibly, check for the signature.  */
  if (!force
      && (*((unsigned short *) (BOOTSEC_LOCATION + BOOTSEC_SIG_OFFSET))
	  != BOOTSEC_SIGNATURE))
    {
      grub_close ();
      errnum = ERR_EXEC_FORMAT;
      kernel_type = KERNEL_TYPE_NONE;
      return 1;
    }

  grub_close ();
  kernel_type = KERNEL_TYPE_CHAINLOADER;
  return 0;
}

static struct builtin builtin_chainloader = {
  "chainloader",
  chainloader_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "chainloader [--force] FILE",
  "Load the chain-loader FILE. If --force is specified, then load it"
    " forcibly, whether the boot loader signature is present or not."
#endif
};

/* color */
/* Set new colors used for the menu interface. Support two methods to
   specify a color name: a direct integer representation and a symbolic
   color name. An example of the latter is "blink-light-gray/blue".  */
static int
color_func (char *arg, int flags)
{
  char *normal;
  char *highlight;
  int new_normal_color;
  int new_highlight_color;
  static char *color_list[16] = {
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15"
  };

  /* Convert the color name STR into the magical number.  */
  static int color_number (char *str)
  {
    char *ptr;
    int i;
    int color = 0;

    /* Find the separator.  */
    for (ptr = str; *ptr && *ptr != '/'; ptr++)
      ;

    /* If not found, return -1.  */
    if (!*ptr)
      return -1;

    /* Terminate the string STR.  */
    *ptr++ = 0;

    /* If STR contains the prefix "blink-", then set the `blink' bit
       in COLOR.  */
    if (substring ("blink-", str) <= 0)
      {
	color = 0x80;
	str += 6;
      }

    /* Search for the color name.  */
    for (i = 0; i < 16; i++)
      if (grub_strcmp (color_list[i], str) == 0)
	{
	  color |= i;
	  break;
	}

    if (i == 16)
      return -1;

    str = ptr;
    nul_terminate (str);

    /* Search for the color name.  */
    for (i = 0; i < 8; i++)
      if (grub_strcmp (color_list[i], str) == 0)
	{
	  color |= i << 4;
	  break;
	}

    if (i == 8)
      return -1;

    return color;
  }

  normal = arg;
  highlight = skip_to (0, arg);

  new_normal_color = color_number (normal);
  if (new_normal_color < 0 && !safe_parse_maxint (&normal, &new_normal_color))
    return 1;

  /* The second argument is optional, so set highlight_color
     to inverted NORMAL_COLOR.  */
  if (!*highlight)
    new_highlight_color = ((new_normal_color >> 4)
			   | ((new_normal_color & 0xf) << 4));
  else
    {
      new_highlight_color = color_number (highlight);
      if (new_highlight_color < 0
	  && !safe_parse_maxint (&highlight, &new_highlight_color))
	return 1;
    }

  normal_color = new_normal_color;
  highlight_color = new_highlight_color;
  return 0;
}

static struct builtin builtin_color = {
  "color",
  color_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "color NORMAL [HIGHLIGHT]",
  "Change the menu colors. The color NORMAL is used for most"
    " lines in the menu, and the color HIGHLIGHT is used to highlight the"
    " line where the cursor points. If you omit HIGHLIGHT, then the"
    " inverted color of NORMAL is used for the highlighted line."
    " The format of a color is \"FG/BG\". FG and BG are symbolic color names."
    " A symbolic color name must be one of these: black, blue, green,"
    " cyan, red, magenta, brown, light-gray, dark-gray, light-blue,"
    " light-green, light-cyan, light-red, light-magenta, yellow and white."
    " But only the first eight names can be used for BG. You can prefix"
    " \"blink-\" to FG if you want a blinking foreground color."
#endif
};

 
/* configfile */
static int
configfile_func (char *arg, int flags)
{
  char *new_config = config_file;

  /* Check if the file ARG is present.  */
  if (!grub_open (arg))
    return 1;

  grub_close ();

  /* Copy ARG to CONFIG_FILE.  */
  while ((*new_config++ = *arg++) != 0)
    ;

  /* Restart cmain.  */
  grub_longjmp (restart_env, 0);

  /* Never reach here.  */
  return 0;
}

static struct builtin builtin_configfile = {
  "configfile",
  configfile_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "configfile FILE",
  "Load FILE as the configuration file."
#endif
};


/* debug */
static int
debug_func (char *arg, int flags)
{
  if (debug)
    {
      debug = 0;
      grub_printf (" Debug mode is turned off\n");
    }
  else
    {
      debug = 1;
      grub_printf (" Debug mode is turned on\n");
    }

  return 0;
}

static struct builtin builtin_debug = {
  "debug",
  debug_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "debug",
  "Turn on/off the debug mode."
#endif
};


/* default */
static int
default_func (char *arg, int flags)
{
#ifndef SUPPORT_DISKLESS
  if (grub_strcmp (arg, "saved") == 0)
    {
      default_entry = saved_entryno;
      return 0;
    }
#endif /* SUPPORT_DISKLESS */

  if (!safe_parse_maxint (&arg, &default_entry))
    return 1;

  return 0;
}

static struct builtin builtin_default = {
  "default",
  default_func,
  BUILTIN_MENU,
#if 0
  "default [NUM | `saved']",
  "Set the default entry to entry number NUM (if not specified, it is"
    " 0, the first entry) or the entry number saved by savedefault."
#endif
};

#ifdef SUPPORT_NETBOOT_NO
/* dhcp */
static int
dhcp_func (char *arg, int flags)
{
  /* For now, this is an alias for bootp.  */
  return bootp_func (arg, flags);
}

static struct builtin builtin_dhcp = {
  "dhcp",
  dhcp_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "dhcp",
  "Initialize a network device via DHCP."
#endif
};
#endif /* SUPPORT_NETBOOT */


/* displaymem */
static int
displaymem_func (char *arg, int flags)
{
  if (get_eisamemsize () != -1)
    grub_printf (" EISA Memory BIOS Interface is present\n");
  if (get_mmap_entry ((void *) SCRATCHADDR, 0) != 0
      || *((int *) SCRATCHADDR) != 0)
    grub_printf (" Address Map BIOS Interface is present\n");

  grub_printf (" Lower memory: %uK, "
	       "Upper memory (to first chipset hole): %uK\n",
	       mbi.mem_lower, mbi.mem_upper);

  if (mbi.flags & MB_INFO_MEM_MAP)
    {
      struct AddrRangeDesc *map = (struct AddrRangeDesc *) mbi.mmap_addr;
      int end_addr = mbi.mmap_addr + mbi.mmap_length;

      grub_printf (" [Address Range Descriptor entries "
		   "immediately follow (values are 64-bit)]\n");
      while (end_addr > (int) map)
	{
	  char *str;

	  if (map->Type == MB_ARD_MEMORY)
	    str = "Usable RAM";
	  else
	    str = "Reserved";
	  grub_printf ("   %s:  Base Address:  0x%x X 4GB + 0x%x,\n"
		       "      Length:   %u X 4GB + %u bytes\n",
		       str,
		       (unsigned long) (map->BaseAddr >> 32),
		       (unsigned long) (map->BaseAddr & 0xFFFFFFFF),
		       (unsigned long) (map->Length >> 32),
		       (unsigned long) (map->Length & 0xFFFFFFFF));

	  map = ((struct AddrRangeDesc *) (((int) map) + 4 + map->size));
	}
    }

  return 0;
}

static struct builtin builtin_displaymem = {
  "displaymem",
  displaymem_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "displaymem",
  "Display what GRUB thinks the system address space map of the"
    " machine is, including all regions of physical RAM installed."
#endif
};


/* fallback */
static int
fallback_func (char *arg, int flags)
{
  if (!safe_parse_maxint (&arg, &fallback_entry))
    return 1;

  return 0;
}

static struct builtin builtin_fallback = {
  "fallback",
  fallback_func,
  BUILTIN_MENU,
#if 0
  "fallback NUM",
  "Go into unattended boot mode: if the default boot entry has any"
    " errors, instead of waiting for the user to do anything, it"
    " immediately starts over using the NUM entry (same numbering as the"
    " `default' command). This obviously won't help if the machine"
    " was rebooted by a kernel that GRUB loaded."
#endif
};

/* fstest */
static int
fstest_func (char *arg, int flags)
{
  if (disk_read_hook)
    {
      disk_read_hook = NULL;
      printf (" Filesystem tracing is now off\n");
    }
  else
    {
      disk_read_hook = disk_read_print_func;
      printf (" Filesystem tracing is now on\n");
    }

  return 0;
}

static struct builtin builtin_fstest = {
  "fstest",
  fstest_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "fstest",
  "Toggle filesystem test mode."
#endif
};


/* geometry */
static int
geometry_func (char *arg, int flags)
{
  struct geometry geom;
  char *msg;
  char *device = arg;

  /* Get the device number.  */
  set_device (device);
  if (errnum)
    return 1;

  /* Check for the geometry.  */
  if (get_diskinfo (current_drive, &geom))
    {
      errnum = ERR_NO_DISK;
      return 1;
    }

  /* Attempt to read the first sector, because some BIOSes turns out not
     to support LBA even though they set the bit 0 in the support
     bitmap, only after reading something actually.  */
  if (biosdisk (BIOSDISK_READ, current_drive, &geom, 0, 1, SCRATCHSEG))
    {
      errnum = ERR_READ;
      return 1;
    }

  if (geom.flags & BIOSDISK_FLAG_LBA_EXTENSION)
    msg = "LBA";
  else
    msg = "CHS";

  grub_printf ("drive 0x%x: C/H/S = %d/%d/%d, "
	       "The number of sectors = %d, %s\n",
	       current_drive,
	       geom.cylinders, geom.heads, geom.sectors,
	       geom.total_sectors, msg);
  real_open_partition (1);

  biosdisk (BIOSDISK_READ, current_drive, &buf_geom, 0, 1, SCRATCHSEG);

#if DEBUG
  {
    int i, j = 0;
    for (i = 0x1BE; i < 0x200; i++)
      {
	grub_printf ("%x ", *((unsigned char *) (SCRATCHSEG << 4) + i));
	j++;
	if (j > 15)
	  {
	    j = 0;
	    grub_printf ("\n");
	  }
      }
  }
#endif

  return 0;
}

static struct builtin builtin_geometry = {
  "geometry",
  geometry_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "geometry DRIVE [CYLINDER HEAD SECTOR [TOTAL_SECTOR]]",
  "Print the information for a drive DRIVE. In the grub shell, you can"
    "set the geometry of the drive arbitrarily. The number of the cylinders,"
    " the one of the heads, the one of the sectors and the one of the total"
    " sectors are set to CYLINDER, HEAD, SECTOR and TOTAL_SECTOR,"
    "respectively. If you omit TOTAL_SECTOR, then it will be calculated based"
    " on the C/H/S values automatically."
#endif
};

int int10 (unsigned short i);

/* halt */
static int
halt_func (char *arg, int flags)
{
  int no_apm;

  pxe_unload ();
  no_apm = (grub_memcmp (arg, "--no-apm", 8) == 0);
  grub_halt (no_apm);

  /* Never reach here.  */
  return 1;
}

static struct builtin builtin_halt = {
  "halt",
  halt_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "halt [--no-apm]",
  "Halt your system. If APM is avaiable on it, turn off the power using"
    " the APM BIOS, unless you specify the option `--no-apm'."
#endif
};

#ifdef HELP_ON

/* help */
#define MAX_SHORT_DOC_LEN	39
#define MAX_LONG_DOC_LEN	66

static int
help_func (char *arg, int flags)
{
  if (!*arg)
    {
      /* Invoked with no argument. Print the list of the short docs.  */
      struct builtin **builtin;
      int left = 1;

      for (builtin = builtin_table; *builtin != 0; builtin++)
	{
	  int len;
	  int i;

	  /* If this cannot be run in the command-line interface,
	     skip this.  */
	  if (!((*builtin)->flags & BUILTIN_CMDLINE))
	    continue;

	  len = grub_strlen ((*builtin)->short_doc);
	  /* If the length of SHORT_DOC is too long, truncate it.  */
	  if (len > MAX_SHORT_DOC_LEN - 1)
	    len = MAX_SHORT_DOC_LEN - 1;

	  for (i = 0; i < len; i++)
	    grub_putchar ((*builtin)->short_doc[i]);

	  for (; i < MAX_SHORT_DOC_LEN; i++)
	    grub_putchar (' ');

	  if (!left)
	    grub_putchar ('\n');

	  left = !left;
	}
    }
  else
    {
      /* Invoked with one or more patterns.  */
      do
	{
	  struct builtin **builtin;
	  char *next_arg;

	  /* Get the next argument.  */
	  next_arg = skip_to (0, arg);

	  /* Terminate ARG.  */
	  nul_terminate (arg);

	  for (builtin = builtin_table; *builtin; builtin++)
	    {
	      /* Skip this if this is only for the configuration file.  */
	      if (!((*builtin)->flags & BUILTIN_CMDLINE))
		continue;

	      if (substring (arg, (*builtin)->name) < 1)
		{
		  char *doc = (*builtin)->long_doc;

		  /* At first, print the name and the short doc.  */
		  grub_printf ("%s: %s\n",
			       (*builtin)->name, (*builtin)->short_doc);

		  /* Print the long doc.  */
		  while (*doc)
		    {
		      int len = grub_strlen (doc);
		      int i;

		      /* If LEN is too long, fold DOC.  */
		      if (len > MAX_LONG_DOC_LEN)
			{
			  /* Fold this line at the position of a space.  */
			  for (len = MAX_LONG_DOC_LEN; len > 0; len--)
			    if (doc[len - 1] == ' ')
			      break;
			}

		      grub_printf ("    ");
		      for (i = 0; i < len; i++)
			grub_putchar (*doc++);
		      grub_putchar ('\n');
		    }
		}
	    }

	  arg = next_arg;
	}
      while (*arg);
    }

  return 0;
}

static struct builtin builtin_help = {
  "help",
  help_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "help [PATTERN ...]",
  "Display helpful information about builtin commands."
#endif
};
#endif


/* hiddenmenu */
static int
hiddenmenu_func (char *arg, int flags)
{
  show_menu = 0;
  return 0;
}

static struct builtin builtin_hiddenmenu = {
  "hiddenmenu",
  hiddenmenu_func,
  BUILTIN_MENU,
#if 0
  "hiddenmenu",
  "Hide the menu."
#endif
};


/* hide */
static int
hide_func (char *arg, int flags)
{
  if (!set_device (arg))
    return 1;

  if (!set_partition_hidden_flag (1))
    return 1;

  return 0;
}

static struct builtin builtin_hide = {
  "hide",
  hide_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "hide PARTITION",
  "Hide PARTITION by setting the \"hidden\" bit in"
    " its partition type code."
#endif
};

/* initrd */
static int
initrd_func (char *arg, int flags)
{
  switch (kernel_type)
    {
    case KERNEL_TYPE_LINUX:
    case KERNEL_TYPE_BIG_LINUX:
      if (!load_initrd (arg))
	return 1;
      break;

    default:
      errnum = ERR_NEED_LX_KERNEL;
      return 1;
    }

  return 0;
}

static struct builtin builtin_initrd = {
  "initrd",
  initrd_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "initrd FILE [ARG ...]",
  "Load an initial ramdisk FILE for a Linux format boot image and set the"
    " appropriate parameters in the Linux setup area in memory."
#endif
};

/* kernel */
static int
kernel_func (char *arg, int flags)
{
  int len;
  kernel_t suggested_type = KERNEL_TYPE_NONE;
  unsigned long load_flags = 0;

  /* Deal with GNU-style long options.  */
  while (1)
    {
      /* If the option `--type=TYPE' is specified, convert the string to
         a kernel type.  */
      if (grub_memcmp (arg, "--type=", 7) == 0)
	{
	  arg += 7;

	  if (grub_memcmp (arg, "netbsd", 6) == 0)
	    suggested_type = KERNEL_TYPE_NETBSD;
	  else if (grub_memcmp (arg, "freebsd", 7) == 0)
	    suggested_type = KERNEL_TYPE_FREEBSD;
	  else if (grub_memcmp (arg, "openbsd", 7) == 0)
	    /* XXX: For now, OpenBSD is identical to NetBSD, from GRUB's
	       point of view.  */
	    suggested_type = KERNEL_TYPE_NETBSD;
	  else if (grub_memcmp (arg, "linux", 5) == 0)
	    suggested_type = KERNEL_TYPE_LINUX;
	  else if (grub_memcmp (arg, "biglinux", 8) == 0)
	    suggested_type = KERNEL_TYPE_BIG_LINUX;
	  else if (grub_memcmp (arg, "multiboot", 9) == 0)
	    suggested_type = KERNEL_TYPE_MULTIBOOT;
	  else
	    {
	      errnum = ERR_BAD_ARGUMENT;
	      return 1;
	    }
	}
      /* If the `--no-mem-option' is specified, don't pass a Linux's mem
         option automatically. If the kernel is another type, this flag
         has no effect.  */
      else if (grub_memcmp (arg, "--no-mem-option", 15) == 0)
	load_flags |= KERNEL_LOAD_NO_MEM_OPTION;
      else
	break;

      /* Try the next.  */
      arg = skip_to (0, arg);
    }

  len = grub_strlen (arg);

  /* Reset MB_CMDLINE.  */
  mb_cmdline = (char *) MB_CMDLINE_BUF;
  if (len + 1 > MB_CMDLINE_BUFLEN)
    {
      errnum = ERR_WONT_FIT;
      return 1;
    }

  /* Copy the command-line to MB_CMDLINE.  */
  grub_memmove (mb_cmdline, arg, len + 1);
  kernel_type = load_image (arg, mb_cmdline, suggested_type, load_flags);
  if (kernel_type == KERNEL_TYPE_NONE)
    return 1;

  mb_cmdline += len + 1;
  return 0;
}

static struct builtin builtin_kernel = {
  "kernel",
  kernel_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "kernel [--no-mem-option] [--type=TYPE] FILE [ARG ...]",
  "Attempt to load the primary boot image from FILE. The rest of the"
    "line is passed verbatim as the \"kernel command line\".  Any modules"
    " must be reloaded after using this command. The option --type is used"
    " to suggest what type of kernel to be loaded. TYPE must be either of"
    " \"netbsd\", \"freebsd\", \"openbsd\", \"linux\", \"biglinux\" and"
    " \"multiboot\". The option --no-mem-option tells GRUB not to pass a"
    " Linux's mem option automatically."
#endif
};


/* makeactive */
static int
makeactive_func (char *arg, int flags)
{
  if (!make_saved_active ())
    return 1;

  return 0;
}

static struct builtin builtin_makeactive = {
  "makeactive",
  makeactive_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "makeactive",
  "Set the active partition on the root disk to GRUB's root device."
    " This command is limited to _primary_ PC partitions on a hard disk."
#endif
};


/* map */
/* Map FROM_DRIVE to TO_DRIVE.  */
static int
map_func (char *arg, int flags)
{
  char *to_drive;
  char *from_drive;
  unsigned long to, from;
  int i;

  to_drive = arg;
  from_drive = skip_to (0, arg);

  /* Get the drive number for TO_DRIVE.  */
  set_device (to_drive);
  if (errnum)
    return 1;
  to = current_drive;

  /* Get the drive number for FROM_DRIVE.  */
  set_device (from_drive);
  if (errnum)
    return 1;
  from = current_drive;

  /* Search for an empty slot in BIOS_DRIVE_MAP.  */
  for (i = 0; i < DRIVE_MAP_SIZE; i++)
    {
      /* Perhaps the user wants to override the map.  */
      if ((bios_drive_map[i] & 0xff) == from)
	break;

      if (!bios_drive_map[i])
	break;
    }

  if (i == DRIVE_MAP_SIZE)
    {
      errnum = ERR_WONT_FIT;
      return 1;
    }

  if (to == from)
    /* If TO is equal to FROM, delete the entry.  */
    grub_memmove ((char *) &bios_drive_map[i],
		  (char *) &bios_drive_map[i + 1],
		  sizeof (unsigned short) * (DRIVE_MAP_SIZE - i));
  else
    bios_drive_map[i] = from | (to << 8);

  return 0;
}

static struct builtin builtin_map = {
  "map",
  map_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "map TO_DRIVE FROM_DRIVE",
  "Map the drive FROM_DRIVE to the drive TO_DRIVE. This is necessary"
    " when you chain-load some operating systems, such as DOS, if such an"
    " OS resides at a non-first drive."
#endif
};


/* module */
static int
module_func (char *arg, int flags)
{
  int len = grub_strlen (arg);

  switch (kernel_type)
    {
    case KERNEL_TYPE_MULTIBOOT:
      if (mb_cmdline + len + 1 > (char *) MB_CMDLINE_BUF + MB_CMDLINE_BUFLEN)
	{
	  errnum = ERR_WONT_FIT;
	  return 1;
	}
      grub_memmove (mb_cmdline, arg, len + 1);
      if (!load_module (arg, mb_cmdline))
	return 1;
      mb_cmdline += len + 1;
      break;

    case KERNEL_TYPE_LINUX:
    case KERNEL_TYPE_BIG_LINUX:
      if (!load_initrd (arg))
	return 1;
      break;

    default:
      errnum = ERR_NEED_MB_KERNEL;
      return 1;
    }

  return 0;
}

static struct builtin builtin_module = {
  "module",
  module_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "module FILE [ARG ...]",
  "Load a boot module FILE for a Multiboot format boot image (no"
    " interpretation of the file contents is made, so users of this"
    " command must know what the kernel in question expects). The"
    " rest of the line is passed as the \"module command line\", like"
    " the `kernel' command."
#endif
};


/* modulenounzip */
static int
modulenounzip_func (char *arg, int flags)
{
  int ret;

#ifndef NO_DECOMPRESSION
  no_decompression = 1;
#endif

  ret = module_func (arg, flags);

#ifndef NO_DECOMPRESSION
  no_decompression = 0;
#endif

  return ret;
}

static struct builtin builtin_modulenounzip = {
  "modulenounzip",
  modulenounzip_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "modulenounzip FILE [ARG ...]",
  "The same as `module', except that automatic decompression is" " disabled."
#endif
};

static int
diskclean_func (char *arg, int flags)
{
  unsigned long sect, lastsect;
  int lasttime, time;
  int perc, oldperc;
  char *mbr = (char *) BUFFERADDR;

  /* Get the drive and the partition.  */
  if (!set_device (arg))
    return 1;

  /* The drive must be a hard disk.  */
  if (!(current_drive & 0x80))
    {
      errnum = ERR_BAD_ARGUMENT;
      return 1;
    }

  for (perc = 0; perc < 63 * 512; perc++)
    mbr[perc] = 0;
  oldperc = -1;
  lastsect = 0;
  lasttime = getrtsecs ();

  printf ("Disk : %s contains %d sectors.\n\n", arg, buf_geom.total_sectors);

  for (sect = 0; sect < buf_geom.total_sectors; sect += 63)
    {
      perc = (100 * sect) / buf_geom.total_sectors;
      if (lasttime != (time = getrtsecs ()))	// One sec has passed...
	{
	  if (oldperc == -1)
	    {
	      printf ("Cleaning : %d/100\r", perc);
	    }
	  else
	    {
	      printf
		("Cleaning : %d/100      (%d sectors/sec)               \r",
		 perc, sect - lastsect);
	      lasttime = time;
	      lastsect = sect;
	    }
	  oldperc = perc;
	}

      if (sect + 63 < buf_geom.total_sectors)
	{
	  buf_track = -1;
	  if (biosdisk
	      (BIOSDISK_WRITE, current_drive, &buf_geom, sect, 63,
	       SCRATCHSEG))
	    {
	      errnum = ERR_WRITE;
	      return 1;
	    }
	}
      else
	{
	  buf_track = -1;
	  if (biosdisk
	      (BIOSDISK_WRITE, current_drive, &buf_geom, sect,
	       buf_geom.total_sectors - sect, SCRATCHSEG))
	    {
	      errnum = ERR_WRITE;
	      return 1;
	    }
	}
    }

  return 0;
}

static struct builtin builtin_diskclean = {
  "diskclean",
  diskclean_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "diskclean DISK",
  "Erase (with zeros) the selected disk..."
#endif
};


/* partnew PART TYPE START LEN */
static int
partnew_func (char *arg, int flags)
{
  int new_type, new_start, new_len;
  int start_cl, start_ch, start_dh;
  int end_cl, end_ch, end_dh;
  int entry;
  char *mbr = (char *) SCRATCHADDR;

  /* Convert a LBA address to a CHS address in the INT 13 format.  */
  auto void lba_to_chs (int lba, int *cl, int *ch, int *dh);
  void lba_to_chs (int lba, int *cl, int *ch, int *dh)
  {
    int cylinder, head, sector;

    sector = lba % buf_geom.sectors + 1;
    head = (lba / buf_geom.sectors) % buf_geom.heads;
    cylinder = lba / (buf_geom.sectors * buf_geom.heads);

    if (cylinder >= buf_geom.cylinders)
      cylinder = buf_geom.cylinders - 1;

    *cl = sector | ((cylinder & 0x300) >> 2);
    *ch = cylinder & 0xFF;
    *dh = head;
  }

  /* Get the drive and the partition.  */
  if (!set_device (arg))
    return 1;

  /* The drive must be a hard disk.  */
  if (!(current_drive & 0x80))
    {
      errnum = ERR_BAD_ARGUMENT;
      return 1;
    }

  /* The partition must a primary partition.  */
  if ((current_partition >> 16) > 3 || (current_partition & 0xFFFF) != 0xFFFF)
    {
      errnum = ERR_BAD_ARGUMENT;
      return 1;
    }

  entry = current_partition >> 16;

  /* Get the new partition type.  */
  arg = skip_to (0, arg);
  if (!safe_parse_maxint (&arg, &new_type))
    return 1;

  /* The partition type is unsigned char.  */
  if (new_type > 0xFF)
    {
      errnum = ERR_BAD_ARGUMENT;
      return 1;
    }

  /* Get the new partition start.  */
  arg = skip_to (0, arg);

  if (grub_memcmp (arg, "-first", 6) == 0)
    new_start = buf_geom.sectors;
  else if (grub_memcmp (arg, "-next", 5) == 0)
    new_start =
      buf_geom.sectors *
      (((PC_SLICE_START (mbr, (entry - 1))) +
	(PC_SLICE_LENGTH (mbr, (entry - 1))) + (buf_geom.sectors -
						1)) / buf_geom.sectors);
  else if (!safe_parse_maxint (&arg, &new_start))
    return 1;

  //if (! safe_parse_maxint (&arg, &new_start))
  //  return 1;

  /* Get the new partition length.  */
  arg = skip_to (0, arg);
  if (!safe_parse_maxint (&arg, &new_len))
    return 1;

  /* Read the MBR.  */
  if (!rawread (current_drive, 0, 0, SECTOR_SIZE, mbr))
    return 1;

  /* Check if the new partition will fit in the disk.  */
  if (new_start + new_len > buf_geom.total_sectors)
    {
      errnum = ERR_GEOM;
      return 1;
    }

  /* Store the partition information in the MBR.  */
  lba_to_chs (new_start, &start_cl, &start_ch, &start_dh);
  lba_to_chs (new_start + new_len - 1, &end_cl, &end_ch, &end_dh);

  //grub_memmove(mbr,mbr_code,446);

  PC_SLICE_FLAG (mbr, entry) = 0x80;
  PC_SLICE_HEAD (mbr, entry) = start_dh;
  PC_SLICE_SEC (mbr, entry) = start_cl;
  PC_SLICE_CYL (mbr, entry) = start_ch;
  PC_SLICE_TYPE (mbr, entry) = new_type;
  PC_SLICE_EHEAD (mbr, entry) = end_dh;
  PC_SLICE_ESEC (mbr, entry) = end_cl;
  PC_SLICE_ECYL (mbr, entry) = end_ch;
  PC_SLICE_START (mbr, entry) = new_start;
  PC_SLICE_LENGTH (mbr, entry) = new_len;

  /* Make sure that the MBR has a valid signature.  */
  PC_MBR_SIG (mbr) = PC_MBR_SIGNATURE;

  /* Write back the MBR to the disk.  */
  buf_track = -1;
  if (biosdisk (BIOSDISK_WRITE, current_drive, &buf_geom, 0, 1, SCRATCHSEG))
    {
      errnum = ERR_WRITE;
      return 1;
    }
  return 0;
}

static struct builtin builtin_partnew = {
  "partnew",
  partnew_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "partnew PART TYPE START LEN",
  "Create a primary partition at the starting address START with the"
    " length LEN, with the type TYPE. START and LEN are in sector units."
#endif
};


static int
mbr_func (char *arg, int flags)
{
  char *mbr = (char *) SCRATCHADDR;
  int save_drive;

  /* Read the MBR.  */
  save_drive = current_drive;
  if (!rawread (current_drive, 0, 0, SECTOR_SIZE, mbr))
    return 1;

  // grub_memmove(mbr,mbr_code,446);
  //
  grub_open (arg);
  grub_printf ("Got : %d bytes\n", grub_read (mbr, 512));
  grub_close ();

  PC_MBR_SIG (mbr) = PC_MBR_SIGNATURE;

  /* Write back the MBR to the disk.  */
  current_drive = save_drive;
  buf_track = -1;
  if (biosdisk (BIOSDISK_WRITE, current_drive, &buf_geom, 0, 1, SCRATCHSEG))
    {
      errnum = ERR_WRITE;
      return 1;
    }

  return 0;
}

static struct builtin builtin_mbr = {
  "mbr",
  mbr_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "mbr",
  "Create a new MBR."
#endif
};

/* parttype PART TYPE */
static int
parttype_func (char *arg, int flags)
{
  int new_type;
  unsigned long part = 0xFFFFFF;
  unsigned long start, len, offset, ext_offset;
  int entry, type;
  char *mbr = (char *) SCRATCHADDR;

  /* Get the drive and the partition.  */
  if (!set_device (arg))
    return 1;

  /* The drive must be a hard disk.  */
  if (!(current_drive & 0x80))
    {
      errnum = ERR_BAD_ARGUMENT;
      return 1;
    }

  /* The partition must be a PC slice.  */
  if ((current_partition >> 16) == 0xFF
      || (current_partition & 0xFFFF) != 0xFFFF)
    {
      errnum = ERR_BAD_ARGUMENT;
      return 1;
    }

  /* Get the new partition type.  */
  arg = skip_to (0, arg);
  if (!safe_parse_maxint (&arg, &new_type))
    return 1;

  /* The partition type is unsigned char.  */
  if (new_type > 0xFF)
    {
      errnum = ERR_BAD_ARGUMENT;
      return 1;
    }

  /* Look for the partition.  */
  while (next_partition (current_drive, 0xFFFFFF, &part, &type,
			 &start, &len, &offset, &entry, &ext_offset, mbr))
    {
      if (part == current_partition)
	{
	  /* Found.  */

	  /* Set the type to NEW_TYPE.  */
	  PC_SLICE_TYPE (mbr, entry) = new_type;

	  /* Write back the MBR to the disk.  */
	  buf_track = -1;
	  if (biosdisk (BIOSDISK_WRITE, current_drive, &buf_geom,
			offset, 1, SCRATCHSEG))
	    {
	      errnum = ERR_WRITE;
	      return 1;
	    }

	  /* Succeed.  */
	  return 0;
	}
    }

  /* The partition was not found.  ERRNUM was set by next_partition.  */
  return 1;
}

static struct builtin builtin_parttype = {
  "parttype",
  parttype_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "parttype PART TYPE",
  "Change the type of the partition PART to TYPE."
#endif
};


/* pause */
static int
pause_func (char *arg, int flags)
{
  int i;
  /* If ESC is returned, then abort this entry.  */
  if (ASCII_CHAR (i = getkey ()) == 27)
    return 1;

  return 0;
}

static struct builtin builtin_pause = {
  "pause",
  pause_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "pause [MESSAGE ...]",
  "Print MESSAGE, then wait until a key is pressed."
#endif
};

extern unsigned char X86;

static int
pciscan_func (char *arg, int flags)
{
  eth_pci_init (NULL);
  return 0;
}


static struct builtin builtin_pciscan = {
  "pciscan",
  pciscan_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "pciscan",
  "Scan for all PCI devices on the bus."
#endif
};

#ifdef SUPPORT_NETBOOT
/* rarp */
static int
rarp_func (char *arg, int flags)
{
  if (!rarp ())
    {
      if (errnum == ERR_NONE)
	errnum = ERR_DEV_VALUES;

      return 1;
    }

  /* Notify the configuration.  */
  print_network_configuration ();
  return 0;
}

static struct builtin builtin_rarp = {
  "rarp",
  rarp_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "rarp",
  "Initialize a network device via RARP."
#endif
};
#endif /* SUPPORT_NETBOOT */


static int
read_func (char *arg, int flags)
{
  int addr;

  if (!safe_parse_maxint (&arg, &addr))
    return 1;

  grub_printf ("Address 0x%x: Value 0x%x\n",
	       addr, *((unsigned *) RAW_ADDR (addr)));
  return 0;
}

static struct builtin builtin_read = {
  "read",
  read_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "read ADDR",
  "Read a 32-bit value from memory at address ADDR and"
    " display it in hex format."
#endif
};


/* reboot */
static int
reboot_func (char *arg, int flags)
{
  pxe_unload ();
  grub_reboot ();

  /* Never reach here.  */
  return 1;
}

static struct builtin builtin_reboot = {
  "reboot",
  reboot_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "reboot",
  "Reboot your system."
#endif
};
 
static int terminal_func (char *arg, int flags);
 
#ifdef SUPPORT_GRAPHICS

static int splashimage_func(char *arg, int flags) {
     char splashimage[64];
     int i;
     
     /* filename can only be 64 characters due to our buffer size */
     if (strlen(arg) > 63)
 	return 1;
     if (flags == BUILTIN_CMDLINE) {
       if (!grub_open(arg)) {
	 return 0;
       }
 	grub_close();
     }
#ifdef SUPPORT_NETBOOT
     if (new_tftpdir(arg+4) <0) {
       return 0;
     }
#endif

     strcpy(splashimage, arg);

     /* get rid of TERM_NEED_INIT from the graphics terminal. */
     for (i = 0; term_table[i].name; i++) {
 	if (grub_strcmp (term_table[i].name, "graphics") == 0) {
 	    term_table[i].flags &= ~TERM_NEED_INIT;
 	    break;
 	}
     }
     
     graphics_set_splash(splashimage);
 
     if (flags == BUILTIN_CMDLINE && graphics_inited) {
 	graphics_end();
 	graphics_init();
 	graphics_cls();
     }
 
     /* FIXME: should we be explicitly switching the terminal as a 
      * side effect here? */
     terminal_func("graphics", flags);

     return 0;
 }
 
 static struct builtin builtin_splashimage =
 {
   "splashimage",
   splashimage_func,
   BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
   "splashimage FILE",
   "Load FILE as the background image when in graphics mode."
 };
 
/* foreground */
 static int
 foreground_func(char *arg, int flags)
{
     if (grub_strlen(arg) == 6) {
 	int r = ((hex(arg[0]) << 4) | hex(arg[1])) >> 2;
 	int g = ((hex(arg[2]) << 4) | hex(arg[3])) >> 2;
 	int b = ((hex(arg[4]) << 4) | hex(arg[5])) >> 2;
 
 	foreground = (r << 16) | (g << 8) | b;
 	if (graphics_inited)
 	    graphics_set_palette(15, r, g, b);
 
 	return (0);
     }
 
     return (1);
 }
 
 static struct builtin builtin_foreground =
 {
   "foreground",
   foreground_func,
   BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
   "foreground RRGGBB",
   "Sets the foreground color when in graphics mode."
   "RR is red, GG is green, and BB blue. Numbers must be in hexadecimal."
 };
 
 /* background */
 static int
 background_func(char *arg, int flags)
 {
     if (grub_strlen(arg) == 6) {
 	int r = ((hex(arg[0]) << 4) | hex(arg[1])) >> 2;
 	int g = ((hex(arg[2]) << 4) | hex(arg[3])) >> 2;
 	int b = ((hex(arg[4]) << 4) | hex(arg[5])) >> 2;
 
 	background = (r << 16) | (g << 8) | b;
 	if (graphics_inited)
 	    graphics_set_palette(0, r, g, b);
 	return (0);
     }
 
     return (1);
 }
 
 static struct builtin builtin_background =
 {
   "background",
   background_func,
   BUILTIN_CMDLINE | BUILTIN_MENU | BUILTIN_HELP_LIST,
   "background RRGGBB",
   "Sets the background color when in graphics mode."
   "RR is red, GG is green, and BB blue. Numbers must be in hexadecimal."
 };
 
#endif /* SUPPORT_GRAPHICS */
 

 /* clear */
 static int 
 clear_func() 
 {
   graphics_cls();
 
   return 0;
 }
 
 static struct builtin builtin_clear =
 {
   "clear",
   clear_func,
   BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
   "clear",
   "Clear the screen"
 };
 


/* Print the root device information.  */
static void
print_root_device (void)
{
  if (saved_drive == NETWORK_DRIVE)
    {
      /* Network drive.  */
      grub_printf (" (nd):");
    }
  else if (saved_drive & 0x80)
    {
      /* Hard disk drive.  */
      grub_printf (" (hd%d", saved_drive - 0x80);

      if ((saved_partition & 0xFF0000) != 0xFF0000)
	grub_printf (",%d", saved_partition >> 16);

      if ((saved_partition & 0x00FF00) != 0x00FF00)
	grub_printf (",%c", ((saved_partition >> 8) & 0xFF) + 'a');

      grub_printf ("):");
    }
  else
    {
      /* Floppy disk drive.  */
      grub_printf (" (fd%d):", saved_drive);
    }

  /* Print the filesystem information.  */
  current_partition = saved_partition;
  current_drive = saved_drive;
  print_fsys_type ();
}

static int
root_func (char *arg, int flags)
{
  int hdbias = 0;
  char *biasptr;
  char *next;

  /* If ARG is empty, just print the current root device.  */
  if (!*arg)
    {
      print_root_device ();
      return 0;
    }

  /* Call set_device to get the drive and the partition in ARG.  */
  next = set_device (arg);
  if (!next)
    return 1;

  /* Ignore ERR_FSYS_MOUNT.  */
  if (!open_device () && errnum != ERR_FSYS_MOUNT)
    return 1;

  /* Clear ERRNUM.  */
  errnum = 0;
  saved_partition = current_partition;
  saved_drive = current_drive;

  /* BSD and chainloading evil hacks !!  */
  biasptr = skip_to (0, next);
  safe_parse_maxint (&biasptr, &hdbias);
  errnum = 0;
  bootdev = set_bootdev (hdbias);

  /* Print the type of the filesystem.  */
  print_fsys_type ();

  return 0;
}

static struct builtin builtin_root = {
  "root",
  root_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "root [DEVICE [HDBIAS]]",
  "Set the current \"root device\" to the device DEVICE, then"
    " attempt to mount it to get the partition size (for passing the"
    " partition descriptor in `ES:ESI', used by some chain-loaded"
    " bootloaders), the BSD drive-type (for booting BSD kernels using"
    " their native boot format), and correctly determine "
    " the PC partition where a BSD sub-partition is located. The"
    " optional HDBIAS parameter is a number to tell a BSD kernel"
    " how many BIOS drive numbers are on controllers before the current"
    " one. For example, if there is an IDE disk and a SCSI disk, and your"
    " FreeBSD root partition is on the SCSI disk, then use a `1' for HDBIAS."
#endif
};


/* rootnoverify */
static int
rootnoverify_func (char *arg, int flags)
{
  /* If ARG is empty, just print the current root device.  */
  if (!*arg)
    {
      print_root_device ();
      return 0;
    }

  if (!set_device (arg))
    return 1;

  saved_partition = current_partition;
  saved_drive = current_drive;
  current_drive = -1;
  return 0;
}

static struct builtin builtin_rootnoverify = {
  "rootnoverify",
  rootnoverify_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "rootnoverify [DEVICE [HDBIAS]]",
  "Similar to `root', but don't attempt to mount the partition. This"
    " is useful for when an OS is outside of the area of the disk that"
    " GRUB can read, but setting the correct root device is still"
    " desired. Note that the items mentioned in `root' which"
    " derived from attempting the mount will NOT work correctly."
#endif
};

/********************************************************************/

#ifdef OLDFUNC
/* testload */
static int
testload_func (char *arg, int flags)
{
  int i;

  kernel_type = KERNEL_TYPE_NONE;

  if (!grub_open (arg))
    return 1;

  disk_read_hook = disk_read_print_func;

  /* Perform filesystem test on the specified file.  */
  /* Read whole file first. */
  grub_printf ("Whole file: ");

  grub_read ((char *) RAW_ADDR (0x100000), -1);

  /* Now compare two sections of the file read differently.  */

  for (i = 0; i < 0x10ac0; i++)
    {
      *((unsigned char *) RAW_ADDR (0x200000 + i)) = 0;
      *((unsigned char *) RAW_ADDR (0x300000 + i)) = 1;
    }

  /* First partial read.  */
  grub_printf ("\nPartial read 1: ");

  grub_seek (0);
  grub_read ((char *) RAW_ADDR (0x200000), 0x7);
  grub_read ((char *) RAW_ADDR (0x200007), 0x100);
  grub_read ((char *) RAW_ADDR (0x200107), 0x10);
  grub_read ((char *) RAW_ADDR (0x200117), 0x999);
  grub_read ((char *) RAW_ADDR (0x200ab0), 0x10);
  grub_read ((char *) RAW_ADDR (0x200ac0), 0x10000);

  /* Second partial read.  */
  grub_printf ("\nPartial read 2: ");

  grub_seek (0);
  grub_read ((char *) RAW_ADDR (0x300000), 0x10000);
  grub_read ((char *) RAW_ADDR (0x310000), 0x10);
  grub_read ((char *) RAW_ADDR (0x310010), 0x7);
  grub_read ((char *) RAW_ADDR (0x310017), 0x10);
  grub_read ((char *) RAW_ADDR (0x310027), 0x999);
  grub_read ((char *) RAW_ADDR (0x3109c0), 0x100);

  grub_printf ("\nHeader1 = 0x%x, next = 0x%x, next = 0x%x, next = 0x%x\n",
	       *((int *) RAW_ADDR (0x200000)),
	       *((int *) RAW_ADDR (0x200004)),
	       *((int *) RAW_ADDR (0x200008)),
	       *((int *) RAW_ADDR (0x20000c)));

  grub_printf ("Header2 = 0x%x, next = 0x%x, next = 0x%x, next = 0x%x\n",
	       *((int *) RAW_ADDR (0x300000)),
	       *((int *) RAW_ADDR (0x300004)),
	       *((int *) RAW_ADDR (0x300008)),
	       *((int *) RAW_ADDR (0x30000c)));

  for (i = 0; i < 0x10ac0; i++)
    if (*((unsigned char *) RAW_ADDR (0x200000 + i))
	!= *((unsigned char *) RAW_ADDR (0x300000 + i)))
      break;

  grub_printf ("Max is 0x10ac0: i=0x%x, filepos=0x%x\n", i, filepos);
  disk_read_hook = 0;
  grub_close ();
  return 0;
}

static struct builtin builtin_testload = {
  "testload",
  testload_func,
  BUILTIN_CMDLINE,
  "testload FILE",
  "Read the entire contents of FILE in several different ways and"
    " compares them, to test the filesystem code. The output is somewhat"
    " cryptic, but if no errors are reported and the final `i=X,"
    " filepos=Y' reading has X and Y equal, then it is definitely"
    " consistent, and very likely works correctly subject to a"
    " consistent offset error. If this test succeeds, then a good next"
    " step is to try loading a kernel."
};
#endif

#ifdef SUPPORT_NETBOOT
/* tftpserver */
static int
tftpserver_func (char *arg, int flags)
{
  if (!*arg || !arp_server_override (arg))
    {
      errnum = ERR_BAD_ARGUMENT;
      return 1;
    }

  print_network_configuration ();
  return 0;
}

static struct builtin builtin_tftpserver = {
  "tftpserver",
  tftpserver_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "tftpserver IPADDR",
  "Override the TFTP server address."
#endif
};
#endif /* SUPPORT_NETBOOT */


/* timeout */
static int
timeout_func (char *arg, int flags)
{
  if (!safe_parse_maxint (&arg, &grub_timeout))
    return 1;

  return 0;
}

static struct builtin builtin_timeout = {
  "timeout",
  timeout_func,
  BUILTIN_MENU,
#if 0
  "timeout SEC",
  "Set a timeout, in SEC seconds, before automatically booting the"
    " default entry (normally the first entry defined)."
#endif
};

/* title */
static int
title_func (char *arg, int flags)
{
  /* This function is not actually used at least currently.  */
  return 0;
}

static struct builtin builtin_title = {
  "title",
  title_func,
  BUILTIN_TITLE,
#if 0
  "title [NAME ...]",
  "Start a new boot entry, and set its name to the contents of the"
    " rest of the line, starting with the first non-space character."
#endif
};

static int
desc_func (char *arg, int flags)
{
  return 0;
}

static struct builtin builtin_desc = {
  "desc",
  desc_func,
  BUILTIN_DESC,
};


/* unhide */
static int
unhide_func (char *arg, int flags)
{
  if (!set_device (arg))
    return 1;

  if (!set_partition_hidden_flag (0))
    return 1;

  return 0;
}

static struct builtin builtin_unhide = {
  "unhide",
  unhide_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "unhide PARTITION",
  "Unhide PARTITION by clearing the \"hidden\" bit in its"
    " partition type code."
#endif
};


/* uppermem */
static int
uppermem_func (char *arg, int flags)
{
  if (!safe_parse_maxint (&arg, (int *) &mbi.mem_upper))
    return 1;

  mbi.flags &= ~MB_INFO_MEM_MAP;
  return 0;
}

static struct builtin builtin_uppermem = {
  "uppermem",
  uppermem_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "uppermem KBYTES",
  "Force GRUB to assume that only KBYTES kilobytes of upper memory are"
    " installed.  Any system address range maps are discarded."
#endif
};


#if defined(SUPPORT_SERIAL) || defined(SUPPORT_HERCULES) || defined(SUPPORT_GRAPHICS)
/* terminal */
static int
terminal_func (char *arg, int flags)
{
  /* The index of the default terminal in TERM_TABLE.  */
  int default_term = -1;
  struct term_entry *prev_term = current_term;
  int to = -1;
  int lines = 0;
  int no_message = 0;
  unsigned long term_flags = 0;
  /* XXX: Assume less than 32 terminals.  */
  unsigned long term_bitmap = 0;

  /* Get GNU-style long options.  */
  while (1)
    {
      if (grub_memcmp (arg, "--dumb", sizeof ("--dumb") - 1) == 0)
	term_flags |= TERM_DUMB;
      else if (grub_memcmp (arg, "--no-echo", sizeof ("--no-echo") - 1) == 0)
	/* ``--no-echo'' implies ``--no-edit''.  */
	term_flags |= (TERM_NO_ECHO | TERM_NO_EDIT);
      else if (grub_memcmp (arg, "--no-edit", sizeof ("--no-edit") - 1) == 0)
	term_flags |= TERM_NO_EDIT;
      else if (grub_memcmp (arg, "--timeout=", sizeof ("--timeout=") - 1) == 0)
	{
	  char *val = arg + sizeof ("--timeout=") - 1;
	  
	  if (! safe_parse_maxint (&val, &to))
	    return 1;
	}
      else if (grub_memcmp (arg, "--lines=", sizeof ("--lines=") - 1) == 0)
	{
	  char *val = arg + sizeof ("--lines=") - 1;

	  if (! safe_parse_maxint (&val, &lines))
	    return 1;

	  /* Probably less than four is meaningless....  */
	  if (lines < 4)
	    {
	      errnum = ERR_BAD_ARGUMENT;
	      return 1;
	    }
	}
      else if (grub_memcmp (arg, "--silent", sizeof ("--silent") - 1) == 0)
	no_message = 1;
      else
	break;

      arg = skip_to (0, arg);
    }
  
  /* If no argument is specified, show current setting.  */
  if (! *arg)
    {
      grub_printf ("%s%s%s%s\n",
		   current_term->name,
		   current_term->flags & TERM_DUMB ? " (dumb)" : "",
		   current_term->flags & TERM_NO_EDIT ? " (no edit)" : "",
		   current_term->flags & TERM_NO_ECHO ? " (no echo)" : "");
      return 0;
    }

  while (*arg)
    {
      int i;
      char *next = skip_to (0, arg);
      
      nul_terminate (arg);

      for (i = 0; term_table[i].name; i++)
	{
	  if (grub_strcmp (arg, term_table[i].name) == 0)
	    {
	      if (term_table[i].flags & TERM_NEED_INIT)
		{
		  errnum = ERR_DEV_NEED_INIT;
		  return 1;
		}
	      
	      if (default_term < 0)
		default_term = i;

	      term_bitmap |= (1 << i);
	      break;
	    }
	}

      if (! term_table[i].name)
	{
	  errnum = ERR_BAD_ARGUMENT;
	  return 1;
	}

      arg = next;
    }

  /* If multiple terminals are specified, wait until the user pushes any
     key on one of the terminals.  */
  if (term_bitmap & ~(1 << default_term))
    {
      int time1, time2 = -1;

      /* XXX: Disable the pager.  */
      count_lines = -1;
      
      /* Get current time.  */
      while ((time1 = getrtsecs ()) == 0xFF)
	;

      /* Wait for a key input.  */
      while (to)
	{
	  int i;

	  for (i = 0; term_table[i].name; i++)
	    {
	      if (term_bitmap & (1 << i))
		{
		  if (term_table[i].checkkey () >= 0)
		    {
		      (void) term_table[i].getkey ();
		      default_term = i;
		      
		      goto end;
		    }
		}
	    }
	  
	  /* Prompt the user, once per sec.  */
	  if ((time1 = getrtsecs ()) != time2 && time1 != 0xFF)
	    {
	      if (! no_message)
		{
		  /* Need to set CURRENT_TERM to each of selected
		     terminals.  */
		  for (i = 0; term_table[i].name; i++)
		    if (term_bitmap & (1 << i))
		      {
			current_term = term_table + i;
			grub_printf ("\rPress any key to continue.\n");
		      }
		  
		  /* Restore CURRENT_TERM.  */
		  current_term = prev_term;
		}
	      
	      time2 = time1;
	      if (to > 0)
		to--;
	    }
	}
    }

 end:
  current_term = term_table + default_term;
  current_term->flags = term_flags;

  if (lines)
    max_lines = lines;
  else
    max_lines = current_term->max_lines;

  /* If the interface is currently the command-line,
     restart it to repaint the screen.  */
  if ((current_term != prev_term) && (flags & BUILTIN_CMDLINE)){
    if (prev_term->shutdown)
      prev_term->shutdown();
    if (current_term->startup)
      current_term->startup();
    grub_longjmp (restart_cmdline_env, 0);
  }
  
  return 0;
}

static struct builtin builtin_terminal =
{
  "terminal",
  terminal_func,
  BUILTIN_MENU | BUILTIN_CMDLINE | BUILTIN_HELP_LIST,
  "terminal [--dumb] [--no-echo] [--no-edit] [--timeout=SECS] [--lines=LINES] [--silent] [console] [serial] [hercules] [graphics]",
  "Select a terminal. When multiple terminals are specified, wait until"
  " you push any key to continue. If both console and serial are specified,"
  " the terminal to which you input a key first will be selected. If no"
  " argument is specified, print current setting. The option --dumb"
  " specifies that your terminal is dumb, otherwise, vt100-compatibility"
  " is assumed. If you specify --no-echo, input characters won't be echoed."
  " If you specify --no-edit, the BASH-like editing feature will be disabled."
  " If --timeout is present, this command will wait at most for SECS"
  " seconds. The option --lines specifies the maximum number of lines."
  " The option --silent is used to suppress messages."
};
#endif /* SUPPORT_SERIAL || SUPPORT_HERCULES || SUPPORT_GRAPHICS */


/* The table of builtin commands. Sorted in dictionary order.  */
struct builtin *builtin_table[] = {
#ifdef SUPPORT_GRAPHICS
  &builtin_background,
#endif
#ifdef OLDFUNC
  &builtin_blocklist,
#endif
  &builtin_boot,
#ifdef SUPPORT_NETBOOT_NO
  &builtin_bootp,
#endif				/* SUPPORT_NETBOOT */
  &builtin_cat,
  &builtin_chainloader,
  &builtin_clear,
#ifdef OLDFUNC
  &builtin_cmp,
#endif
  &builtin_color,
  &builtin_configfile,
  &builtin_debug,
  &builtin_default,
  &builtin_desc,
#ifdef SUPPORT_NETBOOT_NO
  &builtin_dhcp,
#endif				/* SUPPORT_NETBOOT */
  &builtin_diskclean,
  &builtin_displaymem,
//  &builtin_embed,
  &builtin_fallback,
#ifdef OLDFUNC
  &builtin_find,
#endif
#ifdef SUPPORT_GRAPHICS
  &builtin_foreground,
#endif
  &builtin_fstest,
  &builtin_geometry,
  &builtin_halt,
#ifdef HELP_ON
  &builtin_help,
#endif
  &builtin_hiddenmenu,
  &builtin_hide,
  &builtin_identify,
  &builtin_identifyauto,
  &builtin_inc,
  &builtin_initrd,
//  &builtin_install,
  &builtin_kbdfr,
  &builtin_kernel,
  &builtin_makeactive,
  &builtin_map,
  &builtin_mbr,
  &builtin_module,
  &builtin_modulenounzip,
  &builtin_nosecurity,
  &builtin_partcopy,
  &builtin_partnew,
  &builtin_parttype,
  &builtin_pause,
  &builtin_pciscan,
  &builtin_ptabs,
#ifdef SUPPORT_NETBOOT
  &builtin_rarp,
#endif				/* SUPPORT_NETBOOT */
  &builtin_read,
  &builtin_reboot,
  &builtin_root,
  &builtin_rootnoverify,
//  &builtin_savedefault,
#ifdef SUPPORT_SERIAL
  &builtin_serial,
#endif				/* SUPPORT_SERIAL */
//  &builtin_setkey,
//  &builtin_setup,
  &builtin_setdefault,
#ifdef SUPPORT_GRAPHICS
  &builtin_splashimage,
#endif /* SUPPORT_GRAPHICS */
#if 1
  &builtin_terminal,
#endif				/* SUPPORT_SERIAL */
#ifdef OLDFUNC
  &builtin_testload,
#endif
#ifdef SUPPORT_NETBOOT
  &builtin_tftpserver,
#endif				/* SUPPORT_NETBOOT */
  &builtin_timeout,
  &builtin_title,
  &builtin_unhide,
  &builtin_uppermem,
  0
};


