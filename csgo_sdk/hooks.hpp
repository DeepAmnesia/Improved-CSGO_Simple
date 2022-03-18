#pragma once

#include "sdk/csgostructs.hpp"
#include "sdk/utils/vfunc_hook.hpp"
#include "minhook/minhook.h"
#include <d3d9.h>
#define CREATE_HOOK( Address, Function, Original ) if ( MH_CreateHook( ( LPVOID )( Address ), ( LPVOID )( Function ), reinterpret_cast< void** >( &Original ) ) )   Utils::ConsolePrint(#Function); Utils::ConsolePrint("\n");

typedef void(__cdecl* CL_Move_t)(float_t, bool);

namespace index
{
	constexpr auto EmitSound1               = 5;
	constexpr auto EmitSound2               = 6;
    constexpr auto EndScene                 = 42;
    constexpr auto Reset                    = 16;
    constexpr auto PaintTraverse            = 41;
    constexpr auto CreateMove               = 22;
    constexpr auto PlaySound                = 82;
    constexpr auto FrameStageNotify         = 37;
    constexpr auto DrawModelExecute         = 21;
    constexpr auto DoPostScreenSpaceEffects = 44;
	constexpr auto SvCheatsGetBool          = 13;
	constexpr auto OverrideView             = 18;
	constexpr auto LockCursor               = 67;
}

namespace Hooks
{
    void Initialize();
    void Shutdown();

    inline vfunc_hook hlclient_hook;
	inline vfunc_hook direct3d_hook;
	inline vfunc_hook vguipanel_hook;
	inline vfunc_hook vguisurf_hook;
	inline vfunc_hook mdlrender_hook;
	inline vfunc_hook viewrender_hook;
	inline vfunc_hook sound_hook;
	inline vfunc_hook clientmode_hook;
	inline vfunc_hook sv_cheats;
	

	static void __cdecl hkCL_Move(float_t flFrametime, bool bIsFinalTick);
	static long __stdcall hkEndScene(IDirect3DDevice9* device);
	static long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
	static void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket);
	static void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active);
	static void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce);
	static void __fastcall hkEmitSound1(void* _this, int, IRecipientFilter & filter, int iEntIndex, int iChannel, const char * pSoundEntry, unsigned int nSoundEntryHash, const char * pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector * pOrigin, const Vector * pDirection, void * pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk);
	static void __fastcall hkDrawModelExecute(void* _this, int, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
	static void __fastcall hkFrameStageNotify(void* _this, int, ClientFrameStage_t stage);
	static void __fastcall hkOverrideView(void* _this, int, CViewSetup * vsView);
	static void __fastcall hkLockCursor(void* _this);
	static int  __fastcall hkDoPostScreenEffects(void* _this, int, int a1);
	static bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx);
}
