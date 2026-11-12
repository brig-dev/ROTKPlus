// Athelas - "Recommended for this PC": what the PC has (graphics card, video memory, screen, processor) and the
// graphics tier that fits it. Read once, the first time it's asked for.
#pragma once
#include <string>

struct HwInfo {
    std::string gpu;          // the graphics card's name
    unsigned vendor = 0;      // PCI vendor: 0x10DE NVIDIA, 0x1002 AMD, 0x8086 Intel
    unsigned vramMB = 0;      // dedicated video memory
    bool integrated = false;  // graphics built into the processor (or a software adapter)
    int screenW = 0, screenH = 0, refresh = 60;
    int cores = 1;            // logical processors
    int tier = 1;             // 0 light (integrated / handheld), 1 mid-range, 2 strong, 3 very strong
};
// ROTKPlus: read on a thread of its own from the start (hw_info_start): asking the graphics driver can take seconds,
// and on the window's thread the first visit to the Graphics tab could freeze the launcher (owner's report, 11 Nov:
// 10-15 s while clicking through the settings tabs). Until it's ready hw_ready() is false and hw_info() gives defaults.
void hw_info_start();
bool hw_ready();
const HwInfo &hw_info();
const char *hw_tier_name(int tier);
