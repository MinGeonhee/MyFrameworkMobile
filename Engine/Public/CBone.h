#pragma once

#include "CBase.h"
#include "Engine_Struct.h"

class ENGINE_DLL CBone final : public CBase
{
private:
    CBone() = default;
    explicit CBone(const CBone& Prototype);
    ~CBone() override = default;

public:
    HRESULT Initialize(const Engine::BONE_INFO& tBoneInfo);
    const Engine::BONE_INFO& Get_BoneInfo() const { return m_tInfo; }
    const float* Get_BoneMatrixPtr() const { return m_tInfo.matTransform; }
    const float* Get_CombinedTransformationMatrixPtr() const { return m_matCombined; }
    void Set_TransformMatrix(const float* pMatrix);
    void Update_CombinedTransformationMatrix(const std::vector<CBone*>& vecBone, const float* pPreTransform = nullptr);

    static CBone* Create(const Engine::BONE_INFO& tBoneInfo);
    CBone* Clone();
    void Free() override;

private:
    Engine::BONE_INFO m_tInfo{};
    float m_matCombined[16]{};
};
