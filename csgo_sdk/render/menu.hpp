#pragma once

#include <string>

#include "../sdk/utils/singleton.hpp"

#include "../imgui/imgui.h"
#include "../helpers/keybinds.hpp"
#include "../functions/aimbot.hpp"

struct IDirect3DDevice9;

class Menu
    : public Singleton<Menu>
{
public:
    bool AimbotFirstTimeRender = true;
    bool TriggerFirstTimeRender = true;

    void Initialize();
    void Shutdown();

    void OnDeviceLost();
    void OnDeviceReset();

    void Render();

    void Toggle();

    bool IsVisible() const { return _visible; }

private:
    void CreateStyle();

    ImGuiStyle        _style;
    bool              _visible;
};


class Color;
namespace ImGui {
    float CalcMaxPopupHeightFromItemCount(int items_count);
	bool Keybind(const char* str_id, CKeyBind* kbind);
}

