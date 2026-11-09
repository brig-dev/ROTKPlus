// Athelas - the launcher's look and its shared widgets: colours, fonts and scale, the settings rows, tabs, status dots.
#include "app.h"

// ---------------------------------------------------------------------------
// Theme

ImFont *F_BODY, *F_SMALL, *F_HEAD, *F_TITLE, *F_PLAY, *F_TAB;

float g_scale = 1.0f;

float g_scale0 = 1.0f;      // g_scale at the window's first size; the window can be resized (since 1.7)

int g_baseW = 880, g_baseH = 640;   // that first size, in pixels

void setup_style()
{
    ImGuiStyle &st = ImGui::GetStyle();
    st.WindowRounding = 0; st.ChildRounding = 0; st.FrameRounding = S(2); st.PopupRounding = S(2);
    st.GrabRounding = S(2); st.TabRounding = 0; st.ScrollbarRounding = S(2);
    st.WindowBorderSize = 0; st.ChildBorderSize = 1; st.FrameBorderSize = 1; st.PopupBorderSize = 1;
    st.FramePadding = ImVec2(S(10), S(5)); st.ItemSpacing = ImVec2(S(10), S(8)); st.ItemInnerSpacing = ImVec2(S(8), S(6));
    st.WindowPadding = ImVec2(S(18), S(14)); st.GrabMinSize = S(14); st.ScrollbarSize = S(12);
    st.DisabledAlpha = 0.5f;
    ImVec4 *c = st.Colors;
    c[ImGuiCol_Text] = C_TEXT;                    c[ImGuiCol_TextDisabled] = C_DIM;
    c[ImGuiCol_WindowBg] = rgb(0, 0, 0, 0);       c[ImGuiCol_ChildBg] = rgb(9, 12, 20, 0.80f);
    c[ImGuiCol_PopupBg] = rgb(14, 19, 30, 0.98f); c[ImGuiCol_Border] = rgb(81, 119, 140, 0.55f);
    c[ImGuiCol_FrameBg] = rgb(20, 27, 41);        c[ImGuiCol_FrameBgHovered] = rgb(27, 38, 57);
    c[ImGuiCol_FrameBgActive] = rgb(38, 47, 55);
    c[ImGuiCol_Button] = rgb(25, 35, 53);         c[ImGuiCol_ButtonHovered] = rgb(42, 56, 66);
    c[ImGuiCol_ButtonActive] = rgb(52, 73, 86);
    c[ImGuiCol_Header] = rgb(42, 56, 66);         c[ImGuiCol_HeaderHovered] = rgb(52, 73, 86);
    c[ImGuiCol_HeaderActive] = rgb(61, 87, 103);
    c[ImGuiCol_CheckMark] = C_GOLD_H;             c[ImGuiCol_SliderGrab] = C_GOLD; c[ImGuiCol_SliderGrabActive] = C_GOLD_H;
    c[ImGuiCol_Tab] = rgb(18, 24, 36);            c[ImGuiCol_TabHovered] = rgb(46, 63, 75);
    c[ImGuiCol_TabSelected] = rgb(37, 50, 60);    c[ImGuiCol_TabSelectedOverline] = C_GOLD;
    c[ImGuiCol_TabDimmed] = rgb(18, 24, 36);      c[ImGuiCol_TabDimmedSelected] = rgb(37, 50, 60);
    c[ImGuiCol_Separator] = rgb(81, 119, 140, 0.45f);
    c[ImGuiCol_ScrollbarBg] = rgb(0, 0, 0, 0.2f); c[ImGuiCol_ScrollbarGrab] = rgb(55, 73, 85);
    c[ImGuiCol_ScrollbarGrabHovered] = rgb(71, 97, 113); c[ImGuiCol_ScrollbarGrabActive] = C_GOLD;
    c[ImGuiCol_TableRowBg] = rgb(0, 0, 0, 0);     c[ImGuiCol_TableRowBgAlt] = rgb(198, 231, 247, 0.04f);
    c[ImGuiCol_NavCursor] = C_GOLD_H;             c[ImGuiCol_TextSelectedBg] = rgb(61, 87, 103);
    c[ImGuiCol_ModalWindowDimBg] = rgb(0, 0, 0, 0.55f);
    c[ImGuiCol_CheckboxSelectedBg] = rgb(20, 27, 41);   c[ImGuiCol_InputTextCursor] = C_TEXT;
    c[ImGuiCol_TextLink] = C_GOLD_H;                    c[ImGuiCol_TabDimmedSelectedOverline] = C_GOLD;
    c[ImGuiCol_SeparatorHovered] = C_GOLD;              c[ImGuiCol_SeparatorActive] = C_GOLD_H;
    c[ImGuiCol_ResizeGrip] = rgb(0, 0, 0, 0);           c[ImGuiCol_ResizeGripHovered] = rgb(0, 0, 0, 0);
    c[ImGuiCol_ResizeGripActive] = rgb(0, 0, 0, 0);     c[ImGuiCol_DragDropTarget] = C_GOLD_H;
}

// ---------------------------------------------------------------------------
// Widgets

float g_labelW;

static const char *g_rowDesc;

// One setting: label on the left, control on the right; the description shows
// as a tooltip when the mouse is over the row.
void row_begin(const char *label, const char *desc)
{
    g_rowDesc = desc;
    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    if (desc && *desc) {
        ImGui::SameLine(0, S(6));
        ImGui::PushFont(F_SMALL, F_SMALL->LegacySize);
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(rgb(81, 119, 140), "(?)");
        ImGui::PopFont();
    }
    ImGui::SameLine(g_labelW);
    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x - S(4), S(290)));
}

// The controller's (or the keyboard's) cursor is on something inside the item just drawn (a row's group)
static bool nav_cursor_in_last_item()
{
    ImGuiContext &g = *GImGui;
    if (!g.NavCursorVisible || !g.NavWindow || !g.NavId || g.NavWindow != ImGui::GetCurrentWindow()) return false;
    ImRect r = ImGui::WindowRectRelToAbs(g.NavWindow, g.NavWindow->NavRectRel[g.NavLayer]);
    return g.LastItemData.Rect.Contains(r.GetCenter());
}

void row_end()
{
    ImGui::EndGroup();
    ImGui::EndGroup();
    // the description: with the mouse over the row, or the controller's cursor on its control (no menu open)
    bool navHere = nav_cursor_in_last_item() && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    if (g_rowDesc && *g_rowDesc && (navHere || ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled |
                                                                    ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))) {
        ImGui::BeginTooltip();
        ImGui::PushFont(F_SMALL, F_SMALL->LegacySize);
        ImGui::PushTextWrapPos(S(380));
        ImGui::TextUnformatted(g_rowDesc);
        ImGui::PopTextWrapPos();
        ImGui::PopFont();
        ImGui::EndTooltip();
    }
}

// A heading: the title, then a tapering rule ending in a leaf scroll (ornament.cpp)
void section(const char *title)
{
    ImGui::Spacing();
    float right = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    ImGui::PushFont(F_HEAD, F_HEAD->LegacySize);
    ImGui::TextColored(C_GOLD, "%s", title);
    ImGui::PopFont();
    ImVec2 t0 = ImGui::GetItemRectMin(), t1 = ImGui::GetItemRectMax();
    orn_elf_rule(ImGui::GetWindowDrawList(), t1.x + S(16), right - S(4), (t0.y + t1.y) * 0.5f + S(1));
    ImGui::Dummy(ImVec2(0, S(2)));
}

// A choice between a few named options, shown as a drop-down menu.
// Options marked disabled (like "Custom") are shown when current but can't be picked.
bool segmented(const char *id, int *v, const char *const *names, int n, const bool *enabled)
{
    bool changed = false;
    ImGui::PushID(id);
    ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x - S(4), S(290)));
    if (ImGui::BeginCombo("##c", (*v >= 0 && *v < n) ? names[*v] : "")) {
        for (int i = 0; i < n; i++) {
            if (enabled && !enabled[i]) continue;
            if (ImGui::Selectable(names[i], *v == i) && *v != i) { *v = i; changed = true; }
            if (*v == i) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopID();
    return changed;
}

// In a dialog: Escape, or B on a controller, closes it as its Cancel button would (ImGui's modal dialogs don't)
bool dialog_back()
{
    return ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false);
}

bool gold_button(const char *label, ImVec2 size, ImFont *font)
{
    return orn_arch_button(label, size, font, true);
}

// A hover / controller-cursor explanation for the last item (as COGBox's ui_tip): the text stays off the page
void tip(const char *t)
{
    if (!t || !*t) return;
    if (!ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled) && !nav_cursor_in_last_item()) return;
    if (ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) return;
    ImGui::BeginTooltip();
    ImGui::PushFont(F_SMALL, F_SMALL->LegacySize);
    ImGui::PushTextWrapPos(S(380));
    ImGui::TextUnformatted(t);
    ImGui::PopTextWrapPos();
    ImGui::PopFont();
    ImGui::EndTooltip();
}

void set_message(App &a, const std::string &m, bool bad) { a.message = m; a.messageBad = bad; a.messageAt = ImGui::GetTime(); }

void status_dot(bool ok, bool warn, const char *label, const char *tip)
{
    ImVec4 col = ok ? C_GOOD : warn ? C_WARN : C_BAD;
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float h = ImGui::GetTextLineHeight();
    ImGui::BeginGroup();
    dl->AddCircleFilled(ImVec2(p.x + S(5), p.y + h * 0.55f), S(4), ImGui::GetColorU32(col));
    ImGui::SetCursorScreenPos(ImVec2(p.x + S(15), p.y));
    ImGui::TextUnformatted(label);
    ImGui::EndGroup();
    if (tip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(S(380));
        ImGui::TextUnformatted(tip);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// With a controller, LB and RB step to the previous and next tab (+1 / -1 here, 0 for none), unless a control is
// being changed (they speed it up or slow it down then), a menu is open or a button is being set
static int g_padTabFocus;   // frames left to put the controller's cursor on the new tab's first control

int pad_tab_step(const App &a)
{
    if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) || ImGui::IsAnyItemActive() || a.capture >= 0 ||
        a.kbCapture >= 0 || ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
        return 0;
    int step = (int)ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false) - (int)ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false);
    if (step) g_padTabFocus = 2;
    return step;
}

// Just after the open tab's BeginTabItem: after LB / RB, the cursor goes to the new tab itself (Down enters its page)
void pad_tab_focus()
{
    if (g_padTabFocus > 0 && --g_padTabFocus == 0) { ImGui::FocusItem(); ImGui::SetNavCursorVisible(true); }
}

// Tabs spread evenly over the width, each in a carved frame (orn_tab) with its name centred.
// ImGui's own tabs stay underneath, invisible, and only handle the clicks; 'sel' is the tab
// chosen last frame. Call before BeginTabBar, with the tab font pushed; pair with tabs_end().
float tabs_begin(const char *const *labels, int n, int sel)
{
    ImDrawList *dl = ImGui::GetWindowDrawList();
    const float gap = S(8), h = ImGui::GetFrameHeight();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float avail = ImGui::GetContentRegionAvail().x;
    float w = floorf((avail - gap * (n - 1)) / n);
    for (int i = 0; i < n; i++) {
        ImVec2 a(p.x + i * (w + gap), p.y + S(4)), b(a.x + w, p.y + h);
        bool hov = ImGui::IsMouseHoveringRect(a, b);
        orn_tab(dl, a, b, i == sel, hov);
        ImVec2 ts = ImGui::CalcTextSize(tr(labels[i]));
        dl->AddText(ImVec2(floorf(a.x + (w - ts.x) * 0.5f), floorf(a.y + (b.y - a.y - ts.y) * 0.5f)),
                    ImGui::GetColorU32(i == sel ? C_GOLD_H : hov ? C_TEXT : rgb(170, 186, 196)), tr(labels[i]));
    }
    // the line the tabs stand on, open under the chosen one
    float y = p.y + h, sx0 = p.x + sel * (w + gap), sx1 = sx0 + w;
    ImU32 line = ImGui::GetColorU32(rgb(103, 146, 172, 0.55f));
    if (sx0 > p.x) dl->AddLine(ImVec2(p.x, y), ImVec2(sx0, y), line, S(1.2f));
    if (sx1 < p.x + avail) dl->AddLine(ImVec2(sx1, y), ImVec2(p.x + avail, y), line, S(1.2f));
    // ImGui's tabs: the same places, nothing drawn
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(gap, ImGui::GetStyle().ItemInnerSpacing.y));
    ImGui::PushStyleVar(ImGuiStyleVar_TabBarBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_TabBarOverlineSize, 0.0f);
    for (ImGuiCol c : { ImGuiCol_Tab, ImGuiCol_TabHovered, ImGuiCol_TabSelected, ImGuiCol_TabSelectedOverline,
                        ImGuiCol_TabDimmed, ImGuiCol_TabDimmedSelected, ImGuiCol_TabDimmedSelectedOverline })
        ImGui::PushStyleColor(c, ImVec4(0, 0, 0, 0));
    return w;
}

void tabs_end() { ImGui::PopStyleColor(7); ImGui::PopStyleVar(3); }

// ---------------------------------------------------------------------------
// ROTKPlus: the hints at the bottom with the controller's buttons drawn as they look on it (Xbox's coloured letters,
// PlayStation's shapes, Nintendo's letters), the D-pad, the shoulder buttons and triggers, and keyboard keys, each
// followed by what it does. The button that confirms is the one at the bottom of the four (south), whatever its name.

static const ImU32 PG_DARK = IM_COL32(20, 25, 36, 240), PG_EDGE = IM_COL32(170, 190, 220, 150), PG_INK = IM_COL32(226, 232, 240, 255);

static float pg_pill_w(ImFont *f, float fs, const char *t, float gh) { return f->CalcTextSizeA(fs * 0.68f, FLT_MAX, 0, t).x + gh * 0.62f; }

// a rounded label (LB, L1, Options...), its left end at x, centred on cy; returns its width
static float pg_pill(ImDrawList *dl, float x, float cy, const char *t, ImFont *f, float fs, float gh, bool key = false)
{
    const float w = pg_pill_w(f, fs, t, gh), h = gh * (key ? 0.86f : 0.78f);
    dl->AddRectFilled(ImVec2(x, cy - h * 0.5f), ImVec2(x + w, cy + h * 0.5f), PG_DARK, key ? gh * 0.16f : h * 0.5f);
    dl->AddRect(ImVec2(x, cy - h * 0.5f), ImVec2(x + w, cy + h * 0.5f), PG_EDGE, key ? gh * 0.16f : h * 0.5f, S(1));
    if (key) dl->AddLine(ImVec2(x + S(2), cy + h * 0.5f - S(1.5f)), ImVec2(x + w - S(2), cy + h * 0.5f - S(1.5f)), IM_COL32(0, 0, 0, 120), S(2));
    const float ts = fs * 0.68f;
    ImVec2 sz = f->CalcTextSizeA(ts, FLT_MAX, 0, t);
    dl->AddText(f, ts, ImVec2(x + (w - sz.x) * 0.5f, cy - sz.y * 0.5f), PG_INK, t);
    return w;
}

// one of the four face buttons: which 0 south, 1 east, 2 west, 3 north
static void pg_face(ImDrawList *dl, ImVec2 c, float r, int which, int style, ImFont *f, float fs)
{
    if (style == NAMES_PS) {
        dl->AddCircleFilled(c, r, PG_DARK);
        dl->AddCircle(c, r, PG_EDGE, 0, S(1));
        const float k = r * 0.46f, t = std::max(S(1.3f), r * 0.17f);
        switch (which) {
        case 0: { ImU32 col = IM_COL32(130, 175, 240, 255);   // cross
                  dl->AddLine(c - ImVec2(k, k), c + ImVec2(k, k), col, t); dl->AddLine(c + ImVec2(-k, k), c + ImVec2(k, -k), col, t); break; }
        case 1: dl->AddCircle(c, k * 1.05f, IM_COL32(240, 100, 100, 255), 0, t); break;   // circle
        case 2: dl->AddRect(c - ImVec2(k * 0.9f, k * 0.9f), c + ImVec2(k * 0.9f, k * 0.9f), IM_COL32(230, 140, 210, 255), 0, t); break;
        default: { ImVec2 a(c.x, c.y - k * 1.05f), b(c.x + k * 1.05f, c.y + k * 0.75f), d(c.x - k * 1.05f, c.y + k * 0.75f);
                   dl->AddTriangle(a, b, d, IM_COL32(90, 210, 170, 255), t); break; }
        }
        return;
    }
    static const char *const xbox[4] = { "A", "B", "X", "Y" }, *const nin[4] = { "B", "A", "Y", "X" };
    static const ImU32 xcol[4] = { IM_COL32(96, 176, 70, 255), IM_COL32(214, 72, 62, 255), IM_COL32(58, 118, 226, 255),
                                   IM_COL32(232, 186, 46, 255) };
    const bool nintendo = style == NAMES_NINTENDO;
    dl->AddCircleFilled(c, r, nintendo ? PG_DARK : xcol[which]);
    dl->AddCircle(c, r, nintendo ? PG_EDGE : IM_COL32(255, 255, 255, 70), 0, S(1));
    const char *l = nintendo ? nin[which] : xbox[which];
    const float ts = fs * 0.78f;
    ImVec2 sz = f->CalcTextSizeA(ts, FLT_MAX, 0, l);
    dl->AddText(f, ts, ImVec2(c.x - sz.x * 0.5f + S(0.5f), c.y - sz.y * 0.5f), nintendo ? PG_INK : IM_COL32(255, 255, 255, 245), l);
}

// a glyph's width (draw = false) or the glyph itself, its left end at x, centred on cy
static float pg_glyph(ImDrawList *dl, float x, float cy, int g, int style, ImFont *f, float fs, bool draw)
{
    const float gh = fs * 1.2f, r = gh * 0.5f, gap = gh * 0.18f;
    const bool ps = style == NAMES_PS, nin = style == NAMES_NINTENDO;
    switch (g) {
    case PG_SOUTH: case PG_EAST: case PG_WEST: case PG_NORTH:
        if (draw) pg_face(dl, ImVec2(x + r, cy), r, g - PG_SOUTH, style, f, fs);
        return gh;
    case PG_DPAD_UD: {   // a cross, its up and down arms lit
        if (draw) {
            const float a = r * 0.98f, w = r * 0.66f;
            ImVec2 c(x + r, cy);
            const ImU32 fill = IM_COL32(40, 48, 68, 245);
            dl->AddRectFilled(c - ImVec2(w * 0.5f, a), c + ImVec2(w * 0.5f, a), fill, S(2));
            dl->AddRectFilled(c - ImVec2(a, w * 0.5f), c + ImVec2(a, w * 0.5f), fill, S(2));
            dl->AddRect(c - ImVec2(w * 0.5f, a), c + ImVec2(w * 0.5f, a), PG_EDGE, S(2), S(1));
            dl->AddRect(c - ImVec2(a, w * 0.5f), c + ImVec2(a, w * 0.5f), PG_EDGE, S(2), S(1));
            dl->AddRectFilled(c - ImVec2(w * 0.5f - S(1), w * 0.5f - S(1)), c + ImVec2(w * 0.5f - S(1), w * 0.5f - S(1)), fill);
            const float t = w * 0.3f;
            dl->AddTriangleFilled(ImVec2(c.x, c.y - a + S(2.5f)), ImVec2(c.x + t, c.y - a + S(2.5f) + t * 1.2f), ImVec2(c.x - t, c.y - a + S(2.5f) + t * 1.2f), PG_INK);
            dl->AddTriangleFilled(ImVec2(c.x, c.y + a - S(2.5f)), ImVec2(c.x - t, c.y + a - S(2.5f) - t * 1.2f), ImVec2(c.x + t, c.y + a - S(2.5f) - t * 1.2f), PG_INK);
        }
        return gh;
    }
    case PG_SHOULDERS: case PG_TRIGGERS: {
        const char *l = g == PG_SHOULDERS ? (ps ? "L1" : nin ? "L" : "LB") : (ps ? "L2" : nin ? "ZL" : "LT");
        const char *rr = g == PG_SHOULDERS ? (ps ? "R1" : nin ? "R" : "RB") : (ps ? "R2" : nin ? "ZR" : "RT");
        const char *mid = g == PG_SHOULDERS ? "/" : "+";
        const float mw = f->CalcTextSizeA(fs * 0.8f, FLT_MAX, 0, mid).x;
        float w = pg_pill_w(f, fs, l, gh) + gap + mw + gap + pg_pill_w(f, fs, rr, gh);
        if (draw) {
            float xx = x + pg_pill(dl, x, cy, l, f, fs, gh) + gap;
            dl->AddText(f, fs * 0.8f, ImVec2(xx, cy - fs * 0.45f), IM_COL32(150, 162, 182, 255), mid);
            pg_pill(dl, xx + mw + gap, cy, rr, f, fs, gh);
        }
        return w;
    }
    case PG_START: case PG_SELECT: {
        if (ps) { const char *t = g == PG_START ? "OPTIONS" : "SHARE"; if (draw) pg_pill(dl, x, cy, t, f, fs, gh); return pg_pill_w(f, fs, t, gh); }
        if (draw) {
            ImVec2 c(x + r, cy);
            dl->AddCircleFilled(c, r * 0.9f, PG_DARK);
            dl->AddCircle(c, r * 0.9f, PG_EDGE, 0, S(1));
            const float k = r * 0.42f, t = std::max(S(1.2f), r * 0.12f);
            if (nin) {   // + and -
                dl->AddLine(c - ImVec2(k, 0), c + ImVec2(k, 0), PG_INK, t);
                if (g == PG_START) dl->AddLine(c - ImVec2(0, k), c + ImVec2(0, k), PG_INK, t);
            } else if (g == PG_START) {   // Menu: three lines
                for (int i = -1; i <= 1; i++) dl->AddLine(ImVec2(c.x - k, c.y + i * k * 0.62f), ImVec2(c.x + k, c.y + i * k * 0.62f), PG_INK, t);
            } else {                      // View: two squares, one over the other
                dl->AddRect(ImVec2(c.x - k * 0.9f, c.y - k * 0.75f), ImVec2(c.x + k * 0.35f, c.y + k * 0.35f), PG_INK, 0, t * 0.8f);
                dl->AddRectFilled(ImVec2(c.x - k * 0.35f, c.y - k * 0.2f), ImVec2(c.x + k * 0.9f, c.y + k * 0.85f), PG_INK);
            }
        }
        return gh * 0.9f;
    }
    default: {   // keyboard keys
        const char *t = g == PG_KEY_ENTER ? tr("Enter") : g == PG_KEY_F11 ? "F11" : "Esc";
        if (draw) pg_pill(dl, x, cy, t, f, fs, gh, true);
        return pg_pill_w(f, fs, t, gh);
    }
    }
}

// the width of a row of hints, and the row drawn from pos (its top left)
static float pad_hints_do(ImDrawList *dl, ImVec2 pos, const PadHint *h, int n, int style, ImFont *f, float fs, ImU32 col, bool draw)
{
    const float gh = fs * 1.2f, cy = pos.y + gh * 0.5f, sep = gh * 1.1f;
    float x = pos.x;
    for (int i = 0; i < n; i++) {
        if (i) {
            if (draw) dl->AddCircleFilled(ImVec2(x + sep * 0.5f, cy), S(1.8f), IM_COL32(150, 162, 182, 140));
            x += sep;
        }
        x += pg_glyph(dl, x, cy, h[i].glyph, style, f, fs, draw) + gh * 0.32f;
        const char *t = tr(h[i].action);
        ImVec2 sz = f->CalcTextSizeA(fs, FLT_MAX, 0, t);
        if (draw) dl->AddText(f, fs, ImVec2(x, cy - sz.y * 0.5f), col, t);
        x += sz.x;
    }
    return x - pos.x;
}

float pad_hints_width(const PadHint *h, int n, int style, ImFont *f, float fs)
{
    return pad_hints_do(nullptr, ImVec2(0, 0), h, n, style, f, fs, 0, false);
}

void pad_hints_draw(ImDrawList *dl, ImVec2 pos, const PadHint *h, int n, int style, ImFont *f, float fs, ImU32 col)
{
    pad_hints_do(dl, pos, h, n, style, f, fs, col, true);
}

int pad_hints_test()
{
    const char *e = getenv("ROTKPLUS_PADHINTS");
    if (!e || !*e) return -1;
    return !_stricmp(e, "ps") ? NAMES_PS : !_stricmp(e, "nintendo") ? NAMES_NINTENDO : NAMES_XBOX;
}
