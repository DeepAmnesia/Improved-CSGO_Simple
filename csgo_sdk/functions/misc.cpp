#include "misc.hpp"

#include "../configurations.hpp"
#include "../sdk/utils/math.hpp"

void Misc::AutoStrafe(CUserCmd* cmd)
{
	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER)
		return;

	if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	static auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");
	auto side_speed = cl_sidespeed->GetFloat();

	if (g_Configurations.misc_autostrafe)
	{
		static auto old_yaw = 0.0f;

		auto get_velocity_degree = [](float velocity)
		{
			auto tmp = RAD2DEG(atan(30.0f / velocity));

			if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
				return 90.0f;

			else if (tmp < 0.0f)
				return 0.0f;
			else
				return tmp;
		};

		if (g_LocalPlayer->m_nMoveType() != MOVETYPE_WALK)
			return;

		auto velocity = g_LocalPlayer->m_vecVelocity();
		velocity.z = 0.0f;

		auto forwardmove = cmd->forwardmove;
		auto sidemove = cmd->sidemove;

		if (velocity.Length2D() < 5.0f && !forwardmove && !sidemove)
			return;

		static auto flip = false;
		flip = !flip;

		auto turn_direction_modifier = flip ? 1.0f : -1.0f;
		auto viewangles = cmd->viewangles;

		if (forwardmove || sidemove)
		{
			cmd->forwardmove = 0.0f;
			cmd->sidemove = 0.0f;

			auto turn_angle = atan2(-sidemove, forwardmove);
			viewangles.yaw += turn_angle * M_RADPI;
		}
		else if (forwardmove)
			cmd->forwardmove = 0.0f;

		auto strafe_angle = RAD2DEG(atan(15.0f / velocity.Length2D()));

		if (strafe_angle > 90.0f)
			strafe_angle = 90.0f;
		else if (strafe_angle < 0.0f)
			strafe_angle = 0.0f;

		auto temp = Vector(0.0f, viewangles.yaw - old_yaw, 0.0f);
		temp.y = Math::NormalizeYaw(temp.y);

		auto yaw_delta = temp.y;
		old_yaw = viewangles.yaw;

		auto abs_yaw_delta = fabs(yaw_delta);

		if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 30.0f)
		{
			QAngle velocity_angles;
			Math::VectorAngles(velocity, velocity_angles);

			temp = Vector(0.0f, viewangles.yaw - velocity_angles.yaw, 0.0f);
			temp.y = Math::NormalizeYaw(temp.y);

			auto velocityangle_yawdelta = temp.y;
			auto velocity_degree = get_velocity_degree(velocity.Length2D());

			if (velocityangle_yawdelta <= velocity_degree || velocity.Length2D() <= 15.0f)
			{
				if (-velocity_degree <= velocityangle_yawdelta || velocity.Length2D() <= 15.0f)
				{
					viewangles.yaw += strafe_angle * turn_direction_modifier;
					cmd->sidemove = side_speed * turn_direction_modifier;
				}
				else
				{
					viewangles.yaw = velocity_angles.yaw - velocity_degree;
					cmd->sidemove = side_speed;
				}
			}
			else
			{
				viewangles.yaw = velocity_angles.yaw + velocity_degree;
				cmd->sidemove = -side_speed;
			}
		}
		else if (yaw_delta > 0.0f)
			cmd->sidemove = -side_speed;
		else if (yaw_delta < 0.0f)
			cmd->sidemove = side_speed;

		auto move = Vector(cmd->forwardmove, cmd->sidemove, 0.0f);
		auto speed = move.Length();

		QAngle angles_move;
		Math::VectorAngles(move, angles_move);

		auto normalized_x = fmod(cmd->viewangles.pitch + 180.0f, 360.0f) - 180.0f;
		auto normalized_y = fmod(cmd->viewangles.yaw + 180.0f, 360.0f) - 180.0f;

		auto yaw = DEG2RAD(normalized_y - viewangles.yaw + angles_move.yaw);

		if (normalized_x >= 90.0f || normalized_x <= -90.0f || cmd->viewangles.pitch >= 90.0f && cmd->viewangles.pitch <= 200.0f ||
			cmd->viewangles.pitch <= -90.0f && cmd->viewangles.pitch <= 200.0f)
			cmd->forwardmove = -cos(yaw) * speed;
		else
			cmd->forwardmove = cos(yaw) * speed;

		cmd->sidemove = sin(yaw) * speed;
	}
	if (g_Configurations.misc_autostrafe_legit)
	{
		if (!g_InputSystem->IsButtonDown(ButtonCode_t::KEY_SPACE) ||
			g_InputSystem->IsButtonDown(ButtonCode_t::KEY_A) ||
			g_InputSystem->IsButtonDown(ButtonCode_t::KEY_D) ||
			g_InputSystem->IsButtonDown(ButtonCode_t::KEY_S) ||
			g_InputSystem->IsButtonDown(ButtonCode_t::KEY_W))
			return;

		bool on_ground = (g_LocalPlayer->m_fFlags() & FL_ONGROUND) && !(cmd->buttons & IN_JUMP);
		if (on_ground) {
			return;
		}

		static auto side = 1.0f;
		side = -side;

		auto velocity = g_LocalPlayer->m_vecVelocity();
		velocity.z = 0.0f;

		QAngle wish_angle = cmd->viewangles;

		auto speed = velocity.Length2D();
		auto ideal_strafe =Math::clamp(RAD2DEG(atan(15.f / speed)), 0.0f, 90.0f);

		if (cmd->forwardmove > 0.0f)
			cmd->forwardmove = 0.0f;

		static auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");

		static float old_yaw = 0.f;
		auto yaw_delta = std::remainderf(wish_angle.yaw - old_yaw, 360.0f);
		auto abs_angle_delta = abs(yaw_delta);
		old_yaw = wish_angle.yaw;

		if (abs_angle_delta <= ideal_strafe || abs_angle_delta >= 30.0f) {
			QAngle velocity_direction;
			Math::VectorAngles(velocity, velocity_direction);
			auto velocity_delta = std::remainderf(wish_angle.yaw - velocity_direction.yaw, 360.0f);
			auto retrack = Math::clamp(RAD2DEG(atan(30.0f / speed)), 0.0f, 90.0f) * g_Configurations.retrack;
			if (velocity_delta <= retrack || speed <= 15.0f) {
				if (-(retrack) <= velocity_delta || speed <= 15.0f) {
					wish_angle.yaw += side * ideal_strafe;
					cmd->sidemove = cl_sidespeed->GetFloat() * side;
				}
				else {
					wish_angle.yaw = velocity_direction.yaw - retrack;
					cmd->sidemove = cl_sidespeed->GetFloat();
				}
			}
			else {
				wish_angle.yaw = velocity_direction.yaw + retrack;
				cmd->sidemove = -cl_sidespeed->GetFloat();
			}

			Math::MovementFix(cmd, wish_angle, cmd->viewangles);
		}
		else if (yaw_delta > 0.0f) {
			cmd->sidemove = -cl_sidespeed->GetFloat();
		}
		else if (yaw_delta < 0.0f) {
			cmd->sidemove = cl_sidespeed->GetFloat();
		}
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
