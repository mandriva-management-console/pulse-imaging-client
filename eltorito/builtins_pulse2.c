/*
 *  $Id$
 */
/*
 *  GRUB LBS functions
 *  Copyright (C) 2002  Free & Alter Soft
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

#include <shared.h>
//#include <filesys.h>
#include <term.h>

#include "zlib/zlib.h"
#include "shared-pulse2.h"

#define INBUFF 8192
#define OUTBUFF 24064
unsigned char *zcalloc (long toto, int nb, int size);
void zcfree (long toto, unsigned char *zone);

/* ugly global vars */
int fat, ntfs, files, new_len, new_type;

/* auto fill mode ?! */
int auto_fill = 1;

/* total/current bytes of data to decompress */
unsigned int total_kbytes = 0;
unsigned int current_bytes = 0;

/* default = cannot access to grub's command line */
int nosecurity = 0;

/* current image name being restored */
char imgname[32];

/* for decompression */
int bitindex = -1, files = 1;
int mask[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
unsigned long cursect;
int dest_drive = 0;

/* proto */
int new_get (char *file, int sect, int *endsect, int table);

extern unsigned long cdrom_drive;

//#define TFTPBLOCK 1432
#define TFTPBLOCK 1024
#define BLKSIZE "1024"
//#define TFTPBLOCK 2000
//#define BLKSIZE "2000"

typedef struct diskbuffer_
{
//              unsigned char data[OUTBUFF];    -> Direct to BUFFERADDR
  int full;
  int size;
} diskbuffer;

struct tftpbuffer_
{
  unsigned char udp[4];
  unsigned char data[TFTPBLOCK + 16];
  int length;			// 0 if available
  struct tftpbuffer_ *next;
};

typedef struct tftpbuffer_ tftpbuffer;

#define NB_BUF 10

int
get_empty (tftpbuffer * buff, int *i)
{
  int j;

  for (j = 0; j < NB_BUF; j++) 
    {
      if (buff[j].length == 0)
	{
	  *i = j;
	  return 1;
	}
    }
  return 0;
}

int
tot_lg (tftpbuffer * buff)
{
  int i = 0;

  do
    {
      i += buff->length;
      if (buff->next)
	buff = buff->next;
      else
	return i;
    }
  while (1 == 1);
}

unsigned long
parse_maxint (unsigned char *str)
{
  unsigned long i = 0;

  while ((*str >= '0') && (*str <= '9'))
    {
      i = (i * 10) + (*str - '0');
      str++;
    }
  return i;
}


int
val (unsigned char *string, unsigned char *ptr)
{
  char *cp=NULL;
  int i;

  while (*ptr)
    {
      cp = ptr;
      i = 0;
      while (cp[i] == string[i])
	{
	  i++;
	}
      if ((string[i] == 0) && (cp[i] == '='))
	{
	  cp = cp + i + 1;
	  break;
	}
      cp = NULL;
      ptr++;
    }

  if (cp)
    {
      i = parse_maxint (cp);
      return i;
    }

  return -1;
}

int
exist (unsigned char *string, unsigned char *ptr)
{
  char *cp;
  int i;

  while (*ptr)
    {
      cp = ptr;
      i = 0;
      while (cp[i] == string[i])
	{
	  i++;
	}
      if ((string[i] == 0) && (cp[i] == '='))
	{
	  return 1;
	}
      ptr++;
    }

  return 0;
}



/* ptabs */
int
ptabs_func (char *arg, int flags)
{
  char buf[516];
  char *secbuf = (char *) SCRATCHADDR;
  unsigned long sect;
  int save_drive, i, nb = 0;

  if (!set_device (arg))
    return 1;

  save_drive = current_drive;

  arg = skip_to (0, arg);

  /* correct the path */
  grub_sprintf(buf, "(cd)%s", grub_strstr(arg, "/"));

  if (!grub_open (buf))
    return 1;

  /*  */
  if (current_term->setcolorstate)
                  current_term->setcolorstate (COLOR_STATE_HIGHLIGHT);
  grub_printf("\n**** Warning ! ****************************************************************\n\n"
	      "Your Hard Disk will be ERASED !\n"
	      "Continue (Y/N) ?\n\n"
	      );
  if (current_term->setcolorstate)
                  current_term->setcolorstate (COLOR_STATE_NORMAL);

  i = ASCII_CHAR (getkey());
  if (i != 'y' && i != 'Y') {
    errnum = ERR_BAD_ARGUMENT;
    return 1;
  }

  while (grub_read (buf, 516))
    {
      sect = *(unsigned long *) buf;

//    printf("Writing sect : %d\n",sect);

      for (i = 0; i < 512; i++)
	secbuf[i] = buf[i + 4];

      buf_track = -1;
      if (biosdisk
	  (BIOSDISK_WRITE, save_drive, &buf_geom, sect, 1, SCRATCHSEG))
	{			
	  errnum = ERR_WRITE;
	  return 1;
	}
      nb++;
    }

  printf ("Wrote %d sectors\n", nb);

  grub_close ();

  current_bytes = 0;

  return 0;
}


/* partcopy START NAME_PREFIX [type] */
int
partcopy_func (char *arg, int flags)
{
  int new_start;
  /* int start_cl, start_ch, start_dh;
     int end_cl, end_ch, end_dh; */
  int entry;
  char *mbr = (char *) SCRATCHADDR, *path;
  static int sendlog = 1;

  int i, j, curr;
  unsigned long save_drive;
  char name[64];
  int old_perc;
  int l1 = 0, l2 = '0', l3 = '0';


  /* Convert a LBA address to a CHS address in the INT 13 format.  */
  auto void lba_to_chs (int lba, int *cl, int *ch, int *dh);
  static void lba_to_chs (int lba, int *cl, int *ch, int *dh)
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
  dest_drive = current_drive;

  /* check if we need to initialise */
  zcinit();

  /* The partition must a primary partition.  
     if ((current_partition >> 16) > 3
     || (current_partition & 0xFFFF) != 0xFFFF)
     {
     errnum = ERR_BAD_ARGUMENT;
     return 1;
     }
   */

  entry = current_partition >> 16;

  save_drive = current_drive;

  /* update the disk geometry */
  get_diskinfo (current_drive, &buf_geom);
  if (biosdisk (BIOSDISK_READ, current_drive, &buf_geom, 0, 1, SCRATCHSEG))
    {
      grub_printf ("Error reading the 1st sector on disk %d\n",
		   current_drive);
      //return 1;
    }

#if 0
  grub_printf ("GEO : %d, %d, %d, %d E: %d DR: %d err=%d\n",
	       buf_geom.total_sectors, buf_geom.cylinders, buf_geom.heads,
	       buf_geom.sectors, entry, current_drive, 0);
#endif

  /* Get the new partition start.  */
  arg = skip_to (0, arg);

  if (entry <= 3)
    {
      // old code ? remove it ?
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
    }
  else if (!safe_parse_maxint (&arg, &new_start))
    return 1;


  /* get file path */
  path = skip_to (0, arg);
  grub_memmove(path, "(cd)", 4); /* force cdrom destination */

  /* find the image name */
  for (i = grub_strlen(path); i > 0; i--)
    {
      if (path[i] == '/') break;
    }
  for (j = i-1; j > 0; j--)
    {
      if (path[j] == '/') break;      
    }
  grub_memmove(imgname, &path[j+1], i-j);
  imgname[i-j-1] = '\0';

  /* try to get image size */
  if (total_kbytes == 0) {
    int i, olddrive = current_drive;

    strcpy(name, path);
    for (i = grub_strlen(name); i > 0; i--)
      {
	if (name[i] == '/') break;
      }
    strcpy(&name[i+1], "size.txt");

    total_kbytes = 0;
 
    if (grub_open ("(cd)/size.txt")) {
      char buf[17], *ptr = buf;
      
      grub_read(ptr, 16);	
      safe_parse_maxint(&ptr, &total_kbytes);
      grub_printf("\nKB to read from CD: %d\n", total_kbytes);
      grub_close();
      
    }
    current_drive = olddrive;
  }

  grub_printf ("\nCopy from sector : %d\n", new_start);
  cursect = new_start;

  fat = 0;
  ntfs = 0;
  files = 1;
  new_len = -1;
  new_type = -1;
  curr = new_start;


  for (i = 0; i < files; i++)
    {
      //    grub_printf("-> %d/%d : \r",i+1,files);
      old_perc = -1;

      grub_sprintf (name, "%s%d%c%c", path, l1, l2, l3);
      l3++;
      if (l3 > '9')
	{
	  l2++;
	  l3 = '0';
	  if (l2 > '9')
	    {
	      l1++;
	      l2 = '0';
	    }
	}

      if (new_get (name, curr, &curr, 0))
	{
	  errnum = ERR_FILE_NOT_FOUND;
	  return 1;
	}
    }
  grub_printf ("\nOne partition successfully restored.\n\n");

  return 0;
}



void
flushsect (unsigned char *buf, unsigned char *alloc, int nbsect)
{
  int i, j, nb;

  if (bitindex < 0)
    {
      grub_printf ("Bitmap not yet defined...\n");
      return;
    }

  for (i = 0; i < nbsect;)
    {
      while (!(alloc[bitindex >> 3] & mask[bitindex & 7]))
	{
	  cursect++;
	  bitindex++;
	}
      nb = 0;
      while ((alloc[bitindex >> 3] & mask[bitindex & 7])
	     && ((i + nb) < nbsect))
	{
	  nb++;
	  bitindex++;
	}

      // our buffer should be 16 bytes aligned 
      if (biosdisk (BIOSDISK_WRITE, dest_drive, &buf_geom, 
		    cursect, nb, (unsigned int)(buf)/16)) 
	{
	  grub_printf("Write error, sector %d\n", cursect+j);
	  getkey();
	}
      buf += (512 * nb);
      i += nb;
      cursect += nb;
    }
}

int
new_get (char *file, int sect, int *endsect, int table)
{
  unsigned long savesect;
  unsigned char sectbuf[2048], *buffer;
  unsigned char bitmap[24064 - 2048];
  z_stream zptr;
  int state = Z_SYNC_FLUSH, alloclg=22016;
  int ret, firstpass = 1, firstdisp = 1;
  unsigned char progr = 0, progress[] = "/-\\|";

 retry_open:
  if (! grub_open(file)) 
    {
      /* Switch CD */

      grub_printf("\n\nCan't open %s (%s)\nPlease insert the next CD and press a Key (Escape to cancel)", file, err_list[errnum]);

      buf_drive = -1;
      errnum = 0;

      if (getkey() == 0x11b) {
	return 1;
      }      
      goto retry_open;
    }

  zcinit();

  buffer = (char *) BUFFERADDR;
  zptr.zalloc = zptr.zfree = Z_NULL;
  zptr.avail_in = grub_read(sectbuf, 2048);
  current_bytes += zptr.avail_in/1024;
  zptr.next_in = (char *) sectbuf;
  zptr.next_out = (char *) buffer;
  zptr.avail_out = 24064;
  bitindex = -1;

  savesect=cursect;


  ret = inflateInit (&zptr);
  if (ret != Z_OK) {
    grub_printf("inflateinit failed %d\n", ret);
  }


  do
    {
      //grub_printf("Pre : avin = %d , avout = %d , next_in = %x , next_out = %x\n",zptr.avail_in,zptr.avail_out, zptr.next_in, zptr.next_out);
      //grub_printf("%x %x\n", sectbuf[0], sectbuf[1024]);
      if ((current_bytes % 256) == 0 || (current_bytes % 256) == 1 || firstdisp)
	{
	  int percent = 0, i;

	  firstdisp = 0;

	  grub_printf ("%s ==> HD", file);
	  
	  if (total_kbytes != 0) 
	    percent = (((current_bytes)*100)/total_kbytes);
	  if (percent > 99) percent = 99;		    
	  
	  grub_printf (" %c  %d", progress[progr], percent);
	  grub_putchar('%');
	  progr = (progr + 1) & 3;
	  for (i = 0; i < 16+strlen(file); i++)
	    grub_printf ("\b");
	  if (percent >= 10) 
	    grub_printf ("\b");

	  
	}      
      ret = inflate (&zptr, state);

      if ((ret==Z_BUF_ERROR) && (zptr.avail_out==0)) ret=Z_OK;

      if (((ret == Z_OK) && (zptr.avail_out == 0)) || (ret == Z_STREAM_END))
	{
	  if (firstpass)
	    {
//           grub_printf("%s\n",buffer);
	      if (exist ("BLOCKS", buffer))
		{
		  files = val ("BLOCKS", buffer);
		  grub_printf ("\n Nb of files : %d\n", files);
		}
	      if (exist ("SECTORS", buffer))
		{
		  new_len = val ("SECTORS", buffer);
		  grub_printf (" Part. lg : %d\n", new_len);
		}
	      if (exist ("TYPE", buffer))
		{
		  new_type = val ("TYPE", buffer);
		  grub_printf (" Part. Type : %d\n", new_type);
		}
	      if (exist ("ALLOCTABLELG", buffer))
		{
		  alloclg = val ("ALLOCTABLELG", buffer);
		  if (alloclg!=22016) grub_printf (" Alloc Lg : %d\n", alloclg);
		}

	      grub_memmove (bitmap, buffer + 2048, 24064 - 2048);
	      bitindex = 0;
	      firstpass = 0;
	    }
	  else
	    {
	      if ((24064 - zptr.avail_out) % 512)
		{
		  grub_printf ("*** Modulo 512 !=0 ***\n");
		}
	      flushsect (buffer, bitmap, (24064 - zptr.avail_out) / 512);

	      //grub_printf("flush %d\n", 24064 - zptr.avail_out);
	    }

	  zptr.next_out = (char *) buffer;
	  zptr.avail_out = 24064;
	}

      if ((ret == Z_OK) && (zptr.avail_in == 0))
	{
	  zptr.avail_in = grub_read(sectbuf, 2048);
	  current_bytes += zptr.avail_in/1024;
	  // grub_printf("read %d  t=%d k\n", zptr.avail_in, current_bytes);
	  zptr.next_in = (char *) sectbuf;
	}

    }
  while (ret == Z_OK);

  if (ret != Z_STREAM_END) {
    
    grub_printf ("End While : %d  (-2 STREAM_ERROR, -3 DATA, -4 MEM, -5 BUF, -6 VERSION)\n%s\n", ret, zptr.msg);
    grub_printf("avin = %d , avout = %d , next_in = %x , next_out = %x\n",zptr.avail_in,zptr.avail_out, zptr.next_in, zptr.next_out);
  }
  ret = inflate (&zptr, Z_FINISH);
  if (ret != Z_STREAM_END) {
    grub_printf ("End Decompress : %d\n", ret);
  }
  inflateEnd (&zptr);

  cursect=savesect+alloclg*8;

  return 0;
}

