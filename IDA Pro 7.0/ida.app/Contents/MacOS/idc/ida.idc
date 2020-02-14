//
//      This file is automatically executed when IDA is started.
//      You can define your own IDC functions and assign hotkeys to them.
//
//      You may add your frequently used functions here and they will
//      be always available.
//
//
#include <idc.idc>

//-----------------------------------------------------------------------
// A singleton class for managing breakpoints
class BreakpointManager
{
  // Return the breakpoint quantity
  Count()
  {
    return get_bpt_qty();
  }

  // Return a breakpoint object
  Get(index)
  {
    auto count = this.Count();
    if ( index >= count )
      throw form("Invalid breakpoint index %d (0..%d expected).", index, count);
    return Breakpoint(index);
  }

  // Add a breakpoint
  Add(bpt)
  {
    return bpt._add();
  }

  // Add a breakpoint to group
  AddToGroup(bpt, group_name)
  {
    return bpt._add(group_name);
  }

  // Delete a breakpoint
  Delete(bpt)
  {
    return bpt._delete();
  }

  // Update a breakpoint
  // Note: Location attributes cannot be updated, recreation of the
  //       breakpoint is required
  Update(bpt)
  {
    return bpt._update();
  }

  // Find a breakpoint using its location attributes and
  // returns a new breakpoint object or 0
  Find(bpt)
  {
    return bpt._find();
  }
}

// Breakpoint manager class instance
extern Breakpoints;

//-----------------------------------------------------------------------
// Get signed extended 16-bit value
static SWord(ea)
{
  auto v = get_wide_word(ea);
  if ( v & 0x8000 )
    v = v | ~0xFFFF;
  return v;
}

//-----------------------------------------------------------------------
static main(void)
{
  //
  //      This function is executed when IDA is started.
  //
  //      Add statements to fine-tune your IDA here.
  //

  // Instantiate the breakpoints singleton object
  Breakpoints = BreakpointManager();

  // uncomment this line to remove full paths in the debugger process options:
  // set_inf_attr(INF_LFLAGS, LFLG_DBG_NOPATH|get_inf_attr(INF_LFLAGS));
}
