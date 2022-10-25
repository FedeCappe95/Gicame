#ifndef __ERRORHANDLING_H__
#define __ERRORHANDLING_H__


#include <string>
#include "../common.h"


namespace Gicame::Error {

    GICAME_API int getLastCode();
    GICAME_API std::string getLastErrorAsString();

};


#endif
