#include "linmath.lua.h"

#include <lauxlib.h>
#include <lualib.h>


static const char *VEC_DIMENSIONS[4] = { "x", "y", "z", "w" };


static void l_push_vec(lua_State *L, float *v, int dim, const char *metatable_name)
{
    lua_newtable(L);

    for (int i = 0; i < dim; ++i)
    {
        lua_pushstring(L, VEC_DIMENSIONS[i]);
        lua_pushnumber(L, (double)v[i]);
        lua_settable(L, -3);
    }

    lua_getglobal(L, metatable_name);
    lua_setmetatable(L, -2);
}

static void l_get_vec(lua_State *L, int stack_index, float *v, int dim)
{
    luaL_checktype(L, stack_index, LUA_TTABLE);

    for (int i = 0; i < dim; ++i)
    {
        lua_getfield(L, stack_index, VEC_DIMENSIONS[i]);
        v[i] = (float)luaL_checknumber(L, -1);
    }
}


static void l_push_vec2(lua_State *L, vec2 v) { l_push_vec(L, v, 2, "vec2_instance_meta__"); }
static void l_get_vec2(lua_State *L, int stack_index, vec2 v) { l_get_vec(L, stack_index, v, 2); }
static void l_push_vec3(lua_State *L, vec3 v) { l_push_vec(L, v, 3, "vec3_meta__"); }
static void l_get_vec3(lua_State *L, int stack_index, vec3 v) { l_get_vec(L, stack_index, v, 3); }
static void l_push_vec4(lua_State *L, vec4 v) { l_push_vec(L, v, 4, "vec4_meta__"); }
static void l_get_vec4(lua_State *L, int stack_index, vec4 v) { l_get_vec(L, stack_index, v, 4); }
static void l_push_quat(lua_State *L, quat v) { l_push_vec(L, v, 4, "quat_meta__"); }
static void l_get_quat(lua_State *L, int stack_index, quat v) { l_get_vec(L, stack_index, v, 4); }


#define DECL_BINARY_OP(T, op)         \
static int l_##T##_##op(lua_State *L) \
{                                     \
    T r, a, b;                        \
    l_get_##T (L, 1, a);              \
    l_get_##T (L, 2, b);              \
    T##_##op (r, a, b);               \
    l_push_##T (L, r);                \
    return 1;                         \
}

#define DECL_BINARY_TO_FLOAT(T, op)   \
static int l_##T##_##op(lua_State *L) \
{                                     \
    T a, b;                           \
    l_get_##T (L, 1, a);              \
    l_get_##T (L, 2, b);              \
    float r = T##_##op (a, b);        \
    lua_pushnumber(L, (double)r);     \
    return 1;                         \
}

#define DECL_BINARY_WITH_FLOAT(T, op)      \
static int l_##T##_##op(lua_State *L)      \
{                                          \
    T r, t;                                \
    float f;                               \
                                           \
    if (lua_isnumber(L, 1)) {              \
        f = (float)lua_tonumber(L, 1);     \
        l_get_##T (L, 2, t);               \
    } else {                               \
        f = (float)luaL_checknumber(L, 2); \
        l_get_##T (L, 1, t);               \
    }                                      \
                                           \
    T##_##op (r, t, f);                    \
    l_push_##T (L, r);                     \
    return 1;                              \
}

#define DECL_UNARY_OP(T, op)          \
static int l_##T##_##op(lua_State *L) \
{                                     \
    T r, a;                           \
    l_get_##T (L, 1, a);              \
    T##_##op (r, a);                  \
    l_push_##T (L, r);                \
    return 1;                         \
}

#define DECL_UNARY_TO_FLOAT(T, op)    \
static int l_##T##_##op(lua_State *L) \
{                                     \
    T a;                              \
    l_get_##T (L, 1, a);              \
    float r = T##_##op (a);           \
    lua_pushnumber(L, (double)r);     \
    return 1;                         \
}


DECL_BINARY_OP(vec2, add);
DECL_BINARY_OP(vec2, sub);
DECL_BINARY_OP(vec2, min);
DECL_BINARY_OP(vec2, max);
DECL_BINARY_WITH_FLOAT(vec2, scale);
DECL_BINARY_TO_FLOAT(vec2, mul_inner);
DECL_UNARY_OP(vec2, norm);
DECL_UNARY_TO_FLOAT(vec2, len);

DECL_BINARY_OP(vec3, add);
DECL_BINARY_OP(vec3, sub);
DECL_BINARY_OP(vec3, min);
DECL_BINARY_OP(vec3, max);
DECL_BINARY_WITH_FLOAT(vec3, scale);
DECL_BINARY_TO_FLOAT(vec3, mul_inner);
DECL_UNARY_OP(vec3, norm);
DECL_UNARY_TO_FLOAT(vec3, len);

DECL_BINARY_OP(vec4, add);
DECL_BINARY_OP(vec4, sub);
DECL_BINARY_OP(vec4, min);
DECL_BINARY_OP(vec4, max);
DECL_BINARY_WITH_FLOAT(vec4, scale);
DECL_BINARY_TO_FLOAT(vec4, mul_inner);
DECL_UNARY_OP(vec4, norm);
DECL_UNARY_TO_FLOAT(vec4, len);

// TODO rotate, mul_vec3
DECL_BINARY_OP(quat, add);
DECL_BINARY_OP(quat, sub);
DECL_BINARY_OP(quat, mul);
DECL_BINARY_WITH_FLOAT(quat, scale);
DECL_BINARY_TO_FLOAT(quat, inner_product);
DECL_UNARY_OP(quat, conj);
DECL_UNARY_OP(quat, norm);


static int l_vec2(lua_State *L)
{
    vec2 v = {
        lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0,
        lua_isnumber(L, 3) ? lua_tonumber(L, 3) : 0,
    };
    l_push_vec2(L, v);
    return 1;
}

static int l_vec3(lua_State *L)
{
    vec3 v = {
        lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0,
        lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0,
        lua_isnumber(L, 3) ? lua_tonumber(L, 3) : 0
    };
    l_push_vec3(L, v);
    return 1;
}

static int l_vec4(lua_State *L)
{
    vec4 v = {
        lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0,
        lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0,
        lua_isnumber(L, 3) ? lua_tonumber(L, 3) : 0,
        lua_isnumber(L, 4) ? lua_tonumber(L, 3) : 0
    };
    l_push_vec4(L, v);
    return 1;
}

static int l_quat(lua_State *L)
{
    quat v = {
        lua_isnumber(L, 1) ? lua_tonumber(L, 1) : 0,
        lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0,
        lua_isnumber(L, 3) ? lua_tonumber(L, 3) : 0,
        lua_isnumber(L, 4) ? lua_tonumber(L, 3) : 1
    };
    l_push_quat(L, v);
    return 1;
}


typedef struct FuncDef
{
    const char *name;
    lua_CFunction func;
}
FuncDef;

typedef struct TypeDef
{
    const char *type_name;
    const char *instance_metatable_name;
    const char *static_metatable_name;
    lua_CFunction constructor;

    int num_instance_meta_funcs;
    const FuncDef *instance_meta_funcs;

    int num_instance_funcs;
    const FuncDef *instance_funcs;

    int num_static_funcs;
    const FuncDef *static_funcs;
}
TypeDef;


#define DECL_ARRAY_SIZE(size_var, array) static const size_t size_var = (sizeof(array)/sizeof(array[0]));

static const FuncDef vec2_instance_meta_funcs[] = {
    { "__add", l_vec2_add },
    { "__sub", l_vec2_sub },
    { "__mul", l_vec2_scale },
};
DECL_ARRAY_SIZE(num_vec2_instance_meta_funcs, vec2_instance_meta_funcs);

static const FuncDef vec2_instance_funcs[] = {
    { "__add", l_vec2_add },
};
DECL_ARRAY_SIZE(num_vec2_instance_funcs, vec2_instance_funcs);

static const FuncDef vec2_static_funcs[] = {
    { "min", l_vec2_min },
    { "max", l_vec2_max },
};
DECL_ARRAY_SIZE(num_vec2_static_funcs, vec2_static_funcs);


static void build_type(lua_State *L, const TypeDef *type_def)
{
    lua_newtable(L);
        for (int i = 0; i < type_def->num_instance_meta_funcs; ++i)
        {
            lua_pushstring(L, type_def->instance_meta_funcs[i].name);
            lua_pushcfunction(L, type_def->instance_meta_funcs[i].func);
            lua_settable(L, -3);
        }
        lua_pushstring(L, "__index");
        lua_newtable(L);
            for (int i = 0; i < type_def->num_instance_funcs; ++i)
            {
                lua_pushstring(L, type_def->instance_funcs[i].name);
                lua_pushcfunction(L, type_def->instance_funcs[i].func);
                lua_settable(L, -3);
            }
        lua_settable(L, -3);
    lua_setglobal(L, type_def->instance_metatable_name);

    lua_newtable(L);
        lua_pushstring(L, "__call");
        lua_pushcfunction(L, type_def->constructor);
        lua_settable(L, -3);

        lua_pushstring(L, "__index");
        lua_newtable(L);
            for (int i = 0; i < type_def->num_static_funcs; ++i)
            {
                lua_pushstring(L, type_def->static_funcs[i].name);
                lua_pushcfunction(L, type_def->static_funcs[i].func);
                lua_settable(L, -3);
            }
        lua_settable(L, -3);
    lua_setglobal(L, type_def->static_metatable_name);

    lua_newtable(L);
    lua_getglobal(L, type_def->static_metatable_name);
    lua_setmetatable(L, -2);
    lua_setglobal(L, type_def->type_name);
}


#define BUILD_TYPE(T, L) do { \
    TypeDef type_def = { \
        #T, #T"_instance_meta__", #T"_static_meta__", \
        l_##T, \
        num_##T##_instance_meta_funcs, T##_instance_meta_funcs, \
        num_##T##_instance_funcs,      T##_instance_funcs, \
        num_##T##_static_funcs,        T##_static_funcs, \
    }; \
    build_type((L), &type_def); \
} while (0)

void load_linmath_lua(lua_State *L)
{
    BUILD_TYPE(vec2, L);
}
