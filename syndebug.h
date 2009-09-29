#ifndef SYNDEBUG_H
#define SYNDEBUG_H

#include <iostream>

#define DEBUGMSG
#define ERRMSG

#ifdef DEBUGMSG
  #define SYNDEBUG(MSG) std::cout << __FUNCTION__ << "("<< __FILE__ << ":" << __LINE__ << ")\t --- " << MSG << std::endl;
#else
  #define SYNDEBUG(MSG)
#endif

#ifdef ERRMSG
  #define SYNERR(MSG) std::cerr << __FUNCTION__ << "(" << __FILE__ << ":" << __LINE__ << ")\t --- " << MSG << std::endl;
#else
  #define SYNERR(MSG)
#endif

#endif
