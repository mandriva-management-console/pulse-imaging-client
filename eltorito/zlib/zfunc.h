#define MAXALLOCBUFF 10

void zcinit(void);
unsigned char *zcalloc(long toto,int nb,int size);
void zcfree(long toto,unsigned char *ptr);

