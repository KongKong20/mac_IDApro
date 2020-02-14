/*
 *      Hex-Rays Decompiler project
 *      Copyright (c) 2007-2008 by Hex-Rays, support@hex-rays.com
 *      ALL RIGHTS RESERVED.
 *
 *      Sample plugin for the Hex-Rays Decompiler.
 *      It modifies the decompilation output: removes some space characters.
 *
 */

#include <hexrays.hpp>

// Hex-Rays API pointer
hexdsp_t *hexdsp = NULL;

static bool inited = false;

//--------------------------------------------------------------------------
inline bool is_cident_char(char c)
{
  return qisalnum(c) || c == '_';
}

//--------------------------------------------------------------------------
static void remove_spaces(simpleline_t &sl)
{
  char *ptr = &sl.line[0];
  // skip initial spaces, do not compress them
  while ( true )
  {
    ptr = (char*)tag_skipcodes(ptr);
    if ( !qisspace(*ptr) )
      break;
    if ( *ptr == '\0' )
      return;
    ptr++;
  }

  // remove all spaces except in string and char constants
  char delim = 0; // if not zero, then we are skipping until 'delim'
  char last = 0;  // last seen character
  while ( *ptr != '/' || ptr[1] != '/' ) // until comments
  {
    ptr = (char*)tag_skipcodes(tag_advance(ptr, 1));
    if ( *ptr == '\0' )
      break;
    if ( delim != 0 )
    {
      if ( *ptr == '\\' ) // escape character, skip it and the next as well
      {
        ptr++;
      }
      else
      {
        if ( *ptr == delim )
          delim = 0; // found it
      }
    }
    else if ( *ptr == '"' || *ptr == '\'' )
    {
      delim = *ptr;
    }
    else if ( qisspace(*ptr) )
    {
      char *end = ptr + 1;
      while ( qisspace(*end) )
        end++;
      // do not concatenate idents
      const char *nptr = tag_skipcodes(end);
      if ( (!is_cident_char(last) || !is_cident_char(*nptr)) )
        memmove(ptr, end, strlen(end)+1);
    }
    last = *ptr;
  }

}

//--------------------------------------------------------------------------
// This callback handles various hexrays events.
static int idaapi callback(void *, hexrays_event_t event, va_list va)
{
  switch ( event )
  {
    case hxe_func_printed:
      {
        cfunc_t *cfunc = va_arg(va, cfunc_t *);
        strvec_t &sv = cfunc->sv;
        for ( int i=0; i < sv.size(); i++ )
          remove_spaces(sv[i]);
      }
      break;
    default:
      break;
  }
  return 0;
}

//--------------------------------------------------------------------------
// Initialize the plugin.
int idaapi init(void)
{
  if ( !init_hexrays_plugin() )
    return PLUGIN_SKIP; // no decompiler
  install_hexrays_callback(callback, NULL);
  const char *hxver = get_hexrays_version();
  msg("Hex-rays version %s has been detected, %s ready to use\n", hxver, PLUGIN.wanted_name);
  inited = true;
  return PLUGIN_KEEP;
}

//--------------------------------------------------------------------------
void idaapi term(void)
{
  if ( inited )
  {
    remove_hexrays_callback(callback, NULL);
    term_hexrays_plugin();
  }
}

//--------------------------------------------------------------------------
bool idaapi run(size_t)
{
  // This function won't be called because our plugin is invisible (no menu
  // item in the Edit, Plugins menu) because of PLUGIN_HIDE
  return false;
}

//--------------------------------------------------------------------------
static char comment[] = "Sample plugin6 for Hex-Rays decompiler";

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  PLUGIN_HIDE,          // plugin flags
  init,                 // initialize
  term,                 // terminate. this pointer may be NULL.
  run,                  // invoke plugin
  comment,              // long comment about the plugin
                        // it could appear in the status line or as a hint
  "",                   // multiline help about the plugin
  "Hex-Rays space remover", // the preferred short name of the plugin
  ""                    // the preferred hotkey to run the plugin
};
