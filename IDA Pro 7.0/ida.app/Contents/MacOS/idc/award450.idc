//
//      This file for Award BIOS version 4.5 disassemble.
//
// History:
//
// 18.05.96 23:19 Started by Alexey Kulentsov
// 13/07/96 20:03 Edited for IDA 3.06.14 (OpOffset -> OpOff)

#include <idc.idc>

static main(void) {
  auto addr,x,i,base;

  base = 0xF000;   // The segment base
  x = base<<4;

  msg("Make Font at F000:FA6E\n");
  create_byte(x+0xFA6E);
  make_array(x+0xFA6E,1024);
  set_name(x+0xFA6E,"Font8x8");

  addr=0xFEE3;
  msg(form("Flags=%lX\n",get_full_flags(x+addr)));
  if(get_wide_word(x+get_wide_word(x+addr))==0x501E)
  { msg("Award Int_Table at F000:FEE3 found\n");
    for(addr=0xFEE3;addr<0xFF1D;addr=addr+2)
    { get_full_flags(x+addr);
      op_plain_offset(x+addr,-1,x);
    }
  }

  msg("Check for the stackless proc. calling\n");
  for(addr=0;addr<0x10000L;addr=addr+get_item_size(addr+x))
  {
        i=addr+x;
        // Check for the stackless proc. calling:
        //      mov sp, offset @@olabel
        //      jmp proc
        //@@olabel:
        //      dw      offset @@clabel
        //@@clabel:
        //      ; next instructions..
        if(get_wide_byte(i)==0xBC
        && get_wide_word(i+1)==addr+6
        && get_wide_word(i+6)==addr+8
        && get_wide_byte(i+3)==0xE9
        )
        {
                AutoMark(i,AU_CODE);
                AutoMark(i+8,AU_CODE);
                auto_wait();
                del_items(i+6,1);
                create_word(i+6);

                op_plain_offset(i+6,0,base);
                op_plain_offset(i,1,base);
                auto ProcAddr=x+((get_wide_word(i+4)+addr+6)&0xFFFF);
                AutoMark(ProcAddr,AU_CODE);
                AutoMark(ProcAddr,AU_PROC);
                msg(form("RetAddr in [[SP]] proc fixed at %04X\n",addr));
        } else
        // mov bx,const or mov di,const
        if(get_wide_byte(i)==0xBB || get_wide_byte(i)==0xBF)
        {       if(get_wide_word(i+1)==addr+5 && get_wide_byte(i+3)==0xEB)        // jmp short
                {       create_insn(i);create_insn(i+5);auto_wait();
                        op_plain_offset(i,1,base);
                        msg(form("RetAddr in BX or DI proc fixed at %04X\n",addr));
                } else
                if(get_wide_word(i+1)==addr+6 && get_wide_byte(i+3)==0xE9)        // jmp near
                {       create_insn(i);create_insn(i+6);auto_wait();
                        op_plain_offset(i,1,base);
                        msg(form("RetAddr in BX or DI proc fixed at %04X\n",addr));
                }
        }
  }
}
