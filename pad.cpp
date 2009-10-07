#include "syndebug.h"
#include "synparam.h"
#include "pad.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

//
// singleton specific code
//

pthread_mutex_t Synaptics::Pad::mMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

Synaptics::Pad* Synaptics::Pad::getInstance()
{
    SYNDEBUG( "waiting for lock..." );
    pthread_mutex_lock( &mMutex );
    Synaptics::Pad* ret = getInstanceProtected();
    pthread_mutex_unlock( &mMutex );
    SYNDEBUG( "...unlocking done!" );

    return ret;
}

Synaptics::Pad* Synaptics::Pad::getInstanceProtected()
{
    static Synaptics::Pad mInstance;
    return &mInstance;
}

Synaptics::Pad::~Pad() 
{
    // no mutex neccessary since this routine cannot be called twice!
    if ( mSynShm )
    {
        SYNDEBUG( "detach driver" );
        shmdt( mSynShm );
    }
    SYNDEBUG( "deleted singleton" ); 
}

Synaptics::Pad::Pad() 
{ 
    // do some serious stuff...
    SYNDEBUG( "created singleton" );

    registerParameters();

    init();
}

Synaptics::Pad::Pad( const Synaptics::Pad& cc ){}



//
// synaptics pad related code
//



#define PAROFF( verId, par ) \
    offsetof( SynShm, verId ) + offsetof( ShmSegment##verId, par )

#define SETPAR( verId, p, par, t, min, max ) \
    p.offset = PAROFF( verId, par ); \
    p.type = t; \
    p.min_val = min; \
    p.max_val = max;

#define NODRIVERVER VER( 0, 0, 0 )
#define DRIVERVERSION mSynShm? mSynShm->ver.version : VER( 0, 0, 0 )
#define DRIVERSTR intVerToStrVer( DRIVERVERSION )
#define HASPARAM( param ) ( mSupportedDriver[ DRIVERSTR ].find( param ) != \
    mSupportedDriver[ DRIVERSTR ].end() )


//
// "local" static variables
//

bool Synaptics::Pad::mHasShm = false;
Synaptics::DriverVersion Synaptics::Pad::mDetectedDriver = DV_UNKNOWN;

std::map<std::string, Synaptics::Pad::Parameter> Synaptics::Pad::mSupportedDriver;
std::list<std::string> Synaptics::Pad::mSupportedDriverList;
SynShm* Synaptics::Pad::mSynShm = NULL;


//
// methods (private)
//

std::string Synaptics::Pad::intVerToStrVer( int v )
{
    SYNDEBUG( "intVerToStrVer" );
    char ver[10];

    if ( v == 0)
        return std::string( "None" );

    sprintf(
        ver,
        "%d.%d.%d",
        v / 10000,
        ( v / 100 ) % 100,
        v % 100
    );

    return std::string( ver );
}

int Synaptics::Pad::driverVersion()
{
    SYNDEBUG( "driverVersion" );
    return DRIVERVERSION;
}

//
// main methods (public)
//

bool Synaptics::Pad::hasHardware()
{
    SYNDEBUG( "hasHardware" );
    SYNERR( "implement me!" );
    return( false );
}

Synaptics::DriverVersion Synaptics::Pad::driverKind()
{
    SYNDEBUG( "driverKind" );
    return mDetectedDriver;
}

bool Synaptics::Pad::hasDriver()
{
    SYNDEBUG( "hasDriver" );
    return ( mDetectedDriver == DV_SUPPORTED );
}

bool Synaptics::Pad::hasShm()
{
    SYNDEBUG( "hasShm" );
    return( mHasShm );
}

bool Synaptics::Pad::detectDriverVersion()
{
    SYNDEBUG( "detecting driver version..." );

    int shmid;

    mSynShm = NULL;
    mHasShm = true;

    // connect to the shared memory area
    if ( ( shmid = shmget( SHM_SYNAPTICS, sizeof( ShmSegment_Version ), 0 ) ) == -1 )
    {
        if ( ( shmid = shmget( SHM_SYNAPTICS, 0, 0 ) ) == -1 )
        {
            SYNERR( "can't access shared memory area. SHMConfig disabled?" );
            mHasShm = false;
        }
        else
        {
            SYNERR( "incorrect size of shared memory area. Incompatible driver version?" );
        }
    }
    else
    {
        // attach
        if ( ( mSynShm = ( ::SynShm* ) shmat( shmid, NULL, SHM_RDONLY ) ) == NULL)
        {
            perror( "shmat" );
            mHasShm = false;
        }
        else
        {
            SYNDEBUG( "detected driver version " << driverStr() );
            return( true );
        }

    }

    SYNDEBUG( "none found!" );
    return( false );
}


std::string Synaptics::Pad::driverStr()
{
    SYNDEBUG( "driverStr" );
    return( intVerToStrVer( mSynShm? mSynShm->ver.version : VER( 0, 0, 0 ) ) );
}

std::string Synaptics::Pad::libraryStr()
{
    SYNDEBUG( "libraryStr" );
    return( VERSION );
}


bool Synaptics::Pad::hasSynClient()
{
    SYNDEBUG( "hasSynClient" );

    int retVal = system( "synclient >/dev/null" );

    return( retVal == 256 );
}

bool Synaptics::Pad::hasSynDaemon()
{
    SYNDEBUG( "hasSynDaemon" );

    int retVal = system( "syndaemon --help >/dev/null 2>&1" );

    return( retVal == 256 );
}

bool Synaptics::Pad::isSynDaemonRunning()
{
    SYNDEBUG( "isSynDaemonRunning" );
    SYNERR( "IMPLEMENT ME (thread safe!)" );
    return( false );
}

bool Synaptics::Pad::killSynDaemonProcesses()
{
    SYNDEBUG( "killSynDaemonProcesses" );
    SYNERR( "IMPLEMENT ME (thread safe!)" );
    return( false );
}

Synaptics::PadType Synaptics::Pad::getPadType()
{
    SYNDEBUG( "getPadType" );

    PadType pt = PT_NONE;

    // FIXME: detection just based on the assumption that the driver will register as unknown
    if ( mHasShm and mDetectedDriver == DV_UNKNOWN )
        pt = PT_UNKNOWN;
    else
    {
        // TODO: decide between ALPS, SYNAPTICS and MULTISYN
        pt = PT_MULTISYN;
    }

    return( pt );
}

bool Synaptics::Pad::hasParam( std::string param )
{
    SYNDEBUG( "hasParam" );

    bool ret = HASPARAM( param );

    return( ret );
}


double Synaptics::Pad::getParam( std::string param )
{
    SYNDEBUG( "getParam" );
    SYNDEBUG( "NOT YET THREAD SAFE!" );

    Bool rb = 0;
    int ri = 0;
    double rd = 0;


    if ( HASPARAM( param ) )
    {
        Param p = mSupportedDriver[ DRIVERSTR ][ param ];

        if ( ( p.type == PT_BOOL ) or ( p.type == PT_BOOL_RO ) )
        {
            rb = *(Bool*)(((char*)mSynShm) + p.offset );
            return( rb );
        }

        if ( ( p.type == PT_INT ) or ( p.type == PT_INT_RO ) )
        {
            ri = *(int*)(((char*)mSynShm) + p.offset );
            return( ri );
        }

        if ( ( p.type == PT_DOUBLE ) or ( p.type == PT_DOUBLE_RO ) )
        {
            rd = *(double*)(((char*)mSynShm) + p.offset );
            return( rd );
        }
    }
    else
         return( 0 );
}


void Synaptics::Pad::setParam( std::string param, double v )
{
    SYNDEBUG( "setParam" );
    SYNDEBUG( "NOT YET THREAD SAFE!" );

    if ( HASPARAM( param ) )
    {
        Param p = mSupportedDriver[ DRIVERSTR ][ param ];

        if ( ( p.min_val <= v ) and ( p.max_val >= v ) )
        {
            if ( p.type == PT_BOOL )
                (*(Bool*)(((char*)mSynShm) + p.offset)) = (Bool)v;

            if ( p.type == PT_INT )
                (*(int*)(((char*)mSynShm) + p.offset)) = (int)v;

            if ( p.type == PT_DOUBLE )
                (*(double*)(((char*)mSynShm) + p.offset)) = (double)v;

            mSupportedDriver[ DRIVERSTR ][ param ] = p;
        }
    }
}

double Synaptics::Pad::getParamMin( std::string param )
{
    SYNDEBUG( "getParamMin" );

    if ( HASPARAM( param ) )
    {
        Param p = mSupportedDriver[ DRIVERSTR ][ param ];

        return( p.min_val );
    }
    else
    {
        return( 0 );
    }
}

double Synaptics::Pad::getParamMax( std::string param )
{
    SYNDEBUG( "getParamMax" );

    if ( HASPARAM( param ) )
    {
        Param p = mSupportedDriver[ DRIVERSTR ][ param ];

        return( p.max_val );
    }
    else
    {
        return( 0 );
    }
}


Synaptics::ParamType Synaptics::Pad::getParamType( std::string param )
{
    SYNDEBUG( "getParamType" );

    if ( HASPARAM( param ) )
    {
        Param p = mSupportedDriver[ DRIVERSTR ][ param ];

        return( p.type );
    }
    else
    {
        return( Synaptics::PT_VOID );
    }
}

std::list<std::string> Synaptics::Pad::getAvailableParams()
{
    SYNDEBUG( "getAvailableParams" );

    std::list<std::string> myParams;

    Parameter p = mSupportedDriver[ DRIVERSTR ];

    for ( Parameter::iterator it = p.begin(); it != p.end(); it++ )
    {
        myParams.insert( myParams.end(), it->first );
    }

    return( myParams );
}


//
// basic shared shm for all synaptics driver versions
//

#define SETBASEPAR( p, drvVer ) \
    SETPAR( drvVer, p[ LEFTEDGE ],                 left_edge,                  PT_INT,     0,     10000 );\
    SETPAR( drvVer, p[ RIGHTEDGE ],                right_edge,                 PT_INT,     0,     10000 );\
    SETPAR( drvVer, p[ TOPEDGE ],                  top_edge,                   PT_INT,     0,     10000 );\
    SETPAR( drvVer, p[ BOTTOMEDGE ],               bottom_edge,                PT_INT,     0,     10000 );\
    SETPAR( drvVer, p[ FINGERLOW ],                finger_low,                 PT_INT,     0,     255 );\
    SETPAR( drvVer, p[ FINGERHIGH ],               finger_high,                PT_INT,     0,     255 );\
    SETPAR( drvVer, p[ MAXTAPTIME ],               tap_time,                   PT_INT,     0,     1000 );\
    SETPAR( drvVer, p[ MAXTAPMOVE ],               tap_move,                   PT_INT,     0,     2000 );\
    SETPAR( drvVer, p[ MAXDOUBLETAPTIME ],         tap_time_2,                 PT_INT,     0,     1000 );\
    SETPAR( drvVer, p[ CLICKTIME ],                click_time,                 PT_INT,     0,     1000 );\
    SETPAR( drvVer, p[ FASTTAPS ],                 fast_taps,                  PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ EMULATEMIDBUTTONTIME ],     emulate_mid_button_time,    PT_INT,     0,     1000 );\
    SETPAR( drvVer, p[ VERTSCROLLDELTA ],          scroll_dist_vert,           PT_INT,     0,     1000 );\
    SETPAR( drvVer, p[ HORIZSCROLLDELTA ],         scroll_dist_horiz,          PT_INT,     0,     1000 );\
    SETPAR( drvVer, p[ MINSPEED ],                 min_speed,                  PT_DOUBLE,  0,     1.0 );\
    SETPAR( drvVer, p[ MAXSPEED ],                 max_speed,                  PT_DOUBLE,  0,     1.0 );\
    SETPAR( drvVer, p[ ACCELFACTOR ],              accl,                       PT_DOUBLE,  0,     0.2 );\
    SETPAR( drvVer, p[ EDGEMOTIONMINZ ],           edge_motion_min_z,          PT_INT,     1,     255 );\
    SETPAR( drvVer, p[ EDGEMOTIONMAXZ ],           edge_motion_max_z,          PT_INT,     1,     255 );\
    SETPAR( drvVer, p[ EDGEMOTIONMINSPEED ],       edge_motion_min_speed,      PT_INT,     0,     1000 );\
    SETPAR( drvVer, p[ EDGEMOTIONMAXSPEED ],       edge_motion_max_speed,      PT_INT,     0,     1000 );\
    SETPAR( drvVer, p[ EDGEMOTIONUSEALWAYS ],      edge_motion_use_always,     PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ UPDOWNSCROLLING ],          updown_button_scrolling,    PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ LEFTRIGHTSCROLLING ],       leftright_button_scrolling, PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ UPDOWNREPEAT ],             updown_button_repeat,       PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ LEFTRIGHTREPEAT ],          leftright_button_repeat,    PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ SCROLLBUTTONREPEAT ],       scroll_button_repeat,       PT_INT,     SBR_MIN, SBR_MAX );\
    SETPAR( drvVer, p[ TOUCHPADOFF ],              touchpad_off,               PT_INT,     0,     2 );\
    SETPAR( drvVer, p[ GUESTMOUSEOFF ],            guestmouse_off,             PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ LOCKEDDRAGS ],              locked_drags,               PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ RTCORNERBUTTON ],           tap_action[RT_TAP],         PT_INT,     0,     SYN_MAX_BUTTONS );\
    SETPAR( drvVer, p[ RBCORNERBUTTON ],           tap_action[RB_TAP],         PT_INT,     0,     SYN_MAX_BUTTONS );\
    SETPAR( drvVer, p[ LTCORNERBUTTON ],           tap_action[LT_TAP],         PT_INT,     0,     SYN_MAX_BUTTONS );\
    SETPAR( drvVer, p[ LBCORNERBUTTON ],           tap_action[LB_TAP],         PT_INT,     0,     SYN_MAX_BUTTONS );\
    SETPAR( drvVer, p[ TAPBUTTON1 ],               tap_action[F1_TAP],         PT_INT,     0,     SYN_MAX_BUTTONS );\
    SETPAR( drvVer, p[ TAPBUTTON2 ],               tap_action[F2_TAP],         PT_INT,     0,     SYN_MAX_BUTTONS );\
    SETPAR( drvVer, p[ TAPBUTTON3 ],               tap_action[F3_TAP],         PT_INT,     0,     SYN_MAX_BUTTONS );\
    SETPAR( drvVer, p[ CIRCULARSCROLLING ],        circular_scrolling,         PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ CIRCSCROLLDELTA ],          scroll_dist_circ,           PT_DOUBLE,  0.01,  3 );\
    SETPAR( drvVer, p[ CIRCSCROLLTRIGGER ],        circular_trigger,           PT_INT,     0,     8 );\
    SETPAR( drvVer, p[ CIRCULARPAD ],              circular_pad,               PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ PALMDETECT ],               palm_detect,                PT_BOOL,    0,     1 );\
    SETPAR( drvVer, p[ PALMMINWIDTH ],             palm_min_width,             PT_INT,     0,     15 );\
    SETPAR( drvVer, p[ PALMMINZ ],                 palm_min_z,                 PT_INT,     0,     255 );\
    SETPAR( drvVer, p[ COASTINGSPEED ],            coasting_speed,             PT_DOUBLE,  0,     20 );\
    SETPAR( drvVer, p[ ABSCOORDX ],                x,                          PT_INT_RO,  1,     0 );\
    SETPAR( drvVer, p[ ABSCOORDY ],                y,                          PT_INT_RO,  1,     0 );\
    SETPAR( drvVer, p[ VERSIONNUMBER ],            version,                    PT_INT_RO,  1,     0 );



//
// !!!VERSION RELEVANT CODE AHEAD!!!
//

// TODO add support for new driver versions here
void Synaptics::Pad::registerParameters()
{
    Parameter p;

    //
    // register supported versions
    //
    SETBASEPAR( p, _0_14_4 );

    mSupportedDriver[ "0.14.4" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "0.14.4" );

    SETBASEPAR( p, _0_14_5 );
    SETPAR( _0_14_5, p[ SINGLETAPTIMEOUT ], single_tap_timeout, PT_INT, 0, 1000 );
    SETPAR( _0_14_5, p[ VERTEDGESCROLL ],  scroll_edge_vert,  PT_BOOL, 0, 1 );
    SETPAR( _0_14_5, p[ HORIZEDGESCROLL ], scroll_edge_horiz, PT_BOOL, 0, 1 );
    SETPAR( _0_14_5, p[ VERTTWOFINGERSCROLL ],  scroll_twofinger_vert,  PT_BOOL, 0, 1 );
    SETPAR( _0_14_5, p[ HORIZTWOFINGERSCROLL ], scroll_twofinger_horiz, PT_BOOL, 0, 1 );
    SETPAR( _0_14_5, p[ PRESSMOTIONMINZ ], press_motion_min_z, PT_INT, 0, 255 );
    SETPAR( _0_14_5, p[ PRESSMOTIONMAXZ ], press_motion_max_z, PT_INT, 0, 255 );
    SETPAR( _0_14_5, p[ PRESSMOTIONMINFACTOR], press_motion_min_factor, PT_DOUBLE, 0, 10.0 );
    SETPAR( _0_14_5, p[ PRESSMOTIONMAXFACTOR], press_motion_max_factor, PT_DOUBLE, 0, 10.0 );

    mSupportedDriver[ "0.14.5" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "0.14.5" );

    // same struct as in 0.14.5
    mSupportedDriver[ "0.14.6" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "0.14.6" );

    SETBASEPAR( p, _0_15_0 );
    SETPAR( _0_15_0, p[ SINGLETAPTIMEOUT ], single_tap_timeout, PT_INT, 0, 1000 );
    SETPAR( _0_15_0, p[ VERTEDGESCROLL ],  scroll_edge_vert,  PT_BOOL, 0, 1 );
    SETPAR( _0_15_0, p[ HORIZEDGESCROLL ], scroll_edge_horiz, PT_BOOL, 0, 1 );
    SETPAR( _0_15_0, p[ VERTTWOFINGERSCROLL ],  scroll_twofinger_vert,  PT_BOOL, 0, 1 );
    SETPAR( _0_15_0, p[ HORIZTWOFINGERSCROLL ], scroll_twofinger_horiz, PT_BOOL, 0, 1 );
    SETPAR( _0_15_0, p[ PRESSMOTIONMINZ ], press_motion_min_z, PT_INT, 0, 255 );
    SETPAR( _0_15_0, p[ PRESSMOTIONMAXZ ], press_motion_max_z, PT_INT, 0, 255 );
    SETPAR( _0_15_0, p[ PRESSMOTIONMINFACTOR], press_motion_min_factor, PT_DOUBLE, 0, 10.0 );
    SETPAR( _0_15_0, p[ PRESSMOTIONMAXFACTOR], press_motion_max_factor, PT_DOUBLE, 0, 10.0 );

    SETPAR( _0_15_0, p[ FINGERPRESS ], finger_press, PT_INT, 0, 256 );
    SETPAR( _0_15_0, p[ EMULATETWOFINGERMINZ ], emulate_twofinger_z, PT_INT, 0, 1000 );
    SETPAR( _0_15_0, p[ CORNERCOASTING ], scroll_edge_corner, PT_BOOL, 0, 1 );
    SETPAR( _0_15_0, p[ TRACKSTICKSPEED ], trackstick_speed, PT_DOUBLE, 0, 200.0 );
    SETPAR( _0_15_0, p[ LOCKEDDRAGTIMEOUT ], locked_drag_time, PT_INT, 0, 30000 );
    SETPAR( _0_15_0, p[ CLICKFINGER1 ], click_action[F1_CLICK1], PT_INT, 0, SYN_MAX_BUTTONS );
    SETPAR( _0_15_0, p[ CLICKFINGER2 ], click_action[F2_CLICK1], PT_INT, 0, SYN_MAX_BUTTONS );
    SETPAR( _0_15_0, p[ CLICKFINGER3 ], click_action[F3_CLICK1], PT_INT, 0, SYN_MAX_BUTTONS );
    SETPAR( _0_15_0, p[ GRABEVENTDEVICE ], grab_event_device, PT_BOOL, 0, 1 );

    mSupportedDriver[ "0.15.0" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "0.15.0" );

    SETBASEPAR( p, _0_15_1 );
    SETPAR( _0_15_1, p[ SINGLETAPTIMEOUT ], single_tap_timeout, PT_INT, 0, 1000 );
    SETPAR( _0_15_1, p[ VERTEDGESCROLL ],  scroll_edge_vert,  PT_BOOL, 0, 1 );
    SETPAR( _0_15_1, p[ HORIZEDGESCROLL ], scroll_edge_horiz, PT_BOOL, 0, 1 );
    SETPAR( _0_15_1, p[ VERTTWOFINGERSCROLL ],  scroll_twofinger_vert,  PT_BOOL, 0, 1 );
    SETPAR( _0_15_1, p[ HORIZTWOFINGERSCROLL ], scroll_twofinger_horiz, PT_BOOL, 0, 1 );
    SETPAR( _0_15_1, p[ PRESSMOTIONMINZ ], press_motion_min_z, PT_INT, 0, 255 );
    SETPAR( _0_15_1, p[ PRESSMOTIONMAXZ ], press_motion_max_z, PT_INT, 0, 255 );
    SETPAR( _0_15_1, p[ PRESSMOTIONMINFACTOR], press_motion_min_factor, PT_DOUBLE, 0, 10.0 );
    SETPAR( _0_15_1, p[ PRESSMOTIONMAXFACTOR], press_motion_max_factor, PT_DOUBLE, 0, 10.0 );
    SETPAR( _0_15_1, p[ FINGERPRESS ], finger_press, PT_INT, 0, 256 );
    SETPAR( _0_15_1, p[ EMULATETWOFINGERMINZ ], emulate_twofinger_z, PT_INT, 0, 1000 );
    SETPAR( _0_15_1, p[ CORNERCOASTING ], scroll_edge_corner, PT_BOOL, 0, 1 );
    SETPAR( _0_15_1, p[ TRACKSTICKSPEED ], trackstick_speed, PT_DOUBLE, 0, 200.0 );
    SETPAR( _0_15_1, p[ LOCKEDDRAGTIMEOUT ], locked_drag_time, PT_INT, 0, 30000 );
    SETPAR( _0_15_1, p[ CLICKFINGER1 ], click_action[F1_CLICK1], PT_INT, 0, SYN_MAX_BUTTONS );
    SETPAR( _0_15_1, p[ CLICKFINGER2 ], click_action[F2_CLICK1], PT_INT, 0, SYN_MAX_BUTTONS );
    SETPAR( _0_15_1, p[ CLICKFINGER3 ], click_action[F3_CLICK1], PT_INT, 0, SYN_MAX_BUTTONS );
    SETPAR( _0_15_1, p[ GRABEVENTDEVICE ], grab_event_device, PT_BOOL, 0, 1 );

    SETPAR( _0_15_1, p[ SPECIALSCROLLAREARIGHT ], special_scroll_area_right, PT_BOOL, 0, 1 );

    mSupportedDriver[ "0.15.1" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "0.15.1" );

    // same struct as in 0.15.1
    mSupportedDriver[ "0.15.2" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "0.15.2" );
    
    // same struct as in 0.15.2
    mSupportedDriver[ "0.99.1" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "0.99.1" );

    // same struct as in 0.99.1
    mSupportedDriver[ "0.99.2" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "0.99.2" );

    // same struct as in 0.99.2
    mSupportedDriver[ "0.99.3" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "0.99.3" );

    // same struct as in 0.99.3
    mSupportedDriver[ "1.0.0" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "1.0.0" );

    SETBASEPAR( p, _1_1_0 );
    SETPAR( _1_1_0, p[ SINGLETAPTIMEOUT ], single_tap_timeout, PT_INT, 0, 1000 );
    SETPAR( _1_1_0, p[ VERTEDGESCROLL ],  scroll_edge_vert,  PT_BOOL, 0, 1 );
    SETPAR( _1_1_0, p[ HORIZEDGESCROLL ], scroll_edge_horiz, PT_BOOL, 0, 1 );
    SETPAR( _1_1_0, p[ VERTTWOFINGERSCROLL ],  scroll_twofinger_vert,  PT_BOOL, 0, 1 );
    SETPAR( _1_1_0, p[ HORIZTWOFINGERSCROLL ], scroll_twofinger_horiz, PT_BOOL, 0, 1 );
    SETPAR( _1_1_0, p[ PRESSMOTIONMINZ ], press_motion_min_z, PT_INT, 0, 255 );
    SETPAR( _1_1_0, p[ PRESSMOTIONMAXZ ], press_motion_max_z, PT_INT, 0, 255 );
    SETPAR( _1_1_0, p[ PRESSMOTIONMINFACTOR], press_motion_min_factor, PT_DOUBLE, 0, 10.0 );
    SETPAR( _1_1_0, p[ PRESSMOTIONMAXFACTOR], press_motion_max_factor, PT_DOUBLE, 0, 10.0 );
    SETPAR( _1_1_0, p[ FINGERPRESS ], finger_press, PT_INT, 0, 256 );
    SETPAR( _1_1_0, p[ EMULATETWOFINGERMINZ ], emulate_twofinger_z, PT_INT, 0, 1000 );
    SETPAR( _1_1_0, p[ CORNERCOASTING ], scroll_edge_corner, PT_BOOL, 0, 1 );
    SETPAR( _1_1_0, p[ TRACKSTICKSPEED ], trackstick_speed, PT_DOUBLE, 0, 200.0 );
    SETPAR( _1_1_0, p[ LOCKEDDRAGTIMEOUT ], locked_drag_time, PT_INT, 0, 30000 );
    SETPAR( _1_1_0, p[ CLICKFINGER1 ], click_action[F1_CLICK1], PT_INT, 0, SYN_MAX_BUTTONS );
    SETPAR( _1_1_0, p[ CLICKFINGER2 ], click_action[F2_CLICK1], PT_INT, 0, SYN_MAX_BUTTONS );
    SETPAR( _1_1_0, p[ CLICKFINGER3 ], click_action[F3_CLICK1], PT_INT, 0, SYN_MAX_BUTTONS );
    SETPAR( _1_1_0, p[ GRABEVENTDEVICE ], grab_event_device, PT_BOOL, 0, 1 );
    SETPAR( _1_1_0, p[ SPECIALSCROLLAREARIGHT ], special_scroll_area_right, PT_BOOL, 0, 1 );

    SETPAR( _1_1_0, p[ EMULATETWOFINGERMINW ], emulate_twofinger_w, PT_INT, 0, 15 );

    mSupportedDriver[ "1.1.0" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "1.1.0" );

    mSupportedDriver[ "1.1.1" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "1.1.1" );

    mSupportedDriver[ "1.1.2" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "1.1.2" );

    mSupportedDriver[ "1.1.3" ] = p;
    mSupportedDriverList.insert( mSupportedDriverList.end(), "1.1.3" );

    // TODO
    // insert new driver releases here!

    SYNDEBUG( "registration done!" );
}

void Synaptics::Pad::init()
{
    SYNDEBUG( "this version of libsynaptics supports the following driver versions:" );

    std::list<std::string>::iterator it = mSupportedDriverList.begin();

    for ( it =  mSupportedDriverList.begin(); it !=  mSupportedDriverList.end(); it++ )
        SYNDEBUG( " --- "<< *it );

    SYNDEBUG( "initializing fitting driver segment..." );

    //
    // detect driver version
    //
    detectDriverVersion();

    //
    // attach proper segment
    //
    int sizeOfShm;

    switch( driverVersion() )
    {
        case ( NODRIVERVER ):
            SYNERR( "cannot access driver, please install and configure it properly!" );
            mDetectedDriver = DV_UNKNOWN;
            return;

        case ( VER( 0, 14, 4 ) ):
            sizeOfShm = sizeof( ShmSegment_0_14_4 );
            break;

        case ( VER( 0, 14, 5 ) ):
            sizeOfShm = sizeof( ShmSegment_0_14_5 );
            break;

        case ( VER( 0, 14, 6 ) ):
            sizeOfShm = sizeof( ShmSegment_0_14_6 );
            break;

        case ( VER( 0, 15, 0 ) ):
            sizeOfShm = sizeof( ShmSegment_0_15_0 );
            break;

        case ( VER( 0, 15, 1 ) ):
            sizeOfShm = sizeof( ShmSegment_0_15_1 );
            break;

        case ( VER( 0, 15, 2 ) ):
            sizeOfShm = sizeof( ShmSegment_0_15_2 );
            break;

        case ( VER( 0, 99, 1 ) ):
            sizeOfShm = sizeof( ShmSegment_0_99_1 );
            break;

        case ( VER( 0, 99, 2 ) ):
            sizeOfShm = sizeof( ShmSegment_0_99_2 );
            break;

        case ( VER( 0, 99, 3 ) ):
            sizeOfShm = sizeof( ShmSegment_0_99_3 );
            break;

        case ( VER( 1, 0, 0 ) ):
            sizeOfShm = sizeof( ShmSegment_1_0_0 );
            break;

        case ( VER( 1, 1, 0 ) ):
            sizeOfShm = sizeof( ShmSegment_1_1_0 );
            break;

        case ( VER( 1, 1, 1 ) ):
            sizeOfShm = sizeof( ShmSegment_1_1_0 );
            break;

        case ( VER( 1, 1, 2 ) ):
            sizeOfShm = sizeof( ShmSegment_1_1_0 );
            break;

        case ( VER( 1, 1, 3 ) ):
            sizeOfShm = sizeof( ShmSegment_1_1_0 );
            break;

        //
        // TODO insert upcoming driver versions
        //

        default:
            if ( driverVersion() < VER( 0, 14, 4 ) )
            {
                SYNERR( "outdated or unknown driver version, please update at least to version 0.14.4!" );

                mDetectedDriver = DV_OUTDATED;
            }
            else
            {
                SYNERR( "driver is too recent, please downgrade driver or update libsynaptics!" );

                mDetectedDriver = DV_TOORECENT;
            }
            return;
    }


    mDetectedDriver = DV_SUPPORTED;

    // reconnect to the shared memory area
    int shmid;

    if ( ( shmid = shmget( SHM_SYNAPTICS, sizeOfShm, 0 ) ) == -1 )
    {
        if ( ( shmid = shmget( SHM_SYNAPTICS, 0, 0 ) ) == -1 )
        {
            SYNERR( "can't access shared memory area - SHMConfig disabled?" );
        }
        else
        {
            SYNERR( "incorrect size of shared memory area - incompatible driver version?" );
        }
    }
    else
    {
        // attach
        if ( ( mSynShm = ( SynShm* ) shmat( shmid, NULL, 0 ) ) == NULL)
        {
            perror( "shmat" );
        }
        {
            SYNDEBUG( "class is now operateable!" );
        }
    }
}
