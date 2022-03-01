#include "grenade_pred.hpp"

#include "../sdk/utils/math.hpp"
#include "../configurations.hpp"

#include "../render/render.hpp"

std::array<NadePoint, 500> _points{};
bool _predicted = false;

void Grenade_Pred::DrawPrediction()
{
	Vector start, end;

	if (_predicted) 
	{
		for (auto& p : _points) 
		{
			if (!p.m_valid) break;

			bool valid = Math::WorldToScreen(p.m_start, start);
			bool valid2 = Math::WorldToScreen(p.m_end, end);

			if (valid && valid2) 
			{
				Render::Get().RenderLine(start.x, start.y, end.x, end.y, Color(g_Configurations.color_grenade_prediction), 1.f);

				if (p.m_detonate /* || p.m_plane*/)
				{
					Render::Get().RenderCircleFilled(start.x, start.y, 5.5f, 24, p.m_detonate ? Color(255, 0, 0) : Color::White);
					Render::Get().RenderCircle(start.x, start.y, 5.f, 24, Color::Black);
				}
			}
		}
	}
}

void Grenade_Pred::Predict(CUserCmd* ucmd) 
{
	constexpr float restitution = 0.45f;
	constexpr float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
	constexpr float velocity = 403.0f * 0.9f;

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon) 
		return;

	float step, gravity, new_velocity, unk01;
	int index{}, grenade_act{ 1 };
	Vector pos, thrown_direction, start, eye_origin;
	QAngle angles, thrown;

	static auto sv_gravity = g_CVar->FindVar("sv_gravity");

	gravity = sv_gravity->GetFloat() / 8.0f;
	step = g_GlobalVars->interval_per_tick;

	eye_origin = g_LocalPlayer->GetEyePos();
	angles = ucmd->viewangles;

	thrown = angles;

	if (thrown.pitch < 0) 
		thrown.pitch = -10 + thrown.pitch * ((90 - 10) / 90.0f);
	else 
		thrown.pitch = -10 + thrown.pitch * ((90 + 10) / 90.0f);

	auto primary_attack = ucmd->buttons & IN_ATTACK;
	auto secondary_attack = ucmd->buttons & IN_ATTACK2;

	if (primary_attack && secondary_attack) 
		grenade_act = ACT_LOB;
	else if (secondary_attack) 
		grenade_act = ACT_DROP;

	unk01 = power[grenade_act];

	unk01 = unk01 * 0.7f;
	unk01 = unk01 + 0.3f;

	new_velocity = velocity * unk01;

	Math::AngleVectors(thrown, thrown_direction);

	start = eye_origin + thrown_direction * 16.0f;
	thrown_direction = (thrown_direction * new_velocity) + g_LocalPlayer->m_vecVelocity();

	for (auto time = 0.0f; index < 500; time += step) 
	{
		pos = start + thrown_direction * step;

		trace_t trace;
		CTraceFilterSkipEntity filter(g_LocalPlayer);

		Ray_t ray;
		ray.Init(start, pos);
		g_EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

		if (trace.fraction != 1.0f) 
		{
			thrown_direction = trace.plane.normal * -2.0f * thrown_direction.Dot(trace.plane.normal) + thrown_direction;
			thrown_direction *= restitution;
			pos = start + thrown_direction * trace.fraction * step;
			time += (step * (1.0f - trace.fraction));
		}

		bool detonated = false;

		const auto weapon_index = weapon->m_Item().m_iItemDefinitionIndex();

		switch (weapon_index) 
		{
		case WEAPON_SMOKEGRENADE:
		case WEAPON_DECOY:
			if (thrown_direction.Length() <= 0.1f) 
			{
				int det_tick_mod = static_cast<int>(0.2f / step);
				detonated = !(index % det_tick_mod);
			}

		case WEAPON_MOLOTOV:
		case WEAPON_INCGRENADE:
			if (trace.fraction != 1.0f && trace.plane.normal.z > 0.7f)
				detonated = true;
		case WEAPON_FLASHBANG:
		case WEAPON_HEGRENADE: 
		{
			detonated = static_cast<float>(index) * step > 1.5f && !(index % static_cast<int>(0.2f / step));
		}
		default:
			detonated = false;
		}

		_points.at(index++) = NadePoint(start, pos, trace.fraction != 1.0f, true, trace.plane.normal, detonated);
		start = pos;

		thrown_direction.z -= gravity * trace.fraction * step;

		if (detonated) 
			break;
	}

	for (auto n = index; n < 500; ++n) 
		_points.at(n).m_valid = false;

	_predicted = true;
}

bool Grenade_Pred::Detonated(C_BaseCombatWeapon* weapon, float time, trace_t& trace) 
{
	if (!weapon) 
		return true;

	const auto index = weapon->m_Item().m_iItemDefinitionIndex();

	switch (index) 
	{
	case 43:
	case 44:
		if (time > 2.5f) 
			return true;
		break;

	case WEAPON_MOLOTOV:
	case 48:
		if (trace.fraction != 1.0f && trace.plane.normal.z > 0.7f || time > 3.5f) 
			return true;
		break;
	case WEAPON_DECOY:
	case 45:
		if (time > 2.5f) 
			return true;
		break;
	}

	return false;
}

void Grenade_Pred::Trace(CUserCmd* ucmd) 
{
//	if (!(ucmd->buttons & IN_ATTACK) && !(ucmd->buttons & IN_ATTACK2)) 
//	{
//		_predicted = false;
//		return;
//	}

	const static std::vector< int > nades
	{
		(int)WEAPON_FLASHBANG,
		(int)WEAPON_SMOKEGRENADE,
		(int)WEAPON_HEGRENADE,
		(int)WEAPON_MOLOTOV,
		(int)WEAPON_DECOY,
		(int)WEAPON_INCGRENADE,
	};

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon) 
		return;

	if (std::find(nades.begin(), nades.end(), weapon->m_Item().m_iItemDefinitionIndex()) != nades.end()) 
		return Predict(ucmd);

	_predicted = false;
}