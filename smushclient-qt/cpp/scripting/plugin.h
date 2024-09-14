#pragma once
#include "scriptapi.h"
#include "scriptstate.h"

typedef struct Plugin
{
  bool disabled;
  ScriptState state;

  Plugin(ScriptApi *api, const QString &pluginID) : disabled(false), state(api, pluginID) {}
} Plugin;
