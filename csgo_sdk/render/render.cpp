#include "render.hpp"

#include <mutex>

#include "../functions/visuals.hpp"

#include "../sdk/csgostructs.hpp"
#include "../sdk/utils/input.hpp"
#include "../sdk/utils/math.hpp"

#include "menu.hpp"

#include "../configurations.hpp"
#include "../fonts/fonts.hpp"

ImFont* g_pDefaultFont;
ImFont* g_pSecondFont;

ImDrawListSharedData _data;

std::mutex render_mutex;

struct CUSTOMVERTEX {
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
};

void Render::Initialize()
{
	ImGui::CreateContext();

	ImGui_ImplWin32_Init(InputSys::Get().GetMainWindow());
	ImGui_ImplDX9_Init(g_D3DDevice9);

	draw_list = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_act = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_rendering = new ImDrawList(ImGui::GetDrawListSharedData());

	GetFonts();
}

void Render::GetFonts() 
{
	// menu font
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Fonts::Droid_compressed_data, Fonts::Droid_compressed_size, 14.f);
	
	// esp font
	g_pDefaultFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Fonts::Droid_compressed_data, Fonts::Droid_compressed_size, 18.f);
	
	// font for watermark; just example
	g_pSecondFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF( Fonts::Cousine_compressed_data, Fonts::Cousine_compressed_size, 18.f); 
}

void Render::ClearDrawList() 
{
	render_mutex.lock();
	draw_list_act->Clear();
	render_mutex.unlock();
}

void Render::BeginScene() 
{
	draw_list->Clear();
	draw_list->PushClipRectFullScreen();

	if (g_Configurations.misc_watermark)
		Render::Get().RenderText("CSGOSimple", 10, 5, 18.f, g_Configurations.color_watermark, false, true, g_pSecondFont);

	if (g_EngineClient->IsInGame() && g_LocalPlayer && g_Configurations.esp_enabled)
		Visuals::Get().AddToDrawList();
	int w, h;
	g_EngineClient->GetScreenSize(w, h);

	//	draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(w, h), ImGui::GetColorU32(ImVec4(0, 0, 0, alpha)));

	float x = w * 0.5f;
	float y = h * 0.5f;
	if (g_EngineClient->IsInGame() && g_LocalPlayer)
	{
		//	if (g_Options.misc_desync)
			//	Arrows(w, h);

		if (g_Configurations.draw_no_scope_lines && g_LocalPlayer->m_bIsScoped() && g_LocalPlayer->m_hActiveWeapon() && g_Configurations.remove_zoom &&
			g_LocalPlayer->m_hActiveWeapon()->m_iItemDefinitionIndex() != WEAPON_AUG)
		{
			Render::Get().RenderLine((float)0, y, (float)w, y, Color::Black);
			Render::Get().RenderLine(x, (float)0, x, (float)h, Color::Black);
		}



	}
	render_mutex.lock();
	*draw_list_act = *draw_list;
	render_mutex.unlock();
}

ImDrawList* Render::RenderScene() 
{
	if (render_mutex.try_lock()) 
	{
		*draw_list_rendering = *draw_list_act;
		render_mutex.unlock();
	}

	return draw_list_rendering;
}

float Render::RenderText(const std::string& text, ImVec2 pos, float size, Color color, bool center, bool outline, ImFont* pFont)
{
	ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

	if (!pFont->ContainerAtlas) 
		return 0.f;

	draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

	if (center)
		pos.x -= textSize.x / 2.0f;

	if (outline) 
	{
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
	}

	draw_list->AddText(pFont, size, pos, GetU32(color), text.c_str());
	draw_list->PopTextureID();

	return pos.y + textSize.y;
}

void Render::RenderCircle3D(Vector position, float points, float radius, Color color)
{
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
	{
		Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
		Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

		Vector start2d, end2d;
		if (g_DebugOverlay->ScreenPosition(start, start2d) || g_DebugOverlay->ScreenPosition(end, end2d))
			return;

		RenderLine(start2d.x, start2d.y, end2d.x, end2d.y, color);
	}
}

void Render::RenderCircleDualColor(float x, float y, float rad, float rotate, int type, int resolution, DWORD color, DWORD color2) {
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;

	std::vector<CUSTOMVERTEX> circle(resolution + 2);

	float angle = rotate * D3DX_PI / 180, pi = D3DX_PI;

	if (type == 1)
		pi = D3DX_PI; // Full circle
	if (type == 2)
		pi = D3DX_PI / 2; // 1/2 circle
	if (type == 3)
		pi = D3DX_PI / 4; // 1/4 circle

	pi = D3DX_PI / type; // 1/4 circle

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = color2;

	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		circle[i].color = color;
	}

	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		circle[i].x = x + cos(angle) * (circle[i].x - x) - sin(angle) * (circle[i].y - y);
		circle[i].y = y + sin(angle) * (circle[i].x - x) + cos(angle) * (circle[i].y - y);
	}

	g_D3DDevice9->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);

	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();

	g_D3DDevice9->SetTexture(0, NULL);
	g_D3DDevice9->SetPixelShader(NULL);
	g_D3DDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_D3DDevice9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_D3DDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	g_D3DDevice9->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	g_D3DDevice9->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	g_D3DDevice9->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}

void Render::RenderCircleDualColor(float x, float y, float rad, float rotate, int type, int resolution) {
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;

	std::vector<CUSTOMVERTEX> circle(resolution + 2);

	float angle = rotate * D3DX_PI / 180, pi = D3DX_PI;

	if (type == 1)
		pi = D3DX_PI; // Full circle
	if (type == 2)
		pi = D3DX_PI / 2; // 1/2 circle
	if (type == 3)
		pi = D3DX_PI / 4; // 1/4 circle

	pi = D3DX_PI / type; // 1/4 circle

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);

	float hue = 0.f;

	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;

		auto clr = Color::FromHSB(hue, 1.f, 1.f);
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), 150);
		hue += 0.02;
	}

	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));

		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}

	g_D3DDevice9->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);

	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();

	g_D3DDevice9->SetTexture(0, NULL);
	g_D3DDevice9->SetPixelShader(NULL);
	g_D3DDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_D3DDevice9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_D3DDevice9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	g_D3DDevice9->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	g_D3DDevice9->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	g_D3DDevice9->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
