#pragma once

#include "../sdk/utils/singleton.hpp"

class IMatRenderContext;
struct DrawModelState_t;
struct ModelRenderInfo_t;
class matrix3x4_t;
class IMaterial;
class Color;

class Chams
    : public Singleton<Chams>
{
    friend class Singleton<Chams>;

    Chams();
    ~Chams();

public:
	void OnDrawModelExecute(
        IMatRenderContext* ctx,
        const DrawModelState_t &state,
        const ModelRenderInfo_t &pInfo,
        matrix3x4_t *pCustomBoneToWorld);

private:
    void OverrideMaterial(int mat_num, bool ignoreZ, bool wireframe, const Color& rgba);

    IMaterial* materialRegular = nullptr;
    IMaterial* materialFlat = nullptr;
    IMaterial* materialCrystal = nullptr;
    IMaterial* materialGlass = nullptr;
    IMaterial* materialCircuit = nullptr;
    IMaterial* materialGlow = nullptr;
    IMaterial* materialAnimated1 = nullptr;
    IMaterial* materialAnimated2 = nullptr;
    IMaterial* materialAnimated3 = nullptr;
    IMaterial* materialAnimated4 = nullptr;
    IMaterial* materialAnimated5 = nullptr;
};