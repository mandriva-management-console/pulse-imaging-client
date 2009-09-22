#include "shared.h"
#include "zfunc.h"

unsigned char  used[MAXALLOCBUFF];
unsigned char *zone[MAXALLOCBUFF];

void zcinit(void)
{
	int i;
	for (i=0;i<MAXALLOCBUFF;i++) {used[i]=0; zone[i]=(unsigned char *)(0x100000+i*32768);}
}

unsigned char *zcalloc(long toto,int nb,int size) {
                int i;
//              grub_printf("Calloc (%d) ->",nb*size);
                for (i=0;i<10;i++) if (!used[i]) {used[i]=1;
//                                                grub_printf("%d\n",i);
                                                  return zone[i];}
                grub_printf("Calloc error\n");
                return NULL;
}
 
void zcfree(long toto,unsigned char *ptr) {
                int i;
                for(i=0;i<10;i++) if ( ptr==zone[i] ) {used[i]=0;return;}
                grub_printf("Cfree error\n");
}

