#pragma once

#include "../sdk/csgostructs.hpp"
#include "../sdk/utils/singleton.hpp"

class Engine_Prediction : public Singleton<Engine_Prediction>
{
	float _curtime_backup;
	float _frametime_backup;
	CMoveData _movedata;
	CUserCmd* _prevcmd;
	int _fixedtick;

	int32_t* _prediction_seed;
	C_BasePlayer*** _prediction_player;
public:
	void Begin(CUserCmd* cmd) 
	{
		_curtime_backup = g_GlobalVars->curtime;
		_frametime_backup = g_GlobalVars->frametime;

		if (!_prevcmd || _prevcmd->hasbeenpredicted) 
			_fixedtick = g_LocalPlayer->m_nTickBase();
		else 
			_fixedtick++;

		if (!_prediction_seed || !_prediction_player) 
		{
			auto client = GetModuleHandle(TEXT("client.dll"));
			_prediction_seed = *(int32_t**)(Utils::PatternScan(client, "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 0x2);
			_prediction_player = (C_BasePlayer***)(Utils::PatternScan(client, "89 35 ? ? ? ? F3 0F 10 48 20") + 0x2);
		}

		if (_prediction_seed) 
			*_prediction_seed = MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;

		if (_prediction_player) 
			**_prediction_player = g_LocalPlayer;

		g_LocalPlayer->m_pCurrentCommand() = cmd;

		g_GlobalVars->curtime = _fixedtick * g_GlobalVars->interval_per_tick;
		g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

		bool _inpred_backup = *(bool*)((uintptr_t)g_Prediction + 0x8);

		memset(&_movedata, 0, sizeof(CMoveData));

		*(bool*)((uintptr_t)g_Prediction + 0x8) = true;

		g_MoveHelper->SetHost(g_LocalPlayer);
		g_GameMovement->StartTrackPredictionErrors(g_LocalPlayer);
		g_Prediction->SetupMove(g_LocalPlayer, cmd, g_MoveHelper, &_movedata);
		g_GameMovement->ProcessMovement(g_LocalPlayer, &_movedata);
		g_Prediction->FinishMove(g_LocalPlayer, cmd, &_movedata);
		g_GameMovement->FinishTrackPredictionErrors(g_LocalPlayer);

		*(bool*)((uintptr_t)g_Prediction + 0x8) = _inpred_backup;

		if (_prediction_player)
			**_prediction_player = nullptr;

		if (_prediction_seed)
			*_prediction_seed = -1;

		g_LocalPlayer->m_pCurrentCommand() = nullptr;
		g_MoveHelper->SetHost(nullptr);

		_prevcmd = cmd;
	}

	void End() 
	{
		g_GlobalVars->curtime = _curtime_backup;
		g_GlobalVars->frametime = _frametime_backup;
		g_GameMovement->Reset();
	}
};
