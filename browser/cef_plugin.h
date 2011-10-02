
#ifndef _CEFCLIENT_PLUGIN_H
#define _CEFCLIENT_PLUGIN_H

#include "../3rd/cef_binary/include/cef.h"

// Register the V8 extension handler.
extern void InitPlugin();

extern BOOL connect_named_pipe();

extern void close_named_pipe();

#endif // _CEFCLIENT_PLUGIN_H
