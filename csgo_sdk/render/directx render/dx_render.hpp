#pragma once

#include <string>
#include <thread>
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "../../sdk/utils/singleton.hpp"

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

class DirectX_Render : public Singleton<DirectX_Render>
{
private:
	struct D3DTLVERTEX 
	{
		float x, y, z, rhw;
		unsigned long color;
	};

public:
	DirectX_Render() { }
	DirectX_Render(IDirect3DDevice9* device);
	~DirectX_Render();

	void BeginRendering();
	void EndRendering();

	void DirectX_RenderLine(int x0, int y0, int x1, int y1, unsigned long color);
	void DirectX_RenderRect(int x, int y, int w, int h, unsigned long color);
	void DirectX_RenderFilledRect(int x, int y, int w, int h, unsigned long color);
	void DirectX_RenderOutlinedRect(int x, int y, int w, int h, unsigned long color);
	void DirectX_RenderText(std::string_view text, int x, int y, unsigned long color, bool center = true, bool outline = true);

	RECT GetTextDimensions(std::string_view text);

private:
	IDirect3DDevice9* m_device = nullptr;

	ID3DXFont* m_font = nullptr;
	ID3DXLine* m_line = nullptr;
};
