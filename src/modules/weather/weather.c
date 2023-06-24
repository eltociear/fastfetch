#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/networking.h"
#include "modules/weather/weather.h"
#include "util/stringUtils.h"

#define FF_WEATHER_NUM_FORMAT_ARGS 1

static FFNetworkingState state;
static int status = -1;

void ffPrepareWeather(FFWeatherOptions* options)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/?format=");
    ffStrbufAppend(&path, &options->outputFormat);
    status = ffNetworkingSendHttpRequest(&state, "wttr.in", path.chars, "User-Agent: curl/0.0.0\r\n");
}

void ffPrintWeather(FFWeatherOptions* options)
{
    if(status == -1)
        ffPrepareWeather(options);

    if(status == 0)
    {
        ffPrintError(FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, "Failed to connect to 'wttr.in'");
        return;
    }

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(4096);
    bool success = ffNetworkingRecvHttpResponse(&state, &result, options->timeout);
    if (success) ffStrbufSubstrAfterFirstS(&result, "\r\n\r\n");

    if(!success || result.length == 0)
    {
        ffPrintError(FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, "Failed to receive the server response");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        ffPrintFormat(FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, FF_WEATHER_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result}
        });
    }
}

void ffInitWeatherOptions(FFWeatherOptions* options)
{
    options->moduleName = FF_WEATHER_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);

    ffStrbufInitS(&options->outputFormat, "%t+-+%C+(%l)");
    options->timeout = 0;
}

bool ffParseWeatherCommandOptions(FFWeatherOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WEATHER_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "output-format"))
    {
        ffOptionParseString(key, value, &options->outputFormat);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "timeout"))
    {
        options->timeout = ffOptionParseUInt32(key, value);
        return true;
    }

    return false;
}

void ffDestroyWeatherOptions(FFWeatherOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);

    ffStrbufDestroy(&options->outputFormat);
}

void ffParseWeatherJsonObject(yyjson_val* module)
{
    FFWeatherOptions __attribute__((__cleanup__(ffDestroyWeatherOptions))) options;
    ffInitWeatherOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (ffStrEqualsIgnCase(key, "outputFormat"))
            {
                ffStrbufSetS(&options.outputFormat, yyjson_get_str(val));
                continue;
            }

            if (ffStrEqualsIgnCase(key, "timeout"))
            {
                options.timeout = (uint32_t) yyjson_get_uint(val);
                continue;
            }

            ffPrintError(FF_WEATHER_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintWeather(&options);
}
