#ifndef SYNSHM_H
#define SYNSHM_H


#define VER( a, b, c ) ( 10000 * a + 100 * b + 1* c )
#define SHM_SYNAPTICS 23947

// TODO make an entry for upcoming versions :)
#include "Shm_0_14_4"
#include "Shm_0_14_5"
#include "Shm_0_14_6"
#include "Shm_0_15_0"
#include "Shm_0_15_1"
#include "Shm_0_15_2"
#include "Shm_0_99_1"
#include "Shm_0_99_2"
#include "Shm_0_99_3"
#include "Shm_1_0_0"
#include "Shm_1_1_0"
#include "Shm_1_1_1"
#include "Shm_1_1_2"
#include "Shm_1_1_3"

// for getting the driver version
typedef struct
{
    int version;
} ShmSegment_Version;


// for driver independent access
typedef union
{
    ShmSegment_Version ver;
    ShmSegment_0_14_4 _0_14_4;
    ShmSegment_0_14_5 _0_14_5;
    ShmSegment_0_14_6 _0_14_6;
    ShmSegment_0_15_0 _0_15_0;
    ShmSegment_0_15_1 _0_15_1;
    ShmSegment_0_15_2 _0_15_2;
    ShmSegment_0_99_1 _0_99_1;
    ShmSegment_0_99_2 _0_99_2;
    ShmSegment_0_99_3 _0_99_3;
    ShmSegment_1_0_0 _1_0_0;
    ShmSegment_1_1_0 _1_1_0;
    ShmSegment_1_1_1 _1_1_1;
    ShmSegment_1_1_2 _1_1_2;
    ShmSegment_1_1_3 _1_1_3;
    
    // TODO make an entry for upcoming versions :)
} SynShm;

#endif
