

#define MAXAMCMODULELENGTH 32

#ifndef NULL
#define NULL 0
#endif


#define READTEST 0
#define WRITETEST 1

#define VPint *(volatile unsigned int *)

typedef struct _write_para
{
	unsigned int addr;
	unsigned int value;
}write_para;

typedef struct _amc_para
{
	char modulename[MAXAMCMODULELENGTH];
	struct _write_para write;
	struct _write_para read;
	struct _write_para in_reset;
	struct _write_para out_reset;
	int flag;
	/*
		0---normal
		1---for pcie fpga
		2---nfc(only reset ,not clear)
		3---pcm (0 means enable,1 means disable)
		4---for usb
	*/
}amc_para;
