#pragma once
#include "../configurations.hpp"
#include "../sdk/csgostructs.hpp"
#include "backtrack.hpp"
#include "../helpers/keybinds.hpp"

class Aimbot {
private:
	int weapon_index_trigger = 7;
	int weapon_vector_index_trigger = 0;
private:
	bool aimbot_enable;
	bool aimbot_deathmatch;
	bool aimbot_on_key = false;
	bool aimbot_key_pressed = false;
	CKeyBind aimbot_key = CKeyBind(&aimbot_key_pressed, &aimbot_on_key, "Aimbot");
public:
	void OnMove(CUserCmd* pCmd, bool* bSendPacket);
	bool IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos);
private:
	bool IsEnabled(CUserCmd* pCmd);
	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle);
	bool IsRcs();
	float GetSmooth();
	float GetFov();
	void RCS(QAngle& angle, C_BasePlayer* target, bool should_run);
	void Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);
	bool IsNotSilent(float fov);
	C_BasePlayer* GetClosestPlayer(CUserCmd* cmd, int& bestBone);
	legitbot_settings settings;
	float shot_delay_time;
	float shot_delay_time_trigger;
	bool shot_delay = false;
	bool shot_delay_trigger = false;
	bool silent_enabled = false;
	QAngle CurrentPunch = { 0,0,0 };
	QAngle RCSLastPunch = { 0,0,0 };
	bool is_delayed = false;
	bool is_delayed_trigger = false;
	int kill_delay_time;
	bool kill_delay = false;
	C_BasePlayer* target = NULL;
	Vector aim_position = { 0, 0, 0 };
public:
	void MenuAimbot();
	void SetupValues();
};

extern Aimbot g_Aimbot;
