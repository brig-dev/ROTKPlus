// ROTK Fix Launcher - drawn ornaments (see ornament.h). Everything is drawn with ImGui's
// shapes at the current scale; there are no pictures.
#include "ornament.h"
#include "i18n.h"
#include <string>
#include "imgui_internal.h"
#include <math.h>
#include <algorithm>

static float g_s = 1.0f;
static float O(float v) { return v * g_s; }
void orn_set_scale(float s) { g_s = s; }

static ImU32 col(int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); }
static ImVec2 V(float x, float y) { return ImVec2(x, y); }

// ROTKPlus: night blue and moonlit silver

// ---------------------------------------------------------------------------
// Men: the window's granite border, with Gothic quatrefoils and a pointed gable

// ROTKPlus: a plain night-blue band with two silver lines and a small diamond at each corner (Athelas has a granite
// border with Gothic quatrefoils and a pointed gable)
static void diamond(ImDrawList *dl, ImVec2 c, float r, ImU32 fill)
{
    ImVec2 d[4] = { V(c.x - r, c.y), V(c.x, c.y - r), V(c.x + r, c.y), V(c.x, c.y + r) };
    dl->AddConvexPolyFilled(d, 4, fill);
}
void orn_window_frame(ImDrawList *dl, ImVec2 size)
{
    float m = O(2), t = O(12);
    ImVec2 a(m, m), b(size.x - m, size.y - m);
    ImU32 band = IM_COL32(12, 18, 32, 255);
    dl->AddRectFilled(a, V(b.x, a.y + t), band);
    dl->AddRectFilled(V(a.x, b.y - t), b, band);
    dl->AddRectFilled(V(a.x, a.y + t), V(a.x + t, b.y - t), band);
    dl->AddRectFilled(V(b.x - t, a.y + t), V(b.x, b.y - t), band);
    dl->AddRect(a, b, IM_COL32(150, 170, 205, 200), 0, O(1.2f));
    dl->AddRect(V(a.x + t, a.y + t), V(b.x - t, b.y - t), IM_COL32(150, 170, 205, 140), 0, O(1));
    float mid = t * 0.5f;
    for (ImVec2 c : { V(a.x + mid, a.y + mid), V(b.x - mid, a.y + mid), V(a.x + mid, b.y - mid), V(b.x - mid, b.y - mid) })
        diamond(dl, c, O(4), IM_COL32(200, 214, 238, 230));
}

// ---------------------------------------------------------------------------
// Elves: slender lines, notched corners, pointed arches


void orn_panel_fill(ImDrawList *dl, ImVec2 a, ImVec2 b, ImU32 fill)
{
    dl->AddRectFilled(a, b, fill, O(8));   // ROTKPlus: rounded, as its frame
}

void orn_panel_frame(ImDrawList *dl, ImVec2 a, ImVec2 b)
{
    // ROTKPlus: a rounded silver line and a fainter one inside, a small diamond at the top centre
    dl->AddRect(a, b, IM_COL32(170, 188, 220, 220), O(8), O(1.4f), 0);
    float g = O(4);
    dl->AddRect(V(a.x + g, a.y + g), V(b.x - g, b.y - g), IM_COL32(120, 145, 190, 90), O(6), O(0.8f), 0);
    diamond(dl, V((a.x + b.x) * 0.5f, a.y), O(5), IM_COL32(214, 226, 246, 240));
}

void orn_plate(ImDrawList *dl, ImVec2 a, ImVec2 b, ImU32 fill, bool lit)
{
    float r = std::min(O(8), (b.y - a.y) * 0.35f);
    dl->AddRectFilled(a, b, fill, r);
    dl->AddRect(a, b, lit ? IM_COL32(214, 226, 246, 255) : IM_COL32(150, 170, 205, 170), r, O(1.3f), 0);
}

void orn_elf_rule(ImDrawList *dl, float x0, float x1, float y)
{
    // ROTKPlus: a silver line fading to the right, a small diamond where it starts
    if (x1 - x0 < O(60)) return;
    dl->AddRectFilledMultiColor(V(x0, y - O(0.7f)), V(x1, y + O(0.7f)), IM_COL32(190, 206, 236, 230), IM_COL32(190, 206, 236, 0),
                                IM_COL32(190, 206, 236, 0), IM_COL32(190, 206, 236, 230));
    diamond(dl, V(x0 - O(4), y), O(3.5f), IM_COL32(214, 226, 246, 240));
}

// ---------------------------------------------------------------------------
// Buttons

bool orn_arch_button(const char *label, ImVec2 size, ImFont *font, bool enabled)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::BeginDisabled(!enabled);
    bool pressed = ImGui::InvisibleButton(label, size);
    // the controller's / keyboard's cursor on it lights it as the mouse does: an InvisibleButton draws no cursor of
    // its own, so Library, Save and Play showed nothing when reached with a controller (5 Nov)
    bool hov = enabled && (ImGui::IsItemHovered() || (ImGui::IsItemFocused() && GImGui->NavCursorVisible)),
         act = enabled && ImGui::IsItemActive();
    ImGui::EndDisabled();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    // ROTKPlus: a rounded night-blue plate with a silver edge (Athelas has a round arch with a bronze boss)
    float archH = 0.0f, cx = p.x + size.x * 0.5f;
    ImVec2 a(p.x, p.y), b(p.x + size.x, p.y + size.y);
    ImU32 fill = !enabled ? col(40, 44, 54) : act ? col(36, 58, 96) : hov ? col(58, 88, 140) : col(44, 70, 116);
    dl->AddRectFilled(a, b, fill, O(10));
    dl->AddRectFilledMultiColor(V(a.x + O(3), a.y + O(3)), V(b.x - O(3), a.y + size.y * 0.5f), col(255, 255, 255, 28),
                                col(255, 255, 255, 28), col(255, 255, 255, 0), col(255, 255, 255, 0));
    dl->AddRect(a, b, enabled ? col(200, 214, 238) : col(90, 96, 110), O(10), O(1.5f), 0);
    ImGui::PushFont(font, font->LegacySize);
    std::string shownS = tr(std::string(label, ImGui::FindRenderedTextEnd(label)));   // its translation (drawn here, not by ImGui)
    const char *shown = shownS.c_str();
    ImVec2 ts = ImGui::CalcTextSize(shown);
    ImVec2 tp(cx - ts.x * 0.5f, a.y + archH * 0.5f + (size.y - archH * 0.5f - ts.y) * 0.5f);
    dl->AddText(V(tp.x + O(1), tp.y + O(1.5f)), col(4, 12, 25, enabled ? 200 : 60), shown);
    dl->AddText(tp, enabled ? col(188, 220, 233) : col(100, 112, 140), shown);
    ImGui::PopFont();
    return pressed && enabled;
}

bool orn_plate_button(const char *label, ImVec2 size, bool enabled, bool nav)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::BeginDisabled(!enabled);
    bool pressed = ImGui::InvisibleButton(label, size, nav ? ImGuiButtonFlags_EnableNav : ImGuiButtonFlags_None);
    // the controller's / keyboard's cursor on it lights it as the mouse does: an InvisibleButton draws no cursor of
    // its own, so Library, Save and Play showed nothing when reached with a controller (5 Nov)
    bool hov = enabled && (ImGui::IsItemHovered() || (ImGui::IsItemFocused() && GImGui->NavCursorVisible)),
         act = enabled && ImGui::IsItemActive();
    ImGui::EndDisabled();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImU32 fill = !enabled ? col(26, 28, 36, 220) : act ? col(46, 62, 82, 235) : hov ? col(38, 48, 64, 235) : col(24, 26, 34, 225);
    orn_plate(dl, p, V(p.x + size.x, p.y + size.y), fill, enabled && hov);
    std::string shownS = tr(std::string(label, ImGui::FindRenderedTextEnd(label)));   // its translation (drawn here, not by ImGui)
    const char *shown = shownS.c_str();
    ImVec2 ts = ImGui::CalcTextSize(shown);
    ImVec2 tp(p.x + (size.x - ts.x) * 0.5f, p.y + (size.y - ts.y) * 0.5f);
    dl->AddText(tp, enabled ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_TextDisabled), shown);
    return pressed && enabled;
}

// A tab (elves): notched top corners, a bronze edge with a fine silver line inside, open at
// the bottom where it meets the page. The chosen tab is lit, with a bronze diamond on top.
void orn_tab(ImDrawList *dl, ImVec2 a, ImVec2 b, bool selected, bool hovered)
{
    // ROTKPlus: rounded top corners, a silver edge, the chosen one lighter with a line along its bottom
    float r = std::min(O(7), (b.y - a.y) * 0.3f);
    ImU32 fill = selected ? col(44, 70, 116, 235) : hovered ? col(32, 50, 84, 215) : col(12, 18, 32, 190);
    dl->AddRectFilled(a, b, fill, r, ImDrawFlags_RoundTop);
    dl->AddRect(a, V(b.x, b.y + O(1)), selected ? col(214, 226, 246) : hovered ? col(170, 188, 220) : col(110, 130, 170, 160), r,
                O(1.2f), ImDrawFlags_RoundTop);
    if (selected) dl->AddLine(V(a.x + r, b.y - O(2)), V(b.x - r, b.y - O(2)), col(214, 226, 246), O(2));
}
