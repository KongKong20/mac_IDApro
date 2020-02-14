/*
 *      Hex-Rays Decompiler project
 *      Copyright (c) 2007-2008 by Hex-Rays, support@hex-rays.com
 *      ALL RIGHTS RESERVED.
 *
 *      Sample plugin for Hex-Rays Decompiler usage of udc_filter_t
 *      class: decompile svc 0x900001 and svc 0x9000F8 as function calls to
 *      svc_exit() and svc_exit_group() respectively.
 *
 *      The command hotkey is Ctrl+Shift+U.
 *      It is also added into the right-click menu as "Toggle UDC"
 *
 */

#include <hexrays.hpp>
#include <allins.hpp>

// Hex-Rays API pointer
hexdsp_t *hexdsp = NULL;
static bool inited = false;

#define ACTION_NAME "sample8:udcall"
// Shortcut for the new command
#define ACTION_SHORTCUT "Ctrl+Shift+U"

#define SVC_EXIT       0x900001
#define SVC_EXIT_GROUP 0x9000F8

//--------------------------------------------------------------------------
class udc_exit_t : public udc_filter_t
{
  int code;
  bool installed;

public:
  udc_exit_t() : code(0), installed(false) {}
  bool prepare(int svc_code, const char *name)
  {
    char decl[MAXSTR];
    qsnprintf(decl, sizeof(decl), "int __usercall %s@<R0>(int status@<R1>);", name);
    bool ok = init(decl);
    if ( !ok )
      msg("Could not initialize UDC plugin '%s'\n", name);
    code = svc_code;
    return ok;
  }
  void install()
  {
    install_microcode_filter(this, true);
    installed = true;
  }
  void uninstall()
  {
    install_microcode_filter(this, false);
    installed = false;
  }
  void toggle_install()
  {
    if ( installed )
      uninstall();
    else
      install();
  }
  virtual bool match(codegen_t &cdg)
  {
    return cdg.insn.itype == ARM_svc && cdg.insn.Op1.value == code;
  }
};

static udc_exit_t udc_exit;
static udc_exit_t udc_exit_group;

//--------------------------------------------------------------------------
// This callback handles various hexrays events.
static int idaapi callback(void *, hexrays_event_t event, va_list va)
{
  switch ( event )
  {
    case hxe_open_pseudocode:
      {
        vdui_t &vu = *va_arg(va, vdui_t *);
        // Permanently attach that action to that view's context menu.
        attach_action_to_popup(vu.ct, NULL, ACTION_NAME);
      }
      break;

    default:
      break;
  }
  return 0;
}

//--------------------------------------------------------------------------
// menu action handler: installs/uninstalls UDC filter and rebuilds pseudocode
struct toggle_udc_ah_t : public action_handler_t
{
  virtual int idaapi activate(action_activation_ctx_t *ctx)
  {
    udc_exit.toggle_install();
    udc_exit_group.toggle_install();
    vdui_t *vu = get_widget_vdui(ctx->widget);
    vu->refresh_view(true);
    return 1;
  }

  virtual action_state_t idaapi update(action_update_ctx_t *ctx)
  {
    vdui_t *vu = get_widget_vdui(ctx->widget);
    return vu == NULL ? AST_DISABLE_FOR_WIDGET : AST_ENABLE_FOR_WIDGET;
  }
};

static toggle_udc_ah_t toggle_udc_ah;

static const action_desc_t udc_action = ACTION_DESC_LITERAL(
        ACTION_NAME,
        "Toggle UDC",
        &toggle_udc_ah,
        ACTION_SHORTCUT,
        NULL,
        -1);


//--------------------------------------------------------------------------
int idaapi init(void)
{
  if ( !init_hexrays_plugin() )
    return PLUGIN_SKIP; // no decompiler
  if ( ph.id != PLFM_ARM || inf.is_64bit() )
    return false;       // for arm32 only
  install_hexrays_callback(callback, NULL);
  const char *hxver = get_hexrays_version();
  msg("Hex-rays version %s has been detected, %s ready to use\n", hxver, PLUGIN.wanted_name);
  if ( !udc_exit.prepare(SVC_EXIT, "svc_exit") )
    return PLUGIN_SKIP;
  if ( !udc_exit_group.prepare(SVC_EXIT_GROUP, "svc_exit_group") )
    return PLUGIN_SKIP;
  register_action(udc_action);
  inited = true;
  return PLUGIN_KEEP;
}

//--------------------------------------------------------------------------
void idaapi term(void)
{
  if ( inited )
  {
    udc_exit.uninstall();
    udc_exit_group.uninstall();
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
static const char comment[] = "Convert SVC instructions to exit/exit_group function calls";

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  PLUGIN_HIDE,                   // plugin flags
  init,                          // initialize
  term,                          // terminate. this pointer may be NULL.
  run,                           // invoke plugin
  comment,                       // long comment about the plugin
                                 // it could appear in the status line
                                 // or as a hint
  "",                            // multiline help about the plugin
  "Hex-Rays user-defined calls", // the preferred short name of the plugin
  ""                             // the preferred hotkey to run the plugin
};
