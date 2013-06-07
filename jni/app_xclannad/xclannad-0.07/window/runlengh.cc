#include<stdio.h>

int main(int argc, char** argv) {
	if (argc != 2) return 0;
	FILE* f = fopen(argv[1],"rb");
	if (f==0) return 0;
	int i,j;
	fseek(f,0,2);
	int sz=ftell(f);
	fseek(f,0,0);
	char* buf=new char[sz];
	char* out = new char[sz];
	fread(buf,sz,1,f);
	fclose(f);
	int oc=0;
	for (i=0; i<sz; i++) {
		int d = buf[i];
		int c = d & 0xff;
		switch(c) {
		case 0x61: c=1; break;
		case 0xa2: c=2; break;
		case 0xc1: c=3; break;
		case 0xdf: c=4; break;
		case 0xde: c=5; break;
		case 0xff: c=6; break;
		}
		for (j=i; j<sz; j++)
			if (buf[j] != d) break;
		int cnt = j-i;
		if (cnt < 16) {
			out[oc++] = c | 0x08 | (cnt<<4);
		} else if (cnt < 256){
			out[oc++] = c;
			out[oc++] = cnt;
		} else fprintf(stderr,"cnt %d\n",cnt);
		if (j == sz) break;
		i = j-1;
	}
	printf("int button1_cnt = %d;\nchar button1[%d] = {\n\t",oc,oc+1);
	for (i=0; i<oc; i++) {
		printf("0x%02x,",int(out[i])&0xff);
		if ( (i&15) == 15) printf("\n\t");
	}
	printf("\n\t0xff};\n");
	return 0;
}
/* 2834bytes*/
