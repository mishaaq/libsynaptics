#ifndef SYNSHM_H
#define SYNSHM_H


#define VER( a, b, c ) ( 10000 * a + 100 * b + 1* c )
#define SHM_SYNAPTICS 23947

    // for driver version 0.14.4
    // TODO make an entry for upcoming versions :)
    #include "Shm_0_14_4"
    #include "Shm_0_14_5"
    #include "Shm_0_14_6"

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
        // TODO make an entry for upcoming versions :)
    } SynShm;

#endif

