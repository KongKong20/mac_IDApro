#include <idc.idc>

static main()
{
}

// Android Bionic libc
//
// These functions are called while loading startup signatures from
// elf.sig to obtain the address of main.

static get_main_ea(ea, is_pic)
{
  ea = get_wide_dword(ea);
  if ( ea == BADADDR )
    return BADADDR;

  if ( is_pic == 1 )
  {
    auto got = get_gotea();
    if ( got == BADADDR )
      return BADADDR;
    ea = get_wide_dword(got + ea);
  }

  return ea;
}

static get_main_ea_pic(ea)
{
  return get_main_ea(ea, 1);
}

static get_main_ea_abs(ea)
{
  return get_main_ea(ea, 0);
}
