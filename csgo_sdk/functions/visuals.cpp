#include <algorithm>

#include "visuals.hpp"
#include "grenade_pred.hpp"

#include "../configurations.hpp"
#include "../sdk/utils/math.hpp"
#include "../sdk/utils/utils.hpp"
#include "../functions/damage_indicator.h"
#include "../imgui/imgui.h"


RECT GetBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = 
	{
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) 
	{
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) 
	{
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) 
	{
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}

	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}

Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() 
{
	DeleteCriticalSection(&cs);
}

void Visuals::Render() 
{

}



bool Visuals::Player::Begin(C_BasePlayer* pl)
{
	if (!pl->IsAlive())
		return false;

	ctx.pl = pl;
	ctx.is_enemy = g_LocalPlayer->m_iTeamNum() != pl->m_iTeamNum();
	ctx.is_visible = g_LocalPlayer->CanSeePlayer(pl, HITBOX_CHEST);

	if (!ctx.is_enemy && g_Configurations.esp_enemies_only)
		return false;

	ctx.clr = ctx.is_enemy ? (ctx.is_visible ? g_Configurations.color_esp_enemy_visible : g_Configurations.color_esp_enemy_occluded) : (ctx.is_visible ? g_Configurations.color_esp_ally_visible : g_Configurations.color_esp_ally_occluded);

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	head.z += 15;
	auto head2 = pl->GetHitboxPos(HITBOX_HEAD);
	if (!Math::WorldToScreen(head, ctx.head_pos) || !Math::WorldToScreen(origin, ctx.feet_pos) || !Math::WorldToScreen(head2, ctx.head_pos_def))
		return false;

	auto h = fabs(ctx.head_pos.y - ctx.feet_pos.y);
	auto w = h / 2.f;

	ctx.bbox.left = static_cast<long>(ctx.feet_pos.x - w * 0.5f);
	ctx.bbox.right = static_cast<long>(ctx.bbox.left + w);
	ctx.bbox.bottom = static_cast<long>(ctx.feet_pos.y);
	ctx.bbox.top = static_cast<long>(ctx.head_pos.y);

	return true;
}

void Visuals::Player::RenderBox() 
{
	if (g_Configurations.esp_box_filled)
	{
		if (!g_Configurations.esp_box_filled_gradient)
			Render::Get().RenderBoxFilled(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : ctx.clr, 1);
		else
			Render::Get().RenderBoxGradient(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, Color(0, 0, 0, 0), ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.box_gradient_color, 1);
	}
	Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : ctx.clr, 1);
	Render::Get().RenderBoxByType(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(0,0,0, 84) : Color(0, 0, 0, 0) : Color::Black, 1);
	Render::Get().RenderBoxByType(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(0,0,0, 84) : Color(0, 0, 0, 0) : Color::Black, 1);
}

void Visuals::Player::RenderName()
{
	player_info_t info = ctx.pl->GetPlayerInfo();

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, info.szName);

	Render::Get().RenderText(info.szName, ctx.feet_pos.x - sz.x / 2, ctx.head_pos.y - sz.y, 14.f, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.esp_names_color);
}

void Visuals::Player::RenderHealth()
{
	auto  hp = ctx.pl->m_iHealth();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 8;

	int height = (box_h * hp) / 100;

	int green = int(hp * 2.55f);
	int red = 255 - green;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;

	if (!g_Configurations.health_background)
	Render::Get().RenderBox(x, y, x + w, y + h, Color::Black, 1.f, true);
	else
	{
		if (g_Configurations.health_background_gradient)
		{
			Render::Get().RenderBoxGradient(x + 1, y + 1, x + w - 1, y + h - 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.health_background_color, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.health_background_second, 1.f, true);
		}
		else
		{
			Render::Get().RenderBoxFilled(x + 1, y + 1, x + w - 1, y + h - 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.health_background_color, 1.f, true);
		}
	}

	if (!g_Configurations.health_based_on_health)
	Render::Get().RenderBoxFilled(x + 1, y + 1, x + w - 1, y + height - 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : Color(red, green, 0, 255), 1.f, true);
	else
	{
		if (!g_Configurations.health_gradient)
		{
			Render::Get().RenderBoxFilled(x + 1, y + 1, x + w - 1, y + height - 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.health_color, 1.f, true);
		}
		else
		{
			Render::Get().RenderBoxGradient(x + 1, y + 1, x + w - 1, y + height - 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.health_color, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.health_second_color, 1.f, true);
		}
	}
}

void Visuals::Player::DrawHeadDot() {
	Render::Get().RenderBox(ctx.head_pos_def.x - 2, ctx.head_pos_def.y - 2, ctx.head_pos_def.x + 2, ctx.head_pos_def.y + 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.head_dot_color);
	Render::Get().RenderBox(ctx.head_pos_def.x - 1, ctx.head_pos_def.y - 1, ctx.head_pos_def.x + 1, ctx.head_pos_def.y + 1, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(0, 0, 0, 84) : Color(0, 0, 0, 0) : Color::Black);
	Render::Get().RenderBox(ctx.head_pos_def.x - 3, ctx.head_pos_def.y - 3, ctx.head_pos_def.x + 3, ctx.head_pos_def.y + 3, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(0,0,0, 84) : Color(0, 0, 0, 0) : Color::Black);
}

void Visuals::Player::RenderArmour()
{
	auto  armour = ctx.pl->m_ArmorValue();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 4;

	int height = (((box_h * armour) / 100));

	int x = ctx.bbox.right + off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;
	if (!g_Configurations.armour_background)
	Render::Get().RenderBox(x, y, x + w, y + h, Color::Black, 1.f, true);
	else
	{
		if (g_Configurations.armour_background_gradient)
		{
			Render::Get().RenderBoxGradient(x + 1, y + 1, x + w - 1, y + h - 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.armour_background_color, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.armour_background_second, 1.f, true);
		}
		else
		{
			Render::Get().RenderBoxFilled(x + 1, y + 1, x + w - 1, y + h - 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.armour_background_color, 1.f, true);
		}
	}

	if (!g_Configurations.armour_gradient)
	Render::Get().RenderBox(x + 1, y + 1, x + w - 1, y + height - 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.armour_color, 1.f, true);
	else
	{
		Render::Get().RenderBoxGradient(x + 1, y + 1, x + w - 1, y + height - 2, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.armour_color, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : g_Configurations.armour_second_color, 1.f, true);
	}
}

void Visuals::Player::RenderWeaponName()
{
	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	//if (ctx.pl->IsDormant())


	Render::Get().RenderText(weapon->get_name().c_str(), ctx.feet_pos.x, ctx.feet_pos.y, 14.f, ctx.pl->IsDormant()? g_Configurations.dormant_esp? Color(255,255,255,84) : Color(0,0,0,0) : g_Configurations.esp_weapon_color, true, g_pDefaultFont);
}

void Visuals::Player::RenderSnapline()
{

	int screen_w, screen_h;
	g_EngineClient->GetScreenSize(screen_w, screen_h);

	Render::Get().RenderLine(screen_w / 2.f, (float)screen_h,
		ctx.feet_pos.x, ctx.feet_pos.y, ctx.pl->IsDormant() ? g_Configurations.dormant_esp ? Color(255, 255, 255, 84) : Color(0, 0, 0, 0) : ctx.clr);
}


void Visuals::RenderWeapon(C_BaseCombatWeapon* ent)
{
	auto clean_item_name = [](const char* name) -> const char* 
	{
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Configurations.color_esp_weapons);

	auto name = clean_item_name(ent->GetClientClass()->m_pNetworkName);

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;

	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Configurations.color_esp_weapons);
}

void Visuals::RenderDefuseKit(C_BaseEntity* ent)
{
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Configurations.color_esp_defuse);

	auto name = "Defuse Kit";
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;
	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Configurations.color_esp_defuse);
}

void Visuals::RenderPlantedC4(C_BaseEntity* ent)
{
	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Configurations.color_esp_c4);

	int bombTimer = std::ceil(ent->m_flC4Blow() - g_GlobalVars->curtime);
	std::string timer = std::to_string(bombTimer);

	auto name = (bombTimer < 0.f) ? "Bomb" : timer;
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name.c_str());
	int w = bbox.right - bbox.left;

	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Configurations.color_esp_c4);
}

void Visuals::RenderItemEsp(C_BaseEntity* ent)
{
	std::string itemstr = "Undefined";

	const model_t * itemModel = ent->GetModel();
	if (!itemModel)
		return;

	studiohdr_t * hdr = g_MdlInfo->GetStudiomodel(itemModel);
	if (!hdr)
		return;

	itemstr = hdr->szName;

	if (ent->GetClientClass()->m_ClassID == ClassId_CBumpMine)
		itemstr = "";
	else if (itemstr.find("case_pistol") != std::string::npos)
		itemstr = "Pistol Case";
	else if (itemstr.find("case_light_weapon") != std::string::npos)
		itemstr = "Light Case";
	else if (itemstr.find("case_heavy_weapon") != std::string::npos)
		itemstr = "Heavy Case";
	else if (itemstr.find("case_explosive") != std::string::npos)
		itemstr = "Explosive Case";
	else if (itemstr.find("case_tools") != std::string::npos)
		itemstr = "Tools Case";
	else if (itemstr.find("random") != std::string::npos)
		itemstr = "Airdrop";
	else if (itemstr.find("dz_armor_helmet") != std::string::npos)
		itemstr = "Full Armor";
	else if (itemstr.find("dz_helmet") != std::string::npos)
		itemstr = "Helmet";
	else if (itemstr.find("dz_armor") != std::string::npos)
		itemstr = "Armor";
	else if (itemstr.find("upgrade_tablet") != std::string::npos)
		itemstr = "Tablet Upgrade";
	else if (itemstr.find("briefcase") != std::string::npos)
		itemstr = "Briefcase";
	else if (itemstr.find("parachutepack") != std::string::npos)
		itemstr = "Parachute";
	else if (itemstr.find("dufflebag") != std::string::npos)
		itemstr = "Cash Dufflebag";
	else if (itemstr.find("ammobox") != std::string::npos)
		itemstr = "Ammobox";
	else if (itemstr.find("dronegun") != std::string::npos)
		itemstr = "Turrel";
	else if (itemstr.find("exojump") != std::string::npos)
		itemstr = "Exojump";
	else if (itemstr.find("healthshot") != std::string::npos)
		itemstr = "Healthshot";
	else 
		return;
	
	auto bbox = GetBBox(ent);
	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, itemstr.c_str());
	int w = bbox.right - bbox.left;

	Render::Get().RenderText(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Configurations.color_esp_item);
}
void Thirdperson_Init(bool fakeducking, float progress) {

	static float current_fraction = 0.0f;

	auto distance = ((float)g_Configurations.misc_thirdperson_dist) * progress;

	QAngle angles;
	QAngle inverse_angles;
	g_EngineClient->GetViewAngles(&angles);
	g_EngineClient->GetViewAngles(&inverse_angles);


	inverse_angles.roll = distance;

	Vector forward, right, up;
	
	Math::AngleVectors(inverse_angles, forward, right, up);

	auto eye_pos = fakeducking ? g_LocalPlayer->GetAbsOrigin() + g_GameMovement->GetPlayerViewOffset(false) : g_LocalPlayer->GetAbsOrigin() + g_LocalPlayer->m_vecViewOffset();

	auto offset = eye_pos + forward * -distance + right + up;

	CTraceFilterWorldOnly filter;
	trace_t tr;
	g_EngineTrace->TraceRay(Ray_t(eye_pos, offset, Vector(-16.0f, -16.0f, -16.0f), Vector(16.0f, 16.0f, 16.0f)), 131083, &filter, &tr);

	if (current_fraction > tr.fraction)

		current_fraction = tr.fraction;

	else if (current_fraction > 0.9999f)

		current_fraction = 1.0f;

	current_fraction = Math::Interpolate(current_fraction, tr.fraction, g_GlobalVars->frametime * 10.0f);
	angles.roll = distance * current_fraction;
}
void Visuals::ThirdPerson() 
{
	/*
	static auto in_thirdperson = false;

	static bool down = false;
	static bool clicked = false;

	static bool pressed = third_person_pressed;
	if (!g_LocalPlayer || !g_EngineTrace || !g_Input || !g_Configurations.misc_thirdperson)
	{
		g_Configurations.g_Globals.InThirdPerson = false;
		in_thirdperson = false;

		down = false;
		clicked = false;
		return;
	}

	if (third_person_pressed != pressed)
	{
		pressed = third_person_pressed;
		g_Configurations.g_Globals.InThirdPerson = !g_Configurations.g_Globals.InThirdPerson;
		in_thirdperson = !in_thirdperson;
	}
	

//	if (clicked) in_thirdperson = !in_thirdperson;

	static const auto cam_idealdist = g_CVar->FindVar("cam_idealdist");
	static const auto cam_collision = g_CVar->FindVar("cam_collision");

	if (!g_EngineClient->IsInGame() || !g_LocalPlayer) return;

	static auto percent = 0.f;
	
	
	if (g_LocalPlayer->IsAlive() && in_thirdperson) {
		
		

		g_Input->m_fCameraInThirdPerson = true;

		percent = std::clamp(percent + g_GlobalVars->frametime * 3.f, 0.3f, 1.f);
		cam_idealdist->SetValue(g_Configurations.misc_thirdperson_dist * percent);
		cam_collision->SetValue(1);
	}
	else if (g_Input->m_fCameraInThirdPerson) {
		percent = std::clamp(percent - g_GlobalVars->frametime * 3.f, 0.3f, 1.f);

		cam_idealdist->SetValue(g_Configurations.misc_thirdperson_dist * percent);
		cam_collision->SetValue(0);
		if (percent <= 0.3f) {
			g_Input->m_fCameraInThirdPerson = false;
		}
	}
	*/

	

	static float progress;
	static bool in_transition;
	static auto in_thirdperson = false;
	static bool pressed = third_person_pressed;
	if (!g_LocalPlayer || !g_EngineTrace || !g_Input || !g_Configurations.misc_thirdperson)
	{
		g_Configurations.g_Globals.InThirdPerson = false;
		in_thirdperson = false;
		return;
	}

	if (third_person_pressed != pressed)
	{
		pressed = third_person_pressed;
		g_Configurations.g_Globals.InThirdPerson = !g_Configurations.g_Globals.InThirdPerson;
		in_thirdperson = !in_thirdperson;
	}

	if (g_LocalPlayer->IsAlive() && in_thirdperson)
	{
		in_transition = false;
		if (!g_Input->m_fCameraInThirdPerson)
		{
			g_Input->m_fCameraInThirdPerson = true;
		}
	}
	else
	{
		progress -= g_GlobalVars->frametime * 8.f + (progress / 100);
		progress = std::clamp(progress, 0.f, 1.f);
		if (!progress)
			g_Input->m_fCameraInThirdPerson = false;
		else
		{
			in_transition = true;
			g_Input->m_fCameraInThirdPerson = true;
		}
	}
	if (g_Input->m_fCameraInThirdPerson && !in_transition)
	{
		progress += g_GlobalVars->frametime * 8.f + (progress / 100);
		progress = std::clamp(progress, 0.f, 1.f);
	}
	Thirdperson_Init(false, progress);

}
void Visuals::ShowFov() {
	if (!g_Configurations.draw_aim_fov) return;
	static int fov, silent_fov;
	static bool silent_enable = false;

	float fov_fix = g_Configurations.g_Globals.OFOV >= 90.f ? (g_Configurations.g_Globals.SHeight / (80.f * g_Configurations.g_Globals.OFOV / 90.f) / (g_Configurations.g_Globals.OFOV / 90.f)) : (g_Configurations.g_Globals.SHeight / (80.f * g_Configurations.g_Globals.OFOV / 90.f) / (g_Configurations.g_Globals.OFOV / 80.f) * (g_Configurations.g_Globals.OFOV / 80.f));

	if (!g_LocalPlayer)
		return;

	auto wpn = g_LocalPlayer->m_hActiveWeapon();
	if (!wpn)
		return;

	short weapon = wpn.Get()->m_Item().m_iItemDefinitionIndex();

	auto setts = *g_CustomWeaponGroups->GetSettings(weapon);

	fov = setts.fov * fov_fix;
	silent_fov = setts.silent_fov * fov_fix;

	silent_enable = setts.silent;

	Render::Get().RenderCircle(g_Configurations.g_Globals.SWidthHalf, g_Configurations.g_Globals.SHeightHalf, fov, 50, g_Configurations.default_aim_fov, 0.1f);
	if (silent_enable)
		Render::Get().RenderCircle(g_Configurations.g_Globals.SWidthHalf, g_Configurations.g_Globals.SHeightHalf, silent_fov, 50, g_Configurations.silent_aim_fov, 0.1f);
}




void Visuals::AddToDrawList() 
{
	if (g_Configurations.esp_grenade_prediction)
		Grenade_Pred::Get().DrawPrediction();

	for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) 
	{
		auto entity = C_BaseEntity::GetEntityByIndex(i);

		if (!entity)
			continue;
		
		if (entity == g_LocalPlayer && !g_Input->m_fCameraInThirdPerson)
			continue;

		if (i <= g_GlobalVars->maxClients) 
		{
			auto player = Player();
			if (player.Begin((C_BasePlayer*)entity)) 
			{
				if (g_Configurations.esp_player_snaplines) 
					player.RenderSnapline();
				if (g_Configurations.esp_player_boxes)     
					player.RenderBox();
				if (g_Configurations.esp_player_weapons)   
					player.RenderWeaponName();
				if (g_Configurations.esp_player_names)     
					player.RenderName();
				if (g_Configurations.esp_player_health)    
					player.RenderHealth();
				if (g_Configurations.esp_player_armour)    
					player.RenderArmour();
				if (g_Configurations.head_dot)
					player.DrawHeadDot();
			}
		}
		else if (g_Configurations.esp_dropped_weapons && entity->IsWeapon())
			RenderWeapon(static_cast<C_BaseCombatWeapon*>(entity));
		else if (g_Configurations.esp_dropped_weapons && entity->IsDefuseKit())
			RenderDefuseKit(entity);
		else if (entity->IsPlantedC4() && g_Configurations.esp_planted_c4)
			RenderPlantedC4(entity);
		else if (entity->IsLoot() && g_Configurations.esp_items)
			RenderItemEsp(entity);
	}

	//	DrawSounds();
		damage_indicators.paint();
		Visuals::Get().ShowFov();
}

void Visuals::MenuVisuals() {

	ImGui::Checkbox("Third Person", &g_Configurations.misc_thirdperson);
	ImGui::Keybind("##HotKey2", &thirdperson_key);
	ImGui::SliderFloat("Distance", g_Configurations.misc_thirdperson_dist, 0.f, 150.f);
}

void Visuals::SetupValues() {
	g_Config->PushItem(&g_Configurations.g_Globals.InThirdPerson, "visuals", "third_person", g_Configurations.g_Globals.InThirdPerson);
	g_Config->PushItem(&thirdperson_key, "visuals", "3p_key", thirdperson_key);
}
