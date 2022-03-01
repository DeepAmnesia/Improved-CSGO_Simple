#include "dx_render.hpp"

#include <iostream>

//I don't know who needs this, but why not leave it here :)

DirectX_Render::DirectX_Render(IDirect3DDevice9* device) 
{
	if (!device)
		throw std::invalid_argument("dx_renderer::dx_renderer: device is nullptr");

	m_device = device;

	if (FAILED(D3DXCreateLine(m_device, &m_line)))
		throw std::exception("dx_renderer::dx_renderer: failed to create line");

	if (FAILED(D3DXCreateFont(m_device, 12, NULL, FW_HEAVY, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &m_font)))
		throw std::exception("dx_renderer::dx_renderer: failed to create font");
}

DirectX_Render::~DirectX_Render() 
{
	if (m_font)
		m_font->Release();

	if (m_line)
		m_line->Release();
}

void DirectX_Render::BeginRendering()
{
	m_device->Clear(NULL, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.f, 0);
	m_device->BeginScene();
}

void DirectX_Render::EndRendering()
{
	m_device->EndScene();
	m_device->Present(NULL, NULL, NULL, NULL);
}

void DirectX_Render::DirectX_RenderLine(int x0, int y0, int x1, int y1, unsigned long color)
{
	D3DXVECTOR2 lines[2] = 
	{
		D3DXVECTOR2(x0, y0),
		D3DXVECTOR2(x1, y1)
	};

	m_line->Begin();
	m_line->Draw(lines, 2, color);
	m_line->End();
}

void DirectX_Render::DirectX_RenderRect(int x, int y, int w, int h, unsigned long color)
{
	DirectX_RenderLine(x, y, x + w, y, color);
	DirectX_RenderLine(x, y, x, y + h, color);
	DirectX_RenderLine(x + w, y, x + w, y + h, color);
	DirectX_RenderLine(x, y + h, x + w + 1, y + h, color);
}

void DirectX_Render::DirectX_RenderFilledRect(int x, int y, int w, int h, unsigned long color)
{
	D3DTLVERTEX qV[4] = 
	{
		{ float(x), float(y + h), 0.f, 1.f, color },
		{ float(x), float(y), 0.f, 1.f, color },
		{ float(x + w), float(y + h), 0.f, 1.f, color },
		{ float(x + w), float(y) , 0.f, 1.f, color }
	};

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	m_device->SetTexture(0, nullptr);
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, qV, sizeof(D3DTLVERTEX));
}

void DirectX_Render::DirectX_RenderOutlinedRect(int x, int y, int w, int h, unsigned long color)
{
	DirectX_RenderRect(x - 1, y - 1, w + 2, h + 2, D3DCOLOR_RGBA(1, 1, 1, 255));
	DirectX_RenderRect(x + 1, y + 1, w - 2, h - 2, D3DCOLOR_RGBA(1, 1, 1, 255));
	DirectX_RenderRect(x, y, w, h, color);
}

void DirectX_Render::DirectX_RenderText(std::string_view text, int x, int y, unsigned long color, bool center, bool outline)
{
	if (center) 
	{
		RECT dimensions = GetTextDimensions(text);
		x -= (dimensions.right - dimensions.left) / 2;
	}

	auto _text = [&](std::string_view _text, int _x, int _y, unsigned long _color) 
	{
		RECT r{ _x, _y, _x, _y };
		m_font->DrawTextA(NULL, _text.data(), -1, &r, DT_NOCLIP, _color);
	};

	if (outline) 
	{
		_text(text, x - 1, y, D3DCOLOR_RGBA(1, 1, 1, 255));
		_text(text, x + 1, y, D3DCOLOR_RGBA(1, 1, 1, 255));
		_text(text, x, y - 1, D3DCOLOR_RGBA(1, 1, 1, 255));
		_text(text, x, y + 1, D3DCOLOR_RGBA(1, 1, 1, 255));
	}

	_text(text, x, y, color);
}

RECT DirectX_Render::GetTextDimensions(std::string_view text)
{
	RECT r;
	m_font->DrawTextA(NULL, text.data(), -1, &r, DT_CALCRECT, 0xFFFFFFFF);
	return r;
}