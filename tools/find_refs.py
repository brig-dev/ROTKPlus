# Python 3 + pefile + capstone: find the instructions in rotk.exe whose displacement or immediate is an address in
# the given range, disassembling linearly from the start of the function (aligned to 16 after padding). Prints the
# instruction address, the offset of the 32-bit value inside the instruction, and the text.
# Usage: python find_refs.py <from> <to>   (hexadecimal, 'to' excluded)
# The exe is taken from the environment variable ROTK_EXE.
import os, sys, struct, pefile, capstone
exe = os.environ.get('ROTK_EXE')
if not exe:
    sys.exit('set ROTK_EXE to the path of rotk.exe')
pe = pefile.PE(exe, fast_load=True)
B = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
lo, hi = int(sys.argv[1], 16), int(sys.argv[2], 16)
code_lo, code_hi = 0x401000, 0x535000
res = set()
for off in range(code_lo - B, code_hi - B - 4):
    v = struct.unpack_from('<I', img, off)[0]
    if not lo <= v < hi:
        continue
    hit = B + off
    p = hit & ~0xF
    while p > code_lo and not (img[p - 1 - B] in (0x90, 0xCC, 0xC3) and p % 16 == 0 and p < hit - 16):
        p -= 16
    for ins in md.disasm(img[p - B:hit + 16 - B], p):
        if ins.address <= hit < ins.address + ins.size:
            if ins.address + ins.size >= hit + 4:
                res.add((ins.address, hit - ins.address, ins.mnemonic + ' ' + ins.op_str, v))
            break
for a, o, t, v in sorted(res):
    print('%08X +%d %08X  %s' % (a, o, v, t))
