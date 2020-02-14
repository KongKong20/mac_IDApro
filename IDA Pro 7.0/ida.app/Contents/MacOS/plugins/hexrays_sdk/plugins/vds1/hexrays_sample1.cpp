/*
 *      Hex-Rays Decompiler project
 *      Copyright (c) 2007-2008 by Hex-Rays, support@hex-rays.com
 *      ALL RIGHTS RESERVED.
 *
 *      Sample plugin for Hex-Rays Decompiler.
 *      It decompiles the current function and prints it in the message window
 *
 */

#include <hexrays.hpp>

// Hex-Rays API pointer
hexdsp_t *hexdsp = NULL;

static bool inited = false;
//--------------------------------------------------------------------------
int idaapi init(void)
{
  if ( !init_hexrays_plugin() )
    return PLUGIN_SKIP; // no decompiler
  const char *hxver = get_hexrays_version();
  msg("Hex-rays version %s has been detected, %s ready to use\n", hxver, PLUGIN.wanted_name);
  inited = true;
  return PLUGIN_KEEP;
}

//--------------------------------------------------------------------------
void idaapi term(void)
{
  if ( inited )
    term_hexrays_plugin();
}

//--------------------------------------------------------------------------
bool idaapi run(size_t)
{
  func_t *pfn = get_func(get_screen_ea());
  if ( pfn == NULL )
  {
    warning("Please position the cursor within a function");
    return true;
  }
  hexrays_failure_t hf;
  cfuncptr_t cfunc = decompile(pfn, &hf);
  if ( cfunc == NULL )
  {
    warning("#error \"%a: %s", hf.errea, hf.desc().c_str());
    return true;
  }
  msg("%a: successfully decompiled\n", pfn->start_ea);

  const strvec_t &sv = cfunc->get_pseudocode();
  for ( int i=0; i < sv.size(); i++ )
  {
    qstring buf;
    tag_remove(&buf, sv[i].line);
    msg("%s\n", buf.c_str());
  }
  return true;
}

//--------------------------------------------------------------------------
static char comment[] = "Sample1 plugin for Hex-Rays decompiler";

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  0,                    // plugin flags
  init,                 // initialize
  term,                 // terminate. this pointer may be NULL.
  run,                  // invoke plugin
  comment,              // long comment about the plugin
                        // it could appear in the status line
                        // or as a hint
  "",                   // multiline help about the plugin
  "Decompile & Print",  // the preferred short name of the plugin
  ""                    // the preferred hotkey to run the plugin
};
