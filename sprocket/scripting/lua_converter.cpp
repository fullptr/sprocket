#include "lua_converter.h"

#include <lua.hpp>

#include <cassert>
#include <cstddef>

namespace spkt {
namespace lua {

void converter<int>::push(lua_State* L, const int& value)
{
    lua_pushinteger(L, value);
}

int converter<int>::read(lua_State* L, int index)
{
    assert(lua_isinteger(L, index));
    return static_cast<int>(lua_tointeger(L, index));
}

void converter<std::uint32_t>::push(lua_State* L, const std::uint32_t& value)
{
    lua_pushinteger(L, value);
}

std::uint32_t converter<std::uint32_t>::read(lua_State* L, int index)
{
    assert(lua_isinteger(L, index));
    return static_cast<std::uint32_t>(lua_tointeger(L, index));
}

void converter<bool>::push(lua_State* L, const bool& value)
{
    lua_pushboolean(L, value);
}

bool converter<bool>::read(lua_State* L, int index)
{
    assert(lua_isboolean(L, index));
    return static_cast<bool>(lua_toboolean(L, index));
}

void converter<float>::push(lua_State* L, const float& value)
{
    lua_pushnumber(L, value);
}

float converter<float>::read(lua_State* L, int index)
{
    assert(lua_isnumber(L, index));
    return static_cast<float>(lua_tonumber(L, index));
}

void converter<double>::push(lua_State* L, const double& value)
{
    lua_pushnumber(L, value);
}

double converter<double>::read(lua_State* L, int index)
{
    assert(lua_isnumber(L, index));
    return static_cast<double>(lua_tonumber(L, index));
}

void converter<const char*>::push(lua_State* L, const char* value)
{
    lua_pushstring(L, value);
}

const char* converter<const char*>::read(lua_State* L, int index)
{
    assert(lua_isstring(L, index));
    return lua_tostring(L, index);
}

void converter<std::string>::push(lua_State* L, const std::string& value)
{
    lua_pushstring(L, value.c_str());
}

std::string converter<std::string>::read(lua_State* L, int index)
{
    assert(lua_isstring(L, index));
    return lua_tostring(L, index);
}

void converter<void*>::push(lua_State* L, void* value)
{
    lua_pushlightuserdata(L, value);
}

void* converter<void*>::read(lua_State* L, int index)
{
    assert(lua_islightuserdata(L, index));
    return lua_touserdata(L, index);
}

void converter<glm::vec2>::push(lua_State* L, const glm::vec2& value)
{
    glm::vec2* vec = (glm::vec2*)lua_newuserdata(L, sizeof(glm::vec2));
    *vec = value;
    luaL_setmetatable(L, "vec2");
}

glm::vec2 converter<glm::vec2>::read(lua_State* L, int index)
{
    return *(glm::vec2*)luaL_checkudata(L, index, "vec2");
}

void converter<glm::vec3>::push(lua_State* L, const glm::vec3& value)
{
    glm::vec3* vec = (glm::vec3*)lua_newuserdata(L, sizeof(glm::vec3));
    *vec = value;
    luaL_setmetatable(L, "vec3");
}

glm::vec3 converter<glm::vec3>::read(lua_State* L, int index)
{
    return *(glm::vec3*)luaL_checkudata(L, index, "vec3");
}

}
}