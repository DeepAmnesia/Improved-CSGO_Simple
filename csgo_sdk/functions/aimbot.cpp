#include "aimbot.hpp"

#include "../sdk/utils/math.hpp"
#include "../sdk/utils/input.hpp"
#include "../render/menu.hpp"
#include "../config.hpp"
#include "autowall.hpp"
#include "weapon_groups.hpp"
#include <cmath>
#include <random>
#include "../sdk/utils/utils.hpp"

//--------------------------------------------------------------------------------
bool Aimbot::IsRcs() {
	return g_LocalPlayer->m_iShotsFired() >= settings.rcs_start + 1;
}
//--------------------------------------------------------------------------------
float GetRealDistanceFOV(float distance, QAngle angle, QAngle viewangles) {
	Vector aimingAt;
	Math::AngleVectors(viewangles, aimingAt);
	aimingAt *= distance;
	Vector aimAt;
	Math::AngleVectors(angle, aimAt);
	aimAt *= distance;
	return aimingAt.DistTo(aimAt) / 5;
}
//--------------------------------------------------------------------------------
float Aimbot::GetFovToPlayer(QAngle viewAngle, QAngle aimAngle) {
	QAngle delta = aimAngle - viewAngle;
	Math::FixAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}
//--------------------------------------------------------------------------------
bool Aimbot::IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos) {
	static auto LineGoesThroughSmokeFn = (bool(*)(Vector vStartPos, Vector vEndPos))Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	return LineGoesThroughSmokeFn(vStartPos, vEndPos);
}
//--------------------------------------------------------------------------------
bool Aimbot::IsEnabled(CUserCmd* pCmd) {

	if (!aimbot_enable || !g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		return false;
	}

	auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
	if (!pWeapon || !pWeapon->GetCSWeaponData() || !pWeapon->IsGun()) {
		return false;
	}

	if ((pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08) && settings.only_in_zoom && !g_LocalPlayer->m_bIsScoped()) {
		return false;
	}

	if (!pWeapon->HasBullets() || pWeapon->IsReloading() || !pWeapon->CanFire() || Menu::Get().IsVisible()) {
		return false;
	}

	if (aimbot_on_key && aimbot_key_pressed) return true;

	return pCmd->buttons & IN_ATTACK;
}
//--------------------------------------------------------------------------------
float Aimbot::GetSmooth() {
	float smooth = IsRcs() && settings.rcs_smooth_enabled ? settings.rcs_smooth : settings.smooth;
	return smooth;
}
//--------------------------------------------------------------------------------
void Aimbot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle) {
	auto smooth_value = GetSmooth();
	if (smooth_value <= 1) {
		return;
	}

	QAngle delta = aimAngle - currentAngle;
	Math::FixAngles(delta);

	if (settings.smooth_type == 1) {
		float deltaLength = fmaxf(sqrtf((delta.pitch * delta.pitch) + (delta.yaw * delta.yaw)), 0.01f);
		delta *= (1.0f / deltaLength);

		Math::RandomSeed(g_GlobalVars->tickcount);
		float randomize = Math::RandomFloat(-0.1f, 0.1f);
		smooth_value = fminf((g_GlobalVars->interval_per_tick * 64.0f) / (randomize + smooth_value * 0.15f), deltaLength);
	}
	else {
		smooth_value = (g_GlobalVars->interval_per_tick * 64.0f) / smooth_value;
	}

	delta *= smooth_value;
	angle = currentAngle + delta;
	Math::FixAngles(angle);
}
QAngle oldPunch;
//--------------------------------------------------------------------------------
void Aimbot::RCS(QAngle& angle, C_BasePlayer* target, bool should_run) {
	if (!settings.rcs) {
		RCSLastPunch.Init();
		return;
	}

	if (settings.rcs_x == 0 && settings.rcs_y == 0) {
		RCSLastPunch.Init();
		return;
	}

	QAngle punch = g_LocalPlayer->m_aimPunchAngle() * 2.0f;

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (weapon && weapon->m_flNextPrimaryAttack() > g_GlobalVars->curtime) {
		auto delta_angles = punch - RCSLastPunch;
		auto delta = weapon->m_flNextPrimaryAttack() - g_GlobalVars->curtime;
		if (delta >= g_GlobalVars->interval_per_tick)
			punch = RCSLastPunch + delta_angles / static_cast<float>(TIME_TO_TICKS(delta));
	}

	CurrentPunch = punch;
	if (settings.rcs_type == 0 && !should_run)
		punch -= { RCSLastPunch.pitch, RCSLastPunch.yaw, 0.f };

	RCSLastPunch = CurrentPunch;
	if (!IsRcs()) {
		return;
	}

	angle.pitch -= punch.pitch * (settings.rcs_x / 100.0f);
	angle.yaw -= punch.yaw * (settings.rcs_y / 100.0f);

	Math::FixAngles(angle);
}
//--------------------------------------------------------------------------------
float Aimbot::GetFov() {
	if (IsRcs() && settings.rcs && settings.rcs_fov_enabled) return settings.rcs_fov;
	if (!silent_enabled) return settings.fov;
	return settings.silent_fov > settings.fov ? settings.silent_fov : settings.fov;
}
//--------------------------------------------------------------------------------
C_BasePlayer* Aimbot::GetClosestPlayer(CUserCmd* cmd, int& bestBone) {
	QAngle ang;
	Vector eVecTarget;
	Vector pVecTarget = g_LocalPlayer->GetEyePos();

	if (target && !kill_delay && settings.kill_delay > 0 && target->IsNotTarget()) {
		target = NULL;
		shot_delay = false;
		kill_delay = true;
		kill_delay_time = (int)GetTickCount() + settings.kill_delay;
	}
	if (kill_delay) {
		if (kill_delay_time <= (int)GetTickCount()) kill_delay = false;
		else return NULL;
	}

	C_BasePlayer* player;
	target = NULL;
	aim_position = Vector{ 0, 0, 0 };
	int bestHealth = 100.f;
	float bestFov = 9999.f;
	float bestDamage = 0.f;
	float bestBoneFov = 9999.f;
	float bestDistance = 9999.f;
	int health;
	float fov;
	float damage;
	float distance;
	int fromBone = settings.aim_type == 1 ? 0 : settings.hitbox;
	int toBone = settings.aim_type == 1 ? 7 : settings.hitbox;
	QAngle backtrack_ang;
	Vector backtrack_bestpos;
	Vector backtrack_eVecTarget;
	Vector backtrack_pVecTarget = g_LocalPlayer->GetEyePos();
	float backtrack_fov = 0.f;
	float beastbacktrack_fov = 999999;

	for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i) {
		damage = 0.f;
		player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (player->IsNotTarget()) {
			continue;
		}
		if (!aimbot_deathmatch && player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) {
			continue;
		}
		for (int bone = fromBone; bone <= toBone; bone++) {
			eVecTarget = player->GetHitboxPos(bone);
			Math::VectorAngles(eVecTarget - pVecTarget, ang);
			Math::FixAngles(ang);
			distance = pVecTarget.DistTo(eVecTarget);
			if (settings.fov_type == 1)
				fov = GetRealDistanceFOV(distance, ang, cmd->viewangles + RCSLastPunch);
			else
				fov = GetFovToPlayer(cmd->viewangles + RCSLastPunch, ang);

			bool backtrack_enable = g_Backtrack->data.count(player->EntIndex()) > 0 && settings.enable_backtrack && settings.backtrack_ticks > 0;
			Vector local_bb_bone_pos = Vector{ 0,0,0 };

			if (backtrack_enable /*&& !settings.autowall*/ && fov > GetFov()) {
				auto& data = g_Backtrack->data.at(player->EntIndex());
				if (data.size() > 0) {
					Vector best_bb_bone_pos = Vector{ 0,0,0 };

					for (auto& record : data) {
						auto hitbox_head = record.hitboxset->GetHitbox(bone);
						auto hitbox_center = (hitbox_head->bbmin + hitbox_head->bbmax) * 0.5f;

						Math::VectorTransform(hitbox_center, record.boneMatrix[hitbox_head->bone], best_bb_bone_pos);

						backtrack_eVecTarget = best_bb_bone_pos;
						Math::VectorAngles(backtrack_eVecTarget - backtrack_pVecTarget, backtrack_ang);
						Math::FixAngles(backtrack_ang);

						backtrack_fov = GetFovToPlayer(cmd->viewangles + RCSLastPunch, backtrack_ang);

						if (backtrack_fov > (settings.silent_fov > settings.fov ? settings.silent_fov : settings.fov))
							continue;

						if (beastbacktrack_fov > backtrack_fov) {
							beastbacktrack_fov = backtrack_fov;
							local_bb_bone_pos = best_bb_bone_pos;
						}
					}

					if (g_LocalPlayer->CanSeePlayerD(player, local_bb_bone_pos) > 0.9f)
						backtrack_bestpos = local_bb_bone_pos;
				}
			}

			if (fov > GetFov() && (backtrack_bestpos == Vector{ 0,0,0 } || !backtrack_enable || !backtrack_bestpos.IsValid()))
				continue;

			if (!g_LocalPlayer->CanSeePlayer(player, eVecTarget)) {

				if (!settings.autowall)
					continue;

				damage = Autowall::Get().CanHit(eVecTarget);
				if (damage < settings.min_damage)
					continue;

			}
			if ((settings.priority == 1 || settings.priority == 2) && damage == 0.f)
				damage = Autowall::Get().CanHit(eVecTarget);

			health = player->m_iHealth() - damage;
			if (settings.smoke_check && IsLineGoesThroughSmoke(pVecTarget, eVecTarget))
				continue;

			bool OnGround = (g_LocalPlayer->m_fFlags() & FL_ONGROUND);
			if (settings.jump_check && !OnGround)
				continue;

			if (settings.aim_type == 1 && bestBoneFov < fov) {
				continue;
			}
			bestBoneFov = fov;

			if (backtrack_bestpos != Vector{ 0,0,0 } && backtrack_bestpos.IsValid()) {
				aim_position = backtrack_bestpos;
			}

			if (
				(settings.priority == 0 && bestFov > fov) ||
				(settings.priority == 1 && bestHealth > health) ||
				(settings.priority == 2 && bestDamage < damage) ||
				(settings.priority == 3 && distance < bestDistance)
				) {
				bestBone = bone;
				target = player;
				bestFov = fov;
				bestHealth = health;
				bestDamage = damage;
				bestDistance = distance;
			}


		}
	}
	return target;
}
//--------------------------------------------------------------------------------
bool Aimbot::IsNotSilent(float fov) {
	return IsRcs() || !silent_enabled || (silent_enabled && fov > settings.silent_fov);
}
Vector TickPrediction(Vector AimPoint, C_BasePlayer* pTarget)
{
	return AimPoint + (pTarget->m_vecVelocity() * g_GlobalVars->interval_per_tick);
}
void AutoStop(CUserCmd* cmd)
{
	cmd->forwardmove = 0;
	cmd->sidemove = 0;
	cmd->upmove = 0;
}
bool HitChance(QAngle angles, C_BasePlayer* ent, float chance)
{
	auto RandomFloat = [](float a, float b) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = b - a;
		float r = random * diff;
		return a + r;
	};
	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
		return false;

	Vector forward, right, up;
	Vector src = g_LocalPlayer->GetEyePos();
	Math::AngleVectors(angles, forward, right, up);

	int cHits = 0;
	int cNeededHits = static_cast<int> (150.f * (chance / 100.f));

	weapon->UpdateAccuracyPenalty();
	float weap_spread = weapon->GetSpread();
	float weap_inaccuracy = weapon->GetInaccuracy();

	for (int i = 0; i < 150; i++)
	{
		float a = RandomFloat(0.f, 1.f);
		float b = RandomFloat(0.f, 2.f * PI_F);
		float c = RandomFloat(0.f, 1.f);
		float d = RandomFloat(0.f, 2.f * PI_F);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		if (weapon->m_Item().m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.Normalized();

		QAngle viewAnglesSpread;
		Math::VectorAngles(direction, viewAnglesSpread);
		viewAnglesSpread.Normalize();

		Vector viewForward;
		Math::AngleVectors(viewAnglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = src + (viewForward * weapon->GetCSWeaponData()->flRange);

		trace_t tr;
		Ray_t ray;

		ray.Init(src, viewForward);
		g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, ent, &tr);

		if (tr.hit_entity == ent)
			++cHits;

		if (static_cast<int> ((static_cast<float> (cHits) / 150.f) * 100.f) >= chance)
			return true;

		if ((150 - i + cHits) < cNeededHits)
			return false;
	}

	return false;
}
//--------------------------------------------------------------------------------
bool adaptiveHS(float hs) {
	if (hs > 0 && g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() != WEAPON_REVOLVER) {
		float inaccuracy = g_LocalPlayer->m_hActiveWeapon()->GetInaccuracy();

		if (inaccuracy <= 0)
			inaccuracy = 0.0000001;

		inaccuracy = 1 / inaccuracy;

		return ((hs <= inaccuracy) ? true : false);
	}

	return true;
}
//--------------------------------------------------------------------------------
void Aimbot::OnMove(CUserCmd* pCmd, bool* bSendPacket) {
	if (!g_LocalPlayer || !g_EngineClient->IsInGame() || !g_LocalPlayer->IsAlive() || !g_EngineClient->IsConnected()) return;

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data)
		return;

	short index = weapon->m_Item().m_iItemDefinitionIndex();
	settings = *g_CustomWeaponGroups->GetSettings(index);
	if (!IsEnabled(pCmd)) {
		if (settings.rcs_type == 0) {
			auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
			if (pWeapon && pWeapon->GetCSWeaponData() && (pWeapon->IsSniper() || pWeapon->IsPistol() || pWeapon->IsRifle())) {
				RCS(pCmd->viewangles, target, false);
				Math::FixAngles(pCmd->viewangles);
				g_EngineClient->SetViewAngles(&pCmd->viewangles);
			}
		}
		else {
			RCSLastPunch = { 0, 0, 0 };
		}

		is_delayed = false;
		shot_delay = false;
		kill_delay = false;
		silent_enabled = (settings.silent || settings.psilent) && settings.silent_fov > 0;
		target = NULL;
		return;
	}
	QAngle oldAngle;
	g_EngineClient->GetViewAngles(&oldAngle);
	Math::RandomSeed(pCmd->command_number);

	bool should_do_rcs = false;
	QAngle angles = pCmd->viewangles;
	QAngle current = angles;
	float fov = 180.f;
	int bestBone = -1;
	if (!(settings.flash_check && g_LocalPlayer->IsFlashed())) {
		C_BasePlayer* player = GetClosestPlayer(pCmd, bestBone);
		if (player) {
			Vector eVecTarget;
			eVecTarget = target->GetHitboxPos(bestBone);
			if (aim_position != Vector{ 0,0,0 })
				eVecTarget = aim_position;
			angles = Math::CalcAngle(g_LocalPlayer->GetEyePos(), eVecTarget);

			if (settings.smooth > 2 && settings.humanize) {
				float dist = Math::NormalizeYaw(angles.yaw - oldAngle.yaw);
				if (dist > 180.0f) dist = 360.0f - dist;
				eVecTarget += Vector(0, 0, settings.curve * dist);

				angles = Math::CalcAngle(g_LocalPlayer->GetEyePos(), eVecTarget);
			}

			Math::FixAngles(angles);

			if (settings.fov_type == 1)
				fov = GetRealDistanceFOV(g_LocalPlayer->GetEyePos().DistTo(eVecTarget), angles, pCmd->viewangles);
			else
				fov = GetFovToPlayer(pCmd->viewangles, angles);

			should_do_rcs = true;

			//
			std::vector<int> hitgroups;
			if (!(g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER)) 
			{
				hitgroups.push_back(HITGROUP_HEAD);

				hitgroups.push_back(HITGROUP_CHEST);

				hitgroups.push_back(HITGROUP_STOMACH);
			}
			else {
				hitgroups.push_back(HITGROUP_CHEST);
				hitgroups.push_back(HITGROUP_STOMACH);
			}
			Vector rem, forward,
				src = g_LocalPlayer->GetEyePos();

			trace_t tr;
			Ray_t ray;
			CTraceFilter filter;
			filter.pSkip = g_LocalPlayer;

			QAngle viewangles = pCmd->viewangles;

			QAngle rec;

			Math::AngleVectors(viewangles, forward);

			forward *= g_LocalPlayer->m_hActiveWeapon().Get()->GetCSWeaponData()->flRange;

			rem = src + forward;

			ray.Init(src, rem);

			UINT mask = MASK_SHOT | CONTENTS_GRATE;

			mask &= ~(CONTENTS_WINDOW);

			g_EngineTrace->TraceRay(ray, mask, &filter, &tr);

			if (!tr.hit_entity)
			{
			}
			bool dh = false;
			for (auto& hitgroupsorted : hitgroups) {
				if (tr.hitgroup == hitgroupsorted)
					dh = true;
			}
			//

			if (dh) Utils::ConsolePrint("Hovered");

			if (!settings.auto_delay)
			{
				if (!settings.silent && !is_delayed && !shot_delay && settings.shot_delay > 0) {
					is_delayed = true;
					shot_delay = true;
					shot_delay_time = GetTickCount() + settings.shot_delay;
				}

				if (shot_delay && shot_delay_time <= GetTickCount()) {
					shot_delay = false;
				}

				if (shot_delay) {
					pCmd->buttons &= ~IN_ATTACK;
				}
			}
			else
			{
				if (dh)
				{

					pCmd->buttons |= IN_ATTACK;

				}
				else
				{
					pCmd->buttons &= ~IN_ATTACK;
				}
			}


			if (settings.autostop) {
				Vector Velocity = g_LocalPlayer->m_vecVelocity();

				if (Velocity.Length2D() != 0) {

					static float Speed = 450.f;

					QAngle Direction;
					QAngle RealView;
					Math::VectorAngles(Velocity, Direction);
					g_EngineClient->GetViewAngles(&RealView);
					Direction.yaw = RealView.yaw - Direction.yaw;

					Vector Forward;
					Math::AngleVectors(Direction, Forward);
					Vector NegativeDirection = Forward * -Speed;

					pCmd->forwardmove = pCmd->sidemove = 0;
				}
			}
		}
	}

	if (IsNotSilent(fov) && (should_do_rcs || settings.rcs_type == 0)) {
		RCS(angles, target, should_do_rcs);
	}
	if (target && IsNotSilent(fov)) {
		Smooth(current, angles, angles);
	}


	Math::FixAngles(angles);
	pCmd->viewangles = angles;

	if (IsNotSilent(fov)) {
		g_EngineClient->SetViewAngles(&angles);
		//FixMouseDeltas(pCmd, angles, oldAngle);
	}

	if (settings.psilent) {
		QAngle Oldview = pCmd->viewangles;
		QAngle qAimAngles = pCmd->viewangles;
		float Oldsidemove = pCmd->sidemove;
		float Oldforwardmove = pCmd->forwardmove;

		static int ChokedPackets = -1;

		if (ChokedPackets < 6) {
			*bSendPacket = false;
			pCmd->viewangles = angles;
			ChokedPackets++;
		}
		else
		{
			*bSendPacket = true;
			pCmd->viewangles = Oldview;
			pCmd->sidemove = Oldsidemove;
			pCmd->forwardmove = Oldforwardmove;
			ChokedPackets = -1;
		}

		pCmd->viewangles.roll = 0;
	}

	silent_enabled = false;
	if (settings.autopistol) {
		const auto activeWeapon = g_LocalPlayer->m_hActiveWeapon();
		if (activeWeapon && pCmd->buttons & IN_ATTACK && activeWeapon->CanFire()) {
			static bool fire = false;
			if (fire)
				pCmd->buttons &= ~IN_ATTACK;
			else
				pCmd->buttons |= IN_ATTACK;
			fire = !fire;
		}
	}
}


void Aimbot::MenuAimbot() {
	static bool editor_mode{ false };
	static std::string current_group{ "" };

	if (Menu::Get().AimbotFirstTimeRender) {
		if (g_LocalPlayer && g_EngineClient && g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) {
			auto weapon = g_LocalPlayer->m_hActiveWeapon();
			if (weapon) {
				current_group = g_CustomWeaponGroups->GetGroupName(weapon->m_Item().m_iItemDefinitionIndex());
			}
		}
	}
	Menu::Get().AimbotFirstTimeRender = false;

	if (editor_mode) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(27 / 255.f, 27 / 255.f, 27 / 255.f, 1.f));
		if (ImGui::Button("Back", ImVec2(-1, 19)) || InputSys::Get().IsKeyDown(VK_ESCAPE))
			editor_mode = !editor_mode;
		g_CustomWeaponGroups->Menu(current_group);
		return;
	}


	ImGui::Columns(4, nullptr, false);

	ImGui::Text("Selector:");
	ImGui::BeginChild("##1", ImVec2(-1, -1), true); {
		if (ImGui::Button("Editor Mode", ImVec2(-1, 19)))
			editor_mode = !editor_mode;
		g_CustomWeaponGroups->RenderSelector(current_group, 39.f);
	};
	ImGui::EndChild();
	ImGui::NextColumn();

	legitbot_settings* settings = g_CustomWeaponGroups->GetSettings(current_group);

	ImGui::Text("Aimbot:");
	ImGui::BeginChild("##2", ImVec2(-1, -1), true); {
		ImGui::Checkbox("Enable aimbot functions", &aimbot_enable);
		ImGui::Checkbox("Friendly fire", &aimbot_deathmatch);
		ImGui::Checkbox("On key", &aimbot_on_key);
		if (aimbot_on_key)
			ImGui::Keybind("##HotKey", &aimbot_key);

		ImGui::Checkbox("Backtrack", &settings->enable_backtrack);

		if (ImGui::Checkbox("Silent", &settings->silent) && settings->silent)
			settings->psilent = false;

		ImGui::Checkbox("Curve", &settings->humanize);

		ImGui::Checkbox("Smoke Check", &settings->smoke_check);
		ImGui::Checkbox("Flash Check", &settings->flash_check);
		ImGui::Checkbox("Jump Check", &settings->jump_check);
		ImGui::Checkbox("Auto Wall", &settings->autowall);
		ImGui::Checkbox("Auto Stop", &settings->autostop);
		//if (weapon_index_aimbot == WEAPON_P250 ||
		//	weapon_index_aimbot == WEAPON_USP_SILENCER ||
		//	weapon_index_aimbot == WEAPON_GLOCK ||
		//	weapon_index_aimbot == WEAPON_FIVESEVEN ||
		//	weapon_index_aimbot == WEAPON_TEC9 ||
		//	weapon_index_aimbot == WEAPON_DEAGLE ||
		//	weapon_index_aimbot == WEAPON_ELITE ||
		//	weapon_index_aimbot == WEAPON_HKP2000) {
		ImGui::Checkbox("Auto Pistol", &settings->autopistol);
		//}
		//if (weapon_index_aimbot == WEAPON_AWP || weapon_index_aimbot == WEAPON_SSG08 ||
		//	weapon_index_aimbot == WEAPON_AUG || weapon_index_aimbot == WEAPON_SG556) {
		ImGui::Checkbox("Only in zoom", &settings->only_in_zoom);
		//}
	};
	ImGui::EndChild();
	ImGui::NextColumn();

	ImGui::Text("RCS:");
	ImGui::BeginChild("##3", ImVec2(-1, -1), true); {
		ImGui::Checkbox("Enabled##aimbot.rcs", &settings->rcs);
		ImGui::Combo("Type##rcs", &settings->rcs_type, values::rcs_types, 2);
		ImGui::Checkbox("Fov##rcsfov", &settings->rcs_fov_enabled);

		if (settings->rcs_fov_enabled)
			if (ImGui::SliderFloat("FOV##rcs", &settings->rcs_fov, 0, 20))
				settings->rcs_fov = std::clamp(settings->rcs_fov, 0.f, 20.f);

		ImGui::Checkbox("Smooth##rcs_enabled", &settings->rcs_smooth_enabled);

		if (settings->rcs_smooth_enabled)
			if (ImGui::SliderFloat("Smooth##rcs", &settings->rcs_smooth, 1, 15))
				settings->rcs_smooth = std::clamp(settings->rcs_smooth, 1.f, 15.f);

		if (ImGui::SliderInt("X", &settings->rcs_x, 0, 100))
			settings->rcs_x = std::clamp(settings->rcs_x, 0, 100);

		if (ImGui::SliderInt("Y", &settings->rcs_y, 0, 100))
			settings->rcs_y = std::clamp(settings->rcs_y, 0, 100);

		if (ImGui::SliderInt("Start", &settings->rcs_start, 1, 30))
			settings->rcs_start = std::clamp(settings->rcs_start, 1, 30);
	};
	ImGui::EndChild();
	ImGui::NextColumn();

	ImGui::Text("Options:");
	ImGui::BeginChild("##4", ImVec2(-1, -1), true); {
		if (ImGui::SliderFloat("FOV", &settings->fov, 0, 30))
			settings->fov = std::clamp(settings->fov, 0.f, 30.f);

		if (ImGui::SliderFloat("Smooth", &settings->smooth, 1, 20))
			settings->smooth = std::clamp(settings->smooth, 1.f, 20.f);

		if (settings->silent || settings->psilent)
			if (ImGui::SliderFloat("Silent FOV", &settings->silent_fov, 0, 20))
				settings->silent_fov = std::clamp(settings->silent_fov, 0.f, 20.f);

		if (settings->enable_backtrack && ImGui::SliderInt("Backtrack##ticks", &settings->backtrack_ticks, 0, 12))
			settings->backtrack_ticks = std::clamp(settings->backtrack_ticks, 0, 12);

		ImGui::Combo("Priority", &settings->priority, values::priorities, 4);
		ImGui::Combo("Aiming type", &settings->fov_type, values::fov_types, 2);

		if (!settings->silent)
		{
			if (ImGui::SliderInt("Shot Delay", &settings->shot_delay, 0, 3000))
				settings->shot_delay = std::clamp(settings->shot_delay, 0, 3000);
			ImGui::Checkbox("Auto delay", &settings->auto_delay);
		}
		if (ImGui::SliderInt("Kill Delay", &settings->kill_delay, 0, 1000))
			settings->kill_delay = std::clamp(settings->kill_delay, 0, 1000);

		ImGui::Combo("Aim Type", &settings->aim_type, values::aim_types, 2);

		if (settings->aim_type == 0)
			ImGui::Combo("Hitbox", &settings->hitbox, values::hitbox_list, 7);

		ImGui::Combo("Smooth Type", &settings->smooth_type, values::smooth_types, 2);

		if (settings->humanize)
			if (ImGui::SliderFloat("Curves", &settings->curve, -10.f, 10.f))
				settings->curve = std::clamp(settings->curve, -10.f, 10.f);

		if (settings->autowall) {
			if (ImGui::SliderInt("Min Damage", &settings->min_damage, 1, 100))
				settings->min_damage = std::clamp(settings->min_damage, 1, 100);

			if (ImGui::SliderFloat("Hitchance", &settings->hitchance_amount, 1, 100))
				settings->hitchance_amount = std::clamp(settings->hitchance_amount, 1.f, 100.f);
		}
	};
	ImGui::EndChild();
}


void Aimbot::SetupValues() {
	g_Config->PushItem(&aimbot_enable, "aim", "enable", aimbot_enable);
	g_Config->PushItem(&aimbot_deathmatch, "aim", "deathmatch", aimbot_deathmatch);
	g_Config->PushItem(&aimbot_on_key, "aim", "on_key", aimbot_on_key);
	g_Config->PushItem(&aimbot_key, "aim", "key", aimbot_key);
}

Aimbot g_Aimbot;
