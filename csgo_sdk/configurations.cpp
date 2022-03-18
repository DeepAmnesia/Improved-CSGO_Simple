#include "configurations.hpp"
#include "sdk/sdk.hpp"
#include "sdk/csgostructs.hpp"

namespace values {
	const char* bind_types[] = {
		"Hold",
		"Toggle",
		"On",
		"Off"
	};
	const char* aim_types[] = {
		"Hitbox",
		"Nearest"
	};
	const char* smooth_types[] = {
		"Slow near target",
		"Linear"
	};
	const char* hitbox_list[] = {
		"Head",
		"Neck",
		"Pelvis",
		"Stomach",
		"Lower chest",
		"Chest",
		"Upper chest",
	};
	const char* rcs_types[] = {
		"Standalone",
		"Aim"
	};
	const char* priorities[] = {
		"FOV",
		"Health",
		"Damage",
		"Distance"
	};
	const char* fov_types[] = {
		"Static",
		"Dynamic"
	};
	const char* show_spread_types[] = {
		"Color",
		"Rainbow"
	};
	const char* knifebot_filters[] = {
		"All",
		"Team",
		"Enemy"
	};
	const char* knifebot_modes[] = {
		"Auto",
		"Left, Right",
		"Right",
		"Left"
	};
	const char* glow_modes[] = {
		"Default",
		"Pulsing full",
		"Obvodka",
		"Obvodka pulsing"
	};
	const char* legitaa_modes[] = {
		"Static",
		"Balanced (LBY)",
		"Balanced (move)",
		"Test mode"
	};
	const char* resolver_modes[] = {
		"Desync"
	};
	const char* soundesp_modes[] = {
		"Render",
		"Beams"
	};

	std::vector< weapon_name_t> WeaponNames =
	{
	{ WEAPON_AK47, "AK-47" },
	{ WEAPON_AUG, "AUG" },
	{ WEAPON_AWP, "AWP" },
	{ WEAPON_CZ75A, "CZ75 Auto" },
	{ WEAPON_DEAGLE, "Desert Eagle" },
	{ WEAPON_ELITE, "Dual Berettas" },
	{ WEAPON_FAMAS, "FAMAS" },
	{ WEAPON_FIVESEVEN, "Five-SeveN" },
	{ WEAPON_G3SG1, "G3SG1" },
	{ WEAPON_GALILAR, "Galil AR" },
	{ WEAPON_GLOCK, "Glock-18" },
	{ WEAPON_M249, "M249" },
	{ WEAPON_M4A1_SILENCER, "M4A1-S" },
	{ WEAPON_M4A1, "M4A4" },
	{ WEAPON_MAC10, "MAC-10" },
	{ WEAPON_MAG7, "MAG-7" },
	{ WEAPON_MP7, "MP7" },
	{ WEAPON_MP5, "MP5" },
	{ WEAPON_MP9, "MP9" },
	{ WEAPON_NEGEV, "Negev" },
	{ WEAPON_NOVA, "Nova" },
	{ WEAPON_HKP2000, "P2000" },
	{ WEAPON_P250, "P250" },
	{ WEAPON_P90, "P90" },
	{ WEAPON_BIZON, "PP-Bizon" },
	{ WEAPON_REVOLVER, "R8 Revolver" },
	{ WEAPON_SAWEDOFF, "Sawed-Off" },
	{ WEAPON_SCAR20, "SCAR-20" },
	{ WEAPON_SSG08, "SSG 08" },
	{ WEAPON_SG556, "SG 553" },
	{ WEAPON_TEC9, "Tec-9" },
	{ WEAPON_UMP45, "UMP-45" },
	{ WEAPON_USP_SILENCER, "USP-S" },
	{ WEAPON_XM1014, "XM1014" },
	};

	std::vector< weapon_name_t> WeaponNamesFull =
	{
	{WEAPON_KNIFE, "Knife"},
	{GLOVE_T_SIDE, "Glove"},
	{ WEAPON_AK47, "AK-47" },
	{ WEAPON_AUG, "AUG" },
	{ WEAPON_AWP, "AWP" },
	{ WEAPON_CZ75A, "CZ75 Auto" },
	{ WEAPON_DEAGLE, "Desert Eagle" },
	{ WEAPON_ELITE, "Dual Berettas" },
	{ WEAPON_FAMAS, "FAMAS" },
	{ WEAPON_FIVESEVEN, "Five-SeveN" },
	{ WEAPON_G3SG1, "G3SG1" },
	{ WEAPON_GALILAR, "Galil AR" },
	{ WEAPON_GLOCK, "Glock-18" },
	{ WEAPON_M249, "M249" },
	{ WEAPON_M4A1_SILENCER, "M4A1-S" },
	{ WEAPON_M4A1, "M4A4" },
	{ WEAPON_MAC10, "MAC-10" },
	{ WEAPON_MAG7, "MAG-7" },
	{ WEAPON_MP7, "MP7" },
	{ WEAPON_MP5, "MP5" },
	{ WEAPON_MP9, "MP9" },
	{ WEAPON_NEGEV, "Negev" },
	{ WEAPON_NOVA, "Nova" },
	{ WEAPON_HKP2000, "P2000" },
	{ WEAPON_P250, "P250" },
	{ WEAPON_P90, "P90" },
	{ WEAPON_BIZON, "PP-Bizon" },
	{ WEAPON_REVOLVER, "R8 Revolver" },
	{ WEAPON_SAWEDOFF, "Sawed-Off" },
	{ WEAPON_SCAR20, "SCAR-20" },
	{ WEAPON_SSG08, "SSG 08" },
	{ WEAPON_SG556, "SG 553" },
	{ WEAPON_TEC9, "Tec-9" },
	{ WEAPON_UMP45, "UMP-45" },
	{ WEAPON_USP_SILENCER, "USP-S" },
	{ WEAPON_XM1014, "XM1014" },
	};

	

	std::map<short, std::string> WNames = {
		{ WEAPON_AK47, "AK-47" },
		{ WEAPON_AUG, "AUG" },
		{ WEAPON_AWP, "AWP" },
		{ WEAPON_CZ75A, "CZ75 Auto" },
		{ WEAPON_DEAGLE, "Desert Eagle" },
		{ WEAPON_ELITE, "Dual Berettas" },
		{ WEAPON_FAMAS, "FAMAS" },
		{ WEAPON_FIVESEVEN, "Five-SeveN" },
		{ WEAPON_G3SG1, "G3SG1" },
		{ WEAPON_GALILAR, "Galil AR" },
		{ WEAPON_GLOCK, "Glock-18" },
		{ WEAPON_M249, "M249" },
		{ WEAPON_M4A1_SILENCER, "M4A1-S" },
		{ WEAPON_M4A1, "M4A4" },
		{ WEAPON_MAC10, "MAC-10" },
		{ WEAPON_MAG7, "MAG-7" },
		{ WEAPON_MP7, "MP7" },
		{ WEAPON_MP5, "MP5" },
		{ WEAPON_MP9, "MP9" },
		{ WEAPON_NEGEV, "Negev" },
		{ WEAPON_NOVA, "Nova" },
		{ WEAPON_HKP2000, "P2000" },
		{ WEAPON_P250, "P250" },
		{ WEAPON_P90, "P90" },
		{ WEAPON_BIZON, "PP-Bizon" },
		{ WEAPON_REVOLVER, "R8 Revolver" },
		{ WEAPON_SAWEDOFF, "Sawed-Off" },
		{ WEAPON_SCAR20, "SCAR-20" },
		{ WEAPON_SSG08, "SSG 08" },
		{ WEAPON_SG556, "SG 553" },
		{ WEAPON_TEC9, "Tec-9" },
		{ WEAPON_UMP45, "UMP-45" },
		{ WEAPON_USP_SILENCER, "USP-S" },
		{ WEAPON_XM1014, "XM1014" }
	};

	const char* GetWeaponNameById(int id)
	{
		switch (id)
		{
		case 1:
			return "deagle";
		case 2:
			return "elite";
		case 3:
			return "fiveseven";
		case 4:
			return "glock";
		case 7:
			return "ak47";
		case 8:
			return "aug";
		case 9:
			return "awp";
		case 10:
			return "famas";
		case 11:
			return "g3sg1";
		case 13:
			return "galilar";
		case 14:
			return "m249";
		case 60:
			return "m4a1_silencer";
		case 16:
			return "m4a1";
		case 17:
			return "mac10";
		case 19:
			return "p90";
		case 23:
			return "mp5sd";
		case 24:
			return "ump45";
		case 25:
			return "xm1014";
		case 26:
			return "bizon";
		case 27:
			return "mag7";
		case 28:
			return "negev";
		case 29:
			return "sawedoff";
		case 30:
			return "tec9";
		case 32:
			return "hkp2000";
		case 33:
			return "mp7";
		case 34:
			return "mp9";
		case 35:
			return "nova";
		case 36:
			return "p250";
		case 38:
			return "scar20";
		case 39:
			return "sg556";
		case 40:
			return "ssg08";
		case 61:
			return "usp_silencer";
		case 63:
			return "cz75a";
		case 64:
			return "revolver";
		case 508:
			return "knife_m9_bayonet";
		case 500:
			return "bayonet";
		case 505:
			return "knife_flip";
		case 506:
			return "knife_gut";
		case 507:
			return "knife_karambit";
		case 509:
			return "knife_tactical";
		case 512:
			return "knife_falchion";
		case 514:
			return "knife_survival_bowie";
		case 515:
			return "knife_butterfly";
		case 516:
			return "knife_push";

		case 519:
			return "knife_ursus";
		case 520:
			return "knife_gypsy_jackknife";
		case 522:
			return "knife_stiletto";
		case 523:
			return "knife_widowmaker";
		case 5027:
			return "studded_bloodhound_gloves";
		case 5028:
			return "t_gloves";
		case 5029:
			return "ct_gloves";
		case 5030:
			return "sporty_gloves";
		case 5031:
			return "slick_gloves";
		case 5032:
			return "leather_handwraps";
		case 5033:
			return "motorcycle_gloves";
		case 5034:
			return "specialist_gloves";
		case 5035:
			return "studded_hydra_gloves";

		default:
			return "";
		}
	}

};
#include "configurations.hpp"