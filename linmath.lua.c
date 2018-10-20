#include "linmath.lua.h"
#include <lauxlib.h>
#include <lualib.h>

// =====  read/write on lua stack  =============================================

static const char *VEC_DIMENSIONS[] = { "x", "y", "z", "w" };
static const char *COLUMN_NAMES[] = { "c0", "c1", "c2", "c3" };

static void lml_push_vec(lua_State *L, float *v, int dim, const char *metatable_name)
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

static void lml_get_vec(lua_State *L, int stack_index, float *v, int dim)
{
    luaL_checktype(L, stack_index, LUA_TTABLE);

    for (int i = 0; i < dim; ++i)
    {
        lua_getfield(L, stack_index, VEC_DIMENSIONS[i]);
        v[i] = (float)luaL_checknumber(L, -1);
    }
}

void lml_push_vec2(lua_State *L, vec2 v) { lml_push_vec(L, v, 2, "vec2_instance_meta__"); }
void lml_get_vec2(lua_State *L, int stack_index, vec2 v) { lml_get_vec(L, stack_index, v, 2); }
void lml_push_vec3(lua_State *L, vec3 v) { lml_push_vec(L, v, 3, "vec3_instance_meta__"); }
void lml_get_vec3(lua_State *L, int stack_index, vec3 v) { lml_get_vec(L, stack_index, v, 3); }
void lml_push_vec4(lua_State *L, vec4 v) { lml_push_vec(L, v, 4, "vec4_instance_meta__"); }
void lml_get_vec4(lua_State *L, int stack_index, vec4 v) { lml_get_vec(L, stack_index, v, 4); }
void lml_push_quat(lua_State *L, quat v) { lml_push_vec(L, v, 4, "quat_instance_meta__"); }
void lml_get_quat(lua_State *L, int stack_index, quat v) { lml_get_vec(L, stack_index, v, 4); }

void lml_push_mat4x4(lua_State *L, mat4x4 m)
{
    lua_newtable(L);

    for (int i = 0; i < 4; ++i)
    {
        lua_pushstring(L, COLUMN_NAMES[i]);
        lml_push_vec4(L, m[i]);
        lua_settable(L, -3);
    }

    lua_getglobal(L, "mat4x4_instance_meta__");
    lua_setmetatable(L, -2);
}

void lml_get_mat4x4(lua_State *L, int stack_index, mat4x4 m)
{
    luaL_checktype(L, stack_index, LUA_TTABLE);

    for (int i = 0; i < 4; ++i)
    {
        lua_getfield(L, stack_index, COLUMN_NAMES[i]);
        lml_get_vec4(L, -1, m[i]);
    }
}

// =====  operations  ==========================================================

typedef float vec1[1];

static void lml_push_vec1(lua_State *L, vec1 v) { lua_pushnumber(L, (double)v[0]); }
static void lml_get_vec1(lua_State *L, int stack_index, vec1 v) { v[0] = (float)luaL_checknumber(L, stack_index); }

#define DECL_UNARY_OP(R, A, op) \
static int l_##op(lua_State *L) \
{                               \
    R r;                        \
    A a;                        \
    lml_get_##A (L, 1, a);      \
    op(r, a);                   \
    lml_push_##R (L, r);        \
    return 1;                   \
}

#define DECL_BINARY_OP(R, A, B, op) \
static int l_##op(lua_State *L)     \
{                                   \
    R r;                            \
    A a;                            \
    B b;                            \
    lml_get_##A (L, 1, a);          \
    lml_get_##B (L, 2, b);          \
    op(r, a, b);                    \
    lml_push_##R (L, r);            \
    return 1;                       \
}

#define DECL_BINARY_WITH_FLOAT(R, A, op)   \
static int l_##op(lua_State *L)            \
{                                          \
    R r;                                   \
    A a;                                   \
    float f;                               \
                                           \
    if (lua_isnumber(L, 1)) {              \
        f = (float)lua_tonumber(L, 1);     \
        lml_get_##A (L, 2, a);             \
    } else {                               \
        f = (float)luaL_checknumber(L, 2); \
        lml_get_##A (L, 1, a);             \
    }                                      \
                                           \
    op (r, a, f);                          \
    lml_push_##R (L, r);                   \
    return 1;                              \
}

#define DECL_BINARY_TO_FLOAT_WRAPPER(T, name) \
    static void T##_##name##_v1(float *r, const T a, const T b) { *r = T##_##name(a, b); }

DECL_BINARY_TO_FLOAT_WRAPPER(vec2, mul_inner);
DECL_BINARY_TO_FLOAT_WRAPPER(vec3, mul_inner);
DECL_BINARY_TO_FLOAT_WRAPPER(vec4, mul_inner);
DECL_BINARY_TO_FLOAT_WRAPPER(quat, inner_product);

#define DECL_UNARY_TO_FLOAT_WRAPPER(T, name) \
    static void T##_##name##_v1(float *r, const T a) { *r = T##_##name(a); }

DECL_UNARY_TO_FLOAT_WRAPPER(vec2, len);
DECL_UNARY_TO_FLOAT_WRAPPER(vec3, len);
DECL_UNARY_TO_FLOAT_WRAPPER(vec4, len);

DECL_BINARY_OP(vec2, vec2, vec2, vec2_add);
DECL_BINARY_OP(vec2, vec2, vec2, vec2_sub);
DECL_BINARY_OP(vec2, vec2, vec2, vec2_min);
DECL_BINARY_OP(vec2, vec2, vec2, vec2_max);
DECL_BINARY_WITH_FLOAT(vec2, vec2, vec2_scale);
DECL_BINARY_OP(vec1, vec2, vec2, vec2_mul_inner_v1);
DECL_UNARY_OP(vec2, vec2, vec2_norm);
DECL_UNARY_OP(vec1, vec2, vec2_len_v1);

DECL_BINARY_OP(vec3, vec3, vec3, vec3_add);
DECL_BINARY_OP(vec3, vec3, vec3, vec3_sub);
DECL_BINARY_OP(vec3, vec3, vec3, vec3_min);
DECL_BINARY_OP(vec3, vec3, vec3, vec3_max);
DECL_BINARY_WITH_FLOAT(vec3, vec3, vec3_scale);
DECL_BINARY_OP(vec1, vec3, vec3, vec3_mul_inner_v1);
DECL_UNARY_OP(vec3, vec3, vec3_norm);
DECL_UNARY_OP(vec1, vec3, vec3_len_v1);
DECL_BINARY_OP(vec3, vec3, vec3, vec3_mul_cross);
DECL_BINARY_OP(vec3, vec3, vec3, vec3_reflect);

DECL_BINARY_OP(vec4, vec4, vec4, vec4_add);
DECL_BINARY_OP(vec4, vec4, vec4, vec4_sub);
DECL_BINARY_OP(vec4, vec4, vec4, vec4_min);
DECL_BINARY_OP(vec4, vec4, vec4, vec4_max);
DECL_BINARY_WITH_FLOAT(vec4, vec4, vec4_scale);
DECL_BINARY_OP(vec1, vec4, vec4, vec4_mul_inner_v1);
DECL_UNARY_OP(vec4, vec4, vec4_norm);
DECL_UNARY_OP(vec1, vec4, vec4_len_v1);
DECL_BINARY_OP(vec4, vec4, vec4, vec4_mul_cross);
DECL_BINARY_OP(vec4, vec4, vec4, vec4_reflect);

static void quat_rotate_v1(quat r, float *angle, vec3 axis) { quat_rotate(r, *angle, axis); }

DECL_BINARY_OP(quat, quat, quat, quat_add);
DECL_BINARY_OP(quat, quat, quat, quat_sub);
DECL_BINARY_OP(quat, quat, quat, quat_mul);
DECL_BINARY_WITH_FLOAT(quat, quat, quat_scale);
DECL_BINARY_OP(vec1, quat, quat, quat_inner_product_v1);
DECL_UNARY_OP(quat, quat, quat_conj);
DECL_UNARY_OP(quat, quat, quat_norm);
DECL_BINARY_OP(vec3, quat, vec3, quat_mul_vec3);
DECL_BINARY_OP(quat, vec1, vec3, quat_rotate_v1);
DECL_UNARY_OP(quat, mat4x4, quat_from_mat4x4);

DECL_UNARY_OP(mat4x4, mat4x4, mat4x4_dup);
//TODO mat4x4_row
//TODO mat4x4_col
DECL_UNARY_OP(mat4x4, mat4x4, mat4x4_transpose);
DECL_BINARY_OP(mat4x4, mat4x4, mat4x4, mat4x4_add);
DECL_BINARY_OP(mat4x4, mat4x4, mat4x4, mat4x4_sub);
DECL_BINARY_WITH_FLOAT(mat4x4, mat4x4, mat4x4_scale);
// TODO mat4x4_scale_aniso
DECL_BINARY_OP(mat4x4, mat4x4, mat4x4, mat4x4_mul);
DECL_BINARY_OP(vec4, mat4x4, vec4, mat4x4_mul_vec4);
// TODO mat4x4_translate
// TODO mat4x4_translate_in_place
DECL_BINARY_OP(mat4x4, vec3, vec3, mat4x4_from_vec3_mul_outer);
// TODO mat4x4_rotate
// TODO mat4x4_rotate_X
// TODO mat4x4_rotate_Y
// TODO mat4x4_rotate_Z
DECL_UNARY_OP(mat4x4, mat4x4, mat4x4_invert);
DECL_UNARY_OP(mat4x4, mat4x4, mat4x4_orthonormalize);
// TODO mat4x4_frustum
// TODO mat4x4_ortho
// TODO mat4x4_perspective
// TODO mat4x4_look_at
DECL_UNARY_OP(mat4x4, quat, mat4x4_from_quat);
DECL_BINARY_OP(mat4x4, mat4x4, quat, mat4x4o_mul_quat);

// =====  constructors  ========================================================

static int l_vec2(lua_State *L)
{
    vec2 v = {
        lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0,
        lua_isnumber(L, 3) ? lua_tonumber(L, 3) : 0,
    };
    lml_push_vec2(L, v);
    return 1;
}

static int l_vec3(lua_State *L)
{
    vec3 v = {
        lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0,
        lua_isnumber(L, 3) ? lua_tonumber(L, 3) : 0,
        lua_isnumber(L, 4) ? lua_tonumber(L, 4) : 0
    };
    lml_push_vec3(L, v);
    return 1;
}

static int l_vec4(lua_State *L)
{
    vec4 v = {
        lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0,
        lua_isnumber(L, 3) ? lua_tonumber(L, 3) : 0,
        lua_isnumber(L, 4) ? lua_tonumber(L, 4) : 0,
        lua_isnumber(L, 5) ? lua_tonumber(L, 5) : 0
    };
    lml_push_vec4(L, v);
    return 1;
}

static int l_quat(lua_State *L)
{
    quat v = {
        lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0,
        lua_isnumber(L, 3) ? lua_tonumber(L, 3) : 0,
        lua_isnumber(L, 4) ? lua_tonumber(L, 4) : 0,
        lua_isnumber(L, 5) ? lua_tonumber(L, 5) : 1
    };
    lml_push_quat(L, v);
    return 1;
}

static int l_mat4x4(lua_State *L)
{
    mat4x4 v;
    mat4x4_identity(v);
    lml_push_mat4x4(L, v);
    return 1;
}

// =====  lua type definitions  ================================================

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

// -----  vec2  -----

static const FuncDef vec2_instance_meta_funcs[] = {
    { "__add", l_vec2_add },
    { "__sub", l_vec2_sub },
    { "__mul", l_vec2_scale },
};
DECL_ARRAY_SIZE(num_vec2_instance_meta_funcs, vec2_instance_meta_funcs);

static const FuncDef vec2_instance_funcs[] = {
    { "dot", l_vec2_mul_inner_v1 },
    { "normalized", l_vec2_norm },
    { "length", l_vec2_len_v1 },
};
DECL_ARRAY_SIZE(num_vec2_instance_funcs, vec2_instance_funcs);

static const FuncDef vec2_static_funcs[] = {
    { "min", l_vec2_min },
    { "max", l_vec2_max },
};
DECL_ARRAY_SIZE(num_vec2_static_funcs, vec2_static_funcs);

// -----  vec3  -----

static const FuncDef vec3_instance_meta_funcs[] = {
    { "__add", l_vec3_add },
    { "__sub", l_vec3_sub },
    { "__mul", l_vec3_scale },
};
DECL_ARRAY_SIZE(num_vec3_instance_meta_funcs, vec3_instance_meta_funcs);

static const FuncDef vec3_instance_funcs[] = {
    { "dot", l_vec3_mul_inner_v1 },
    { "cross", l_vec3_mul_cross },
    { "reflect", l_vec3_reflect },
    { "normalized", l_vec3_norm },
    { "length", l_vec3_len_v1 },
};
DECL_ARRAY_SIZE(num_vec3_instance_funcs, vec3_instance_funcs);

static const FuncDef vec3_static_funcs[] = {
    { "min", l_vec3_min },
    { "max", l_vec3_max },
};
DECL_ARRAY_SIZE(num_vec3_static_funcs, vec3_static_funcs);

// -----  vec4  -----

static const FuncDef vec4_instance_meta_funcs[] = {
    { "__add", l_vec4_add },
    { "__sub", l_vec4_sub },
    { "__mul", l_vec4_scale },
};
DECL_ARRAY_SIZE(num_vec4_instance_meta_funcs, vec4_instance_meta_funcs);

static const FuncDef vec4_instance_funcs[] = {
    { "dot", l_vec4_mul_inner_v1 },
    { "cross", l_vec4_mul_cross },
    { "reflect", l_vec4_reflect },
    { "normalized", l_vec4_norm },
    { "length", l_vec4_len_v1 },
};
DECL_ARRAY_SIZE(num_vec4_instance_funcs, vec4_instance_funcs);

static const FuncDef vec4_static_funcs[] = {
    { "min", l_vec4_min },
    { "max", l_vec4_max },
};
DECL_ARRAY_SIZE(num_vec4_static_funcs, vec4_static_funcs);

// -----  quat  -----

static const FuncDef quat_instance_meta_funcs[] = {
    { "__add", l_quat_add },
    { "__sub", l_quat_sub },
    { "__mul", l_quat_mul },
};
DECL_ARRAY_SIZE(num_quat_instance_meta_funcs, quat_instance_meta_funcs);

static const FuncDef quat_instance_funcs[] = {
    { "scale", l_quat_scale },
    { "conjugate", l_quat_conj },
    { "inner_product", l_quat_inner_product_v1 },
    { "normalized", l_quat_norm },
    { "mul", l_quat_mul_vec3 },
};
DECL_ARRAY_SIZE(num_quat_instance_funcs, quat_instance_funcs);

static const FuncDef quat_static_funcs[] = {
    { "angle_axis", l_quat_rotate_v1 },
    { "from_mat4x4", l_quat_from_mat4x4 },
};
DECL_ARRAY_SIZE(num_quat_static_funcs, quat_static_funcs);

// -----  mat4x4  -----

static const FuncDef mat4x4_instance_meta_funcs[] = {
    { "__add", l_vec3_add },
};
DECL_ARRAY_SIZE(num_mat4x4_instance_meta_funcs, mat4x4_instance_meta_funcs);

static const FuncDef mat4x4_instance_funcs[] = {
    { "xxx", l_vec3_add },
};
DECL_ARRAY_SIZE(num_mat4x4_instance_funcs, mat4x4_instance_funcs);

static const FuncDef mat4x4_static_funcs[] = {
    { "xxx", l_vec3_add },
};
DECL_ARRAY_SIZE(num_mat4x4_static_funcs, mat4x4_static_funcs);

// =============================================================================

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

#define BUILD_TYPE(T, L) do {                                   \
    TypeDef type_def = {                                        \
        #T, #T"_instance_meta__", #T"_static_meta__",           \
        l_##T,                                                  \
        num_##T##_instance_meta_funcs, T##_instance_meta_funcs, \
        num_##T##_instance_funcs,      T##_instance_funcs,      \
        num_##T##_static_funcs,        T##_static_funcs,        \
    };                                                          \
    build_type((L), &type_def);                                 \
} while (0)

void lml_load_types(lua_State *L)
{
    BUILD_TYPE(vec2, L);
    BUILD_TYPE(vec3, L);
    BUILD_TYPE(vec4, L);
    BUILD_TYPE(quat, L);
    BUILD_TYPE(mat4x4, L);
}