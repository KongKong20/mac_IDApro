
//
//      Redefine the predefined bits for 8051 processor.
//      Before using the script you need to create
//      a enum called "BitFlags" and enter the bit
//      definitions there.
//
//      This file is provided by an IDA user.
//-------
//      By just defining the bits I want
//      in "BitFlags", only those have the operand overriden.
//      Cross references even work for the new names.  Life is good!
//      I do have to re-run this every time I add a new bit
//      to the enumeration.


#include <idc.idc>

static main() {
  auto ea, enumID, constID;

  enumID = get_enum("BitFlags");
  if ( enumID == -1 )
        return;
  for ( ea = get_inf_attr(INF_MIN_EA); ea != BADADDR; ea=find_code(ea,1) ) {
    auto x;

    x = get_operand_type(ea,0);
    if ( x != 8 ) continue;

    msg("addr %x\n",ea);
    x = get_wide_byte(ea+1);
    constID = GetConst(enumID,x,-1);
    if ( constID != -1 ){
        op_man(ea,0,get_enum_member_name(constID));
    }
  }
}

