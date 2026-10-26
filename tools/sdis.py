# Python 3 + pefile + capstone: disassemble rotk.exe from an address.
# Usage: python sdis.py <hex address> [number of instructions]
# The exe is taken from the environment variable ROTK_EXE (path of rotk.exe 1.0, 1,703,936 bytes).
# Do not commit the exe: it is the game's own code.
import os, sys, pefile, capstone
exe = os.environ.get('ROTK_EXE')
if not exe:
    sys.exit('set ROTK_EXE to the path of rotk.exe')
pe = pefile.PE(exe, fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
a = int(sys.argv[1], 16)
n = int(sys.argv[2]) if len(sys.argv) > 2 else 40
for i, ins in enumerate(md.disasm(img[a - base:a - base + n * 16], a)):
    if i >= n:
        break
    print('%08X  %-8s %s' % (ins.address, ins.mnemonic, ins.op_str))
