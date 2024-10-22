#pragma once
#include <QtCore/QString>
#include "scriptenums.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

struct lua_State;

QString formatCompileError(lua_State *L);

QString formatPanic(lua_State *L);

QString formatRuntimeError(lua_State *L);

bool api_pcall(lua_State *L, int nargs, int nreturn);

int addErrorHandler(lua_State *L);

int pushErrorHandler(lua_State *L);

template <ApiCode NotFound, ApiCode Conflict>
constexpr ApiCode convertClientResultCode(int code)
{
  switch (code)
  {
  case (int)SenderAccessResult::Ok:
    return ApiCode::OK;
  case (int)SenderAccessResult::NotFound:
    return NotFound;
  case (int)SenderAccessResult::Locked:
    return ApiCode::ItemInUse;
  case (int)SenderAccessResult::BadParameter:
    return ApiCode::BadParameter;
  default:
    return code < 0 ? Conflict : ApiCode::OK;
  }
}

constexpr ApiCode convertAliasCode(int code)
{
  return convertClientResultCode<ApiCode::AliasNotFound, ApiCode::AliasAlreadyExists>(code);
}

constexpr ApiCode convertTimerCode(int code)
{
  return convertClientResultCode<ApiCode::TimerNotFound, ApiCode::TimerAlreadyExists>(code);
}

constexpr ApiCode convertTriggerCode(int code)
{
  return convertClientResultCode<ApiCode::TriggerNotFound, ApiCode::TriggerAlreadyExists>(code);
}
