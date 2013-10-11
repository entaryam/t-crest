/*
    A small test program testing the communication between patmos processors
    through the NoC

    Author: Rasmus Bo Sorensen
    Copyright: DTU, BSD License
*/

#include <machine/spm.h>
#include "init.h" // Init.h is auto generated by Poseidon the T-CREST scheduler
                  // https://github.com/t-crest/poseidon.git



#define UART_STATUS *((volatile _SPM int *) 0xF0000800)
#define UART_DATA   *((volatile _SPM int *) 0xF0000804)
#define CPU_ID      *((volatile _SPM int *) 0xF0000000)

#define VALID_BIT 0x08000
#define DONE_BIT 0x04000

#define WRITE(data,len) do { \
  unsigned i; \
  for (i = 0; i < (len); i++) {        \
    while ((UART_STATUS & 0x01) == 0); \
    UART_DATA = (data)[i];             \
  } \
} while(0)

struct network_interface
{
    volatile _SPM int *dma;
    volatile _SPM int *dma_p;
    volatile _SPM int *st;
    volatile _SPM int *spm;
} ni = {
    (volatile _SPM int *) 0xE0000000,
    (volatile _SPM int *) 0xE1000000,
    (volatile _SPM int *) 0xE2000000,
    (volatile _SPM int *) 0xE8000000
};

void init_ni(){
    int i;
    for (i = 0; i < TIMESLOTS; ++i)
    {
        *(ni.st+i) = 0x04 | init_array[CPU_ID][0][i];
    }
    for (i = 0; i < DMAS; ++i)
    {
        *(ni.dma_p+i) = init_array[CPU_ID][1][i];
    }
}

int send(int rcv_id, short read_ptr, short write_ptr, int size){ // The size in in number of double
    if ((rcv_id == CPU_ID) || (rcv_id < 0) || (rcv_id > CORES-1)){return 0;}
    if ((*(ni.dma+(rcv_id<<1)) & VALID_BIT) != 0)
    {
        if ((*(ni.dma+(rcv_id<<1)) & DONE_BIT) != 0){return 0;}
    }
    *(ni.dma+(rcv_id<<1)+1) = (read_ptr << 16) | write_ptr; // Read pointer and write pointer in the dma table
    *(ni.dma+(rcv_id<<1)) = size | (1 << 15) ; // DWord count and valid bit
    return 1;
}

int main() {

    volatile int *dummy = (int *) 0x123;

    volatile _SPM int *led_ptr = (volatile _SPM int *) 0xF0000900;

    init_ni();

    if (CPU_ID == 0)
    {
        int k;
        for(k = 0; k < 8; k++){
            *(ni.spm+k) = k; // Fill numbers in communication scratch pad
        }

        //*(ni.dma+1) = (0 << 16) | 4; // Read pointer and write pointer in the dma table
        //*(ni.dma+0) = 4 | (1 << 15) ; // DWord count and valid bit

        // Sending the 8 words to processor 1
        while(!send(1,0,0,4));


        // Sending the 8 words to processor 2
        //while(!send(2,0,0,4));  // Uncomment either of these two send() calls
        //send(2,0,0,4);          // and the program counter becomes undefined.
        *(ni.dma+5) = (0 << 16) | 0;
        *(ni.dma+4) = 4 | (1 << 15);

        // Sending the 8 words to processor 3
        *(ni.dma+7) = (0 << 16) | 0;
        *(ni.dma+6) = 4 | (1 << 15);

    } else {
        int k;
        for(k = 0; k < 8; k++){
            *(ni.spm+k) = 0; // Fill numbers in communication scratch pad
        }

        while(*(ni.spm+1) == 0);
    }

    char msg[] = "Hello world, from core: ";
    char cid = (char)(CPU_ID + (int)'0');

    WRITE(msg,24);
    WRITE(&cid,1);
    WRITE("\n",1);


    int i, j;

    for (;;) {
        UART_DATA = '1';
        for (i=2000; i!=0; --i)
            for (j=2000; j!=0; --j)
                *led_ptr = 1;


        UART_DATA = '0';
        for (i=2000; i!=0; --i)
            for (j=2000; j!=0; --j)
                *led_ptr = 0;

    }

    return 0;
}
