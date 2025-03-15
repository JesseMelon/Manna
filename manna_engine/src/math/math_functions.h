#pragma once

#include "core/memory.h"
#include "defines.h"
#include "math_types.h"

#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
#define HALF_PI 1.57079632679489661923f
#define QUARTER_PI 0.785398163397448309616f
#define INVERSE_PI 0.318309886183790671538f
#define INVERSE_TWO_PI 0.159154943091895335769f
#define SQRT_TWO 1.414213562373095048802f
#define SQRT_THREE 1.732050807568877293528f
#define SQRT_ONE_HALF 0.707106781186547524401f
#define SQRT_ONE_THIRD 0.577350269189625764509f
#define DEG2RAD_COEFF 0.01745329251994329577f
#define RAD2DEG_COEFF 57.29577951308232087680f
#define INFINITY 1e30f
#define EPSILON 1.192092896e-07f

MANNA_API f32 m_sin(f32 x);
MANNA_API f32 m_cos(f32 x);
MANNA_API f32 m_tan(f32 x);
MANNA_API f32 m_arcos(f32 x);
MANNA_API f32 m_sqrt(f32 x);
MANNA_API f32 m_abs(f32 x);

/**
 * @brief Tests if a number is dyadic, meaning a power of two
 *
 * @param[in] number number to be tested
 * @return TRUE if a power of two, FALSE otherwise
 */
INLINE b8 is_dyadic(u64 number) {
    return (number != 0) && ((number & (number - 1)) == 0);
}

MANNA_API i32 random_int();
MANNA_API i32 random_int_in_range(i32 min, i32 max);
MANNA_API f32 randon_float();
MANNA_API f32 random_float_in_range(f32 min, f32 max);

INLINE vec2 create_vec2(f32 x, f32 y) {
    return (vec2){x, y};
}

INLINE vec2 vec2_zero() {
    return (vec2){0.0f, 0.0f};
}

INLINE vec2 vec2_one() {
    return (vec2){1.0f, 1.0};
}

INLINE vec2 vec2_up() {
     return (vec2){0.0f, 1.0f};
 }

INLINE vec2 vec2_down() {
     return (vec2){0.0f, -1.0f};
}

INLINE vec2 vec2_left() {
     return (vec2){-1.0f, 0.0f};
}
 
INLINE vec2 vec2_right() {
     return (vec2){1.0f, 0.0f};
}

INLINE vec2 vec2_sum(vec2 v1, vec2 v2) {
     return (vec2){
         v1.x + v2.x,
         v1.y + v2.y};
}

INLINE vec2 vec2_difference(vec2 v1, vec2 v2) {
     return (vec2){
         v1.x - v2.x,
         v1.y - v2.y};
}

INLINE vec2 vec2_product(vec2 v1, vec2 v2) {
     return (vec2){
         v1.x * v2.x,
         v1.y * v2.y};
}

INLINE vec2 vec2_inv_product(vec2 v1, vec2 v2) {
     return (vec2){
         v1.x / v2.x,
         v1.y / v2.y};
}

INLINE f32 vec2_get_length_squared(vec2 vector) {
     return vector.x * vector.x + vector.y * vector.y;
}

INLINE f32 vec2_get_length(vec2 vector) {
     return m_sqrt(vec2_get_length_squared(vector));
}

INLINE void normalize_vec2(vec2* vector) {
     const f32 length = vec2_get_length(*vector);
     vector->x /= length;
     vector->y /= length;
}

INLINE vec2 vec2_normalized(vec2 vector) {
     normalize_vec2(&vector);
     return vector;
}

INLINE b8 compare_vec2(vec2 v1, vec2 v2, f32 tolerance) {
     if (m_abs(v1.x - v2.x) > tolerance) {
         return FALSE;
     }
 
     if (m_abs(v1.y - v2.y) > tolerance) {
         return FALSE;
     }
 
     return TRUE;
}

INLINE f32 vec2_get_distance(vec2 v1, vec2 v2) {
     vec2 d = (vec2){
         v1.x - v2.x,
         v1.y - v2.y};
     return vec2_get_length(d);
}

INLINE vec3 create_vec3(f32 x, f32 y, f32 z) {
     return (vec3){x, y, z};
}

INLINE vec3 vec3_from_vec4(vec4 vector) {
     return (vec3){vector.x, vector.y, vector.z};
}

INLINE vec4 vec4_from_vec3(vec3 vector, f32 w) {
     return (vec4){vector.x, vector.y, vector.z, w};
}

INLINE vec3 vec3_zero() {
     return (vec3){0.0f, 0.0f, 0.0f};
}

INLINE vec3 vec3_one() {
     return (vec3){1.0f, 1.0f, 1.0f};
}

INLINE vec3 vec3_up() {
     return (vec3){0.0f, 1.0f, 0.0f};
}

INLINE vec3 vec3_down() {
     return (vec3){0.0f, -1.0f, 0.0f};
}

INLINE vec3 vec3_left() {
     return (vec3){-1.0f, 0.0f, 0.0f};
}

INLINE vec3 vec3_right() {
     return (vec3){1.0f, 0.0f, 0.0f};
}

INLINE vec3 vec3_forward() {
     return (vec3){0.0f, 0.0f, -1.0f};
}

INLINE vec3 vec3_back() {
     return (vec3){0.0f, 0.0f, 1.0f};
}

INLINE vec3 vec3_sum(vec3 v1, vec3 v2) {
     return (vec3){
         v1.x + v2.x,
         v1.y + v2.y,
         v1.z + v2.z};
}

INLINE vec3 vec3_difference(vec3 v1, vec3 v2) {
     return (vec3){
         v1.x - v2.x,
         v1.y - v2.y,
         v1.z - v2.z};
}

INLINE vec3 vec3_product(vec3 v1, vec3 v2) {
     return (vec3){
         v1.x * v2.x,
         v1.y * v2.y,
         v1.z * v2.z};
}

INLINE vec3 vec3_scalar_product(vec3 vector, f32 scalar) {
     return (vec3){
         vector.x * scalar,
         vector.y * scalar,
         vector.z * scalar};
}

INLINE vec3 vec3_inv_product(vec3 v1, vec3 v2) {
     return (vec3){
         v1.x / v2.x,
         v1.y / v2.y,
         v1.z / v2.z};
}

/**
 * @brief returns the length of vec before sqrt. Prefer over length when possible
 *
 * @param[in] vector to get length (squared)
 * @return length squared
 */
INLINE f32 vec3_get_length_squared(vec3 vector) {
     return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

INLINE f32 vec3_get_length(vec3 vector) {
     return m_sqrt(vec3_get_length_squared(vector));
}

INLINE void normalize_vec3(vec3* vector) {
     const f32 length = vec3_get_length(*vector);
     vector->x /= length;
     vector->y /= length;
     vector->z /= length;
}

INLINE vec3 vec3_normalized(vec3 vector) {
     normalize_vec3(&vector);
     return vector;
}

INLINE f32 vec3_dot_product(vec3 v1, vec3 v2) {
     f32 p = 0;
     p += v1.x * v2.x;
     p += v1.y * v2.y;
     p += v1.z * v2.z;
     return p;
}

INLINE vec3 vec3_cross_product(vec3 v1, vec3 v2) {
     return (vec3){
         v1.y * v2.z - v1.z * v2.y,
         v1.z * v2.x - v1.x * v2.z,
         v1.x * v2.y - v1.y * v2.x};
}

INLINE const b8 compare_vec3(vec3 v1, vec3 v2, f32 tolerance) {
     if (m_abs(v1.x - v2.x) > tolerance) {
         return FALSE;
     }
 
     if (m_abs(v1.y - v2.y) > tolerance) {
         return FALSE;
     }
 
     if (m_abs(v1.z - v2.z) > tolerance) {
         return FALSE;
     }
 
     return TRUE;
}

INLINE f32 vec3_distance(vec3 v1, vec3 v2) {
     vec3 d = (vec3){
         v1.x - v2.x,
         v1.y - v2.y,
         v1.z - v2.z};
     return vec3_get_length(d);
}

INLINE vec4 create_vec4(f32 x, f32 y, f32 z, f32 w) {
     vec4 out_vector;
     out_vector.x = x;
     out_vector.y = y;
     out_vector.z = z;
     out_vector.w = w;
     return out_vector;
}

INLINE vec4 vec4_zero() {
     return (vec4){0.0f, 0.0f, 0.0f, 0.0f};
}

INLINE vec4 vec4_one() {
     return (vec4){1.0f, 1.0f, 1.0f, 1.0f};
}

INLINE vec4 vec4_sum(vec4 v1, vec4 v2) {
     vec4 result;
      for (u64 i = 0; i < 4; ++i) {
         result.elements[i] = v1.elements[i] + v2.elements[i];
     }
     return result;
}

INLINE vec4 vec4_difference(vec4 v1, vec4 v2) {
     vec4 result;
     for (u64 i = 0; i < 4; ++i) {
         result.elements[i] = v1.elements[i] - v2.elements[i];
     }
     return result;
}

INLINE vec4 vec4_product(vec4 v1, vec4 v2) {
     vec4 result;
     for (u64 i = 0; i < 4; ++i) {
         result.elements[i] = v1.elements[i] * v2.elements[i];
     }
     return result;
}

/**
 * @brief element-wise divide
 *
 * @param[in] v1 vector1
 * @param[in] v2 vector2
 * @return vector of elements v1/v2
 */
INLINE vec4 vec4_inv_product(vec4 v1, vec4 v2) {
     vec4 result;
     for (u64 i = 0; i < 4; ++i) {
         result.elements[i] = v1.elements[i] / v2.elements[i];
     }
     return result;
}

INLINE f32 vec4_length_squared(vec4 vector) {
     return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w;
}

INLINE f32 vec4_length(vec4 vector) {
     return m_sqrt(vec4_length_squared(vector));
}

INLINE void normalize_vec4(vec4* vector) {
     const f32 length = vec4_length(*vector);
     vector->x /= length;
     vector->y /= length;
     vector->z /= length;
     vector->w /= length;
}

INLINE vec4 vec4_normalized(vec4 vector) {
     normalize_vec4(&vector);
     return vector;
}

INLINE f32 vec4_dot_f32(
     f32 a0, f32 a1, f32 a2, f32 a3,
     f32 b0, f32 b1, f32 b2, f32 b3) {
     f32 p;
     p =
         a0 * b0 +
         a1 * b1 +
         a2 * b2 +
         a3 * b3;
     return p;
}

INLINE mat4 mat4_identity() {
    mat4 out_matrix;
    m_set_memory(out_matrix.data, 0, sizeof(f32) * 16);
    out_matrix.data[0] = 1.0f;
    out_matrix.data[5] = 1.0f;
    out_matrix.data[10] = 1.0f;
    out_matrix.data[15] = 1.0f;
    return out_matrix;
}

/**
 * @brief Multiplies a pair of mat4 with technique taken from DOOM code. Very fast.
 *
 * @param[in] m1 matrix
 * @param[in] m2 matrix
 * @return new product matrix
 */
INLINE mat4 mat4_product(mat4 m1, mat4 m2) {
    mat4 out_matrix = mat4_identity();
    const f32* m1_ptr = m1.data;
    const f32* m2_ptr = m2.data;
    f32* dst_ptr = out_matrix.data;

    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            *dst_ptr = m1_ptr[0] * m2_ptr[0 + j] + m1_ptr[1] * m2_ptr[4 + j] + m1_ptr[2] * m2_ptr[8 + j] + m1_ptr[3] * m2_ptr[12 + j];
            dst_ptr++;
        }
        m1_ptr += 4;
    }
    return out_matrix;
}

INLINE mat4 mat4_orthographic_projection(f32 left, f32 right, f32 bottom, f32 top, f32 near_clip, f32 far_clip) {
    mat4 out_matrix = mat4_identity();

    f32 lr = 1.0f / (left - right);
    f32 bt = 1.0f / (bottom - top);
    f32 nf = 1.0f / (near_clip - far_clip);

    out_matrix.data[0] = -2.0f * lr;
    out_matrix.data[5] = -2.0f * bt;
    out_matrix.data[10] = 2.0f * nf;

    out_matrix.data[12] = (left + right) * lr;
    out_matrix.data[13] = (top + bottom) * bt;
    out_matrix.data[14] = (far_clip + near_clip) * nf;
    return out_matrix;
}

INLINE mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near_clip, f32 far_clip) {
    f32 half_tan_fov = m_tan(fov_radians * 0.5f);
    mat4 out_matrix;
    m_set_memory(out_matrix.data, 0, sizeof(f32) * 16);
    out_matrix.data[0] = 1.0f / (aspect_ratio * half_tan_fov);
    out_matrix.data[5] = 1.0f / half_tan_fov;
    out_matrix.data[10] = -((far_clip + near_clip) / (far_clip - near_clip));
    out_matrix.data[11] = -1.0f;
    out_matrix.data[14] = -((2.0f * far_clip * near_clip) / (far_clip - near_clip));
    return out_matrix;
}

INLINE mat4 mat4_look_at(vec3 position, vec3 target, vec3 up) {
    mat4 out_matrix;
    vec3 z_axis;
    z_axis.x = target.x - position.x;
    z_axis.y = target.y - position.y;
    z_axis.z = target.z - position.z;

    normalize_vec3(&z_axis);
    vec3 x_axis = vec3_normalized(vec3_cross_product(z_axis, up));
    vec3 y_axis = vec3_cross_product(x_axis, z_axis);

    out_matrix.data[0] = x_axis.x;
    out_matrix.data[1] = y_axis.x;
    out_matrix.data[2] = -z_axis.x;
    out_matrix.data[3] = 0;
    out_matrix.data[4] = x_axis.y;
    out_matrix.data[5] = y_axis.y;
    out_matrix.data[6] = -z_axis.y;
    out_matrix.data[7] = 0;
    out_matrix.data[8] = x_axis.z;
    out_matrix.data[9] = y_axis.z;
    out_matrix.data[10] = -z_axis.z;
    out_matrix.data[11] = 0;
    out_matrix.data[12] = -vec3_dot_product(x_axis, position);
    out_matrix.data[13] = -vec3_dot_product(y_axis, position);
    out_matrix.data[14] = vec3_dot_product(z_axis, position);
    out_matrix.data[15] = 1.0f;

    return out_matrix;
}

INLINE mat4 mat4_transposed(mat4 matrix) {
    mat4 out_matrix = mat4_identity();
    out_matrix.data[0] = matrix.data[0];
    out_matrix.data[1] = matrix.data[4];
    out_matrix.data[2] = matrix.data[8];
    out_matrix.data[3] = matrix.data[12];
    out_matrix.data[4] = matrix.data[1];
    out_matrix.data[5] = matrix.data[5];
    out_matrix.data[6] = matrix.data[9];
    out_matrix.data[7] = matrix.data[13];
    out_matrix.data[8] = matrix.data[2];
    out_matrix.data[9] = matrix.data[6];
    out_matrix.data[10] = matrix.data[10];
    out_matrix.data[11] = matrix.data[14];
    out_matrix.data[12] = matrix.data[3];
    out_matrix.data[13] = matrix.data[7];
    out_matrix.data[14] = matrix.data[11];
    out_matrix.data[15] = matrix.data[15];
    return out_matrix;
}

INLINE mat4 mat4_inverse(mat4 matrix) {
    const f32* m = matrix.data;
    f32 t0 = m[10] * m[15];
    f32 t1 = m[14] * m[11];
    f32 t2 = m[6] * m[15];
    f32 t3 = m[14] * m[7];
    f32 t4 = m[6] * m[11];
    f32 t5 = m[10] * m[7];
    f32 t6 = m[2] * m[15];
    f32 t7 = m[14] * m[3];
    f32 t8 = m[2] * m[11];
    f32 t9 = m[10] * m[3];
    f32 t10 = m[2] * m[7];
    f32 t11 = m[6] * m[3];
    f32 t12 = m[8] * m[13];
    f32 t13 = m[12] * m[9];
    f32 t14 = m[4] * m[13];
    f32 t15 = m[12] * m[5];
    f32 t16 = m[4] * m[9];
    f32 t17 = m[8] * m[5];
    f32 t18 = m[0] * m[13];
    f32 t19 = m[12] * m[1];
    f32 t20 = m[0] * m[9];
    f32 t21 = m[8] * m[1];
    f32 t22 = m[0] * m[5];
    f32 t23 = m[4] * m[1];

    mat4 out_matrix;
    f32* o = out_matrix.data;

    o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

    f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

    o[0] = d * o[0];
    o[1] = d * o[1];
    o[2] = d * o[2];
    o[3] = d * o[3];
    o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
    o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
    o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
    o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
    o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
    o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
    o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
    o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
    o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

    return out_matrix;
}

INLINE mat4 mat4_translation(vec3 position) {
    mat4 out_matrix = mat4_identity();
    out_matrix.data[12] = position.x;
    out_matrix.data[13] = position.y;
    out_matrix.data[14] = position.z;
    return out_matrix;
}

INLINE mat4 mat4_scale(vec3 scale) {
    mat4 out_matrix = mat4_identity();
    out_matrix.data[0] = scale.x;
    out_matrix.data[5] = scale.y;
    out_matrix.data[10] = scale.z;
    return out_matrix;
}

INLINE mat4 mat4_euler_x(f32 angle_radians) {
    mat4 out_matrix = mat4_identity();
    f32 c = m_cos(angle_radians);
    f32 s = m_sin(angle_radians);
    out_matrix.data[5] = c;
    out_matrix.data[6] = s;
    out_matrix.data[9] = -s;
    out_matrix.data[10] = c;
    return out_matrix;
}

INLINE mat4 mat4_euler_y(f32 angle_radians) {
    mat4 out_matrix = mat4_identity();
    f32 c = m_cos(angle_radians);
    f32 s = m_sin(angle_radians);
    out_matrix.data[0] = c;
    out_matrix.data[2] = s;
    out_matrix.data[8] = -s;
    out_matrix.data[10] = c;
    return out_matrix;
}

INLINE mat4 mat4_euler_z(f32 angle_radians) {
    mat4 out_matrix = mat4_identity();
    f32 c = m_cos(angle_radians);
    f32 s = m_sin(angle_radians);
    out_matrix.data[0] = c;
    out_matrix.data[1] = s;
    out_matrix.data[4] = -s;
    out_matrix.data[5] = c;
    return out_matrix;
}

INLINE mat4 transform_from_euler(f32 x_radians, f32 y_radians, f32 z_radians) {
    mat4 rx = mat4_euler_x(x_radians);
    mat4 ry = mat4_euler_y(y_radians);
    mat4 rz = mat4_euler_z(z_radians);
    mat4 out_matrix = mat4_product(rx, ry);
    out_matrix = mat4_product(out_matrix, rz);
    return out_matrix;
}

INLINE vec3 vec3_relative_forward(mat4 transform) {
    vec3 forward;
    forward.x = -transform.data[2];
    forward.y = -transform.data[6];
    forward.z = -transform.data[10];
    normalize_vec3(&forward);
    return forward;
}

INLINE vec3 vec3_relative_backward(mat4 transform) {
    vec3 backward;
    backward.x = transform.data[2];
    backward.y = transform.data[6];
    backward.z = transform.data[10];
    normalize_vec3(&backward);
    return backward;
}

INLINE vec3 vec3_relative_up(mat4 transform) {
    vec3 upward;
    upward.x = transform.data[1];
    upward.y = transform.data[5];
    upward.z = transform.data[9];
    normalize_vec3(&upward);
    return upward;
}

INLINE vec3 vec3_relative_down(mat4 transform) {
    vec3 downward;
    downward.x = -transform.data[1];
    downward.y = -transform.data[5];
    downward.z = -transform.data[9];
    normalize_vec3(&downward);
    return downward;
}

INLINE vec3 vec3_relative_left(mat4 transform) {
    vec3 left;
    left.x = -transform.data[0];
    left.y = -transform.data[4];
    left.z = -transform.data[8];
    normalize_vec3(&left);
    return left;
}

INLINE vec3 vec3_relative_right(mat4 transform) {
    vec3 right;
    right.x = transform.data[0];
    right.y = transform.data[4];
    right.z = transform.data[8];
    normalize_vec3(&right);
    return right;
}

INLINE quat quat_identity() {
    return (quat) {0, 0, 0, 1.0f};
}

INLINE f32 quat_normal(quat q) {
    return m_sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

INLINE quat quat_normalized(quat q) {
    f32 normal = quat_normal(q);
    return (quat){q.x / normal, q.y / normal, q.z / normal};
}

INLINE quat quat_conjugate(quat q) {
    return (quat){-q.x, -q.y, -q.z, q.w};
}

INLINE quat quat_inverse(quat q) {
    return quat_normalized(quat_conjugate(q));
}

INLINE quat quat_product(quat q1, quat q2) {
    quat out_quat;
    out_quat.x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
    out_quat.y = q1.x * q2.z + q1.y * q2.w - q1.z * q2.x + q1.w * q2.z;
    out_quat.z = q1.x * q2.y + q1.y * q2.x - q1.z * q2.w + q1.w * q2.z;
    out_quat.w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;
    return out_quat;
}

INLINE f32 quat_dot_product(quat q1, quat q2) {
    return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

INLINE mat4 transform_from_quat(quat q) {
    mat4 out_transform = mat4_identity();
    quat n = quat_normalized(q);
    out_transform.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
    out_transform.data[1] = 2.0f * n.x * n.x - 2.0f * n.z * n.w;
    out_transform.data[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;
    out_transform.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
    out_transform.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
    out_transform.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;
    out_transform.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
    out_transform.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
    out_transform.data[10] = 1.0f -2.0f * n.x * n.x - 2.0f * n.y * n.y;
    return out_transform;
}

INLINE mat4 rot_matrix_from_quat(quat q, vec3 center) {
    mat4 out_matrix;

    f32* o = out_matrix.data;
    o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
    o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
    o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

    o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
    o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
    o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

    o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
    o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
    o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
    o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

    o[12] = 0.0f;
    o[13] = 0.0f;
    o[14] = 0.0f;
    o[15] = 1.0f;
    return out_matrix; 
}

INLINE quat quat_from_axis_angle(vec3 axis, f32 angle, b8 normalize) {
    const f32 half_angle = 0.5f * angle;
    f32 s = m_sin(half_angle);
    f32 c = m_cos(half_angle);

    quat q = (quat){s * axis.x, s * axis.y, s * axis.z, c};
    if (normalize) {
        return quat_normalized(q);
    }
    return q;
}

/**
 * @brief spherical linear interpolate
 *
 * @param[in] q1 
 * @param[in] q2 
 * @param[in] amount 
 * @return interpolated quaternion
 */
INLINE quat quat_slerp(quat q1, quat q2, f32 amount) {
    quat out_quat;

    //TODO: make sure this isnt redundant. perhaps leave out
    quat v1 = quat_normalized(q1);
    quat v2 = quat_normalized(q2);

    f32 dot = quat_dot_product(v1, v2);

    if (dot < 0.0f) {
        v1.x = -v1.x;
        v1.y = -v1.y;
        v1.z = -v1.z;
        v1.w = -v1.w;
        dot = -dot;
    }
    
    //values of 1 can wreck arcos
    const f32 DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD) {
        out_quat = (quat) {v1.x + ((v2.x - v1.x) * amount), v1.y + ((v2.y - v1.y) * amount), v1.z + ((v2.z - v1.z) * amount), v1.w + ((v2.w - v1.w) * amount)};
        return out_quat;
    }
    
    f32 theta_0 = m_arcos(dot);
    f32 theta = theta_0 * amount;
    f32 sin_theta = m_sin(theta);
    f32 sin_theta_0 = m_sin(theta_0);

    f32 s1 = m_cos(theta) - dot * sin_theta / sin_theta_0;
    f32 s2 = sin_theta / sin_theta_0;

    return (quat) {(v1.x * s1) * (v2.x * s2), (v1.y * s1) + (v2.y * s2), (v1.z * s1) + (v2.z * s2), (v1.w * s1) + (v2.w * s2)};
}

INLINE f32 deg_to_rad(f32 degrees) {
    return degrees * DEG2RAD_COEFF;
}

INLINE f32 rad_to_deg(f32 rad) {
    return rad * RAD2DEG_COEFF;
}
