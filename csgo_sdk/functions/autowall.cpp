#include "autowall.hpp"
#include "Math.h"
#include "../sdk/interfaces/IEngineTrace.hpp"
#include "../sdk/utils/math.hpp"
#include "../sdk/utils/utils.hpp"

#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_GEAR 10

float Autowall::GetHitgroupDamageMultiplier(int iHitGroup) {
	switch (iHitGroup) {
	case HITGROUP_GENERIC:
		return 0.5f;
	case HITGROUP_HEAD:
		return 2.0f;
	case HITGROUP_CHEST:
		return 0.5f;
	case HITGROUP_STOMACH:
		return 0.75f;
	case HITGROUP_LEFTARM:
		return 0.5f;
	case HITGROUP_RIGHTARM:
		return 0.5f;
	case HITGROUP_LEFTLEG:
		return 0.375f;
	case HITGROUP_RIGHTLEG:
		return 0.375f;
	case HITGROUP_GEAR:
		return 0.5f;
	default:
		return 1.0f;
	}
	return 1.0f;
}

void Autowall::ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, ITraceFilter* filter, trace_t* tr) {
	float smallestFraction = tr->fraction;
	constexpr float maxRange = 60.0f;

	Vector delta(vecAbsEnd - vecAbsStart);
	const float delta_length = delta.Length();
	delta.NormalizeInPlace();

	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);

	for (int i = 1; i <= g_GlobalVars->maxClients; ++i) {
		auto ent = C_BasePlayer::GetPlayerByIndex(i);
		if (!ent || ent->IsDormant() || ent->m_lifeState() != LIFE_ALIVE)
			continue;

		if (filter && !filter->ShouldHitEntity(ent, mask))
			continue;

		auto collideble = ent->GetCollideable();
		auto mins = collideble->OBBMins();
		auto maxs = collideble->OBBMaxs();

		auto obb_center = (maxs + mins) * 0.5f;
		auto extend = (obb_center - vecAbsStart);
		auto rangeAlong = delta.Dot(extend);

		float range;
		if (rangeAlong >= 0.0f) {
			if (rangeAlong <= delta_length)
				range = Vector(obb_center - ((delta * rangeAlong) + vecAbsStart)).Length();
			else
				range = -(obb_center - vecAbsEnd).Length();
		}
		else {
			range = -extend.Length();
		}

		if (range >= 0.0f && range <= maxRange) {
			trace_t playerTrace;
			g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ent, &playerTrace);
			if (playerTrace.fraction < smallestFraction) {
				*tr = playerTrace;
				smallestFraction = playerTrace.fraction;
			}
		}
	}
}

void Autowall::ScaleDamage(int hitgroup, C_BasePlayer* enemy, float weapon_armor_ratio, float& current_damage) {
	static auto mp_damage_scale_ct_head = g_CVar->FindVar("mp_damage_scale_ct_head");
	static auto mp_damage_scale_t_head = g_CVar->FindVar("mp_damage_scale_t_head");
	static auto mp_damage_scale_ct_body = g_CVar->FindVar("mp_damage_scale_ct_body");
	static auto mp_damage_scale_t_body = g_CVar->FindVar("mp_damage_scale_t_body");

	auto team = enemy->m_iTeamNum();
	auto head_scale = team == 2 ? mp_damage_scale_ct_head->GetFloat() : mp_damage_scale_t_head->GetFloat();
	auto body_scale = team == 2 ? mp_damage_scale_ct_body->GetFloat() : mp_damage_scale_t_body->GetFloat();

	auto armor_heavy = enemy->m_bHasHeavyArmor();
	auto armor_value = static_cast<float>(enemy->m_ArmorValue());

	if (armor_heavy)
		head_scale *= 0.5f;

	// ref: CCSPlayer::TraceAttack
	switch (hitgroup) {
	case HITGROUP_HEAD:
		current_damage = (current_damage * 4.f) * head_scale;
		break;
	case HITGROUP_CHEST:
	case 8:
		current_damage *= body_scale;
		break;
	case HITGROUP_STOMACH:
		current_damage = (current_damage * 1.25f) * body_scale;
		break;
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		current_damage *= body_scale;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage = (current_damage * 0.75f) * body_scale;
		break;
	default:
		break;
	}

	static auto IsArmored = [](C_BasePlayer* player, int hitgroup) {
		auto has_helmet = player->m_bHasHelmet();
		auto armor_value = static_cast<float>(player->m_ArmorValue());

		if (armor_value > 0.f) {
			switch (hitgroup) {
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
			case 8:
				return true;
				break;
			case HITGROUP_HEAD:
				return has_helmet || player->m_bHasHeavyArmor();
				break;
			default:
				return player->m_bHasHeavyArmor();
				break;
			}
		}

		return false;
	};

	if (IsArmored(enemy, hitgroup)) {
		auto armor_scale = 1.f;
		auto armor_ratio = (weapon_armor_ratio * 0.5f);
		auto armor_bonus_ratio = 0.5f;

		if (armor_heavy) {
			armor_ratio *= 0.2f;
			armor_bonus_ratio = 0.33f;
			armor_scale = 0.25f;
		}

		float new_damage = current_damage * armor_ratio;
		float estiminated_damage = (current_damage - (current_damage * armor_ratio)) * (armor_scale * armor_bonus_ratio);
		if (estiminated_damage > armor_value)
			new_damage = (current_damage - (armor_value / armor_bonus_ratio));

		current_damage = new_damage;
	}
}

void Autowall::ScaleDamage(CGameTrace& enterTrace, CCSWeaponInfo* weaponData, float& currentDamage)
{

	C_BasePlayer* pLocal = g_LocalPlayer;
	C_BaseCombatWeapon* weapon = pLocal->m_hActiveWeapon();
	bool hasHeavyArmor = false;
	int armorValue = ((C_BasePlayer*)enterTrace.hit_entity)->m_ArmorValue();
	int hitGroup = enterTrace.hitgroup;

	if (!pLocal || !weapon || !weapon->GetCSWeaponData()) {
		return;
	}

	if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER || weapon->IsGrenade() || weapon->IsKnife()) {
		return;
	}

	auto IsArmored = [&enterTrace]()->bool
	{
		C_BasePlayer* targetEntity = (C_BasePlayer*)enterTrace.hit_entity;
		switch (enterTrace.hitgroup)
		{
		case HITGROUP_HEAD:
			return !!targetEntity->m_bHasHelmet();
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (hitGroup)
	{
	case HITGROUP_HEAD:
		currentDamage *= 2.f;
		break;
	case HITGROUP_STOMACH:
		currentDamage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		currentDamage *= 0.75f;
		break;
	default:
		break;
	}

	if (armorValue > 0 && IsArmored())
	{
		float bonusValue = 1.f, armorBonusRatio = 0.5f, armorRatio = weaponData->flArmorRatio / 2.f;

		if (hasHeavyArmor)
		{
			armorBonusRatio = 0.33f;
			armorRatio *= 0.5f;
			bonusValue = 0.33f;
		}

		auto NewDamage = currentDamage * armorRatio;

		if (((currentDamage - (currentDamage * armorRatio)) * (bonusValue * armorBonusRatio)) > armorValue)
		{
			NewDamage = currentDamage - (armorValue / armorBonusRatio);
		}

		currentDamage = NewDamage;
	}
}

bool Autowall::TraceToExit(Vector& end, trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace) {
	float distance = 0.0f;
	while (distance <= 90.0f) {
		distance += 4.0f;

		end = start + dir * distance;
		auto point_contents = g_EngineTrace->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL);
		if (point_contents & MASK_SHOT_HULL && !(point_contents & CONTENTS_HITBOX))
			continue;

		auto new_end = end - (dir * 4.0f);
		Ray_t ray;
		ray.Init(end, new_end);
		g_EngineTrace->TraceRay(ray, MASK_SHOT, 0, exit_trace);
		if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX) {
			ray.Init(end, start);
			CTraceFilter filter;
			filter.pSkip = exit_trace->hit_entity;
			g_EngineTrace->TraceRay(ray, 0x600400B, &filter, exit_trace);
			if ((exit_trace->fraction < 1.0f || exit_trace->allsolid) && !exit_trace->startsolid) {
				end = exit_trace->endpos;
				return true;
			}
			continue;
		}

		if (!(exit_trace->fraction < 1.0 || exit_trace->allsolid || exit_trace->startsolid) || exit_trace->startsolid) {
			if (exit_trace->hit_entity) {
				if (enter_trace->hit_entity && enter_trace->hit_entity == g_EntityList->GetClientEntity(0))
					return true;
			}
			continue;
		}

		if (exit_trace->surface.flags >> 7 & 1 && !(enter_trace->surface.flags >> 7 & 1))
			continue;

		if (exit_trace->plane.normal.Dot(dir) <= 1.0f) {
			auto fraction = exit_trace->fraction * 4.0f;
			end = end - (dir * fraction);
			return true;
		}
	}
	return false;
}
void Autowall::TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	g_EngineTrace->TraceRay(ray, mask, &filter, ptr);
}
bool Autowall::BreakableEntity(IClientEntity* entity)
{

	ClientClass* pClass = (ClientClass*)entity->GetClientClass();

	if (!pClass)
	{
		return false;
	}

	if (pClass == nullptr)
	{
		return false;
	}

	return pClass->m_ClassID == ClassId_CBreakableProp || pClass->m_ClassID == ClassId_CBreakableSurface;

}
bool Autowall::TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction)
{
	Vector start, end;
	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
	int firstContents = 0;

	while (currentDistance <= maxDistance)
	{
		currentDistance += rayExtension;

		start = startPosition + direction * currentDistance;

		if (!firstContents)
			firstContents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		int pointContents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		if (!(pointContents & MASK_SHOT_HULL) || pointContents & CONTENTS_HITBOX && pointContents != firstContents)
		{
			end = start - (direction * rayExtension);

			TraceLine(start, end, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exitTrace);

			if (exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
			{
				TraceLine(start, startPosition, MASK_SHOT_HULL, exitTrace.hit_entity, &exitTrace);

				if (exitTrace.DidHit() && !exitTrace.startsolid)
				{
					start = exitTrace.endpos;
					return true;
				}
				continue;
			}

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{

				if (BreakableEntity(enterTrace.hit_entity) && BreakableEntity(exitTrace.hit_entity))
				{
					return true;
				}

				if (enterTrace.surface.flags & SURF_NODRAW || !(exitTrace.surface.flags & SURF_NODRAW) && (exitTrace.plane.normal.Dot(direction) <= 1.f))
				{
					float multAmount = exitTrace.fraction * 4.f;
					start -= direction * multAmount;
					return true;
				}

				continue;
			}

			if (!exitTrace.DidHit() || exitTrace.startsolid)
			{
				if (enterTrace.DidHitNonWorldEntity() && BreakableEntity(enterTrace.hit_entity))
				{
					//auto t = enterTrace;
					exitTrace = enterTrace;
					exitTrace.endpos = start + direction;
					return true;
				}

				continue;
			}
		}
	}
	return false;
}

bool Autowall::HandleBulletPenetration(CCSWeaponInfo* weaponInfo, FireBulletData& data) {
	surfacedata_t* enter_surface_data = g_PhysSurface->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;
	data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
	data.current_damage *= powf(weaponInfo->flRangeModifier, data.trace_length * 0.002f);

	if (data.trace_length > 3000.f || enter_surf_penetration_mod < 0.1f)
		data.penetrate_count = 0;

	if (data.penetrate_count <= 0)
		return false;

	Vector dummy;
	trace_t trace_exit;
	if (!TraceToExit(dummy, &data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit))
		return false;

	surfacedata_t* exit_surface_data = g_PhysSurface->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_mod = *(float*)((uint8_t*)exit_surface_data + 76);
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;
	if ((data.enter_trace.contents & CONTENTS_GRATE) != 0 || enter_material == 89 || enter_material == 71) {
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f;
	}
	else
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;

	if (enter_material == exit_material) {
		if (exit_material == 87 || exit_material == 85)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)
			combined_penetration_modifier = 2.0f;
	}

	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / weaponInfo->flPenetration) * 1.25f);
	float thickness = (trace_exit.endpos - data.enter_trace.endpos).Length();

	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;

	float lost_damage = fmaxf(0.0f, v35 + thickness);
	if (lost_damage > data.current_damage)
		return false;

	if (lost_damage >= 0.0f)
		data.current_damage -= lost_damage;

	if (data.current_damage < 1.0f)
		return false;

	data.src = trace_exit.endpos;
	data.penetrate_count--;
	return true;
}
bool Autowall::PenetrateWall(C_BasePlayer* pBaseEntity, C_BaseCombatWeapon* pWeapon, const Vector& vecPoint, float& flDamage)
{
	if (!pBaseEntity || !pWeapon)
		return false;
	FireBulletData BulletData;
	BulletData.filter.pSkip = pBaseEntity;
	QAngle qAngles;


	Vector vDelta = BulletData.src - vecPoint;

	float fHyp = (vDelta.x * vDelta.x) + (vDelta.y * vDelta.y);

	float fRoot;


	__asm
	{
		sqrtss xmm0, fHyp
		movss fRoot, xmm0
	}


	qAngles.pitch = RAD2DEG(atan(vDelta.z / fRoot));

	qAngles.yaw = RAD2DEG(atan(vDelta.y / vDelta.x));

	if (vDelta.x >= 0.0f)
		qAngles.yaw += 180.0f;




	Math::AngleVectors(qAngles, BulletData.direction);

	Math::Normalize3(BulletData.direction);

	if (!SimulateFireBullet(pWeapon, BulletData))
	{
		return false;
	}

	flDamage = BulletData.current_damage;

	return true;
}
void TraceLine(Vector vecAbsStart, Vector vecAbsEnd, unsigned int mask, C_BasePlayer* ignore, trace_t* ptr) {
	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;
	g_EngineTrace->TraceRay(ray, mask, &filter, ptr);
}

bool Autowall::SimulateFireBullet(C_BaseCombatWeapon* pWeapon, FireBulletData& data) {
	CCSWeaponInfo* weaponInfo = pWeapon->GetCSWeaponData();
	data.penetrate_count = 4;
	data.trace_length = 0.0f;
	data.current_damage = (float)weaponInfo->iDamage;
	while (data.penetrate_count > 0 && data.current_damage >= 1.0f) {
		data.trace_length_remaining = weaponInfo->flRange - data.trace_length;
		Vector end = data.src + data.direction * data.trace_length_remaining;
		TraceLine(data.src, end, MASK_SHOT, g_LocalPlayer, &data.enter_trace);

		CTraceFilter local_filter;
		local_filter.pSkip = g_LocalPlayer;
		ClipTraceToPlayers(data.src, end + data.direction * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &local_filter, &data.enter_trace);
		if (data.enter_trace.fraction == 1.0f)
			break;

		if (data.enter_trace.hitgroup <= HITGROUP_RIGHTLEG && data.enter_trace.hitgroup > HITGROUP_GENERIC) {
			data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
			data.current_damage *= powf(weaponInfo->flRangeModifier, data.trace_length * 0.002f);

			C_BasePlayer* player = (C_BasePlayer*)data.enter_trace.hit_entity;
			Autowall::ScaleDamage(data.enter_trace.hitgroup, player, weaponInfo->flArmorRatio, data.current_damage);
			return true;
		}

		if (!HandleBulletPenetration(weaponInfo, data))
			break;
	}
	return false;
}

float Autowall::GetDamage(const Vector& point, FireBulletData& data) {
	float damage = 0.f;
	Vector dst = point;
	data.src = g_LocalPlayer->GetEyePos();
	data.filter.pSkip = g_LocalPlayer;
	data.direction = dst - data.src;
	data.direction.NormalizeInPlace();
	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(g_LocalPlayer->m_hActiveWeapon());
	if (!activeWeapon)
		return -1.0f;
	if (SimulateFireBullet(activeWeapon, data))
		damage = data.current_damage;
	return damage;
}
float Autowall::GetDamage(const Vector& point) {
	float damage = 0.f;
	Vector dst = point;
	FireBulletData data;
	data.src = g_LocalPlayer->GetEyePos();
	data.filter.pSkip = g_LocalPlayer;
	data.direction = dst - data.src;
	data.direction.NormalizeInPlace();
	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(g_LocalPlayer->m_hActiveWeapon());
	if (!activeWeapon)
		return -1.0f;
	if (SimulateFireBullet(activeWeapon, data))
		damage = data.current_damage;
	return damage;
}
float Autowall::CanHit(Vector& point)
{
	C_BasePlayer* local = g_LocalPlayer;

	if (!local || !local->IsAlive())
	{
		return -1.f;
	}

	FireBulletData data(local->GetEyePos());// = FireBulletData(local->GetEyePosition());
	data.filter = CTraceFilter();
	data.filter.pSkip = local;
	Vector angles, direction;
	Vector tmp = point - local->GetEyePos();
	float currentDamage = 0;

	//VectorAngles(tmp, angles);
	//AngleVectors(angles, &direction);
	direction = tmp;
	direction.NormalizeInPlace();

	if (FireBullet(local->m_hActiveWeapon(), direction, currentDamage))
	{
		return currentDamage;
	}
	return -1.f;
}

float Autowall::CanHit(Vector& start, Vector& point)
{
	C_BasePlayer* local = g_LocalPlayer;

	if (!local || !local->IsAlive())
	{
		return -1.f;
	}

	FireBulletData data(start);
	data.filter = CTraceFilter();
	data.filter.pSkip = local;
	Vector angles, direction;
	Vector tmp = point - start;
	float currentDamage = 0;

	direction = tmp;
	direction.NormalizeInPlace();

	if (FireBullet(local->m_hActiveWeapon(), direction, currentDamage))
	{
		return currentDamage;
	}
	return -1.f;
}

float Autowall::CanHit(C_BasePlayer* ent, Vector& point)
{
	if (!ent || !ent->IsAlive())
	{
		return -1.f;
	}

	FireBulletData data(ent->GetEyePos());
	data.filter = CTraceFilter();
	data.filter.pSkip = ent;
	Vector angles, direction;
	Vector tmp = point - ent->GetEyePos();
	float currentDamage = 0;
	direction = tmp;
	direction.NormalizeInPlace();

	if (FireBullet(ent->m_hActiveWeapon(), direction, currentDamage))
	{
		return currentDamage;
	}
	return -1.f;
}

void Autowall::GetBulletTypeParameters(float& maxRange, float& maxDistance, char* bulletType, bool sv_penetration_type)
{
	if (sv_penetration_type)
	{
		maxRange = 35.0;
		maxDistance = 3000.0;
	}
	else
	{
		//Play tribune to framerate. Thanks, stringcompare
		//Regardless I doubt anyone will use the old penetration system anyway; so it won't matter much.
		if (!strcmp(bulletType, ("BULLET_PLAYER_338MAG")))
		{
			maxRange = 45.0;
			maxDistance = 8000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_762MM")))
		{
			maxRange = 39.0;
			maxDistance = 5000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_556MM")) || !strcmp(bulletType, ("BULLET_PLAYER_556MM_SMALL")) || !strcmp(bulletType, ("BULLET_PLAYER_556MM_BOX")))
		{
			maxRange = 35.0;
			maxDistance = 4000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_57MM")))
		{
			maxRange = 30.0;
			maxDistance = 2000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_50AE")))
		{
			maxRange = 30.0;
			maxDistance = 1000.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_357SIG")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_SMALL")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_P250")) || !strcmp(bulletType, ("BULLET_PLAYER_357SIG_MIN")))
		{
			maxRange = 25.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_9MM")))
		{
			maxRange = 21.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_45ACP")))
		{
			maxRange = 15.0;
			maxDistance = 500.0;
		}
		if (!strcmp(bulletType, ("BULLET_PLAYER_BUCKSHOT")))
		{
			maxRange = 0.0;
			maxDistance = 0.0;
		}
	}
}


bool Autowall::FireBullet(C_BaseCombatWeapon* pWeapon, Vector& direction, float& currentDamage)
{
	if (!pWeapon)
	{
		return false;
	}

	C_BasePlayer* local = g_LocalPlayer;

	bool sv_penetration_type;

	float currentDistance = 0.f, penetrationPower, penetrationDistance, maxRange, ff_damage_reduction_bullets, ff_damage_bullet_penetration, rayExtension = 40.f;
	Vector eyePosition = local->GetEyePos();

	static ConVar* penetrationSystem = g_CVar->FindVar(("sv_penetration_type"));
	static ConVar* damageReductionBullets = g_CVar->FindVar(("ff_damage_reduction_bullets"));
	static ConVar* damageBulletPenetration = g_CVar->FindVar(("ff_damage_bullet_penetration"));

	sv_penetration_type = penetrationSystem->GetBool();
	ff_damage_reduction_bullets = damageReductionBullets->GetFloat();
	ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();

	CCSWeaponInfo* weaponData = pWeapon->GetCSWeaponData();
	CGameTrace enterTrace;
	CTraceFilter filter;

	filter.pSkip = local;

	if (!weaponData)
	{
		return false;
	}

	maxRange = weaponData->flRange;

	GetBulletTypeParameters(penetrationPower, penetrationDistance, weaponData->szBulletType, sv_penetration_type);

	if (sv_penetration_type)
	{
		penetrationPower = weaponData->flPenetration;
	}

	int possibleHitsRemaining = 4;

	currentDamage = weaponData->iDamage;

	while (possibleHitsRemaining > 0 && currentDamage >= 1.f)
	{
		maxRange -= currentDistance;

		Vector end = eyePosition + direction * maxRange;

		TraceLine(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, local, &enterTrace);
		ClipTraceToPlayers(eyePosition, end + direction * rayExtension, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace); //  | CONTENTS_HITBOX

		surfacedata_t* enterSurfaceData = g_PhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps);

		float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;

		int enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.f)
		{
			break;
		}

		currentDistance += enterTrace.fraction * maxRange;

		currentDamage *= pow(weaponData->flRangeModifier, (currentDistance / 500.f));

		if (currentDistance > penetrationDistance && weaponData->flPenetration > 0.f || enterSurfPenetrationModifier < 0.1f)
		{
			break;
		}

		bool canDoDamage = (enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC);

		if (canDoDamage && static_cast<C_BasePlayer*>(enterTrace.hit_entity)->m_iTeamNum() != g_LocalPlayer->m_iTeamNum())
		{
			ScaleDamage(enterTrace, weaponData, currentDamage);
			return true;
		}

		if (!HandleBulletPenetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, sv_penetration_type, ff_damage_reduction_bullets, ff_damage_bullet_penetration))
		{
			break;
		}
	}
	return false;
}

bool Autowall::HandleBulletPenetration(CCSWeaponInfo* weaponData, CGameTrace& enterTrace, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration)
{
	//Because there's been issues regarding this- putting this here.
	if (&currentDamage == nullptr)
	{
		handle_penetration = false;
		return false;
	}

	C_BasePlayer* local = g_LocalPlayer;//(IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	FireBulletData data(local->GetEyePos());
	data.filter = CTraceFilter();
	data.filter.pSkip = local;
	CGameTrace exitTrace;
	C_BasePlayer* pEnemy = (C_BasePlayer*)enterTrace.hit_entity;
	surfacedata_t* enterSurfaceData = g_PhysSurface->GetSurfaceData(enterTrace.surface.surfaceProps);
	int enterMaterial = enterSurfaceData->game.material;

	float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
	float enterDamageModifier = enterSurfaceData->game.flDamageModifier;
	float thickness, modifier, lostDamage, finalDamageModifier, combinedPenetrationModifier;
	bool isSolidSurf = ((enterTrace.contents >> 3) & CONTENTS_SOLID);
	bool isLightSurf = ((enterTrace.surface.flags >> 7) & SURF_LIGHT);

	if (possibleHitsRemaining <= 0
		|| (enterTrace.surface.name == (const char*)0x2227c261 && exitTrace.surface.name == (const char*)0x2227c868)
		|| (!possibleHitsRemaining && !isLightSurf && !isSolidSurf && enterMaterial != CHAR_TEX_GRATE && enterMaterial != CHAR_TEX_GLASS)
		|| weaponData->flPenetration <= 0.f
		|| !TraceToExit(enterTrace, exitTrace, enterTrace.endpos, direction)
		&& !(g_EngineTrace->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL, nullptr) & MASK_SHOT_HULL))
	{
		handle_penetration = false;
		return false;
	}

	surfacedata_t* exitSurfaceData = g_PhysSurface->GetSurfaceData(exitTrace.surface.surfaceProps);
	int exitMaterial = exitSurfaceData->game.material;
	float exitSurfPenetrationModifier = exitSurfaceData->game.flPenetrationModifier;
	float exitDamageModifier = exitSurfaceData->game.flDamageModifier;

	if (sv_penetration_type)
	{
		if (enterMaterial == CHAR_TEX_GRATE || enterMaterial == CHAR_TEX_GLASS)
		{
			combinedPenetrationModifier = 3.f;
			finalDamageModifier = 0.05f;
		}
		else if (isSolidSurf || isLightSurf)
		{
			combinedPenetrationModifier = 1.f;
			finalDamageModifier = 0.16f;
		}
		else if (enterMaterial == CHAR_TEX_FLESH && (pEnemy->m_iTeamNum() == g_LocalPlayer->m_iTeamNum() && ff_damage_reduction_bullets == 0.f))
		{
			if (ff_damage_bullet_penetration == 0.f)
			{
				handle_penetration = false;
				return false;
			}
			combinedPenetrationModifier = ff_damage_bullet_penetration;
			finalDamageModifier = 0.16f;
		}
		else
		{
			combinedPenetrationModifier = (enterSurfPenetrationModifier + exitSurfPenetrationModifier) / 2.f;
			finalDamageModifier = 0.16f;
		}

		if (enterMaterial == exitMaterial)
		{
			if (exitMaterial == CHAR_TEX_CARDBOARD || exitMaterial == CHAR_TEX_WOOD)
			{
				combinedPenetrationModifier = 3.f;
			}
			else if (exitMaterial == CHAR_TEX_PLASTIC)
			{
				combinedPenetrationModifier = 2.f;
			}
		}

		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();
		modifier = fmaxf(1.f / combinedPenetrationModifier, 0.f);

		lostDamage = fmaxf(
			((modifier * thickness) / 24.f)
			+ ((currentDamage * finalDamageModifier)
				+ (fmaxf(3.75f / penetrationPower, 0.f) * 3.f * modifier)), 0.f);

		if (lostDamage > currentDamage)
		{
			handle_penetration = false;
			return false;
		}

		if (lostDamage > 0.f)
		{
			currentDamage -= lostDamage;
		}

		if (currentDamage < 1.f)
		{
			handle_penetration = false;
			return false;
		}

		eyePosition = exitTrace.endpos;
		--possibleHitsRemaining;

		handle_penetration = true;
		return true;
	}
	else
	{
		combinedPenetrationModifier = 1.f;

		if (isSolidSurf || isLightSurf)
		{
			finalDamageModifier = 0.99f;
		}
		else
		{
			finalDamageModifier = fminf(enterDamageModifier, exitDamageModifier);
			combinedPenetrationModifier = fminf(enterSurfPenetrationModifier, exitSurfPenetrationModifier);
		}

		if (enterMaterial == exitMaterial && (exitMaterial == CHAR_TEX_METAL || exitMaterial == CHAR_TEX_WOOD))
		{
			combinedPenetrationModifier += combinedPenetrationModifier;
		}

		thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();

		if (sqrt(thickness) <= combinedPenetrationModifier * penetrationPower)
		{
			currentDamage *= finalDamageModifier;
			eyePosition = exitTrace.endpos;
			--possibleHitsRemaining;
			handle_penetration = true;
			return true;
		}
		handle_penetration = false;
		return false;
	}
}
