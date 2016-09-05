#ifndef __KERNEL_BOOT_HEADER
#define __KERNEL_BOOT_HEADER

struct e820map {
    unsigned long long offset;
    unsigned long long size;
    unsigned int type;
    unsigned int pad;
};

extern struct kernel_boot_header {
    struct {
        unsigned short int offset, segment;
    } ivt[256];
    char bda[512];
    char ebda[1024];
    struct e820map e820_map[64];
    void *phdrs;
    unsigned long lfp;
} *bootheader;

#endif

