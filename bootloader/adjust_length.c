#include <stdlib.h>
#include <stdio.h>

void main(int argc,char *argv[])
{
 unsigned char command[132];
 unsigned long lg;
 FILE *fi;

 system("ls -l revoboot.pxe | awk '{print $5}' >pxe.size");

 fi=fopen("pxe.size","rt");
 fscanf(fi,"%ld",&lg);
 fclose(fi);

 printf("File length : %ld\n",lg);
 lg-=512;

 fi=fopen("revoboot.pxe","r+b");
 fseek(fi,32,SEEK_SET);
 fwrite(&lg,4,1,fi);
 fwrite(&lg,4,1,fi);
 fclose(fi);

}
