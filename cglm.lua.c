#include "cglm.lua.h"
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
        lua_pop(L, 1);
    }
}

void lml_push_vec2(lua_State *L, vec2 v) { lml_push_vec(L, v, 2, "vec2_instance_meta__"); }
void lml_get_vec2(lua_State *L, int stack_index, vec2 v) { lml_get_vec(L, stack_index, v, 2); }
void lml_push_vec3(lua_State *L, vec3 v) { lml_push_vec(L, v, 3, "vec3_instance_meta__"); }
void lml_get_vec3(lua_State *L, int stack_index, vec3 v) { lml_get_vec(L, stack_index, v, 3); }
void lml_push_vec4(lua_State *L, vec4 v) { lml_push_vec(L, v, 4, "vec4_instance_meta__"); }
void lml_get_vec4(lua_State *L, int stack_index, vec4 v) { lml_get_vec(L, stack_index, v, 4); }
void lml_push_quat(lua_State *L, versor v) { lml_push_vec(L, v, 4, "quat_instance_meta__"); }
void lml_get_quat(lua_State *L, int stack_index, versor v) { lml_get_vec(L, stack_index, v, 4); }

void lml_push_mat4(lua_State *L, mat4 m)
{
    lua_newtable(L);

    for (int i = 0; i < 4; ++i)
    {
        lua_pushstring(L, COLUMN_NAMES[i]);
        lml_push_vec4(L, m[i]);
        lua_settable(L, -3);
    }

    lua_getglobal(L, "mat4_instance_meta__");
    lua_setmetatable(L, -2);
}

void lml_get_mat4(lua_State *L, int stack_index, mat4 m)
{
    luaL_checktype(L, stack_index, LUA_TTABLE);

    for (int i = 0; i < 4; ++i)
    {
        lua_getfield(L, stack_index, COLUMN_NAMES[i]);
        lml_get_vec4(L, -1, m[i]);
        lua_pop(L, 1);
    }
}

// =====  vec2 functions  ======================================================

static void ex_vec2_add(vec2 a, vec2 b, vec2 r)
{
    r[0] = a[0] + b[0];
    r[1] = a[1] + b[1];
}
static void ex_vec2_sub(vec2 a, vec2 b, vec2 r)
{
    r[0] = a[0] - b[0];
    r[1] = a[1] - b[1];
}
static void ex_vec2_scale(vec2 a, float b, vec2 r)
{
    r[0] = a[0] * b;
    r[1] = a[1] * b;
}
static void ex_vec2_dot(vec2 a, vec2 b, float *r)
{
    *r = a[0]*b[0] + a[1]*b[1];
}
static void ex_vec2_cross(vec2 a, vec2 b, float *r)
{
    *r = a[0]*b[1] - a[1]*b[0];
}
static void ex_vec2_norm2(vec2 a, float *r)
{
    ex_vec2_dot(a, a, r);
}
static void ex_vec2_normalize_to(vec2 a, vec2 r)
{
    float sqr_len;
    ex_vec2_norm2(a, &sqr_len);
    ex_vec2_scale(a, 1.f / sqrtf(sqr_len), r);
}

// =====  operations  ==========================================================

typedef float vec1[1];

static void lml_push_vec1(lua_State *L, vec1 v) { lua_pushnumber(L, (double)v[0]); }
static void lml_get_vec1(lua_State *L, int stack_index, vec1 v) { v[0] = (float)luaL_checknumber(L, stack_index); }

#define DECL_UNARY_OP(A, R, op) \
static int l_##op(lua_State *L) \
{                               \
    R r;                        \
    A a;                        \
    lml_get_##A (L, 1, a);      \
    op(a, r);                   \
    lml_push_##R (L, r);        \
    return 1;                   \
}

#define DECL_BINARY_OP(A, B, R, op) \
static int l_##op(lua_State *L)     \
{                                   \
    R r;                            \
    A a;                            \
    B b;                            \
    lml_get_##A (L, 1, a);          \
    lml_get_##B (L, 2, b);          \
    op(a, b, r);                    \
    lml_push_##R (L, r);            \
    return 1;                       \
}

#define DECL_BINARY_WITH_FLOAT(A, R, op)   \
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
    op (a, f, r);                          \
    lml_push_##R (L, r);                   \
    return 1;                              \
}

#define DECL_BINARY_TO_FLOAT_WRAPPER(T, name) \
    static void name##_wrap(const T a, const T b, float *r) { *r = name(a, b); }

#define DECL_UNARY_TO_FLOAT_WRAPPER(T, name) \
    static void name##_wrap(const T a, float *r) { *r = name(a); }


DECL_BINARY_OP(vec2, vec2, vec2, ex_vec2_add);
DECL_BINARY_OP(vec2, vec2, vec2, ex_vec2_sub);
DECL_BINARY_WITH_FLOAT(vec2, vec2, ex_vec2_scale);
DECL_BINARY_OP(vec2, vec2, vec1, ex_vec2_dot);
DECL_BINARY_OP(vec2, vec2, vec1, ex_vec2_cross);
DECL_UNARY_OP(vec2, vec2, ex_vec2_normalize_to);
DECL_UNARY_OP(vec2, vec1, ex_vec2_norm2);


DECL_BINARY_TO_FLOAT_WRAPPER(vec3, glm_vec_dot);
DECL_UNARY_TO_FLOAT_WRAPPER(vec3, glm_vec_norm2);

DECL_BINARY_OP(vec3, vec3, vec3, glm_vec_add);
DECL_BINARY_OP(vec3, vec3, vec3, glm_vec_sub);
DECL_BINARY_WITH_FLOAT(vec3, vec3, glm_vec_scale);
DECL_BINARY_OP(vec3, vec3, vec1, glm_vec_dot_wrap);
DECL_UNARY_OP(vec3, vec3, glm_vec_normalize_to);
DECL_UNARY_OP(vec3, vec1, glm_vec_norm2_wrap);
DECL_BINARY_OP(vec3, vec3, vec3, glm_vec_cross);


DECL_BINARY_TO_FLOAT_WRAPPER(vec4, glm_vec4_dot);
DECL_UNARY_TO_FLOAT_WRAPPER(vec4, glm_vec4_norm2);

DECL_BINARY_OP(vec4, vec4, vec4, glm_vec4_add);
DECL_BINARY_OP(vec4, vec4, vec4, glm_vec4_sub);
DECL_BINARY_WITH_FLOAT(vec4, vec4, glm_vec4_scale);
DECL_BINARY_OP(vec4, vec4, vec1, glm_vec4_dot_wrap);
DECL_UNARY_OP(vec4, vec4, glm_vec4_normalize_to);
DECL_UNARY_OP(vec4, vec1, glm_vec4_norm2_wrap);

/*
DECL_BINARY_TO_FLOAT_WRAPPER(quat, glm_quat_dot);
static void quat_rotate_wrap(quat r, float *angle, vec3 axis) { quat_rotate(r, *angle, axis); }

DECL_BINARY_OP(quat, quat, quat, quat_add);
DECL_BINARY_OP(quat, quat, quat, quat_sub);
DECL_BINARY_OP(quat, quat, quat, quat_mul);
DECL_BINARY_WITH_FLOAT(quat, quat, quat_scale);
DECL_BINARY_OP(vec1, quat, quat, quat_inner_product_wrap);
DECL_UNARY_OP(quat, quat, quat_conj);
DECL_UNARY_OP(quat, quat, quat_norm);
DECL_BINARY_OP(vec3, quat, vec3, quat_mul_vec3);
DECL_BINARY_OP(quat, vec1, vec3, quat_rotate_wrap);
DECL_UNARY_OP(quat, mat4, quat_from_mat4);

static void mat4_row_wrap(vec4 r, mat4 M, float *i) { mat4_row(r, M, (int)(*i)); }
static void mat4_rotate_X_wrap(mat4 Q, mat4 M, float *angle) { mat4_rotate_X(Q, M, *angle); }
static void mat4_rotate_Y_wrap(mat4 Q, mat4 M, float *angle) { mat4_rotate_Y(Q, M, *angle); }
static void mat4_rotate_Z_wrap(mat4 Q, mat4 M, float *angle) { mat4_rotate_Z(Q, M, *angle); }

DECL_UNARY_OP(mat4, mat4, mat4_dup);
DECL_UNARY_OP(mat4, mat4, mat4_transpose);
DECL_BINARY_OP(vec4, mat4, vec1, mat4_row_wrap);
DECL_BINARY_OP(mat4, mat4, mat4, mat4_add);
DECL_BINARY_OP(mat4, mat4, mat4, mat4_sub);
DECL_BINARY_WITH_FLOAT(mat4, mat4, mat4_scale);
DECL_BINARY_OP(mat4, mat4, mat4, mat4_mul);
DECL_BINARY_OP(vec4, mat4, vec4, mat4_mul_vec4);
DECL_BINARY_OP(mat4, vec3, vec3, mat4_from_vec3_mul_outer);
DECL_BINARY_OP(mat4, mat4, vec1, mat4_rotate_X_wrap);
DECL_BINARY_OP(mat4, mat4, vec1, mat4_rotate_Y_wrap);
DECL_BINARY_OP(mat4, mat4, vec1, mat4_rotate_Z_wrap);
DECL_UNARY_OP(mat4, mat4, mat4_invert);
DECL_UNARY_OP(mat4, mat4, mat4_orthonormalize);
DECL_UNARY_OP(mat4, quat, mat4_from_quat);
DECL_BINARY_OP(mat4, mat4, quat, mat4o_mul_quat);

// TODO mat4_scale_aniso(mat4 M, mat4 a, float x, float y, float z)
// TODO mat4_translate(mat4 T, float x, float y, float z)
// TODO mat4_translate_in_place(mat4 T, float x, float y, float z) // T is in/out
// TODO mat4_rotate(mat4 R, mat4 M, float x, float y, float z, float angle)
// TODO mat4_frustum(mat4 M, float l, float r, float b, float t, float n, float f)
// TODO mat4_ortho(mat4 M, float l, float r, float b, float t, float n, float f)
// TODO mat4_perspective(mat4 m, float y_fov, float aspect, float n, float f)
// TODO mat4_look_at(mat4 m, vec3 eye, vec3 center, vec3 up)
*/

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

/*
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

static int l_mat4(lua_State *L)
{
    mat4 v;
    mat4_identity(v);
    lml_push_mat4(L, v);
    return 1;
}
*/
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
    { "__add", l_ex_vec2_add },
    { "__sub", l_ex_vec2_sub },
    { "__mul", l_ex_vec2_scale },
};
DECL_ARRAY_SIZE(num_vec2_instance_meta_funcs, vec2_instance_meta_funcs);

static const FuncDef vec2_instance_funcs[] = {
    { "dot", l_ex_vec2_dot },
    { "cross", l_ex_vec2_cross },
    { "normalize", l_ex_vec2_normalize_to },
    { "sqrLength", l_ex_vec2_norm2 },
};
DECL_ARRAY_SIZE(num_vec2_instance_funcs, vec2_instance_funcs);

static const FuncDef vec2_static_funcs[] = { 0 };
static const size_t num_vec2_static_funcs = 0;
//DECL_ARRAY_SIZE(num_vec2_static_funcs, vec2_static_funcs);

// -----  vec3  -----

static const FuncDef vec3_instance_meta_funcs[] = {
    { "__add", l_glm_vec_add },
    { "__sub", l_glm_vec_sub },
    { "__mul", l_glm_vec_scale },
};
DECL_ARRAY_SIZE(num_vec3_instance_meta_funcs, vec3_instance_meta_funcs);

static const FuncDef vec3_instance_funcs[] = {
    { "dot", l_glm_vec_dot_wrap },
    { "cross", l_glm_vec_cross },
    { "normalize", l_glm_vec_normalize_to },
    { "sqrLength", l_glm_vec_norm2_wrap },
};
DECL_ARRAY_SIZE(num_vec3_instance_funcs, vec3_instance_funcs);

static const FuncDef vec3_static_funcs[] = { 0 };
static const size_t num_vec3_static_funcs = 0;
//DECL_ARRAY_SIZE(num_vec3_static_funcs, vec3_static_funcs);

// -----  vec4  -----

static const FuncDef vec4_instance_meta_funcs[] = {
    { "__add", l_glm_vec4_add },
    { "__sub", l_glm_vec4_sub },
    { "__mul", l_glm_vec4_scale },
};
DECL_ARRAY_SIZE(num_vec4_instance_meta_funcs, vec4_instance_meta_funcs);

static const FuncDef vec4_instance_funcs[] = {
    { "dot", l_glm_vec4_dot_wrap },
    { "normalize", l_glm_vec4_normalize_to },
    { "sqrLength", l_glm_vec4_norm2_wrap },
};
DECL_ARRAY_SIZE(num_vec4_instance_funcs, vec4_instance_funcs);

static const FuncDef vec4_static_funcs[] = { 0 };
static const size_t num_vec4_static_funcs = 0;
//DECL_ARRAY_SIZE(num_vec4_static_funcs, vec4_static_funcs);

// -----  quat  -----
/*
static const FuncDef quat_instance_meta_funcs[] = {
    { "__add", l_quat_add },
    { "__sub", l_quat_sub },
    { "__mul", l_quat_mul },
};
DECL_ARRAY_SIZE(num_quat_instance_meta_funcs, quat_instance_meta_funcs);

static const FuncDef quat_instance_funcs[] = {
    { "scale", l_quat_scale },
    { "conjugate", l_quat_conj },
    { "inner_product", l_quat_inner_product_wrap },
    { "normalize", l_quat_norm },
    { "mul_vec3", l_quat_mul_vec3 },
};
DECL_ARRAY_SIZE(num_quat_instance_funcs, quat_instance_funcs);

static const FuncDef quat_static_funcs[] = {
    { "angle_axis", l_quat_rotate_wrap },
    { "from_mat4", l_quat_from_mat4 },
};
DECL_ARRAY_SIZE(num_quat_static_funcs, quat_static_funcs);

// -----  mat4  -----

static const FuncDef mat4_instance_meta_funcs[] = {
    { "__add", l_mat4_add },
    { "__sub", l_mat4_sub },
    { "__mul", l_mat4_mul },
};
DECL_ARRAY_SIZE(num_mat4_instance_meta_funcs, mat4_instance_meta_funcs);

static const FuncDef mat4_instance_funcs[] = {
    { "row", l_mat4_row_wrap },
    { "clone", l_mat4_dup },
    { "scale", l_mat4_scale },
//  { "scale_aniso", l_mat4_scale_aniso },
//  { "translate", l_mat4_translate_in_place },
    { "transpose", l_mat4_transpose },
    { "invert", l_mat4_invert },
    { "orthonormalize", l_mat4_orthonormalize },
    { "mul_vec4", l_mat4_mul_vec4 },
    { "ortho_mul_quat", l_mat4o_mul_quat },
    { "rotate_x", l_mat4_rotate_X_wrap },
    { "rotate_y", l_mat4_rotate_Y_wrap },
    { "rotate_z", l_mat4_rotate_Z_wrap },
//  { "rotate", l_mat4_rotate },
};
DECL_ARRAY_SIZE(num_mat4_instance_funcs, mat4_instance_funcs);

static const FuncDef mat4_static_funcs[] = {
    { "from_vec3_mul_outer", l_mat4_from_vec3_mul_outer },
    { "from_quat", l_mat4_from_quat },
//  { "translate", l_mat4_translate },
//  { "frustum", l_mat4_frustum },
//  { "ortho", l_mat4_ortho },
//  { "perspective", l_mat4_perspective },
//  { "look_at", l_mat4_look_at },
};
DECL_ARRAY_SIZE(num_mat4_static_funcs, mat4_static_funcs);
*/

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
//  BUILD_TYPE(quat, L);
//  BUILD_TYPE(mat4, L);
}