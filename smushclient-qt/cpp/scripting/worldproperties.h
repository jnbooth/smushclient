#include <string>
extern "C"
{
#include "lua.h"
}

const char *canonicalProperty(const std::string &name);
void pushPropertiesList(lua_State *L);
