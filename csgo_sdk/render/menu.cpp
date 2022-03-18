#include "Menu.hpp"

#define NOMINMAX

#include <Windows.h>
#include <chrono>

#include "../sdk/csgostructs.hpp"
#include "../sdk/utils/input.hpp"
#include "../configurations.hpp"
#include "../config.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

#include "../functions/misc.hpp"
#include "../helpers/keybinds.hpp"
#include "../functions/SpecList.hpp"
#include "../functions/visuals.hpp"

const char* keys[] = {
    "[-]",
    "[M1]",
    "[M2]",
    "[CN]",
    "[M3]",
    "[M4]",
    "[M5]",
    "[-]",
    "[BAC]",
    "[TAB]",
    "[-]",
    "[-]",
    "[CLR]",
    "[RET]",
    "[-]",
    "[-]",
    "[SHI]",
    "[CTL]",
    "[MEN]",
    "[PAU]",
    "[CAP]",
    "[KAN]",
    "[-]",
    "[JUN]",
    "[FIN]",
    "[KAN]",
    "[-]",
    "[ESC]",
    "[CON]",
    "[NCO]",
    "[ACC]",
    "[MAD]",
    "[SPA]",
    "[PGU]",
    "[PGD]",
    "[END]",
    "[HOM]",
    "[LEF]",
    "[UP]",
    "[RIG]",
    "[DOW]",
    "[SEL]",
    "[PRI]",
    "[EXE]",
    "[PRI]",
    "[INS]",
    "[DEL]",
    "[HEL]",
    "[0]",
    "[1]",
    "[2]",
    "[3]",
    "[4]",
    "[5]",
    "[6]",
    "[7]",
    "[8]",
    "[9]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[A]",
    "[B]",
    "[C]",
    "[D]",
    "[E]",
    "[F]",
    "[G]",
    "[H]",
    "[I]",
    "[J]",
    "[K]",
    "[L]",
    "[M]",
    "[N]",
    "[O]",
    "[P]",
    "[Q]",
    "[R]",
    "[S]",
    "[T]",
    "[U]",
    "[V]",
    "[W]",
    "[X]",
    "[Y]",
    "[Z]",
    "[WIN]",
    "[WIN]",
    "[APP]",
    "[-]",
    "[SLE]",
    "[NUM]",
    "[NUM]",
    "[NUM]",
    "[NUM]",
    "[NUM]",
    "[NUM]",
    "[NUM]",
    "[NUM]",
    "[NUM]",
    "[NUM]",
    "[MUL]",
    "[ADD]",
    "[SEP]",
    "[MIN]",
    "[DEC]",
    "[DIV]",
    "[F1]",
    "[F2]",
    "[F3]",
    "[F4]",
    "[F5]",
    "[F6]",
    "[F7]",
    "[F8]",
    "[F9]",
    "[F10]",
    "[F11]",
    "[F12]",
    "[F13]",
    "[F14]",
    "[F15]",
    "[F16]",
    "[F17]",
    "[F18]",
    "[F19]",
    "[F20]",
    "[F21]",
    "[F22]",
    "[F23]",
    "[F24]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[NUM]",
    "[SCR]",
    "[EQU]",
    "[MAS]",
    "[TOY]",
    "[OYA]",
    "[OYA]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[-]",
    "[SHI]",
    "[SHI]",
    "[CTR]",
    "[CTR]",
    "[ALT]",
    "[ALT]"
};

float ImGui::CalcMaxPopupHeightFromItemCount(int items_count) {
    ImGuiContext& g = *GImGui;
    if (items_count <= 0)
        return FLT_MAX;
    return (19 * items_count + items_count % 4) + 5;
}

bool ImGui::Keybind(const char* str_id, CKeyBind* kbind) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    SameLine(window->Size.x - 20);


    ImGuiContext& g = *GImGui;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(str_id);
    ImGuiIO* io = &GetIO();

    ImVec2 label_size = CalcTextSize(keys[kbind->key]);
    ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + label_size);
    ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(window->Pos.x + window->Size.x - window->DC.CursorPos.x, label_size.y));
    ItemSize(total_bb, style.FramePadding.y);
    char buf_display[64] = "[-]";

    if (kbind->key != 0 && g.ActiveId != id)
        strcpy_s(buf_display, keys[kbind->key]);
    else if (g.ActiveId == id)
        strcpy_s(buf_display, "[-]");

    total_bb.Min.x -= label_size.x;
    frame_bb.Min.x -= label_size.x;

    if (!ItemAdd(total_bb, id, &frame_bb))
        return false;

    const bool hovered = IsItemHovered();
    const bool edit_requested = hovered && io->MouseClicked[0];
    const bool style_requested = hovered && io->MouseClicked[1];

    if (edit_requested) {
        if (g.ActiveId != id) {
            memset(io->MouseDown, 0, sizeof(io->MouseDown));
            memset(io->KeysDown, 0, sizeof(io->KeysDown));
            kbind->key = 0;
        }

        SetActiveID(id, window);
        FocusWindow(window);
    }
    else if (!hovered && io->MouseClicked[0] && g.ActiveId == id)
        ClearActiveID();

    bool value_changed = false;
    int key = kbind->key;

    if (g.ActiveId == id) {
        for (auto i = 0; i < 5; i++) {
            if (io->MouseDown[i]) {
                switch (i) {
                case 0:
                    key = VK_LBUTTON;
                    break;
                case 1:
                    key = VK_RBUTTON;
                    break;
                case 2:
                    key = VK_MBUTTON;
                    break;
                case 3:
                    key = VK_XBUTTON1;
                    break;
                case 4:
                    key = VK_XBUTTON2;
                }
                value_changed = true;
                ClearActiveID();
            }
        }

        if (!value_changed) {
            for (auto i = VK_BACK; i <= VK_RMENU; i++) {
                if (io->KeysDown[i]) {
                    key = i;
                    value_changed = true;
                    ClearActiveID();
                }
            }
        }

        if (ImGui::IsKeyPressedMap(ImGuiKey_Escape)) {
            kbind->key = 0;
            ClearActiveID();
        }
        else
            kbind->key = key;
    }
    else if (kbind->need_modes) {
        bool popup_open = IsPopupOpen(id);

        if (style_requested && !popup_open)
            OpenPopupEx(id);

        if (popup_open) {
            SetNextWindowSize(ImVec2(100, ImGui::CalcMaxPopupHeightFromItemCount(5)));

            char name[16];
            ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.OpenPopupStack.size()); // Recycle windows based on depth

            // Peak into expected window size so we can position it
            if (ImGuiWindow* popup_window = FindWindowByName(name))
                if (popup_window->WasActive)
                {
                    ImVec2 size_expected = CalcWindowExpectedSize(popup_window);
                    ImRect r_outer = GetWindowAllowedExtentRect(popup_window);
                    ImVec2 pos = FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, frame_bb, ImGuiPopupPositionPolicy_ComboBox);
                    SetNextWindowPos(pos);
                }

            // Horizontally align ourselves with the framed text
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
            PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));
            bool ret = Begin(name, NULL, window_flags);
            PopStyleVar();

            if (Selectable("Toggle", kbind->mode == KeyBindMode_Toggle))
                kbind->mode = KeyBindMode_Toggle;
            if (Selectable("Hold ON", kbind->mode == KeyBindMode_HoldOn))
                kbind->mode = KeyBindMode_HoldOn;
            if (Selectable("Hold OFF", kbind->mode == KeyBindMode_HoldOff))
                kbind->mode = KeyBindMode_HoldOff;
            if (Selectable("Always", kbind->mode == KeyBindMode_Always))
                kbind->mode = KeyBindMode_Always;

            EndPopup();
        }
    }

    window->DrawList->AddText(frame_bb.Min, g.ActiveId == id ? ImColor(255 / 255.f, 16 / 255.f, 16 / 255.f, g.Style.Alpha) : ImColor(90 / 255.f, 90 / 255.f, 90 / 255.f, g.Style.Alpha), buf_display);
    //PopFont();

    return value_changed;
}

static char* sidebar_tabs[] = 
{
    "ESP",
    "GLOW",
    "CHAMS",
    "AIM",
    "TRIGGER",
    "MISC",
    "CONFIG"
};

constexpr static float get_sidebar_item_width() { return 150.0f; }
constexpr static float get_sidebar_item_height() { return  40.0f; }

enum 
{
	TAB_ESP,
    TAB_GLOW,
    TAB_CHAMS,
	TAB_AIMBOT,
    TAB_TRIGGER,
	TAB_MISC,
	TAB_CONFIG
};

namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };

        if(ImGui::ColorEdit4(label, &clr.x, show_alpha)) 
        {
            v->SetColor(clr.x, clr.y, clr.z, clr.w);
            return true;
        }

        return false;
    }
    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h, bool sameline)
{
    for(auto i = 0; i < N; ++i)
    {
        if(ImGui::Button(names[i], ImVec2{ w, h })) 
        {
            activetab = i;
        }

        if(sameline && i < N - 1)
            ImGui::SameLine();
    }
}

ImVec2 get_sidebar_size()
{
    constexpr float padding = 10.0f;
    constexpr auto size_w = padding * 2.0f + get_sidebar_item_width();
    constexpr auto size_h = padding * 2.0f + (sizeof(sidebar_tabs) / sizeof(char*)) * get_sidebar_item_height();

    return ImVec2{ size_w, ImMax(325.0f, size_h) };
}

void RenderEspTab()
{
    auto& style = ImGui::GetStyle();

    ImGui::BeginGroup();
    {
        ImGui::Columns(2, nullptr, false);

        ImGui::Checkbox("Enabled", g_Configurations.esp_enabled);
        ImGui::Checkbox("Team check", g_Configurations.esp_enemies_only);
        ImGui::Checkbox("Dormant", g_Configurations.dormant_esp);
        ImGui::Checkbox("Boxes", g_Configurations.esp_player_boxes);
        ImGui::SameLine();
        if (ImGui::Button("+##boxes"))
            ImGui::OpenPopup("##flagboxes");

        if (ImGui::BeginPopup("##flagboxes")) {
            ImGui::PushItemWidth(160.f);
            ImGui::Checkbox("Filled", g_Configurations.esp_box_filled);
            ImGui::SameLine();
            if (ImGui::Button("+##Filled"))
                ImGui::OpenPopup("##flagFilled");

            if (ImGui::BeginPopup("##flagFilled")) {
                ImGui::PushItemWidth(160.f);
                ImGui::Checkbox("Gradient", g_Configurations.esp_box_filled_gradient);
                ImGuiEx::ColorEdit3("Gradient color", &g_Configurations.box_gradient_color);
                ImGui::PopItemWidth();
                ImGui::EndPopup();
            }

            ImGuiEx::ColorEdit3("Allies Visible", g_Configurations.color_esp_ally_visible);
            ImGuiEx::ColorEdit3("Enemies Visible", g_Configurations.color_esp_enemy_visible);
            ImGuiEx::ColorEdit3("Allies Occluded", g_Configurations.color_esp_ally_occluded);
            ImGuiEx::ColorEdit3("Enemies Occluded", g_Configurations.color_esp_enemy_occluded);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Names", g_Configurations.esp_player_names);
        ImGui::SameLine();
        if (ImGui::Button("+##Names"))
            ImGui::OpenPopup("##flagNames");

        if (ImGui::BeginPopup("##flagNames")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Names Color", &g_Configurations.esp_names_color);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Health", g_Configurations.esp_player_health);
        ImGui::SameLine();
        if (ImGui::Button("+##Health"))
            ImGui::OpenPopup("##flagHealth");

        if (ImGui::BeginPopup("##flagHealth")) {
            ImGui::PushItemWidth(160.f);
            ImGui::Checkbox("Custom color", g_Configurations.health_based_on_health);
            ImGuiEx::ColorEdit3("Health Color", &g_Configurations.health_color);
            ImGui::Checkbox("Gradient health", g_Configurations.health_gradient);
            ImGuiEx::ColorEdit3("Health second color", &g_Configurations.health_second_color);
            ImGui::Checkbox("Background filled", g_Configurations.health_background);
            ImGuiEx::ColorEdit3("Background color", &g_Configurations.health_background_color);
            ImGui::Checkbox("Gradient background", g_Configurations.health_background_gradient);
            ImGuiEx::ColorEdit3("Background second color", &g_Configurations.health_background_second);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Armour", g_Configurations.esp_player_armour);
        ImGui::SameLine();
        if (ImGui::Button("+##armour"))
            ImGui::OpenPopup("##flagarmour");

        if (ImGui::BeginPopup("##flagarmour")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Armour Color", &g_Configurations.armour_color);
            ImGui::Checkbox("Gradient Armour", g_Configurations.armour_gradient);
            ImGuiEx::ColorEdit3("Armour second color", &g_Configurations.armour_second_color);
            ImGui::Checkbox("Background filled", g_Configurations.armour_background);
            ImGuiEx::ColorEdit3("Background color", &g_Configurations.armour_background_color);
            ImGui::Checkbox("Gradient background", g_Configurations.armour_background_gradient);
            ImGuiEx::ColorEdit3("Background second color", &g_Configurations.armour_background_second);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Weapon", g_Configurations.esp_player_weapons);
        ImGui::SameLine();
        if (ImGui::Button("+##Weapon"))
            ImGui::OpenPopup("##flagWeapon");

        if (ImGui::BeginPopup("##flagWeapon")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Weapon Color", &g_Configurations.esp_weapon_color);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Snaplines", g_Configurations.esp_player_snaplines);
        ImGui::SameLine();
        if (ImGui::Button("+##Snaplines"))
            ImGui::OpenPopup("##flagSnaplines");

        if (ImGui::BeginPopup("##flagSnaplines")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Allies Visible", g_Configurations.color_esp_ally_visible);
            ImGuiEx::ColorEdit3("Enemies Visible", g_Configurations.color_esp_enemy_visible);
            ImGuiEx::ColorEdit3("Allies Occluded", g_Configurations.color_esp_ally_occluded);
            ImGuiEx::ColorEdit3("Enemies Occluded", g_Configurations.color_esp_enemy_occluded);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Sound ESP", &g_Configurations.sound_esp);
        ImGui::SameLine();
        if (ImGui::Button("+##soundesp"))
            ImGui::OpenPopup("##flagsoundesp");

        if (ImGui::BeginPopup("##flagsoundesp")) {
            ImGui::PushItemWidth(160.f);

            ImGui::Combo("Type", &g_Configurations.sound_esp_type, values::soundesp_modes, 2);
            if (ImGui::SliderFloat("Radius", &g_Configurations.sound_esp_radius, 15.f, 150.f))
                g_Configurations.sound_esp_radius = std::clamp(g_Configurations.sound_esp_radius, 15.f, 150.f);
            ImGuiEx::ColorEdit4("Color", &g_Configurations.esp_sounds_color);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Head dot", &g_Configurations.head_dot);
        ImGui::SameLine();
        if (ImGui::Button("+##head_dot"))
            ImGui::OpenPopup("##flaghead_dot");

        if (ImGui::BeginPopup("##flaghead_dot")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit4("Color", &g_Configurations.head_dot_color);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::NextColumn();


        ImGui::Checkbox("Dropped Weapons", g_Configurations.esp_dropped_weapons);
        ImGui::SameLine();
        if (ImGui::Button("+##dr_weapons"))
            ImGui::OpenPopup("##flagdr_weapons");

        if (ImGui::BeginPopup("##flagdr_weapons")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Dropped Weapons", g_Configurations.color_esp_weapons);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Defuse Kit", g_Configurations.esp_defuse_kit);
        ImGui::SameLine();
        if (ImGui::Button("+##def_kit"))
            ImGui::OpenPopup("##flagdef_kit");

        if (ImGui::BeginPopup("##flagdef_kit")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Defuse Kit", g_Configurations.color_esp_defuse);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Planted C4", g_Configurations.esp_planted_c4);
        ImGui::SameLine();
        if (ImGui::Button("+##plc4"))
            ImGui::OpenPopup("##flagplc4");

        if (ImGui::BeginPopup("##flagplc4")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Planted C4", g_Configurations.color_esp_c4);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Item Esp", g_Configurations.esp_items);
        ImGui::SameLine();
        if (ImGui::Button("+##itemesp"))
            ImGui::OpenPopup("##flagitemesp");

        if (ImGui::BeginPopup("##flagitemesp")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Item Esp", g_Configurations.color_esp_item);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Grenade prediction", g_Configurations.esp_grenade_prediction);
        ImGui::SameLine();
        if (ImGui::Button("+##gr_pred"))
            ImGui::OpenPopup("##flaggr_pred");

        if (ImGui::BeginPopup("##flaggr_pred")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Grenade prediction", g_Configurations.color_grenade_prediction);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Damage indicator", g_Configurations.damage_indicator);
        ImGui::Combo("Hit marker", g_Configurations.hit_marker, u8"Disabled\0World\0Crosshair");
        ImGui::Combo("Skybox",g_Configurations.skybox_num,  
            "None\0Tibet\0Baggage\0Italy\0Aztec\0Vertigo\0 Daylight\0Daylight 2\0Clouds\0Clouds 2\0Gray\0Clear\0Canals\0Cobblestone\0Assault\0Clouds dark\0Night\0Night 2\0Night flat\0Dusty\0Rainy\0Custom");
        ImGuiEx::ColorEdit4("Sky color", &g_Configurations.skybox_color);
        ImGui::Checkbox("Enable fog", g_Configurations.enable_fog);
        ImGui::SameLine();
        if (ImGui::Button("+##fog_set"))
            ImGui::OpenPopup("##flagfog_set");

        if (ImGui::BeginPopup("##flagfog_set")) {
            ImGui::PushItemWidth(160.f);
            ImGui::SliderFloat("Start distance", g_Configurations.fog_start_distance, 0.f, 5000.f);
            ImGui::SliderFloat("End distance", g_Configurations.fog_end_distance, 0.f, 5000.f);
            ImGui::SliderFloat("Density", g_Configurations.fog_density, 0.f, 100.f);
            ImGuiEx::ColorEdit4("Fog color", &g_Configurations.fog_color);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
       
        ImGui::SliderFloat("Asus walls", g_Configurations.asus_walls, 0.f, 1.f);
        ImGui::SliderFloat("Asus props", g_Configurations.asus_props, 0.f, 1.f);
        ImGui::Checkbox("Enable NM", &g_Configurations.enable_nightmode);
        ImGuiEx::ColorEdit3("Nightmode ", &g_Configurations.nightmode_color);
     //   ImGui::Combo("Enemy bullet tracers", g_Configurations.enemy_bullet_tracers, "Disabled\0Line\0Beam");
     //   ImGui::Combo("Local bullet tracers", g_Configurations.local_bullet_tracers, "Disabled\0Beam\0Line");

        ImGui::Columns(1, nullptr, false);
    }
    ImGui::EndGroup();
}

void RednerGlowTab()
{
    ImGui::BeginGroup();
    {
        ImGui::Columns(3, nullptr, false);

        ImGui::Checkbox("Enabled", g_Configurations.glow_enabled);
        ImGui::Checkbox("Team check", g_Configurations.glow_enemies_only);
        ImGui::Checkbox("Players", g_Configurations.glow_players);
        ImGui::Checkbox("Chickens", g_Configurations.glow_chickens);
        ImGui::Checkbox("C4 Carrier", g_Configurations.glow_c4_carrier);
        ImGui::Checkbox("Planted C4", g_Configurations.glow_planted_c4);
        ImGui::Checkbox("Defuse Kits", g_Configurations.glow_defuse_kits);
        ImGui::Checkbox("Weapons", g_Configurations.glow_weapons);

        ImGui::NextColumn();

        ImGui::PushItemWidth(100);
        ImGuiEx::ColorEdit3("Ally", g_Configurations.color_glow_ally);
        ImGuiEx::ColorEdit3("Enemy", g_Configurations.color_glow_enemy);
        ImGuiEx::ColorEdit3("Chickens", g_Configurations.color_glow_chickens);
        ImGuiEx::ColorEdit3("C4 Carrier", g_Configurations.color_glow_c4_carrier);
        ImGuiEx::ColorEdit3("Planted C4", g_Configurations.color_glow_planted_c4);
        ImGuiEx::ColorEdit3("Defuse Kits", g_Configurations.color_glow_defuse);
        ImGuiEx::ColorEdit3("Weapons", g_Configurations.color_glow_weapons);
        ImGui::PopItemWidth();

        ImGui::NextColumn();

        ImGui::Columns(1, nullptr, false);
    }
    ImGui::EndGroup();
}

void RenderChamsTab()
{
    ImGui::BeginGroup();
    {
        ImGui::Columns(2, nullptr, false);

        ImGui::BeginGroup();
        {
            ImGui::Text("Players");
            ImGui::Spacing();
            ImGui::Checkbox("Enabled ##players", g_Configurations.chams_player_enabled);
            ImGui::Checkbox("Team Check ##players", g_Configurations.chams_player_enemies_only);
            ImGui::Checkbox("Wireframe visible##players", g_Configurations.chams_player_wireframe_visible);

            ImGui::Checkbox("Ignore-Z ##players", g_Configurations.chams_player_ignorez);
            ImGui::Checkbox("Wireframe invisible##players", g_Configurations.chams_player_wireframe_invisible);
            ImGui::Combo("Material invisible ##players", g_Configurations.chams_players_material_num_invisible, u8" Regular\0 Flat\0 Crystal\0 Glass\0 Glow\0 Circuit\0 Animated1\0 Animated2\0 Animated3\0 Animated4\0 Animated5\0");
            ImGui::Combo("Material visible ##players", g_Configurations.chams_players_material_num_visible, u8" Regular\0 Flat\0 Crystal\0 Glass\0 Glow\0 Circuit\0 Animated1\0 Animated2\0 Animated3\0 Animated4\0 Animated5\0");
            ImGui::PushItemWidth(110);
            ImGuiEx::ColorEdit4("Ally (Visible) ##players", g_Configurations.color_chams_player_ally_visible);
            ImGuiEx::ColorEdit4("Ally (Occluded) ##players", g_Configurations.color_chams_player_ally_occluded);
            ImGuiEx::ColorEdit4("Enemy (Visible) ##players", g_Configurations.color_chams_player_enemy_visible);
            ImGuiEx::ColorEdit4("Enemy (Occluded) ##players", g_Configurations.color_chams_player_enemy_occluded);
            ImGui::PopItemWidth();
        }
        ImGui::EndGroup();
        ImGui::BeginGroup();
        {
            ImGui::Text("Arms");
            ImGui::Spacing();
            ImGui::Checkbox("Enabled ##arms", g_Configurations.chams_arms_enabled);
            ImGui::Checkbox("Wireframe ##arms", g_Configurations.chams_arms_wireframe);
            ImGui::Checkbox("Ignore-Z ##arms", g_Configurations.chams_arms_ignorez);
            ImGui::Combo("Material ##arms", g_Configurations.chams_hands_material_num, u8" Regular\0 Flat\0 Crystal\0 Glass\0 Glow\0 Circuit\0 Animated1\0 Animated2\0 Animated3\0 Animated4\0 Animated5\0");
            ImGui::PushItemWidth(110);
            ImGuiEx::ColorEdit4("Color (Visible) ##arms", g_Configurations.color_chams_arms_visible);
            ImGuiEx::ColorEdit4("Color (Occluded) ##arms", g_Configurations.color_chams_arms_occluded);
            ImGui::PopItemWidth();
        }
        ImGui::EndGroup();

        ImGui::BeginGroup();
        {
            ImGui::Text("Attachment");
            ImGui::Spacing();
            ImGui::Checkbox("Enabled ##Attachment", g_Configurations.chams_attachment_enabled);
            ImGui::Checkbox("Wireframe ##Attachment", g_Configurations.chams_attachment_wireframe);
            ImGui::Checkbox("Ignore-Z ##Attachment", g_Configurations.chams_attachment_ignorez);
            ImGui::Combo("Material ##Attachment", g_Configurations.chams_attachment_material_num, u8" Regular\0 Flat\0 Crystal\0 Glass\0 Glow\0 Circuit\0 Animated1\0 Animated2\0 Animated3\0 Animated4\0 Animated5\0");
            ImGui::PushItemWidth(110);
            ImGuiEx::ColorEdit4("Color (Visible) ##Attachment", g_Configurations.attachment_chams_visible);
            ImGuiEx::ColorEdit4("Color (Occluded) ##Attachment", g_Configurations.attachment_chams_invisible);
            ImGui::PopItemWidth();
        }
        ImGui::EndGroup();

        ImGui::NextColumn();

        ImGui::BeginGroup();
        {
            ImGui::Text("Weapon");
            ImGui::Spacing();
            ImGui::Checkbox("Enabled ##weapon", g_Configurations.chams_weapon_enabled);
            ImGui::Checkbox("Wireframe ##weapon", g_Configurations.chams_weapon_wireframe);
            ImGui::Checkbox("Ignore-Z ##weapon", g_Configurations.chams_weapon_ignorez);
            ImGui::Combo("Material ##weapon", g_Configurations.chams_weapon_material_num, u8" Regular\0 Flat\0 Crystal\0 Glass\0 Glow\0 Circuit\0 Animated1\0 Animated2\0 Animated3\0 Animated4\0 Animated5\0");
            ImGui::PushItemWidth(110);
            ImGuiEx::ColorEdit4("Color (Visible) ##weapon", g_Configurations.color_chams_weapon_visible);
            ImGuiEx::ColorEdit4("Color (Occluded) ##weapon", g_Configurations.color_chams_weapon_occluded);
            ImGui::PopItemWidth();
        }
        ImGui::EndGroup();

        ImGui::BeginGroup();
        {
            ImGui::Text("Sleeve");
            ImGui::Spacing();
            ImGui::Checkbox("Enabled ##sleeve", g_Configurations.chams_sleeve_enabled);
            ImGui::Checkbox("Wireframe ##sleeve", g_Configurations.chams_sleeve_wireframe);
            ImGui::Checkbox("Ignore-Z ##sleeve", g_Configurations.chams_sleeve_ignorez);
            ImGui::Combo("Material ##sleeve", g_Configurations.chams_sleeves_material_num, u8" Regular\0 Flat\0 Crystal\0 Glass\0 Glow\0 Circuit\0 Animated1\0 Animated2\0 Animated3\0 Animated4\0 Animated5\0");
            ImGui::PushItemWidth(110);
            ImGuiEx::ColorEdit4("Color (Visible) ##sleeve", g_Configurations.color_chams_sleeve_visible);
            ImGuiEx::ColorEdit4("Color (Occluded) ##sleeve", g_Configurations.color_chams_sleeve_occluded);
            ImGui::PopItemWidth();
        }
        ImGui::EndGroup();

       

      

        ImGui::Columns(1, nullptr, false);
    }
    ImGui::EndGroup();
}

void RenderMiscTab()
{
   
    ImGui::BeginGroup();
    {
        ImGui::Columns(2, nullptr, false);
        ImGui::BeginChild("##554534455", ImVec2(-1, -1), true); {
        ImGui::Checkbox("Bunny hop", g_Configurations.misc_bhop);
        ImGui::Checkbox("Auto strafe (Rage)", g_Configurations.misc_autostrafe);
        ImGui::Checkbox("WASD", g_Configurations.misc_wasdstrafes);
        ImGui::Checkbox("Speed boost", g_Configurations.misc_boostspeed);
        ImGui::Checkbox("Fast stop", g_Configurations.fast_stop);
        ImGui::Checkbox("Edge jump", g_Configurations.edge_jump);
        ImGui::Checkbox("Remove flash", g_Configurations.remove_flash);
        ImGui::Checkbox("Remove smoke", g_Configurations.remove_smoke);
        ImGui::Checkbox("Remove scope", g_Configurations.remove_zoom);
        if (g_Configurations.remove_zoom)
            ImGui::Checkbox("Draw lines", g_Configurations.draw_no_scope_lines);
          
        ImGui::Checkbox("Remove visual recoil", g_Configurations.remove_visualrecoil);
        ImGui::Checkbox("Remove post processing", g_Configurations.remove_post_processing);
        ImGui::Checkbox("Remove panorama blur", g_Configurations.remove_panorama_blur);
        ImGui::Checkbox("Aspect ratio", g_Configurations.aspect_ratio);
        if (g_Configurations.aspect_ratio)
            ImGui::SliderFloat("Scale", g_Configurations.aspect_ratio_scale, 0.1f, 4.f);
	
        Visuals::Get().MenuVisuals();
        ImGui::Checkbox("Enable offsets", g_Configurations.enable_offsets);
        ImGui::SameLine();
        if (ImGui::Button("+##offsets"))
            ImGui::OpenPopup("##flagoffsets");

        if (ImGui::BeginPopup("##flagoffsets")) {
            ImGui::PushItemWidth(160.f);
            ImGui::SliderFloat("X", g_Configurations.viewmodel_offset_x, -30.f, 30.f);
            ImGui::SliderFloat("Y", g_Configurations.viewmodel_offset_y, -30.f, 30.f);
            ImGui::SliderFloat("Z", g_Configurations.viewmodel_offset_z, -30.f, 30.f);
            ImGui::SliderFloat("Roll", g_Configurations.viewmodel_offset_roll, -30.f, 30.f);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
      
        ImGui::Checkbox("No scope crosshair", g_Configurations.no_scope_crosshair);
        ImGui::SliderFloat("Camera Fov", g_Configurations.camera_fov, 68.f, 120.f);
        ImGui::Checkbox("Force fov in scope", g_Configurations.force_fov_in_zoom);
        ImGui::SliderInt("Viewmodel fov", g_Configurations.viewmodel_fov, 68, 120);
		ImGui::Checkbox("Rank reveal", g_Configurations.misc_showranks);
		ImGui::Checkbox("Watermark##hc", g_Configurations.misc_watermark);
        };
        ImGui::EndChild();
		ImGui::NextColumn();

        
        ImGui::Checkbox("Post processing", &g_Configurations.enable_post_proc);
        ImGui::SameLine();
        if (ImGui::Button("+##post_proc"))
            ImGui::OpenPopup("##flagpost_proc");

        if (ImGui::BeginPopup("##flagpost_proc")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Post proc color", &g_Configurations.post_processing);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Preserve Killfeed", g_Configurations.preserve_killfeed);
        ImGui::Checkbox("Quick reload", g_Configurations.quick_reload);
        ImGui::Checkbox("Hit sound", g_Configurations.hitsound);
        ImGui::Checkbox("Draw binds", g_Configurations.draw_binds);
        ImGui::Checkbox("Draw spectators", g_Configurations.spectator_list);
        ImGui::Checkbox("Draw aim fov", &g_Configurations.draw_aim_fov);
        ImGui::SameLine();
        if (ImGui::Button("+##aimfov"))
            ImGui::OpenPopup("##flagaimfov");

        if (ImGui::BeginPopup("##flagaimfov")) {
            ImGui::PushItemWidth(160.f);
            ImGuiEx::ColorEdit3("Default aim", &g_Configurations.default_aim_fov);
            ImGuiEx::ColorEdit3("Silent aim", &g_Configurations.silent_aim_fov);
            ImGui::PopItemWidth();
            ImGui::EndPopup();
        }

     
        ImGui::Columns(1, nullptr, false);
    }
    ImGui::EndGroup();
   
}

void RenderEmptyTab()
{
	ImGui::BeginGroup();
	{
		auto message = "There's nothing here. Add something you want!";

		auto pos = ImGui::GetCurrentWindow()->Pos;
		auto wsize = ImGui::GetCurrentWindow()->Size;

		pos = pos + wsize / 2.0f;

		ImGui::RenderText(pos - ImGui::CalcTextSize(message) / 2.0f, message);
	}
	ImGui::EndGroup();
}

void RenderConfigTab()
{
    ImGui::BeginGroup();
    {
	
    }
    ImGui::EndGroup();
}

void Menu::Initialize()
{
	CreateStyle();
    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

void Menu::Render()
{
    g_KeyBinds->Draw();
    g_SpecList->Draw();
    ImGuiStyle* style = &ImGui::GetStyle();

    style->WindowPadding = ImVec2(6, 6);
    style->ItemSpacing = ImVec2(4, 2);

    ImGui::GetStyle().WindowRounding = 0.0f;

    ImGui::GetStyle().ScrollbarRounding = 0.0f;

    ImGui::GetStyle().ScrollbarSize = 5.0f;
    ImGui::GetIO().MouseDrawCursor = _visible && g_EngineClient->IsInGame();

    if (!_visible)
        return;

    const auto sidebar_size = get_sidebar_size();
    static int active_sidebar_tab = 0;
    ImGui::SetNextWindowSize(ImVec2(800, 550));
    if (ImGui::Begin("CSGO Simple", &_visible, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        {
            ImGui::BeginGroup();
            {
                render_tabs(sidebar_tabs, active_sidebar_tab, get_sidebar_item_width(), get_sidebar_item_height(), false);
            }
            ImGui::EndGroup();
        }
        ImGui::PopStyleVar();
        ImGui::SameLine();

        auto size = ImVec2{ 0.0f, sidebar_size.y };

        ImGui::BeginGroup();
        if (active_sidebar_tab == TAB_ESP)
        {
            RenderEspTab();
        }
        else if (active_sidebar_tab == TAB_GLOW)
        {
            RednerGlowTab();
        }
        else if (active_sidebar_tab == TAB_CHAMS)
        {
            RenderChamsTab();
        }
        else if (active_sidebar_tab == TAB_AIMBOT)
        {
            g_Aimbot.MenuAimbot();
        }
        else if (active_sidebar_tab == TAB_TRIGGER)
        {
           
        }
        else if (active_sidebar_tab == TAB_MISC)
        {
            RenderMiscTab();
        }
        else if (active_sidebar_tab == TAB_CONFIG)
        {
            RenderConfigTab();
        }
        ImGui::EndGroup();

        ImGui::End();
    }
}

void Menu::Toggle()
{
    _visible = !_visible;
}

void Menu::CreateStyle()
{
	ImGui::StyleColorsDark();
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
    ImGui::GetIO().LogFilename = NULL;
    ImGui::GetIO().IniFilename = NULL;
	_style.FrameRounding = 0.f;
	_style.WindowRounding = 0.f;
	_style.ChildRounding = 0.f;
	_style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_Header] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.000f);
	_style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.0f);
	_style.Colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.009f, 0.120f, 0.940f);
	_style.Colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.143f, 0.209f, 1.000f);
	ImGui::GetStyle() = _style;
}

