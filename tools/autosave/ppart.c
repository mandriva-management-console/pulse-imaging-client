/*
 * (c) 2003-2007 Linbox FAS, http://linbox.com
 * (c) 2008-2009 Mandriva, http://www.mandriva.com
 *
 * $Id$
 *
 * This file is part of Pulse 2, http://pulse2.mandriva.org
 *
 * Pulse 2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Pulse 2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pulse 2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define DEBUG(a)

unsigned long s_min=0xFFFFFFFF,s_max=0;

int main(int argc,char *argv[])
{
 unsigned char buffer[512];
 int i,index=0,nb,pnum=1,dnum;
 int fi;
 FILE *fo;
 unsigned long base,nbase=0,ebase=0,start,lg,fsect,up,ptr;
 __off64_t ret;
 unsigned int ttype[20],poff[20];
 unsigned long tmin[20],tmax[20];

 if (argc!=3) {printf("Usage : ppart device BIOSnum\n");exit(1);}

 sprintf(buffer,"rm -f core* CDNUM ./store/raw/* ./store/iso/* ./store/iso_raw/*");
 system(buffer);
 sprintf(buffer,"mkdir -p ./store/raw ./store/iso ./store/iso_raw");
 system(buffer);

 sscanf(argv[2],"%d",&dnum);

 fi=open(argv[1],O_RDONLY | O_LARGEFILE);

 do
 {
  base=nbase;
  DEBUG(printf("Base : %ld (offset %lld)\n",base,512*(__off64_t)base);)
  ret=lseek64(fi,512*(__off64_t)base,SEEK_SET);
  DEBUG(printf("lseek64 returned : %lld\n",ret);)
  read(fi,buffer,512);

  for(i=0;i<4;i++)
  {
   fsect=*(unsigned long *)(&buffer[0x1be +i*16+8]);
   start=fsect+nbase;
   lg=*(unsigned long *)(&buffer[0x1be +i*16+12]);

   if ( (buffer[0x1be +i*16+4]!=0) && !((buffer[0x1be +i*16+4]==5) && (ebase!=0)) )
   {
      DEBUG(printf(" Start %ld (%ld), lg %ld, type %d\n", fsect,start,lg,buffer[0x1be +i*16+4]);)

      if (start<s_min)    s_min=start;
      if (start+lg>s_max) s_max=start+lg;
      if (buffer[0x1be +i*16+4]!=0x05) {ttype[index]=buffer[0x1be +i*16+4];
                                    tmin[index]=start; tmax[index]=start+lg-1;
                                        poff[index++]=pnum++;}
   }
   if (buffer[0x1be +i*16+4]==5)
      {nbase=ebase+fsect;
       if (ebase==0) {ebase=nbase;pnum=5;}}
  }
 }
 while (nbase!=base);

 s_min=0;
 fo=fopen("./store/raw/CONF","wt");
 fprintf(fo,"D:%d L:%ld\n",dnum,s_max);
 fprintf(fo,"R\n");
 for(i=0;i<index;i++)
  if (ttype[i]==0x83)
   fprintf(fo,"P%d , S:%ld , E:%ld , t:%d\n",poff[i],tmin[i],tmax[i],ttype[i]);
  else
   fprintf(fo,"#P%d , S:%ld , E:%ld , t:%d\n",poff[i],tmin[i],tmax[i],ttype[i]);
 fprintf(fo,"E\n");
 fclose(fo);

 fo=fopen("./store/raw/PTABS","wb");
 ptr=s_min;
 while (ptr<s_max)
 {
  up=s_max; nb=-1;
  for(i=0;i<index;i++)
  {
   if ((tmin[i]<ptr) && (tmax[i]<ptr)) continue;
   if (tmin[i]<up) {up=tmin[i]; nb=i;}
  }

  //printf("From : %ld , to : %ld\n",ptr,up-1);
  for (start=ptr;start<up;start++)
  {
   //printf(".");
   lseek64(fi,512*(__off64_t)start,SEEK_SET);
   read(fi,buffer,512);
   fwrite(&start,1,4,fo);
   fwrite(buffer,1,512,fo);
  }
  //printf("\n");
  ptr=tmax[nb]+1;
 }
 fclose(fo);
 close(fi);


 fo=fopen("./store/iso/log.txt","wt");
 for(i=0;i<index;i++)
 {
  char command[80];
  printf("P%d , S:%ld , E:%ld , t:%d\n",poff[i],tmin[i],tmax[i],ttype[i]);
  sprintf(command,"./image_reiserfs %s%d ?",argv[1],poff[i]);
  if (system(command)==0)
  { sprintf(command,"./image_reiserfs %s%d ./store/raw/P%d",argv[1],poff[i],poff[i]);
    system(command);
    system("./check_cd");
    continue; }
  sprintf(command,"./image_e2fs %s%d ?",argv[1],poff[i]);
  if (system(command)==0)
  { sprintf(command,"./image_e2fs %s%d ./store/raw/P%d",argv[1],poff[i],poff[i]);
    system(command);
    system("./check_cd");
    continue; }
  fprintf(fo,"Unsupported FS...(part %d, type %x)\n",poff[i],ttype[i]);
 }
 fclose(fo);

 sprintf(buffer,"./check_cd FLUSH");
 system(buffer);

 return 0;
}
