#include <stdio.h>
#include <string.h>
#include "../lua/lua.h"
#include "../lua/lauxlib.h"
#include "../lua/lualib.h"
#include "../linmath.lua.h"

static int l_print (lua_State *L)
{
    const char *d = luaL_checkstring(L, 1);
    printf("%s\n", d);
    return 1;
}

int main (int argc, char **argv)
{
    int error;
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    load_linmath_lua(L);

    lua_pushcfunction(L, l_print);
    lua_setglobal(L, "print");

    lua_getglobal(L, "dofile");
    lua_pushstring(L, "test.lua");

    error = lua_pcall(L, 1, 0, 0);
    if (error) {
        fprintf(stderr, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    lua_close(L);
    return error;
}
