/*
 *      Hex-Rays Decompiler project
 *      Copyright (c) 2007-2008 by Hex-Rays, support@hex-rays.com
 *      ALL RIGHTS RESERVED.
 *
 *      Sample plugin for the Hex-Rays Decompiler.
 *      It demonstrates how to use cblock_t::iterator
 *
 */

#include <hexrays.hpp>

// Hex-Rays API pointer
hexdsp_t *hexdsp = NULL;

static bool inited = false;

//--------------------------------------------------------------------------
// This callback handles various hexrays events.
static int idaapi callback(void *, hexrays_event_t event, va_list va)
{
  switch ( event )
  {
    case hxe_maturity:
      {
        cfunc_t *func = va_arg(va, cfunc_t *);
        ctree_maturity_t mat = va_argi(va, ctree_maturity_t);
        if ( mat == CMAT_BUILT )
        {
          struct ida_local cblock_visitor_t : public ctree_visitor_t
          {
            cblock_visitor_t(void) : ctree_visitor_t(CV_FAST) {}
            int idaapi visit_insn(cinsn_t *ins)
            {
              if ( ins->op == cit_block )
                dump_block(ins->ea, ins->cblock);
              return 0;
            }
            void dump_block(ea_t ea, cblock_t *b)
            {
              // iterate over all block instructions
              msg("dumping block %a\n", ea);
              for ( cblock_t::iterator p=b->begin(); p != b->end(); ++p )
              {
                cinsn_t &i = *p;
                msg("  %a: insn %d\n", i.ea, i.op);
              }
            }
          };
          cblock_visitor_t cbv;
          cbv.apply_to(&func->body, NULL);
        }
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
static char comment[] = "Sample plugin7 for Hex-Rays decompiler";

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
  "Hex-Rays block iterator", // the preferred short name of the plugin
  ""                    // the preferred hotkey to run the plugin
};
