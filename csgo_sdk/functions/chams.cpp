#include "chams.hpp"

#include <fstream>

#include "../sdk/csgostructs.hpp"
#include "../configurations.hpp"
#include "../sdk/misc/KeyValues.hpp"
#include "../hooks.hpp"

#include "../sdk/utils/input.hpp"

IMaterial* CreateMaterial(bool lit, const std::string& material_data)
{
	static auto created = 0;
	auto matname = "mat_num_" + std::to_string(created);
	++created;
	auto keyValues = new KeyValues(matname.c_str());
	keyValues->LoadFromBuffer(keyValues, matname.c_str(), material_data.c_str());
	auto material = g_MatSystem->CreateMaterial(matname.c_str(), keyValues);
	material->IncrementReferenceCount();
	return material;
}

void ColorModulate(Color color, IMaterial* material)
{
	auto found = false;
	auto var = material->FindVar("$envmaptint", &found);

	if (found)
		var->SetVecValue(color.r()/255.f, color.g()/255.f, color.b()/255.f);

	g_RenderView->SetColorModulation(color.r() / 255.f, color.g() / 255.f, color.b() / 255.f);
}

Chams::Chams()
{
	materialRegular = CreateMaterial(true, R"#("VertexLitGeneric"
	{
		"$basetexture"				"vgui/white"
			"$ignorez"					"0"
			"$envmap"					" "
			"$nofog"					"1"
			"$model"					"1"
			"$nocull"					"0"
			"$selfillum"				"1"
			"$halflambert"				"1"
			"$znearer"					"0"
			"$flat"						"0"
			"$wireframe"				"0"
	}
	)#");
	materialFlat = CreateMaterial(false, R"#("UnlitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"1"
				"$wireframe"				"0"
			}
		)#");
	materialCrystal = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/crystal_clear", nullptr);
	materialGlass = g_MatSystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", nullptr);
	materialCircuit = g_MatSystem->FindMaterial("dev/glow_armsrace.vmt", nullptr);
	materialGlow = CreateMaterial(true, R"#("VertexLitGeneric" 
			{ 
				"$additive"					"1" 
				"$envmap"					"models/effects/cube_white" 
				"$envmaptint"				"[1 1 1]" 
				"$envmapfresnel"			"1" 
				"$envmapfresnelminmaxexp" 	"[0 1 2]" 
				"$alpha" 					"0.8" 
			}
		)#");

	materialAnimated1 = CreateMaterial(true, R"#("VertexLitGeneric"
		    {
		        "$basetexture"				"dev/zone_warning"
		        "$additive"					"1"
		        "$envmap"					"editor/cube_vertigo"
		        "$envmaptint"				"[0 0.5 0.55]"
		        "$envmapfresnel"			"1"
		        "$envmapfresnelminmaxexp"   "[0.00005 0.6 6]"
		        "$alpha"					"1"
   
		        Proxies
		        {
		            TextureScroll
		            {
		                "texturescrollvar"			"$baseTextureTransform"
		                "texturescrollrate"			"0.25"
		                "texturescrollangle"		"270"
		            }
		            Sine
		            {
		                "sineperiod"				"2"
		                "sinemin"					"0.1"
		                "resultVar"					"$envmapfresnelminmaxexp[1]"
		            }
		        }
		    }
		)#");

	materialAnimated2 = CreateMaterial(true, R"#("VertexLitGeneric"
		    {
				"$basetexture"              "dev/zone_warning"
		        "$envmap"					"editor/cube_vertigo"
		        "$envmapcontrast"    		"1"
				"$envmaptint"               "[0.7 0.7 0.7]"
		        Proxies
		        {
		            TextureScroll
		            {
		                "texturescrollvar"			"$baseTextureTransform"
		                "texturescrollrate"			"0.6"
		                "texturescrollangle"		"90"
		            }
		        }
		    }
		)#");

	materialAnimated3 = g_MatSystem->FindMaterial("models/inventory_items/dogtags/dogtags_lightray", nullptr);
	materialAnimated4 = g_MatSystem->FindMaterial("models/inventory_items/music_kit/darude_01/mp3_detail", nullptr);
	materialAnimated5 = g_MatSystem->FindMaterial("models/inventory_items/dreamhack_trophies/dreamhack_star_blur", nullptr);

}
Chams::~Chams() 
{

}


void Chams::OverrideMaterial(int mat_num, bool ignoreZ, bool wireframe, const Color& rgba)
{
	IMaterial* material = nullptr;
	switch (mat_num)
	{
	case 0: material = materialRegular;
		break;
	case 1: material = materialFlat;
		break;
	case 2: material = materialCrystal;
		break;
	case 3: material = materialGlass;
		break;
	case 4: material = materialGlow;
		break;
	case 5: material = materialCircuit;
		break;
	case 6: material = materialAnimated1;
		break;
	case 7: material = materialAnimated2;
		break;
	case 8: material = materialAnimated3;
		break;
	case 9: material = materialAnimated4;
		break;
	case 10: material = materialAnimated5;
		break;

	}


	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);
	ColorModulate(rgba, material);
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


	bool is_weapon_on_back = strstr(mdl->szName, "_dropped.mdl") && strstr(mdl->szName, "models/weapons/w") && !strstr(mdl->szName, "arms") && !strstr(mdl->szName, "ied_dropped");
	bool is_weapon_enemy_hands = strstr(mdl->szName, "models/weapons/w") && !strstr(mdl->szName, "arms") && !strstr(mdl->szName, "ied_dropped");
	bool is_defuse_kit = strstr(mdl->szName, "defuser") && !strstr(mdl->szName, "arms") && !strstr(mdl->szName, "ied_dropped");

	static auto flash = g_MatSystem->FindMaterial("effects/flashbang", TEXTURE_GROUP_CLIENT_EFFECTS);
	static auto flash_white = g_MatSystem->FindMaterial("effects/flashbang_white", TEXTURE_GROUP_CLIENT_EFFECTS);

	static std::vector <const char*> smoke_materials =
	{
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev1_fire"
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
	if ((is_weapon_on_back || is_weapon_enemy_hands || is_defuse_kit) && g_Configurations.chams_attachment_enabled)
	{
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;

		if (g_Configurations.chams_attachment_ignorez)
		{
			OverrideMaterial(g_Configurations.chams_attachment_material_num, true, g_Configurations.chams_attachment_wireframe, g_Configurations.attachment_chams_invisible);
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			OverrideMaterial(g_Configurations.chams_attachment_material_num, false, g_Configurations.chams_attachment_wireframe, g_Configurations.attachment_chams_visible);
		}
		else
		{
			OverrideMaterial(g_Configurations.chams_attachment_material_num, false, g_Configurations.chams_attachment_wireframe, g_Configurations.attachment_chams_visible);
		}
	}
	else if (is_player && g_Configurations.chams_player_enabled)
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

				OverrideMaterial(g_Configurations.chams_players_material_num_invisible, true, g_Configurations.chams_player_wireframe_invisible, clr_back);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(g_Configurations.chams_players_material_num_visible, false, g_Configurations.chams_player_wireframe_visible, clr_front);
			}
			else
			{
				OverrideMaterial(g_Configurations.chams_players_material_num_visible, false, g_Configurations.chams_player_wireframe_visible, clr_front);
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
			OverrideMaterial(g_Configurations.chams_sleeves_material_num, true, g_Configurations.chams_sleeve_wireframe, g_Configurations.color_chams_sleeve_occluded);
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			OverrideMaterial(g_Configurations.chams_sleeves_material_num, false, g_Configurations.chams_sleeve_wireframe, g_Configurations.color_chams_sleeve_visible);
		}
		else
		{
			OverrideMaterial(g_Configurations.chams_sleeves_material_num, false, g_Configurations.chams_sleeve_wireframe, g_Configurations.color_chams_sleeve_visible);
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
			OverrideMaterial(g_Configurations.chams_hands_material_num, true, g_Configurations.chams_arms_wireframe, g_Configurations.color_chams_arms_occluded);
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			OverrideMaterial(g_Configurations.chams_hands_material_num, false, g_Configurations.chams_arms_wireframe, g_Configurations.color_chams_arms_visible);
		}
		else
		{
			OverrideMaterial(g_Configurations.chams_hands_material_num, false, g_Configurations.chams_arms_wireframe, g_Configurations.color_chams_arms_visible);
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
			OverrideMaterial(g_Configurations.chams_weapon_material_num, true, g_Configurations.chams_weapon_wireframe, g_Configurations.color_chams_weapon_occluded);
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			OverrideMaterial(g_Configurations.chams_weapon_material_num, false, g_Configurations.chams_weapon_wireframe, g_Configurations.color_chams_weapon_visible);
		}
		else
		{
			OverrideMaterial(g_Configurations.chams_weapon_material_num, false, g_Configurations.chams_weapon_wireframe, g_Configurations.color_chams_weapon_visible);
		}
	}
	
}