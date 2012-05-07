#ifndef __MTR_API
#define __MTR_API

#define NIPQUAD(addr) \
            ((unsigned char *)&addr)[3], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[0]
#define NIPHALF(addr) \
            ((unsigned short *)&addr)[1], \
        ((unsigned short *)&addr)[0]

int SetMtrEntry(struct mtr_args *opt, unsigned int cmd);
int MtrGetAllEntries(struct mtr_list_args *opt);

#endif
