#include <Windows.h>
#include "KeyValues.hpp"
#include "../csgostructs.hpp"
#include "../utils/utils.hpp"


typedef KeyValues* (__thiscall* FindKey_t)(void*, const char*, bool);
typedef const char* (__thiscall* GetString_t)(void*, const char*, const char*);
typedef bool(__thiscall* LoadFromBuffer_t)(KeyValues*, const char*, const char*, void*, void*, void*, void*);
typedef void(__thiscall* SetString_t)(KeyValues*, const char*);

void* KeyValues::operator new(size_t allocatedsize)
{
    static PVOID pKeyValuesSystem;
    if (!pKeyValuesSystem)
        pKeyValuesSystem = (reinterpret_cast<PVOID(__cdecl*)()>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "KeyValuesSystem")))();

    return CallVFunction< PVOID(__thiscall*)(PVOID, size_t) >(pKeyValuesSystem, 1)(pKeyValuesSystem, allocatedsize);
}

void KeyValues::operator delete(void* mem)
{
    static PVOID pKeyValuesSystem;
    if (!pKeyValuesSystem)
        pKeyValuesSystem = (reinterpret_cast<PVOID(__cdecl*)()>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "KeyValuesSystem")))();

    return CallVFunction< void(__thiscall*)(PVOID, PVOID) >(pKeyValuesSystem, 2)(pKeyValuesSystem, mem);
}

void KeyValues::Init()
{
    m_iKeyName = -1;
    m_iDataType = TYPE_NONE;

    m_pSub = NULL;
    m_pPeer = NULL;
    m_pChain = NULL;

    m_sValue = NULL;
    m_wsValue = NULL;
    m_pValue = NULL;

    m_bHasEscapeSequences = false;

    memset(unused, 0, sizeof(unused));
}

KeyValues* KeyValues::FindKey(const char* strKeyName, bool bCreate)
{
    return ((FindKey_t)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 EC 1C 53 8B D9 85 DB")))(this, strKeyName, bCreate);
}

const char* KeyValues::GetString(KeyValues* pThis, const char* strKeyName, const char* strDefaultValue)
{
    return ((GetString_t)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 C0 81 EC ? ? ? ? 53 8B 5D 08")))(pThis, strKeyName, strDefaultValue);
}

bool KeyValues::LoadFromBuffer(KeyValues* pThis, const char* pszFirst, const char* pszSecond, PVOID pSomething, PVOID pAnother, PVOID pLast, PVOID pAnother2)
{
    return ((LoadFromBuffer_t)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89 4C 24 04")))(pThis, pszFirst, pszSecond, pSomething, pAnother, pLast, pAnother2);
}

void KeyValues::SetString(const char* strName, const char* strValue)
{
    KeyValues* pKeyValues = this->FindKey(strName, 1);
    if (pKeyValues)
        ((SetString_t)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC A1 ? ? ? ? 53 56 57 8B F9 8B 08 8B 01")))(pKeyValues, strValue);
}

void KeyValues::SetInt(const char* strName, int iValue)
{
    auto pKeyValues = FindKey(strName, 1);
    if (!pKeyValues)
        return;

    m_iValue = iValue;
    m_iDataType = 2;
}

void KeyValues::SetFloat(const char* strName, float flValue)
{
    auto pKeyValues = FindKey(strName, 1);
    if (!pKeyValues)
        return;

    m_flValue = flValue;
    m_iDataType = 3;
}