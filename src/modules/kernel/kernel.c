#include "fastfetch.h"
#include "common/printing.h"
#include "modules/kernel/kernel.h"

#define FF_KERNEL_MODULE_NAME "Kernel"
#define FF_KERNEL_NUM_FORMAT_ARGS 4

void ffPrintKernel(FFinstance* instance, FFKernelOptions* options)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs.key);
        ffStrbufWriteTo(&instance->state.platform.systemRelease, stdout);

        #ifdef _WIN32
            if(instance->state.platform.systemVersion.length > 0)
                printf(" (%s)", instance->state.platform.systemVersion.chars);
        #endif

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_KERNEL_MODULE_NAME, 0, &options->moduleArgs, FF_KERNEL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemRelease},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemArchitecture}
        });
    }
}

void ffInitKernelOptions(FFKernelOptions* options)
{
    options->moduleName = FF_KERNEL_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseKernelCommandOptions(FFKernelOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_KERNEL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyKernelOptions(FFKernelOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
bool ffParseKernelJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module)
{
    if (strcasecmp(type, FF_KERNEL_MODULE_NAME) != 0)
        return false;

    FFKernelOptions __attribute__((__cleanup__(ffDestroyKernelOptions))) options;
    ffInitKernelOptions(&options);

    if (module)
    {
        struct lh_entry* entry;
        lh_foreach(data->ffjson_object_get_object(module), entry)
        {
            const char* key = (const char *)lh_entry_k(entry);
            if (strcasecmp(key, "type") == 0)
                continue;
            json_object* val = (struct json_object *)lh_entry_v(entry);

            if (ffJsonConfigParseModuleArgs(data, key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_KERNEL_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintKernel(instance, &options);
    return true;
}
#endif
