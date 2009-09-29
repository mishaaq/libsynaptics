#ifndef PAD_H
#define PAD_H

#include "syndebug.h"
#include "synshm.h"
#include "synaptics.h"

#include <pthread.h>
#include <map>
#include <list>

namespace Synaptics
{

class Pad
{
    public:
        // get an instance
        static Pad* getInstance();

        // empty destructor, kills the static instance anyways
        ~Pad();

    private:
        static pthread_mutex_t mMutex;
        static Pad mInstance;

        // constructor private
        Pad();

        // protect copy operator 
        Pad( const Pad& cc );

        // get instance (not thread safe)
        static Synaptics::Pad* getInstanceProtected();


        //
        //
        //

        
        // register support for several driver versions' parameters 
        void registerParameters();

        // initialize the fitting shm segment and attach it
        void init();

        // detect driver version by reading "version" member from the SHM
        // receive: whether a driver could be detected
        bool detectDriverVersion();

        // create a version string from an integer value
        // give: version (integer value)
        // receive: version string, if no driver is present it returns "None"
        std::string intVerToStrVer( int );

        // whether shm access is possible
        static bool mHasShm;

        // store type of detected driver version
        static DriverVersion mDetectedDriver;

        static SynShm* mSynShm;

        typedef std::map<std::string, Synaptics::Param> Parameter;
        static std::map<std::string, Parameter> mSupportedDriver;
        static std::list<std::string> mSupportedDriverList;

    public:
        // is pad hardware present?
        // receive: whether a touch pad is present
        // TODO implement me (I don't know if it's even possible!)
        bool hasHardware();

        // is a driver present?
        // receive: whether a usable driver has been detected
        bool hasDriver();

        // is shared memory accessible?
        // receive: whether shared memory access is possible
        bool hasShm();

        // returns kind of detected driver
        // receive: kind of detected driver
        DriverVersion driverKind();

        // returns version number string of detected driver
        // receive: version number string of detected driver
        std::string driverStr();

        // returns version number string of used library
        static std::string libraryStr();

        // returns version number ( 10000 * major + 100 * minor + 1 * rev )
        // receive: version number ( 10000 * major + 100 * minor + 1 * rev )
        int driverVersion();

        // returns pad type
        // receive: pad type
        // TODO implement me ( I don't know if it's even possible!)
        PadType getPadType();

        // returns whether a given parameter is available
        // give: parameter name
        // receive: whether the parameter is available
        bool hasParam( std::string );

        // returns a given parameter, if the parameter is not available returns 0
        // give: name of parameter
        // receive: given parameter value, if it doesn't exist 0
        double getParam( std::string );

        // sets a given parameter if available
        // give: a parameter's name and the correspondent value
        void setParam( std::string, double );

        // gets max value of a parameter
        // give: name of parameter
        // receive: max value
        double getParamMax( std::string );

        // gets min value of a parameter
        // give: name of parameter
        // receive: min value
        double getParamMin( std::string );

        // gets type of a parameter
        // give: name of parameter
        // receive: type of parameter - PT_VOID if no parameter at all!
        Synaptics::ParamType getParamType( std::string );

        // returns a string list containing all available parameters
        // receive: string list containing all available parameters
        std::list<std::string> getAvailableParams();

        //
        // the following procedures are only neccessary for qsynaptics and old versions of ksynaptics
        //

        // is the system capable of running synclient?
        // receive: whether synclient is runnable
        // TODO: implement me
        bool hasSynClient();

        // is the system capable of running syndaemon?
        // receive: whether syndaemon is runnable
        // TODO implement me
        bool hasSynDaemon();

        // is syndaemon being run?
        // receive: whether syndaemon is being run
        // TODO implement me
        bool isSynDaemonRunning();

        // kill syndaemon processes
        // receive: whether killing succeeded
        // TODO implement me
        bool killSynDaemonProcesses();
};
}

#endif

