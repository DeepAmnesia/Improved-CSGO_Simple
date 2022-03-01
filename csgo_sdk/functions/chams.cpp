#include "chams.hpp"

#include <fstream>

#include "../sdk/csgostructs.hpp"
#include "../configurations.hpp"

#include "../hooks.hpp"

#include "../sdk/utils/input.hpp"


Chams::Chams() 
{
	materialRegular = g_MatSystem->FindMaterial("debug/debugambientcube");
	materialFlat = g_MatSystem->FindMaterial("debug/debugdrawflat");
}

Chams::~Chams() 
{

}

void Chams::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, const Color& rgba) 
{
	IMaterial* material = nullptr;

	if (flat) 
		material = materialFlat;
	else 
		material = materialRegular;

	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);

	if (glass) 
	{
		material = materialFlat;
		material->AlphaModulate(0.45f);
	}
	else 
		material->AlphaModulate(rgba.a() / 255.0f);

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);

	g_MdlRender->ForcedMaterialOverride(material);
}

void Chams::OnDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix)
{
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	const auto mdl = info.pModel;

	bool is_arm = strstr(mdl->szName, "arms") != nullptr;
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
	bool is_weapon = strstr(mdl->szName, "weapons/v_")  != nullptr;

	static auto flash = g_MatSystem->FindMaterial("effects/flashbang", TEXTURE_GROUP_CLIENT_EFFECTS);
	static auto flash_white = g_MatSystem->FindMaterial("effects/flashbang_white", TEXTURE_GROUP_CLIENT_EFFECTS);

	static std::vector <const char*> smoke_materials =
	{
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		"particle/vistasmokev1/vistasmokev1_smokegrenade"
	/*	"effects/overlaysmoke",
		"particle/beam_smoke_01",
		"particle/particle_smokegrenade",
		"particle/particle_smokegrenade1",
		"particle/particle_smokegrenade2",
		"particle/particle_smokegrenade3",
		"particle/particle_smokegrenade_sc",
		"particle/smoke1/smoke1",
		"particle/smoke1/smoke1_ash",
		"particle/smoke1/smoke1_nearcull",
		"particle/smoke1/smoke1_nearcull2",
		"particle/smoke1/smoke1_snow",
		"particle/smokesprites_0001",
		"particle/smokestack",
		"particle/vistasmokev1/vistasmokev1",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		"particle/vistasmokev1/vistasmokev1_fire",
		"particle/vistasmokev1/vistasmokev1_nearcull",
		"particle/vistasmokev1/vistasmokev1_nearcull_fog",
		"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev4_emods_nocull",
		"particle/vistasmokev1/vistasmokev4_nearcull",
		"particle/vistasmokev1/vistasmokev4_nocull" */


	};

	for (auto material_name : smoke_materials)
	{
		auto material = g_MatSystem->FindMaterial(material_name, TEXTURE_GROUP_OTHER);

		if (!material)
			continue;

		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Configurations.remove_smoke);
	}

	static std::vector <const char*> skope_materials =
	{
		"dev/blurfilterx_nohdr",
		"dev/blurfiltery_nohdr",
		"models/weapons/shared/scope/scope_dot_green",
		"particle/fire_burning_character/fire_env_fire",
		"models/weapons/shared/scope/scope_dot_red",
		"dev/scope_bluroverlay"

	};

	for (auto material_name2 : skope_materials)
	{
		auto material = g_MatSystem->FindMaterial(material_name2, TEXTURE_GROUP_OTHER);

		if (!material)
			continue;

		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Configurations.remove_zoom);
	}

	flash->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Configurations.remove_flash);
	flash_white->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Configurations.remove_flash);

	if (is_player && g_Configurations.chams_player_enabled) 
	{
		// 
		// Draw player chams.
		// 
		auto ent = C_BasePlayer::GetPlayerByIndex(info.entity_index);

		if (ent && g_LocalPlayer && ent->IsAlive()) 
		{
			const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
			if (!enemy && g_Configurations.chams_player_enemies_only)
				return;

			const auto clr_front = enemy ? g_Configurations.color_chams_player_enemy_visible : g_Configurations.color_chams_player_ally_visible;
			const auto clr_back = enemy ? g_Configurations.color_chams_player_enemy_occluded : g_Configurations.color_chams_player_ally_occluded;

			if (g_Configurations.chams_player_ignorez) 
			{
				OverrideMaterial(true, g_Configurations.chams_player_flat, g_Configurations.chams_player_wireframe, false, clr_back);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial( false, g_Configurations.chams_player_flat, g_Configurations.chams_player_wireframe, false, clr_front);
			}
			else 
			{
				OverrideMaterial(false, g_Configurations.chams_player_flat, g_Configurations.chams_player_wireframe, g_Configurations.chams_player_glass, clr_front);
			}
		}
	}
	else if (is_sleeve && g_Configurations.chams_sleeve_enabled)
	{
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		// 
		// Draw sleeve chams.
		//
		if (g_Configurations.chams_sleeve_ignorez)
		{
			OverrideMaterial(true, g_Configurations.chams_sleeve_flat, g_Configurations.chams_sleeve_wireframe, false, g_Configurations.color_chams_sleeve_occluded);
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			OverrideMaterial(false, g_Configurations.chams_sleeve_flat, g_Configurations.chams_sleeve_wireframe, false, g_Configurations.color_chams_sleeve_visible);
		}
		else
		{
			OverrideMaterial(false, g_Configurations.chams_sleeve_flat, g_Configurations.chams_sleeve_wireframe, g_Configurations.chams_sleeve_glass,
				g_Configurations.color_chams_sleeve_visible);
		}
	}
	else if (is_arm && g_Configurations.chams_arms_enabled)
	{
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;

		if (g_Configurations.misc_no_hands)
		{
			// 
			// No hands.
			// 
			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			g_MdlRender->ForcedMaterialOverride(material);
		}

		if (g_Configurations.chams_arms_ignorez)
		{
			OverrideMaterial(true, g_Configurations.chams_arms_flat, g_Configurations.chams_arms_wireframe, false, g_Configurations.color_chams_arms_occluded);
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			OverrideMaterial(false, g_Configurations.chams_arms_flat, g_Configurations.chams_arms_wireframe, false, g_Configurations.color_chams_arms_visible);
		}
		else
		{
			OverrideMaterial(false, g_Configurations.chams_arms_flat, g_Configurations.chams_arms_wireframe, g_Configurations.chams_arms_glass,
				g_Configurations.color_chams_arms_visible);
		}
	}
	else if (is_weapon && g_Configurations.chams_weapon_enabled && !is_arm)
	{
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		// 
		// Draw weapon chams.
		//
		if (g_Configurations.chams_weapon_ignorez)
		{
			OverrideMaterial(true, g_Configurations.chams_weapon_flat, g_Configurations.chams_weapon_wireframe, false, g_Configurations.color_chams_weapon_occluded);
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			OverrideMaterial(false, g_Configurations.chams_weapon_flat, g_Configurations.chams_weapon_wireframe, false, g_Configurations.color_chams_weapon_visible);
		}
		else
		{
			OverrideMaterial(false, g_Configurations.chams_weapon_flat, g_Configurations.chams_weapon_wireframe, g_Configurations.chams_weapon_glass,
				g_Configurations.color_chams_weapon_visible);
		}
	}
}