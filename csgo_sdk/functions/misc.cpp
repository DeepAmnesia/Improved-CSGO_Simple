#include "misc.hpp"

#include "../configurations.hpp"
#include "../sdk/utils/math.hpp"
void Misc::EdgeJump(CUserCmd* pCmd)
{
	if (g_Configurations.edge_jump)
	{
		if ((g_LocalPlayer->m_fFlags() & FL_ONGROUND) && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
			pCmd->buttons |= IN_JUMP;
	}
}

void Misc::MouseCorrection(CUserCmd* pCmd)
{
	QAngle angOldViewAngles;
	g_EngineClient->GetViewAngles(&angOldViewAngles);

	float delta_x = std::remainderf(pCmd->viewangles.pitch- angOldViewAngles.pitch, 360.0f);
	float delta_y = std::remainderf(pCmd->viewangles.pitch - angOldViewAngles.yaw - angOldViewAngles.yaw, 360.0f);

	if (delta_x != 0.0f)
	{
		float mouse_y = -((delta_x / g_CVar->FindVar("m_pitch")->GetFloat()) / g_CVar->FindVar("sensitivity")->GetFloat());
		short mousedy;
		if (mouse_y <= 32767.0f) {
			if (mouse_y >= -32768.0f) {
				if (mouse_y >= 1.0f || mouse_y < 0.0f) {
					if (mouse_y <= -1.0f || mouse_y > 0.0f)
						mousedy = static_cast<short>(mouse_y);
					else
						mousedy = -1;
				}
				else {
					mousedy = 1;
				}
			}
			else {
				mousedy = 0x8000u;
			}
		}
		else {
			mousedy = 0x7FFF;
		}

		pCmd->mousedy = mousedy;
	}

	if (delta_y != 0.0f)
	{
		float mouse_x = -((delta_y / g_CVar->FindVar("m_yaw")->GetFloat()) / g_CVar->FindVar("sensitivity")->GetFloat());
		short mousedx;
		if (mouse_x <= 32767.0f) {
			if (mouse_x >= -32768.0f) {
				if (mouse_x >= 1.0f || mouse_x < 0.0f) {
					if (mouse_x <= -1.0f || mouse_x > 0.0f)
						mousedx = static_cast<short>(mouse_x);
					else
						mousedx = -1;
				}
				else {
					mousedx = 1;
				}
			}
			else {
				mousedx = 0x8000u;
			}
		}
		else {
			mousedx = 0x7FFF;
		}

		pCmd->mousedx = mousedx;
	}
}
void Misc::FastStop(CUserCmd* pCmd)
{
	if (!g_Configurations.fast_stop)
		return;

	if (!g_LocalPlayer)
		return;

	if (!g_LocalPlayer->IsAlive())
		return;

	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	if (pCmd->buttons & (IN_JUMP | IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK))
		return;

	if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		return;

	if (g_LocalPlayer->m_vecVelocity().Length2D() <= g_LocalPlayer->m_flMaxspeed() * 0.34f)
	{
		pCmd->forwardmove = pCmd->sidemove = 0.0f;
		return;
	}

	QAngle angResistance = QAngle(0, 0, 0);
	Math::VectorAngles((g_LocalPlayer->m_vecVelocity() * -1.f), angResistance);

	angResistance.yaw = pCmd->viewangles.yaw - angResistance.yaw;
	angResistance.pitch = pCmd->viewangles.pitch - angResistance.pitch;

	Vector vecResistance = Vector(0, 0, 0);
	Math::AngleVectors(angResistance, vecResistance);

	pCmd->forwardmove =Math::clamp(vecResistance.x, -450.f, 450.0f);
	pCmd->sidemove = Math::clamp(vecResistance.y, -450.f, 450.0f);
}

void Misc::AutoStrafe(CUserCmd* pCmd)
{
	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	static auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");
	auto side_speed = cl_sidespeed->GetFloat();

	if (g_Configurations.misc_autostrafe)
	{
		if (g_LocalPlayer->m_vecVelocity().Length2D() <= 5.0f)
		{
			C_BaseCombatWeapon* pCombatWeapon = g_LocalPlayer->m_hActiveWeapon().Get();
			if (pCombatWeapon)
			{
				if (pCombatWeapon->m_iItemDefinitionIndex() != WEAPON_SSG08)
				{
					if (!g_Configurations.misc_boostspeed)
						return;
				}
				else
					return;
			}
		}

		if (!g_Configurations.misc_wasdstrafes)
		{
			pCmd->forwardmove = (10000.f / g_LocalPlayer->m_vecVelocity().Length2D() > 450.f) ? 450.f : 10000.f / g_LocalPlayer->m_vecVelocity().Length2D();
			pCmd->sidemove = (pCmd->mousedx != 0) ? (pCmd->mousedx < 0.0f) ? -450.f : 450.f : (pCmd->command_number & 1) ? -450.f : 450.f;

			return;
		}

		static bool bFlip = true;
		static float flOldYaw = pCmd->viewangles.yaw;

		Vector vecVelocity = g_LocalPlayer->m_vecVelocity();
		vecVelocity.z = 0.0f;

		float_t flForwardMove = pCmd->forwardmove;
		float_t flSideMove = pCmd->sidemove;

		float flTurnVelocityModifier = bFlip ? 1.5f : -1.5f;
		QAngle angViewAngles = pCmd->viewangles;

		if (flForwardMove || flSideMove)
		{
			pCmd->forwardmove = 0.0f;
			pCmd->sidemove = 0.0f;

			float m_flTurnAngle = atan2(-flSideMove, flForwardMove);
			angViewAngles.yaw += m_flTurnAngle * M_RADPI;
		}
		else if (flForwardMove)
			pCmd->forwardmove = 0.0f;

		float flStrafeAngle = RAD2DEG(atan(15.0f / vecVelocity.Length2D()));
		if (flStrafeAngle > 90.0f)
			flStrafeAngle = 90.0f;
		else if (flStrafeAngle < 0.0f)
			flStrafeAngle = 0.0f;

		Vector vecTemp = Vector(0.0f, angViewAngles.yaw - flOldYaw, 0.0f);
		vecTemp.y = Math::NormalizeAngle(vecTemp.y);
		flOldYaw = angViewAngles.yaw;

		float flYawDelta = vecTemp.y;
		float flAbsYawDelta = fabs(flYawDelta);
		if (flAbsYawDelta <= flStrafeAngle || flAbsYawDelta >= 30.0f)
		{
			QAngle angVelocityAngle;
			Math::VectorAngles(vecVelocity, angVelocityAngle);

			vecTemp = Vector(0.0f, angViewAngles.yaw - angVelocityAngle.yaw, 0.0f);
			vecTemp.y = Math::NormalizeAngle(vecTemp.y);

			float flVelocityAngleYawDelta = vecTemp.y;
			float flVelocityDegree = Math::clamp(RAD2DEG(atan(30.0f / vecVelocity.Length2D())), 0.0f, 90.0f) * 0.01f;

			if (flVelocityAngleYawDelta <= flVelocityDegree || vecVelocity.Length2D() <= 15.0f)
			{
				if (-flVelocityDegree <= flVelocityAngleYawDelta || vecVelocity.Length2D() <= 15.0f)
				{
					angViewAngles.yaw += flStrafeAngle * flTurnVelocityModifier;
					pCmd->sidemove = 450.0f * flTurnVelocityModifier;
				}
				else
				{
					angViewAngles.yaw = angVelocityAngle.yaw - flVelocityDegree;
					pCmd->sidemove = 450.0f;
				}
			}
			else
			{
				angViewAngles.yaw = angVelocityAngle.yaw + flVelocityDegree;
				pCmd->sidemove = -450.0f;
			}
		}
		else if (flYawDelta > 0.0f)
			pCmd->sidemove = 450.0f;
		else if (flYawDelta < 0.0f)
			pCmd->sidemove = 450.0f;

		Vector vecMove = Vector(pCmd->forwardmove, pCmd->sidemove, 0.0f);
		float flSpeed = vecMove.Length();

		QAngle angMoveAngle;
		Math::VectorAngles(vecMove, angMoveAngle);

		float flNormalizedX = fmod(pCmd->viewangles.pitch + 180.0f, 360.0f) - 180.0f;
		float flNormalizedY = fmod(pCmd->viewangles.yaw + 180.0f, 360.0f) - 180.0f;
		float flYaw = DEG2RAD((flNormalizedY - angViewAngles.yaw) + angMoveAngle.yaw);

		if (pCmd->viewangles.pitch <= 200.0f && (flNormalizedX >= 90.0f || flNormalizedX <= -90.0f || (pCmd->viewangles.pitch >= 90.0f && pCmd->viewangles.pitch <= 200.0f) || pCmd->viewangles.pitch <= -90.0f))
			pCmd->forwardmove = -cos(flYaw) * flSpeed;
		else
			pCmd->forwardmove = cos(flYaw) * flSpeed;

		pCmd->sidemove = sin(flYaw) * flSpeed;

		bFlip = !bFlip;
	}
}

void Misc::QuickReload(CUserCmd* cmd)
{
	if (!g_Configurations.quick_reload)
		return;
	static C_BaseCombatWeapon* ReloadedWeapon{ nullptr };
	auto Weapons = g_LocalPlayer->m_hMyWeapons();

	if (ReloadedWeapon)
	{
		for (auto i = 0; Weapons[i].IsValid(); i++)
		{
			if (Weapons[i] == -1)
				break;

			if ((C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(Weapons[i]) == ReloadedWeapon)
			{
				cmd->weaponselect = ReloadedWeapon->EntIndex();
				cmd->weaponsubtype = ReloadedWeapon->SubWeaponType();
				break;
			}
		}
		ReloadedWeapon = nullptr;
	}

	if (auto ActiveWeapon{ g_LocalPlayer->m_hActiveWeapon() }; ActiveWeapon && ActiveWeapon->IsReloading() && ActiveWeapon->m_iClip1() == ActiveWeapon->GetCSWeaponData()->iMaxClip1)
	{
		ReloadedWeapon = ActiveWeapon;

		for (auto i = 0; Weapons[i].IsValid(); i++)
		{
			if (Weapons[i] == -1)
				break;

			if (auto Weapon{ (C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(Weapons[i]) }; Weapon && Weapon != ReloadedWeapon)
			{
				if (!Weapon)
					continue;

				cmd->weaponselect = Weapon->EntIndex();
				cmd->weaponsubtype = Weapon->SubWeaponType();
				break;
			}
		}
	}
}
