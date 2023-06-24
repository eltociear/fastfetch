#pragma once

#include "fastfetch.h"

#define FF_HOST_MODULE_NAME "Host"

void ffPrintHost(FFHostOptions* options);
void ffInitHostOptions(FFHostOptions* options);
bool ffParseHostCommandOptions(FFHostOptions* options, const char* key, const char* value);
void ffDestroyHostOptions(FFHostOptions* options);
void ffParseHostJsonObject(yyjson_val* module);
