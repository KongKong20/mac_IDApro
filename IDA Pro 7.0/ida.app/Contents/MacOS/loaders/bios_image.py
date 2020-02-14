import idaapi
from idc import *

# -----------------------------------------------------------------------
def accept_file(li, filename):
    # ignore any trailer; align on 64KB boundary

    size = li.size()
    if (size & 0xFFF) != 0:
        size &= ~0xFFFF
    # check the code at F000:FFF0
    li.seek(size-16);
    jump = li.read(16)
    # skip any nops
    while jump[:1] == '\x90':
        jump = jump[1:]
    # skip wbinvd
    if jump.startswith('\x0F\x09'):
        jump = jump[2:]

    # is it a jump?
    if (
        jump.startswith('\xEA')       # jmp ptr16:16  EA oo oo ss ss
        and len(jump) >= 5
        and 0xF0 <= ord(jump[4]) <= 0xFE # segment should be above F000
       ) or (
        jump.startswith('\xE9')       # jmp rel16    E9 ll hh
        and len(jump) >= 3
        # and (ord(jump[2]) & 0x80) != 0 # jump backwards
       ) or (
        jump.startswith('\xEB')       # jmp rel8     EB dd
        and len(jump) >= 2
        and (ord(jump[1]) & 0x80) != 0 # jump backwards
       ):
        return {'format': "BIOS Image", 'processor':'metapc'} # accept the file

    return 0


def myAddSeg(startea, endea, base, use32, name, clas):
    s = idaapi.segment_t()
    s.start_ea = startea
    s.end_ea   = endea
    s.sel      = idaapi.setup_selector(base)
    s.bitness  = use32
    s.align    = idaapi.saRelPara
    s.comb     = idaapi.scPub
    idaapi.add_segm_ex(s, name, clas, idaapi.ADDSEG_NOSREG|idaapi.ADDSEG_OR_DIE)

# -----------------------------------------------------------------------
def load_file(li, neflags, format):
    chunksize = 0x10000
    base  = 0xF000
    start = base << 4;
    size = li.size()
    if (size & 0xFFF) != 0:
      size &= ~0xFFFF
    offs = size - chunksize

    idaapi.set_processor_type("metapc", idaapi.SETPROC_LOADER)

    if size < chunksize:
      offs = 0
      start += (chunksize - size)
      chunksize = size

    # make E and F segments for real-mode part
    myAddSeg(start, start+chunksize, base, 0, "BIOS_F", "CODE")
    li.file2base(offs, start, start+chunksize, 1)
    if offs > 0 and base > 0xE000:
      base  -= 0x1000
      start -= chunksize
      offs  -= chunksize
      myAddSeg(start, start+chunksize, base, 0, "BIOS_%X" % (base>>12), "CODE")
      li.file2base(offs, start, start+chunksize, 1)
      # set default ds to point to the current segment
      set_default_sreg_value(start, "ds", base)

    if offs > 0:
      # file is bigger than 128KB
      # make a flat 32-bit segment for the flash alias area
      start = (-size) & 0xFFFFFFFF # place it so that it ends at 4GB
      chunksize = size
      if not __EA64__:
        chunksize -= 2 # truncate last two bytes to avoid address space overlap
      # map the whole file
      offs = 0
      base = 0
      myAddSeg(start, start+chunksize, base, 1, "BIOS_FLASH", "CODE")
      li.file2base(offs, start, start+chunksize, 1)

    # set the entry registers to F000:FFF0
    set_inf_attr(INF_START_IP, 0xFFF0)
    set_inf_attr(INF_START_CS, 0xF000)
    # turn off "Convert 32bit instruction operand to offset", too many false positives in high areas
    set_inf_attr(INF_AF, get_inf_attr(INF_AF) & ~AF_IMMOFF)
    # turn off "Create function tails"
    set_inf_attr(INF_AF, get_inf_attr(INF_AF) & ~AF_FTAIL)

    return 1

# -----------------------------------------------------------------------
def move_segm(frm, to, sz, fileformatname):
    warning("move_segm(from=%s, to=%s, sz=%d, formatname=%s" % (hex(frm), hex(to), sz, fileformatname))
    return 0
