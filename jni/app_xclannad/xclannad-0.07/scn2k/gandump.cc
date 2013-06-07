#include<stdio.h>
#include<string.h>

char* ganname[6] = {"ptn", "x", "y", "time", "alpha", "?"};
int main(int argc, char** argv) {
	if (argc != 2) return 0;
	FILE* f = fopen(argv[1],"rb");
	if (f==0) return 0;
	fseek(f,0,2); int sz=ftell(f); fseek(f,0,0);
	char* buf = new char[sz];
	char* bufo = buf;
	fread(buf,1,sz,f);
	fclose(f);

	int i;
	for (i=0; i<16; i++) {
		printf("%02x ",int(buf[i])&0xff);
	}
	printf("\n");
	buf+=16;
	printf("file '%s'\n",buf);
	buf += strlen(buf)+1;
	printf("N1 = %3d\n", *(int*)buf); buf+=4;
	int ptns = *(int*)buf; buf += 4;
	for (i=0; i<ptns; i++) {
		int j;
		printf("\tN2 = %3d\n",*(int*)buf); buf += 4;
		int n = *(int*)buf; buf += 4;
		for (j=0; j<n; j++) {
			printf("\t%3d: ",j);
			int k; int d1,d2;
			for (k=0; k<6; k++) {
				d1 = *(int*)(buf+k*8);
				d2 = *(int*)(buf+k*8+4);
				if (d1 != 30100+k) printf("err: d1 %d\n",d1);
				printf("%s: %5d, ",ganname[k],d2);
			}
			printf("\n");
			buf += 6*8;
			int d = *(int*)buf; buf += 4;
			if (d != 999999) printf("err: dd %d\n",d);
		}
	}
	if (buf != bufo+sz) { printf("orig sz %d read %d\n",sz,buf-bufo);}
}
