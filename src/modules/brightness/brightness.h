#pragma once

#include "fastfetch.h"

void ffPrintBrightness(FFinstance* instance, FFBrightnessOptions* options);
void ffInitBrightnessOptions(FFBrightnessOptions* options);
bool ffParseBrightnessCommandOptions(FFBrightnessOptions* options, const char* key, const char* value);
void ffDestroyBrightnessOptions(FFBrightnessOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
bool ffParseBrightnessJsonObject(FFinstance* instance, const char* type, json_object* module);
#endif
