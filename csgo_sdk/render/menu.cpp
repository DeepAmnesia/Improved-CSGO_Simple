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


static char* sidebar_tabs[] = 
{
    "ESP",
    "GLOW",
    "CHAMS",
    "AIM",
    "MISC",
    "CONFIG"
};

constexpr static float get_sidebar_item_width() { return 150.0f; }
constexpr static float get_sidebar_item_height() { return  50.0f; }

enum 
{
	TAB_ESP,
    TAB_GLOW,
    TAB_CHAMS,
	TAB_AIMBOT,
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
        ImGui::Columns(3, nullptr, false);

        ImGui::Checkbox("Enabled", g_Configurations.esp_enabled);
        ImGui::Checkbox("Team check", g_Configurations.esp_enemies_only);
        ImGui::Checkbox("Boxes", g_Configurations.esp_player_boxes);
        ImGui::Checkbox("Names", g_Configurations.esp_player_names);
        ImGui::Checkbox("Health", g_Configurations.esp_player_health);
        ImGui::Checkbox("Armour", g_Configurations.esp_player_armour);
        ImGui::Checkbox("Weapon", g_Configurations.esp_player_weapons);
        ImGui::Checkbox("Snaplines", g_Configurations.esp_player_snaplines);

        ImGui::NextColumn();

        ImGui::Checkbox("Crosshair", g_Configurations.esp_crosshair);
        ImGui::Checkbox("Dropped Weapons", g_Configurations.esp_dropped_weapons);
        ImGui::Checkbox("Defuse Kit", g_Configurations.esp_defuse_kit);
        ImGui::Checkbox("Planted C4", g_Configurations.esp_planted_c4);
        ImGui::Checkbox("Item Esp", g_Configurations.esp_items);
        ImGui::Checkbox("Grenade prediction", g_Configurations.esp_grenade_prediction);

        ImGui::NextColumn();

        ImGui::PushItemWidth(100);
        ImGuiEx::ColorEdit3("Allies Visible", g_Configurations.color_esp_ally_visible);
        ImGuiEx::ColorEdit3("Enemies Visible", g_Configurations.color_esp_enemy_visible);
        ImGuiEx::ColorEdit3("Allies Occluded", g_Configurations.color_esp_ally_occluded);
        ImGuiEx::ColorEdit3("Enemies Occluded", g_Configurations.color_esp_enemy_occluded);
        ImGuiEx::ColorEdit3("Crosshair", g_Configurations.color_esp_crosshair);
        ImGuiEx::ColorEdit3("Dropped Weapons", g_Configurations.color_esp_weapons);
        ImGuiEx::ColorEdit3("Defuse Kit", g_Configurations.color_esp_defuse);
        ImGuiEx::ColorEdit3("Planted C4", g_Configurations.color_esp_c4);
        ImGuiEx::ColorEdit3("Item Esp", g_Configurations.color_esp_item);
        ImGuiEx::ColorEdit3("Grenade prediction", g_Configurations.color_grenade_prediction);
        ImGui::PopItemWidth();

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
        ImGui::Columns(4, nullptr, false);

        ImGui::BeginGroup();
        {
            ImGui::Text("Players");
            ImGui::Spacing();
            ImGui::Checkbox("Enabled ##players", g_Configurations.chams_player_enabled);
            ImGui::Checkbox("Team Check ##players", g_Configurations.chams_player_enemies_only);
            ImGui::Checkbox("Wireframe ##players", g_Configurations.chams_player_wireframe);
            ImGui::Checkbox("Flat ##players", g_Configurations.chams_player_flat);
            ImGui::Checkbox("Behind wall ##players", g_Configurations.chams_player_ignorez);
            ImGui::Checkbox("Glass ##players", g_Configurations.chams_player_glass);
            ImGui::PushItemWidth(110);
            ImGuiEx::ColorEdit4("Ally (Visible) ##players", g_Configurations.color_chams_player_ally_visible);
            ImGuiEx::ColorEdit4("Ally (Occluded) ##players", g_Configurations.color_chams_player_ally_occluded);
            ImGuiEx::ColorEdit4("Enemy (Visible) ##players", g_Configurations.color_chams_player_enemy_visible);
            ImGuiEx::ColorEdit4("Enemy (Occluded) ##players", g_Configurations.color_chams_player_enemy_occluded);
            ImGui::PopItemWidth();
        }
        ImGui::EndGroup();

        ImGui::NextColumn();

        ImGui::BeginGroup();
        {
            ImGui::Text("Arms");
            ImGui::Spacing();
            ImGui::Checkbox("Enabled ##arms", g_Configurations.chams_arms_enabled);
            ImGui::Checkbox("Wireframe ##arms", g_Configurations.chams_arms_wireframe);
            ImGui::Checkbox("Flat ##arms", g_Configurations.chams_arms_flat);
            ImGui::Checkbox("Ignore-Z ##arms", g_Configurations.chams_arms_ignorez);
            ImGui::Checkbox("Glass ##arms", g_Configurations.chams_arms_glass);
            ImGui::PushItemWidth(110);
            ImGuiEx::ColorEdit4("Color (Visible) ##arms", g_Configurations.color_chams_arms_visible);
            ImGuiEx::ColorEdit4("Color (Occluded) ##arms", g_Configurations.color_chams_arms_occluded);
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
            ImGui::Checkbox("Flat ##weapon", g_Configurations.chams_weapon_flat);
            ImGui::Checkbox("Ignore-Z ##weapon", g_Configurations.chams_weapon_ignorez);
            ImGui::Checkbox("Glass ##weapon", g_Configurations.chams_weapon_glass);
            ImGui::PushItemWidth(110);
            ImGuiEx::ColorEdit4("Color (Visible) ##weapon", g_Configurations.color_chams_weapon_visible);
            ImGuiEx::ColorEdit4("Color (Occluded) ##weapon", g_Configurations.color_chams_weapon_occluded);
            ImGui::PopItemWidth();
        }
        ImGui::EndGroup();

        ImGui::NextColumn();

        ImGui::BeginGroup();
        {
            ImGui::Text("Sleeve");
            ImGui::Spacing();
            ImGui::Checkbox("Enabled ##sleeve", g_Configurations.chams_sleeve_enabled);
            ImGui::Checkbox("Wireframe ##sleeve", g_Configurations.chams_sleeve_wireframe);
            ImGui::Checkbox("Flat ##sleeve", g_Configurations.chams_sleeve_flat);
            ImGui::Checkbox("Ignore-Z ##sleeve", g_Configurations.chams_sleeve_ignorez);
            ImGui::Checkbox("Glass ##sleeve", g_Configurations.chams_sleeve_glass);
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

        ImGui::Checkbox("Bunny hop", g_Configurations.misc_bhop);
        ImGui::Checkbox("Auto strafe", g_Configurations.misc_autostrafe);
        ImGui::Checkbox("Remove flash", g_Configurations.remove_flash);
        ImGui::Checkbox("Remove smoke", g_Configurations.remove_smoke);
        ImGui::Checkbox("Remove scope", g_Configurations.remove_zoom);
        if (g_Configurations.remove_zoom)
            ImGui::Checkbox("Draw lines", g_Configurations.draw_no_scope_lines);
          
        ImGui::Checkbox("Remove visual recoil", g_Configurations.remove_visualrecoil);
        ImGui::Checkbox("Remove post processing", g_Configurations.remove_post_processing);
        ImGui::Checkbox("Remove Panorama blur", g_Configurations.remove_panorama_blur);
        ImGui::Checkbox("Aspect ratio", g_Configurations.aspect_ratio);
        if (g_Configurations.aspect_ratio)
            ImGui::SliderFloat("Scale", g_Configurations.aspect_ratio_scale, 0.1f, 4.f);
		ImGui::Checkbox("Third Person", g_Configurations.misc_thirdperson);

		if(g_Configurations.misc_thirdperson)
			ImGui::SliderFloat("Distance", g_Configurations.misc_thirdperson_dist, 0.f, 150.f);
        ImGui::SliderFloat("Viewmodel offset X", g_Configurations.viewmodel_offset_x, -30.f, 30.f);
        ImGui::SliderFloat("Viewmodel offset Y", g_Configurations.viewmodel_offset_y, -30.f, 30.f);
        ImGui::SliderFloat("Viewmodel offset Z", g_Configurations.viewmodel_offset_z, -30.f, 30.f);
        ImGui::SliderFloat("Viewmodel offset Roll", g_Configurations.viewmodel_offset_roll, -30.f, 30.f);
        ImGui::Checkbox("No scope crosshair", g_Configurations.no_scope_crosshair);
        ImGui::SliderFloat("Camera Fov", g_Configurations.camera_fov, 68.f, 120.f);
        ImGui::Checkbox("Force fov in scoper", g_Configurations.force_fov_in_zoom);
		ImGui::Checkbox("Rank reveal", g_Configurations.misc_showranks);
		ImGui::Checkbox("Watermark##hc", g_Configurations.misc_watermark);

		ImGui::NextColumn();

        ImGui::SliderInt("viewmodel_fov:", g_Configurations.viewmodel_fov, 68, 120);
		ImGui::Text("Postprocessing:");
        ImGui::SliderFloat("Red", g_Configurations.mat_ambient_light_r, 0, 1);
        ImGui::SliderFloat("Green", g_Configurations.mat_ambient_light_g, 0, 1);
        ImGui::SliderFloat("Blue", g_Configurations.mat_ambient_light_b, 0, 1);

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
		if (ImGui::Button("Save cfg")) 
			Config::Get().Save();

		if (ImGui::Button("Load cfg")) 
			Config::Get().Load();
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
    ImGui::GetIO().MouseDrawCursor = _visible;

    if (!_visible)
        return;

    const auto sidebar_size = get_sidebar_size();
    static int active_sidebar_tab = 0;

    if (ImGui::Begin("CSGOSimple", &_visible, ImGuiWindowFlags_NoCollapse))
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
            RenderEmptyTab();
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

