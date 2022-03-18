#include "nightmode.h"
#include "../configurations.hpp"
std::vector <MaterialBackup> materials;

void nightmode::clear_stored_materials()
{
	materials.clear();
}

void nightmode::modulate(MaterialHandle_t i, IMaterial* material, bool backup = false)
{
	auto name = material->GetTextureGroupName();

	if (strstr(name, "World"))
	{
		if (backup)
			materials.emplace_back(MaterialBackup(i, material));
		if (g_Configurations.enable_nightmode)
		material->ColorModulate((float)g_Configurations.nightmode_color.r() / 255.0f, (float)g_Configurations.nightmode_color.g() / 255.0f, (float)g_Configurations.nightmode_color.b() / 255.0f);
		else
		material->ColorModulate(1.f, 1.f, 1.f);
	}
	else if (strstr(name, "StaticProp") || strstr(name, "Prop"))
	{
		if (backup)
			materials.emplace_back(MaterialBackup(i, material));
		if (g_Configurations.enable_nightmode)
			material->ColorModulate(((float)g_Configurations.nightmode_color.r() / 255.0f) * 0.8f, ((float)g_Configurations.nightmode_color.g() / 255.0f) * 0.8f, ((float)g_Configurations.nightmode_color.b() / 255.0f) * 0.8f);
		else
			material->ColorModulate(1.f, 1.f, 1.f);
	}
}
void nightmode::modulate_transp(MaterialHandle_t i, IMaterial* material, bool backup = false)
{
	auto name = material->GetTextureGroupName();
	if (strstr(name, "StaticProp"))
	{
		if (backup)
			materials.emplace_back(MaterialBackup(i, material));
		material->AlphaModulate((float)1 - g_Configurations.asus_props);
	}
	else if (strstr(name, "World"))
	{
		if (backup)
			materials.emplace_back(MaterialBackup(i, material));
		material->AlphaModulate((float)1 - g_Configurations.asus_walls);
	}
}
void nightmode::apply()
{
	if (!materials.empty())
	{
		for (auto i = 0; i < (int)materials.size(); i++)
			modulate(materials[i].handle, materials[i].material);

		return;
	}
	materials.clear();
	auto materialsystem = g_MatSystem;
	for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
	{
		auto material = materialsystem->GetMaterial(i);
		if (!material)
			continue;
		if (material->IsErrorMaterial())
			continue;
		modulate(i, material, true);
	}
}

void nightmode::asus()
{
	if (!materials.empty())
	{
		for (auto i = 0; i < (int)materials.size(); i++)
			modulate_transp(materials[i].handle, materials[i].material);
		return;
	}
	materials.clear();
	auto materialsystem = g_MatSystem;

	for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
	{
		auto material = materialsystem->GetMaterial(i);
		if (!material)
			continue;
		if (material->IsErrorMaterial())
			continue;
		modulate_transp(i, material, true);
	}
}

void nightmode::remove()
{
	for (auto i = 0; i < materials.size(); i++)
	{
		if (!materials[i].material)
			continue;

		if (materials[i].material->IsErrorMaterial())
			continue;

		materials[i].restore();
		materials[i].material->Refresh();
	}

	materials.clear();
}