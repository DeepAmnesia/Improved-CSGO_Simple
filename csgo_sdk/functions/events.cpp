#include "../functions/events.hpp"
#include "../render/render.hpp"
#include "../render/menu.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <limits>
#include <ctime>

template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan(GetModuleHandleA("client.dll"), "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto FindHudElement = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)FindHudElement(pThis, name);
}

void C_HookedEvents::FireGameEvent(IGameEvent* pEvent)
{
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	if (!pEvent)
		return;

	if (!strcmp(pEvent->GetName(), "player_hurt"))
	{
		C_BasePlayer* hurt = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"))));
		C_BasePlayer* attacker = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")));
		if (g_Configurations.hitsound)
		{
			if (hurt != g_LocalPlayer && attacker == g_LocalPlayer)
			{
				g_VGuiSurface->PlaySound_("buttons\\arena_switch_press_02.wav");
			}
		}
		if (g_Configurations.damage_indicator)
		{
			int dmg_health = pEvent->GetInt("dmg_health");
			int hitgroup = pEvent->GetInt("hitgroup");
			DamageIndicator_t DmgIndicator;
			DmgIndicator.startTime = g_GlobalVars->curtime;
			DmgIndicator.hitgroup = hitgroup;
			DmgIndicator.hitPosition = damage_indicators.c_impactpos;
			DmgIndicator.damage = dmg_health;
			DmgIndicator.randomIdx = Math::RandomFloat(-20.f, 20.f);
			DmgIndicator.valid = true;
			if (damage_indicators.c_impactpos.x)
				damage_indicators.floatingTexts.push_back(DmgIndicator);
		}
	}
	if (!strcmp(pEvent->GetName(), "bullet_impact"))
	{
		float x = pEvent->GetFloat("x");
		float y = pEvent->GetFloat("y");
		float z = pEvent->GetFloat("z");

		C_BasePlayer* target = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"))));

		if (!target || target != g_LocalPlayer)
			return;

		damage_indicators.c_impactpos = Vector(x, y, z);
	}

	if (!strcmp(pEvent->GetName(), "player_death"))
	{
		static DWORD* DeathNotice = FindHudElement<DWORD>("CCSGO_HudDeathNotice");
		static void(__thiscall * ClearNotices)(DWORD) = (void(__thiscall*)(DWORD))Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 0C 53 56 8B 71 58");
		if (DeathNotice)
			*(float*)((DWORD)DeathNotice + 0x50) = g_Configurations.preserve_killfeed ? FLT_MAX : 1.5;
	}
}

int C_HookedEvents::GetEventDebugID(void)
{
	return EVENT_DEBUG_ID_INIT;
}

void C_HookedEvents::RegisterSelf()
{
	m_iDebugId = EVENT_DEBUG_ID_INIT;
	g_GameEvents->AddListener(this, "player_footstep", false);
	g_GameEvents->AddListener(this, "player_hurt", false);
	g_GameEvents->AddListener(this, "player_death", false);
	g_GameEvents->AddListener(this, "weapon_fire", false);
	g_GameEvents->AddListener(this, "item_purchase", false);
	g_GameEvents->AddListener(this, "bullet_impact", false);
	g_GameEvents->AddListener(this, "round_start", false);
	g_GameEvents->AddListener(this, "round_freeze_end", false);
	g_GameEvents->AddListener(this, "bomb_defused", false);
	g_GameEvents->AddListener(this, "bomb_begindefuse", false);
	g_GameEvents->AddListener(this, "bomb_beginplant", false);
}

void C_HookedEvents::RemoveSelf()
{
	g_GameEvents->RemoveListener(this);
}