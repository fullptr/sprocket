#include "Maths.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

namespace Sprocket {
namespace Maths {

// Matrix Modifiers
mat4 Translate(const mat4& matrix, const vec3& translation)
{
    return glm::translate(matrix, translation);
}

mat4 Scale(const mat4& matrix, const vec3& scales)
{
    return glm::scale(matrix, scales);
}

mat4 Scale(const mat4& matrix, float scale)
{
    return glm::scale(matrix, {scale, scale, scale});
}

mat4 Rotate(const mat4& matrix, const vec3& axis, float radians)
{
    return glm::rotate(matrix, radians, axis);
}

mat4 Inverse(const mat4& matrix)
{
    return glm::inverse(matrix);
}

mat4 Transpose(const mat4& matrix)
{
    return glm::transpose(matrix);
}

// Matrix Constructors
mat4 Transform(const vec3& position, const quat& orientation)
{
    mat4 m = ToMat3(orientation);
    m[3][0] = position.x;
    m[3][1] = position.y;
    m[3][2] = position.z;
    m[3][3] = 1.0f;
    return m;
}

mat4 Perspective(float aspectRatio, float fov, float nearPlane, float farPlane)
{
    return glm::perspective(fov, aspectRatio, nearPlane, farPlane);  
}

mat4 View(const vec3& position, float pitch, float yaw, float roll)
{
    mat4 matrix(1.0);
    matrix = glm::rotate(matrix, Radians(pitch), vec3(1, 0, 0));
    matrix = glm::rotate(matrix, Radians(yaw), vec3(0, 1, 0));
    matrix = glm::rotate(matrix, Radians(roll), vec3(0, 0, 1));
    matrix = glm::translate(matrix, -position);
    return matrix;
}

mat4 LookAt(const vec3& position, const vec3& target, const vec3& up)
{
    return glm::lookAt(position, target, up);
}

mat4 Ortho(float left, float right, float bottom, float top)
{
    return glm::ortho(left, right, bottom, top);   
}

// Quaternion Modifiers
quat Rotate(const vec3& axis, float degrees)
{
    return glm::rotate(Maths::identity, Radians(degrees), axis);
}

quat Rotate(const quat& orig, const vec3& axis, float degrees)
{
    return glm::rotate(orig, Radians(degrees), axis);
}

quat Inverse(const quat& quaternion)
{
    return glm::inverse(quaternion);
}

quat Normalise(const quat& q)
{
    return glm::normalize(q);
}

// Conversions
mat3 ToMat3(const quat& q)
{
    return glm::mat3_cast(q);
}

mat4 ToMat4(const quat& q)
{
    return glm::mat4_cast(q);
}

quat ToQuat(const mat3& m)
{
    return glm::quat_cast(m);
}

vec3 ToEuler(const quat& q)
{
    return glm::eulerAngles(q);
}

float* Cast(const mat3& m)
{
    return (float*)&m[0][0];
}

float* Cast(const mat4& m)
{
    return (float*)&m[0][0];
}

// Vector Maths
vec3 Cross(const vec3& lhs, const vec3& rhs)
{
    return glm::cross(lhs, rhs);
}

vec3 GetTranslation(const mat4& m)
{
    return vec3{m[3][0], m[3][1], m[3][2]};
}

float Distance(const Maths::vec2& A, const Maths::vec2& B)
{
    return glm::distance(A, B);
}

float Length(const vec3& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float LengthSquare(const vec3& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

void Normalise(vec3& vec)
{
    float length = Length(vec);
    if (length != 0) {
        vec.x /= length;
        vec.y /= length;
        vec.z /= length;
    }
}

// Trig
float Radians(float degrees)
{
    return glm::radians(degrees);
}

float Degrees(float radians)
{
    return glm::degrees(radians);
}

float Sind(float degrees)
{
    return std::sin(Radians(degrees));
}

float Cosd(float degrees)
{
    return std::cos(Radians(degrees));
}

// General Helpers
void Clamp(float& value, float min, float max)
{
    value = std::min(std::max(value, min), max);
}

// Printing
std::string ToString(const vec3& v, const std::optional<int>& dp)
{
    std::stringstream ss;
    if (dp.has_value()) {
        ss << std::fixed << std::setprecision(dp.value());
    }
    ss << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return ss.str();
}

std::string ToString(float x, const std::optional<int>& dp)
{
    std::stringstream ss;
    if (dp.has_value()) {
        ss << std::fixed << std::setprecision(dp.value());
    }
    ss << x;
    return ss.str();
}

std::string ToString(bool t)
{
    if (t) {
        return "True";
    }
    return "False";
}

}
}