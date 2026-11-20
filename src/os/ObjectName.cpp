#include "os/ObjectName.h"

#ifdef WINDOWS
#include <windows.h>
#else
#include <limits.h>
#endif


bool Gicame::Os::isValidObjectName(const std::string& name) {
	if (name.empty())
		return false;

#ifdef WINDOWS
	// '\' separates the kernel object namespaces, as in the "Local\" prefix.
	return name.find('\\') == std::string::npos && (std::string("Local\\") + name).length() < MAX_PATH;
#else
	// '/' is allowed only as the leading character, which Gicame adds.
	return name.find('/') == std::string::npos && name.length() < NAME_MAX;
#endif
}
