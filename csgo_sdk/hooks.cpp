#include "hooks.hpp"
#include <intrin.h>  

#include "render/render.hpp"
#include "render/menu.hpp"
#include "configurations.hpp"
#include "sdk/utils/input.hpp"
#include "sdk/utils/utils.hpp"
#include "functions/bhop.hpp"
#include "functions/chams.hpp"
#include "functions/visuals.hpp"
#include "functions/glow.hpp"
#include "functions/misc.hpp"
#include "functions/prediction.hpp"
#include "functions/grenade_pred.hpp"
#include "functions/events.hpp"
#pragma intrinsic(_ReturnAddress)

C_HookedEvents HookedEvents;

void anti_cheat_fix()
{
	const char* modules[] ={ "client.dll", "engine.dll", "server.dll", "studiorender.dll", "materialsystem.dll", "shaderapidx9.dll", "vstdlib.dll", "vguimatsurface.dll" };
	long long sub_21445 = 0x69690004C201B0;
	for (auto test : modules)
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)Utils::PatternScan(GetModuleHandleA(test), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"), &sub_21445, 7, 0);
}

namespace Hooks 
{
	void Initialize()
	{
		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		sound_hook.setup(g_EngineSound);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode);
		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);

		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hkEmitSound1);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);

		anti_cheat_fix();
	}

	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();

		Glow::Get().Shutdown();
	}
	
	void __cdecl hkCL_Move(float_t flFrametime, bool bIsFinalTick)
	{
		
	}

	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);

		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
		static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
		static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
		static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");


		viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_fov->SetValue(g_Configurations.viewmodel_fov);
		mat_ambient_light_r->SetValue(g_Configurations.mat_ambient_light_r);
		mat_ambient_light_g->SetValue(g_Configurations.mat_ambient_light_g);
		mat_ambient_light_b->SetValue(g_Configurations.mat_ambient_light_b);
		


		static ConVar* zoom_sensitivity_ratio_mouse = g_CVar->FindVar("zoom_sensitivity_ratio_mouse");
		if (g_Configurations.remove_zoom)
			zoom_sensitivity_ratio_mouse->SetValue(0);
		else
			zoom_sensitivity_ratio_mouse->SetValue(1);

		static auto aspect_ratio = g_CVar->FindVar("r_aspectratio");
		if (g_EngineClient->IsInGame() && g_LocalPlayer)
		{
			if (g_Configurations.aspect_ratio)
			{
				aspect_ratio->SetValue(g_Configurations.aspect_ratio_scale);
			}
			else
			{
				aspect_ratio->SetValue(0);
			}
		}

		if (g_EngineClient->IsInGame())
		{
			static ConVar* weapon_debug_spread_show = g_CVar->FindVar("weapon_debug_spread_show");
			if (g_LocalPlayer->IsAlive())
				weapon_debug_spread_show->SetValue(g_Configurations.no_scope_crosshair && !g_LocalPlayer->m_bIsScoped() ? 3 : 0);
		}

		*reinterpret_cast<int*>((DWORD)&g_CVar->FindVar("viewmodel_offset_x")->m_fnChangeCallbacks + 0xC) = 0;
		*reinterpret_cast<int*>((DWORD)&g_CVar->FindVar("viewmodel_offset_y")->m_fnChangeCallbacks + 0xC) = 0;
		*reinterpret_cast<int*>((DWORD)&g_CVar->FindVar("viewmodel_offset_z")->m_fnChangeCallbacks + 0xC) = 0;

		static auto viewmodel_offset_x = g_CVar->FindVar("viewmodel_offset_x");
		static auto viewmodel_offset_y = g_CVar->FindVar("viewmodel_offset_y");
		static auto viewmodel_offset_z = g_CVar->FindVar("viewmodel_offset_z");

		viewmodel_offset_x->SetValue(g_Configurations.viewmodel_offset_x);
		viewmodel_offset_y->SetValue(g_Configurations.viewmodel_offset_y);
		viewmodel_offset_z->SetValue(g_Configurations.viewmodel_offset_z);

		DWORD colorwrite, srgbwrite;
		IDirect3DVertexDeclaration9* vert_dec = nullptr;
		IDirect3DVertexShader9* vert_shader = nullptr;
		DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vert_dec);
		pDevice->GetVertexShader(&vert_shader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);
	
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		auto esp_drawlist = Render::Get().RenderScene();

		Menu::Get().Render();
	
		ImGui::Render(esp_drawlist);

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vert_dec);
		pDevice->SetVertexShader(vert_shader);

		return oEndScene(pDevice);
	}

	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>(index::Reset);

		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
			Menu::Get().OnDeviceReset();

		return hr;
	}

	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		static auto oCreateMove = hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>(index::CreateMove);

		oCreateMove(g_CHLClient, 0, sequence_number, input_sample_frametime, active);

		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);

		if (!cmd || !cmd->command_number)
			return;
		
		if (Menu::Get().IsVisible())
			cmd->buttons &= ~IN_ATTACK;

		if (g_Configurations.misc_bhop)
			BunnyHop::OnCreateMove(cmd);

		if (g_Configurations.misc_showranks && cmd->buttons & IN_SCORE)
			g_CHLClient->DispatchUserMessage(CS_UM_ServerRankRevealAll, 0, 0, nullptr);

		Misc::Get().QuickReload(cmd);
		Engine_Prediction::Get().Begin(cmd);
		{
				Misc::Get().AutoStrafe(cmd);

			if (g_Configurations.esp_grenade_prediction)
				Grenade_Pred::Get().Trace(cmd);


		}
		Engine_Prediction::Get().End();

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
	}

	__declspec(naked) void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx; not sure if we need this
			push esp
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}

	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);
		if (!strcmp("HudZoom", g_VGuiPanel->GetName(panel)) && g_Configurations.remove_zoom)
			return;

		static auto blur = g_CVar->FindVar("@panorama_disable_blur");
		blur->SetValue(g_Configurations.remove_panorama_blur);
		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel) 
		{
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			Render::Get().BeginScene();
		}
	}

	void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);


		if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

	}

	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1)
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);

		if (g_LocalPlayer && g_Configurations.glow_enabled)
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, edx, a1);
	}

	QAngle aim_punch;
	QAngle view_punch;

	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);
		if (g_EngineClient->IsInGame() && g_LocalPlayer)
		{
			if (stage == FRAME_RENDER_START)
			{

				if (g_Configurations.remove_visualrecoil)
				{
					aim_punch = g_LocalPlayer->m_aimPunchAngle();
					view_punch = g_LocalPlayer->m_viewPunchAngle();

					g_LocalPlayer->m_aimPunchAngle() = QAngle{};
					g_LocalPlayer->m_viewPunchAngle() = QAngle{};
				}

				static ConVar* PostProcVar = g_CVar->FindVar("mat_postprocess_enable");
				PostProcVar->SetValue(!g_Configurations.remove_post_processing);

			}
			else if (stage == FRAME_RENDER_END)
			{
				if (!aim_punch.IsZero() || !view_punch.IsZero())
				{
					g_LocalPlayer->m_aimPunchAngle() = aim_punch;
					g_LocalPlayer->m_viewPunchAngle() = view_punch;

					aim_punch = QAngle{};
					view_punch = QAngle{};
				}
			}
		}
		ofunc(g_CHLClient, edx, stage);
	}

	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);

		if (g_EngineClient->IsInGame() && vsView)
		{
			if (!g_LocalPlayer->m_bIsScoped())
				vsView->fov = g_Configurations.camera_fov;
			else if (g_LocalPlayer->m_bIsScoped() && g_Configurations.force_fov_in_zoom && g_LocalPlayer->m_hActiveWeapon() &&
				g_LocalPlayer->m_hActiveWeapon()->m_iTeamNum() != WEAPON_AUG)
				vsView->fov = g_Configurations.camera_fov;

			auto viewmodel = (C_BaseEntity*)g_EntityList->GetClientEntityFromHandle(g_LocalPlayer->m_hViewModel());

			if (viewmodel)
			{
				auto eyeAng = vsView->angles;
				eyeAng.z -= (float)g_Configurations.viewmodel_offset_roll;

				viewmodel->SetAbsAngles(eyeAng);

			}
			Visuals::Get().ThirdPerson();
		}
		ofunc(g_ClientMode, edx, vsView);
	}

	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible()) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);

	}

	void __fastcall hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

		if (g_MdlRender->IsForcedMaterialOverride() &&
			!strstr(pInfo.pModel->szName, "arms") &&
			!strstr(pInfo.pModel->szName, "weapons/v_")) {
			return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
		}

		Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

		ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}

	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 86");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall *)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}
}
