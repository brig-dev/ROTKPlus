// ROTKPlus - what the launcher still needs from Athelas' Xbox ports, without them: the "Recommended for this PC" button
// of the PC game's presets (it lived with the ports' pages) and empty stand-ins for the ports' functions the shared
// pages still name (no Xbox game is ever listed here: library.cpp keeps The Return of the King on PC only). The ports'
// pages (xbox_pages.cpp), their builder (builder.cpp) and the Mods tab (mods.cpp) are no longer built.
#include "app.h"
#include "hwrec.h"

bool rec_button(const char *id, const std::vector<std::string> &changes, bool canUndo, bool *undo)
{
    bool applied = false;
    const HwInfo &h = hw_info();
    // (a std::string: a long translation in a fixed buffer would cut off the "##rec" ID, or a character in two)
    std::string btn = std::string(tr("Recommended for this PC")) + "##rec" + id;
    row_begin("", "Looks at this PC's graphics card, video memory, screen and processor, and picks the settings that "
                  "should run the game smoothly. You see what changes before anything does.");
    static bool testOpen = getenv("ATHELAS_REC_OPEN") != nullptr;   // testing: the box open at once
    if (ImGui::Button(btn.c_str()) || testOpen) { testOpen = false; ImGui::OpenPopup((std::string("recpop") + id).c_str()); }
    if (canUndo) {
        ImGui::SameLine();
        if (ImGui::Button((std::string(tr("Undo")) + "##recundo" + id).c_str())) *undo = true;
    }
    if (ImGui::BeginPopup((std::string("recpop") + id).c_str())) {
        ImGui::TextColored(C_GOLD_H, "%s", tr("This PC"));
        ImGui::Text("%s%s, %u MB", h.gpu.empty() ? tr("Unknown graphics card") : h.gpu.c_str(),
                    h.integrated ? tr(" (built-in)") : "", h.vramMB);
        ImGui::Text(tr("Screen %dx%d at %d Hz, %d processor threads"), h.screenW, h.screenH, h.refresh, h.cores);
        ImGui::Text(tr("Rated: %s"), tr(hw_tier_name(h.tier)));
        ImGui::Separator();
        if (changes.empty()) ImGui::TextUnformatted(tr("Your settings already match the recommendation."));
        else {
            ImGui::TextColored(C_GOLD_H, "%s", tr("Will change"));
            for (auto &c : changes) ImGui::BulletText("%s", c.c_str());
            if (ImGui::Button(tr("Apply"))) { applied = true; ImGui::CloseCurrentPopup(); }
            ImGui::SameLine();
        }
        if (ImGui::Button(tr("Close"))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    row_end();
    return applied;
}

void draw_tt(App &, const LibGame &) {}
void tt_put(App &, const char *, const char *, const std::string &) {}
void tt_apply_refresh(App &) {}
int tt_button_count(App &) { return 0; }
bool tt_assign_button(App &, int, int) { return false; }
void tt_tabs(App &, const LibGame &) {}
std::string tt_summary(const App &) { return std::string(); }
void builder_cleanup_old() {}
std::string xbe_sha1(const LibGame &) { return std::string(); }
