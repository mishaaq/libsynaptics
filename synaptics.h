//
// License: GPL2
// Author: Stefan Kombrink
// Description:
//    a library to access the synaptics touch pad (nearly) independent of the used driver version
//

#ifndef SYNAPTICS_H

#define SYNAPTICS_H

#include "synparam.h"

namespace Synaptics
{
    //
    // type of detected pad
    //
    typedef enum
    {
        PT_NONE = 0,
        PT_ALPS,
        PT_SYNAPTICS,
        PT_MULTISYN,
        PT_UNKNOWN
    } PadType;

    //
    // type of detected driver version
    //
    typedef enum
    {
        DV_UNKNOWN = 0,     // unknown driver version or - more likely - no driver at all
        DV_OUTDATED,        // driver version is outdated and hence not supported
        DV_TOORECENT,       // driver version is too recent and hence not yet supported
        DV_SUPPORTED,       // driver version is supported
    } DriverVersion;


    // the different possible types of the shm elements
    enum ParamType
    {
        PT_VOID,
        PT_INT,
        PT_BOOL,
        PT_DOUBLE,
        PT_INT_RO,
        PT_BOOL_RO,
        PT_DOUBLE_RO
    };

    // parameter type
    typedef struct
    {
        int offset;                             // offset in shared memory area
        enum ParamType type;                    // type of parameter
        double min_val;                         // minimum allowed value
        double max_val;                         // maximum allowed value
    } Param;


    typedef enum
    {
        NoTrigger=-1,
        AllCorners=0,
        TopEdge,
        TopRightCorner,
        RightEdge,
        BottomRightCorner,
        BottomEdge,
        BottomLeftCorner,
        LeftEdge,
        TopLeftCorner
    } ScrollTrigger;

    typedef enum
    {
        NoButton=0,
        Left,
        Middle,
        Right
    } Button;

    typedef enum
    {
        RightTop = 0,          // Right top corner
        RightBottom,           // Right bottom corner
        LeftTop,               // Left top corner
        LeftBottom,            // Left bottom corner
        OneFinger,             // Non-corner tap, one finger
        TwoFingers,            // Non-corner tap, two fingers
        ThreeFingers,          // Non-corner tap, three fingers
        MaxTap
    } TapType;
}

#include "pad.h"

#endif
