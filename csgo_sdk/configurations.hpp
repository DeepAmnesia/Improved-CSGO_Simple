#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "sdk/misc/Color.hpp"
#include <map>
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
struct weapon_name_t {
	constexpr weapon_name_t(int32_t definition_index, const char* name) :
		definition_index(definition_index),
		name(name) {
	}

	int32_t definition_index = 0;
	const char* name = nullptr;
};
struct weapon_info_t {
	constexpr weapon_info_t(const char* model, const char* icon = nullptr) :
		model(model),
		icon(icon)
	{}

	const char* model;
	const char* icon;
};
struct legitbot_settings {
	bool autopistol = false;
	bool smoke_check = false;
	bool flash_check = false;
	bool jump_check = false;
	bool autowall = false;
	float autowall_int = 0.f;
	bool silent = false;
	bool psilent = false;
	bool rcs = false;
	bool rcs_fov_enabled = false;
	bool rcs_smooth_enabled = false;
	bool autostop = false;
	bool only_in_zoom = true;
	int aim_type = 1;
	int priority = 0;
	int fov_type = 0;
	int smooth_type = 0;
	float curve = 0.f;
	bool humanize = false;
	int rcs_type = 0;
	int hitbox = 1;
	float fov = 0.f;
	bool auto_delay = false;
	bool trigger_enable = false;
	bool trigger_check_smoke = false;
	bool trigger_check_flash = false;
	float trigger_hit_chance = 0.f;
	float trigger_delay = 0.f;
	bool trigger_hitgroup_head = false;
	bool trigger_hitgroup_chest = false;
	bool trigger_hitgroup_stomach = false;
	bool trigger_hitgroup_left_arm = false;
	bool trigger_hitgroup_right_arm = false;
	bool trigger_hitgroup_left_leg = false;
	bool trigger_hitgroup_right_leg = false;

	bool enable_backtrack = false;
	int  backtrack_ticks = 0;

	bool hitchance = false;
	float hitchance_amount = 0.f;

	float silent_fov = 0.f;
	float rcs_fov = 0.f;
	float smooth = 1;
	float rcs_smooth = 1;
	int shot_delay = 0;
	int kill_delay = 0;
	int rcs_x = 100;
	int rcs_y = 100;
	int rcs_start = 1;
	int min_damage = 1;
};

namespace values {
	extern const char* bind_types[];
	extern const char* aim_types[];
	extern const char* smooth_types[];
	extern const char* hitbox_list[];
	extern const char* rcs_types[];
	extern const char* priorities[];
	extern const char* fov_types[];
	extern const char* show_spread_types[];
	extern const char* knifebot_filters[];
	extern const char* knifebot_modes[];
	extern const char* glow_modes[];
	extern const char* legitaa_modes[];
	extern const char* resolver_modes[];
	extern const char* soundesp_modes[];

	extern const std::map<size_t, weapon_info_t> WeaponInfo;
	extern const std::vector<weapon_name_t> GloveNames;
	extern std::vector<weapon_name_t> WeaponNames;
	extern std::vector<weapon_name_t> WeaponNamesFull;
	extern std::map<short, std::string> WNames;

	extern const char* GetWeaponNameById(int id);
};
class Configurations
{
public:
	struct {
		bool InThirdPerson = false;
		float OFOV = 0;
		int SHeight = 0;
		int SWidth = 0;
		float SWidthHalf = 0;
		float SHeightHalf = 0;
		bool change_materials = false;
	

	} g_Globals;
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
		CONFIGURATION(bool, esp_box_filled, false);
		CONFIGURATION(int, esp_box_type, 0);
		CONFIGURATION(bool, esp_box_filled_gradient, false);
		CONFIGURATION(bool, health_based_on_health, false);
		CONFIGURATION(bool, health_gradient, false);
		CONFIGURATION(bool, health_background, false);
		CONFIGURATION(bool, health_background_gradient , false);
		CONFIGURATION(bool, dormant_esp, false);

		CONFIGURATION(bool, armour_gradient, false);
		CONFIGURATION(bool, armour_background, false);
		CONFIGURATION(bool, armour_background_gradient, false);
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
		CONFIGURATION(int, chams_players_material_num_invisible, 0);
		CONFIGURATION(int, chams_players_material_num_visible, 0);
		CONFIGURATION(int, chams_hands_material_num, 0);
		CONFIGURATION(int, chams_sleeves_material_num, 0);
		CONFIGURATION(int, chams_weapon_material_num, 0);
		CONFIGURATION(int, chams_attachment_material_num, 0);
		CONFIGURATION(bool, chams_player_enabled, false);
		CONFIGURATION(bool, chams_player_enemies_only, false);
		CONFIGURATION(bool, chams_player_wireframe_invisible, false);
		CONFIGURATION(bool, chams_player_wireframe_visible, false);
		CONFIGURATION(bool, chams_player_ignorez, false);

		CONFIGURATION(bool, chams_sleeve_enabled, false);
		CONFIGURATION(bool, chams_sleeve_wireframe, false);
		CONFIGURATION(bool, chams_sleeve_ignorez, false);

		CONFIGURATION(bool, chams_arms_enabled, false);
		CONFIGURATION(bool, chams_arms_wireframe, false);
		CONFIGURATION(bool, chams_arms_ignorez, false);

		CONFIGURATION(bool, chams_weapon_enabled, false);
		CONFIGURATION(bool, chams_weapon_wireframe, false);
		CONFIGURATION(bool, chams_weapon_ignorez, false);

		CONFIGURATION(bool, chams_attachment_enabled, false);
		CONFIGURATION(bool, chams_attachment_wireframe, false);
		CONFIGURATION(bool, chams_attachment_ignorez, false);

		//
		// MISC
		//
		CONFIGURATION(bool, edge_jump, false);
		CONFIGURATION(bool, misc_bhop, false);
		CONFIGURATION(bool, misc_autostrafe, false);
		CONFIGURATION(bool, misc_no_hands, false);
		bool misc_thirdperson= false;
		bool enable_post_proc = false;
		bool enable_nightmode = false;
		CONFIGURATION(bool, misc_showranks, true);
		CONFIGURATION(bool, misc_wasdstrafes, true);
		CONFIGURATION(bool, misc_boostspeed, true);
		CONFIGURATION(bool, misc_watermark, false);
		CONFIGURATION(bool, fast_stop, false);
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
		CONFIGURATION(bool, draw_binds, false);
		CONFIGURATION(bool, no_scope_crosshair, false);


		CONFIGURATION(bool, enable_offsets, false);
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
		CONFIGURATION(int, enemy_bullet_tracers, 0);
		CONFIGURATION(int, local_bullet_tracers, 0);

		CONFIGURATION(bool, use_enemy_bullet_tracers, false);
		CONFIGURATION(bool, use_local_bullet_tracers, false);

		CONFIGURATION(float, asus_walls, 0.0f);
		CONFIGURATION(float, asus_props, 0.0f);

		CONFIGURATION(bool, enable_fog, false);
		CONFIGURATION(float, fog_start_distance, 0.0f);
		CONFIGURATION(float, fog_end_distance, 0.0f);
		CONFIGURATION(float, fog_density, 0.0f);

		CONFIGURATION(bool, spectator_list, false);

		CONFIGURATION(int, skybox_num, false);
		float sound_esp_radius= 15.0f;
		int sound_esp_type= 0;
		bool sound_esp= false;
		bool head_dot = false;

		bool draw_aim_fov = false;
		// 
		// COLORS
		// 
		CONFIGURATION(Color, attachment_chams_visible, Color(255, 255, 255));
		CONFIGURATION(Color, attachment_chams_invisible, Color(255, 255, 255));

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

		CONFIGURATION(Color, color_watermark, Color(255, 255, 255));
		Color nightmode_color = Color(255, 255, 255);
		Color skybox_color = Color(255, 255, 255);
		Color fog_color = Color(255, 255, 255);
		Color esp_sounds_color = Color(255, 255, 255);
		Color head_dot_color = Color(255, 255, 255);
		Color esp_names_color = Color(255, 255, 255);
		Color esp_weapon_color = Color(255, 255, 255);
		Color box_gradient_color = Color(255, 255, 255);

		Color health_color = Color(255, 255, 255);
		Color health_second_color = Color(255, 255, 255);
		Color health_background_color = Color(255, 255, 255);
		Color health_background_second = Color(255, 255, 255);

		Color armour_color = Color(255, 255, 255);
		Color armour_second_color = Color(255, 255, 255);
		Color armour_background_color = Color(255, 255, 255);
		Color armour_background_second = Color(255, 255, 255);

		Color default_aim_fov = Color(255, 255, 255);
		Color silent_aim_fov = Color(255, 255, 255);
		Color post_processing = Color(255, 255, 255);

		Color enemy_bullet_tracer = Color(255, 255, 255);
		Color local_bullet_tracer = Color(255, 255, 255);
};

inline Configurations g_Configurations;
inline bool g_Unload;
