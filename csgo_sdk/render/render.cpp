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