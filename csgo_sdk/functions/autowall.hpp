#pragma once
#include "../sdk/csgostructs.hpp"
#include "../sdk/utils/math.hpp"

#define CHAR_TEXT_WOOD 87
#define CHAT_TEXT_GLASS 2
#define CHAT_TEXT_METAL 60

#define	HITGROUP_GENERIC	0
#define	HITGROUP_HEAD		1
#define	HITGROUP_CHEST		2
#define	HITGROUP_STOMACH	3
#define HITGROUP_LEFTARM	4	
#define HITGROUP_RIGHTARM	5
#define HITGROUP_LEFTLEG	6
#define HITGROUP_RIGHTLEG	7
#define HITGROUP_GEAR		10			// alerts NPC, but doesn't do damage or bleed (1/100th damage)
#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1
#define DAMAGE_YES		2
#define DAMAGE_AIM		3
#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< wierd-looking jello effect for advisor shield.

struct FireBulletData
{
	Vector src;
	trace_t enter_trace;
	Vector direction;
	CTraceFilter filter;
	float trace_length;
	float trace_length_remaining;
	float current_damage;
	int penetrate_count;
	FireBulletData(const Vector& eye_pos) : src(eye_pos) { }
	FireBulletData() { }
};

class Autowall : public Singleton<Autowall>
{
public:

	void ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, ITraceFilter* filter, trace_t* tr);
	float GetHitgroupDamageMultiplier(int iHitGroup);
	bool IsBreakableEntity(C_BasePlayer* pBaseEntity);
	bool HandleBulletPenetration(CCSWeaponInfo* wpn_data, FireBulletData& data);
	bool HandleBulletPenetration(CCSWeaponInfo* weaponData, CGameTrace& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration);
	bool PenetrateWall(C_BasePlayer* pBaseEntity, C_BaseCombatWeapon* pWeapon, const Vector& vecPoint, float& flDamage);
	bool SimulateFireBullet(C_BaseCombatWeapon* pWeapon, FireBulletData& data);
	bool TraceDidHitWorld(trace_t* pTrace);
	float GetDamage(const Vector& point, FireBulletData& data);
	bool TraceDidHitNonWorldEntity(trace_t* pTrace);
	bool TraceToExit(Vector& vecEnd, trace_t* pEnterTrace, Vector vecStart, Vector vecDir, trace_t* pExitTrace);
	void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, unsigned int nMask, C_BasePlayer* pCSIgnore, trace_t* pTrace);
	void TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr);
	void ScaleDamage(int iHitgroup, C_BasePlayer* pBaseEntity, float flWeaponArmorRatio, float& flDamage);
	void ScaleDamage(CGameTrace& enterTrace, CCSWeaponInfo* weaponData, float& currentDamage);
	float GetDamage(const Vector& point);
	float CanHit(Vector& point);
	float CanHit(Vector& start, Vector& point);
	float CanHit(C_BasePlayer* ent, Vector& point);
	bool FireBullet(C_BaseCombatWeapon* pWeapon, Vector& direction, float& currentDamage);
	void GetBulletTypeParameters(float& maxRange, float& maxDistance, char* bulletType, bool sv_penetration_type);
	bool TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction);
	bool BreakableEntity(IClientEntity* entity);


	bool handle_penetration = false;
};