#pragma once 

#include "IClientNetworkable.hpp"
#include "IClientRenderable.hpp"
#include "IClientUnknown.hpp"
#include "IClientThinkable.hpp"

struct SpatializationInfo_t;

class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	virtual void Release(void) = 0;
    // Network origin + angles
    virtual const Vector& GetAbsOrigin(void) const = 0;
    virtual const Vector& GetAbsAngles(void) const = 0;
};

#pragma pack(push, 1)
class CCSWeaponInfo { //xSeeker
public:
    char pad_0x0000[0x4];
    char* szWeaponName;
    char pad_0x0008[0xC];
    int32_t iMaxClip1;
    char pad_0x0018[0xC];
    int32_t iMaxReservedAmmo;
    char pad_0x0028[0x4];
    char* szWeaponModelPath;
    char pad_0x0030[0x4];
    char* szDroppedWeaponModelPath;
    char pad_0x0038[0x48];
    char* szBulletType;
    char pad_0x0084[0x4];
    char* szLocalizationToken;
    char pad_0x008C[0x40];
    int32_t iWeaponType;
    int32_t iWeaponPrice;
    int32_t iWeaponReward;
    char* m_szWeaponGroup;
    char pad_0x00DC[0x10];
    unsigned char m_nFullAuto;
    char pad_0x00ED[0x3];
    int32_t iDamage;
    float flArmorRatio;
    int32_t iAmmo;
    float flPenetration;
    char pad_0x00F8[0x8];
    float flRange;
    float flRangeModifier;
    char pad_0x0110[0x10];
    unsigned char nSilencer;
    char pad_0x0121[0xF];
    float flSpread;
    float flSpreadAlt;
    char pad_0x0138[0x4C];
    int32_t iRecoilSeed;
    char pad_0x0188[0x68];
    char* szWeaponTracesType;
    char pad_0x01F4[0x638];
};
#pragma pack(pop)

class IWeaponSystem
{
	virtual void unused0() = 0;
	virtual void unused1() = 0;
public:
	virtual CCSWeaponInfo* GetWpnData(unsigned ItemDefinitionIndex) = 0;
};