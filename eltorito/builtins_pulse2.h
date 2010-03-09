/*
 * $Id$
 */


#define HELP_ON 1

int partcopy_func (char *arg, int flags);
int ptabs_func (char *arg, int flags);

/* variables */

/* grub builtins */

static struct builtin builtin_partcopy = {
  "partcopy",
  partcopy_func,
  BUILTIN_CMDLINE | BUILTIN_MENU,
#ifdef HELP_ON
  "partcopy START PREFIXNAME [TYPE]",
  "Create a primary partition at the starting address START with the"
    " compressed files beginning with PREFIXNAME. Update partition table "
    "with the partition TYPE type."
#endif
};

static struct builtin builtin_ptabs = {
  "ptabs",
  ptabs_func,
  BUILTIN_CMDLINE,
#ifdef HELP_ON
  "ptabs DISK FILE",
  "Copy uncompressed sectors from FILE (LBA,DATA) to disk DISK."
#endif
};

