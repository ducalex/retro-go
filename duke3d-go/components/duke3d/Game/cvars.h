#ifndef _CVARS_H_
#define _CVARS_H_

#define REGCONVAR(varname, varhelp, variable, function) CVAR_RegisterCvar(varname, varhelp, &variable, &function)
#define REGCONFUNC(varname, varhelp, function) CVAR_RegisterCvar(varname, varhelp, NULL, &function)

#ifdef _WIN32
   #include "../../Engine/src/windows/inttypes.h"
#else
   #include <inttypes.h>
#endif

typedef void (*function_t) (void* binding);

typedef struct t_cvar_binding
{
  char           name[64];
  char           help[64];
  void*         variable;
  function_t	function;
} cvar_binding;


void            CVAR_RegisterCvar(const char * varname, const char * varhelp, void* variable, function_t function);
int             CVAR_GetNumCvarBindings();
cvar_binding*   CVAR_GetCvarBinding(unsigned int nBinding);
void            CVAR_Render();

#endif