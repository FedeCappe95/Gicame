#ifndef __GICAME__OS__OBJECTNAME_H__
#define __GICAME__OS__OBJECTNAME_H__


#include <string>
#include "../common.h"


namespace Gicame::Os {

	/**
	 * @brief Whether the operating system accepts name for a named object created by Gicame
	 *        (shared memory, mutex...)
	 *
	 * The rules are the system ones: not empty, no '/' on POSIX, no '\' on Windows, and a length
	 * limit that accounts for the prefix Gicame adds ("/" on POSIX, "Local\" on Windows).
	 */
	GICAME_API bool isValidObjectName(const std::string& name);

};


#endif
