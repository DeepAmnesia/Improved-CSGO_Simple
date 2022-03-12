#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "sdk/misc/Color.hpp"

#define A( s ) #s
#define CONFIGURATION(type, var, val) Var<type> var = {A(var), val}

template <typename T = bool>
class Var 
{
public:
	std::string name;
	std::shared_ptr<T> value;
	int32_t size;
	Var(std::string name, T v) : name(name) 
	{
		value = std::make_shared<T>(v);
		size = sizeof(T);
	}
	operator T() { return *value; }
	operator T*() { return &*value; }
	operator T() const { return *value; }
	//operator T*() const { return value; }
};

class Configurations
{
public:
		// 
		// ESP
		// 
		CONFIGURATION(bool, esp_enabled, false);
		CONFIGURATION(bool, esp_enemies_only, false);
		CONFIGURATION(bool, esp_player_boxes, false);
		CONFIGURATION(bool, esp_player_names, false);
		CONFIGURATION(bool, esp_player_health, false);
		CONFIGURATION(bool, esp_player_armour, false);
		CONFIGURATION(bool, esp_player_weapons, false);
		CONFIGURATION(bool, esp_player_snaplines, false);
		CONFIGURATION(bool, esp_dropped_weapons, false);
		CONFIGURATION(bool, esp_defuse_kit, false);
		CONFIGURATION(bool, esp_planted_c4, false);
		CONFIGURATION(bool, esp_items, false);
		CONFIGURATION(bool, esp_grenade_prediction, false);

		// 
		// GLOW
		// 
		CONFIGURATION(bool, glow_enabled, false);
		CONFIGURATION(bool, glow_enemies_only, false);
		CONFIGURATION(bool, glow_players, false);
		CONFIGURATION(bool, glow_chickens, false);
		CONFIGURATION(bool, glow_c4_carrier, false);
		CONFIGURATION(bool, glow_planted_c4, false);
		CONFIGURATION(bool, glow_defuse_kits, false);
		CONFIGURATION(bool, glow_weapons, false);

		//
		// CHAMS
		//
		CONFIGURATION(bool, chams_player_enabled, false);
		CONFIGURATION(bool, chams_player_enemies_only, false);
		CONFIGURATION(bool, chams_player_wireframe, false);
		CONFIGURATION(bool, chams_player_flat, false);
		CONFIGURATION(bool, chams_player_ignorez, false);
		CONFIGURATION(bool, chams_player_glass, false);

		CONFIGURATION(bool, chams_sleeve_enabled, false);
		CONFIGURATION(bool, chams_sleeve_wireframe, false);
		CONFIGURATION(bool, chams_sleeve_flat, false);
		CONFIGURATION(bool, chams_sleeve_ignorez, false);
		CONFIGURATION(bool, chams_sleeve_glass, false);

		CONFIGURATION(bool, chams_arms_enabled, false);
		CONFIGURATION(bool, chams_arms_wireframe, false);
		CONFIGURATION(bool, chams_arms_flat, false);
		CONFIGURATION(bool, chams_arms_ignorez, false);
		CONFIGURATION(bool, chams_arms_glass, false);

		CONFIGURATION(bool, chams_weapon_enabled, false);
		CONFIGURATION(bool, chams_weapon_wireframe, false);
		CONFIGURATION(bool, chams_weapon_flat, false);
		CONFIGURATION(bool, chams_weapon_ignorez, false);
		CONFIGURATION(bool, chams_weapon_glass, false);

		//
		// MISC
		//
		CONFIGURATION(bool, misc_bhop, false);
		CONFIGURATION(bool, misc_autostrafe, false);
		CONFIGURATION(bool, misc_autostrafe_legit, false);
		CONFIGURATION(float, retrack, 0.f);
		CONFIGURATION(bool, misc_no_hands, false);
		CONFIGURATION(bool, misc_thirdperson, false);
		CONFIGURATION(bool, misc_showranks, true);
		CONFIGURATION(bool, misc_watermark, true);
		CONFIGURATION(float, misc_thirdperson_dist, 50.f);
		CONFIGURATION(int, viewmodel_fov, 68);
		CONFIGURATION(float, mat_ambient_light_r, 0.0f);
		CONFIGURATION(float, mat_ambient_light_g, 0.0f);
		CONFIGURATION(float, mat_ambient_light_b, 0.0f);
		CONFIGURATION(bool, remove_smoke, false);
		CONFIGURATION(bool, remove_flash, false);
		CONFIGURATION(bool, remove_zoom, false);
		CONFIGURATION(bool, remove_panorama_blur, false);
		CONFIGURATION(bool, remove_blur, false);
		CONFIGURATION(bool, remove_visualrecoil, false);
		CONFIGURATION(bool, remove_post_processing, false);
		CONFIGURATION(bool, draw_no_scope_lines, false);
		CONFIGURATION(bool, aspect_ratio, false);
		CONFIGURATION(float, aspect_ratio_scale, 1.0f);

		CONFIGURATION(bool, no_scope_crosshair, false);

		CONFIGURATION(float, viewmodel_offset_x, 0.0f);
		CONFIGURATION(float, viewmodel_offset_y, 0.0f);
		CONFIGURATION(float, viewmodel_offset_z, 0.0f);
		CONFIGURATION(float, viewmodel_offset_roll, 0.0f);

		CONFIGURATION(float, camera_fov, 90.0f);
		CONFIGURATION(bool, force_fov_in_zoom, false);
		CONFIGURATION(bool,  preserve_killfeed, false);
		CONFIGURATION(bool, quick_reload, false);
		CONFIGURATION(bool, hitsound, false);
		CONFIGURATION(bool, damage_indicator, false);
		CONFIGURATION(int, hit_marker, false);
		CONFIGURATION(int, enemy_bullet_tracers, false);
		CONFIGURATION(int, local_bullet_tracers, false);
		// 
		// COLORS
		// 
		CONFIGURATION(Color, color_esp_ally_visible, Color(255, 255, 255));
		CONFIGURATION(Color, color_esp_enemy_visible, Color(255, 255, 255));
		CONFIGURATION(Color, color_esp_ally_occluded, Color(255, 255, 255));
		CONFIGURATION(Color, color_esp_enemy_occluded, Color(255, 255, 255));
		CONFIGURATION(Color, color_esp_weapons, Color(255, 255, 255));
		CONFIGURATION(Color, color_esp_defuse, Color(255, 255, 255));
		CONFIGURATION(Color, color_esp_c4, Color(255, 255, 255));
		CONFIGURATION(Color, color_esp_item, Color(255, 255, 255));
		CONFIGURATION(Color, color_grenade_prediction, Color(255, 255, 255));

		CONFIGURATION(Color, color_glow_ally, Color(255, 255, 255));
		CONFIGURATION(Color, color_glow_enemy, Color(255, 255, 255));
		CONFIGURATION(Color, color_glow_chickens, Color(255, 255, 255));
		CONFIGURATION(Color, color_glow_c4_carrier, Color(255, 255, 255));
		CONFIGURATION(Color, color_glow_planted_c4, Color(255, 255, 255));
		CONFIGURATION(Color, color_glow_defuse, Color(255, 255, 255));
		CONFIGURATION(Color, color_glow_weapons, Color(255, 255, 255));

		CONFIGURATION(Color, color_chams_player_ally_visible, Color(255, 255, 255));
		CONFIGURATION(Color, color_chams_player_ally_occluded, Color(255, 255, 255));
		CONFIGURATION(Color, color_chams_player_enemy_visible, Color(255, 255, 255));
		CONFIGURATION(Color, color_chams_player_enemy_occluded, Color(255, 255, 255));

		CONFIGURATION(Color, color_chams_arms_visible, Color(255, 255, 255));
		CONFIGURATION(Color, color_chams_arms_occluded, Color(255, 255, 255));

		CONFIGURATION(Color, color_chams_sleeve_visible, Color(255, 255, 255));
		CONFIGURATION(Color, color_chams_sleeve_occluded, Color(255, 255, 255));

		CONFIGURATION(Color, color_chams_weapon_visible, Color(255, 255, 255));
		CONFIGURATION(Color, color_chams_weapon_occluded, Color(255, 255, 255));

		CONFIGURATION(Color, color_watermark, Color(255, 255, 255)); // no menu config cuz its useless
};

inline Configurations g_Configurations;
inline bool g_Unload;
