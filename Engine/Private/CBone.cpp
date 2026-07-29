#include "pch.h"
#include "CBone.h"

namespace
{
void Identity(float* pOut)
{
    std::fill(pOut, pOut + 16, 0.f);
    pOut[0] = pOut[5] = pOut[10] = pOut[15] = 1.f;
}

void Multiply(const float* a, const float* b, float* pOut)
{
    float Result[16]{};
    for (uint32 r = 0; r < 4; ++r)
        for (uint32 c = 0; c < 4; ++c)
            for (uint32 k = 0; k < 4; ++k)
                Result[r * 4 + c] += a[r * 4 + k] * b[k * 4 + c];
    std::copy(std::begin(Result), std::end(Result), pOut);
}
}

CBone::CBone(const CBone& Prototype)
    : CBase{Prototype}, m_tInfo{Prototype.m_tInfo}
{
    std::copy(std::begin(Prototype.m_matCombined), std::end(Prototype.m_matCombined), m_matCombined);
}

HRESULT CBone::Initialize(const Engine::BONE_INFO& tBoneInfo)
{
    if (tBoneInfo.strName.empty())
        return E_INVALIDARG;
    m_tInfo = tBoneInfo;
    std::copy(std::begin(m_tInfo.matTransform), std::end(m_tInfo.matTransform), m_matCombined);
    return S_OK;
}

void CBone::Set_TransformMatrix(const float* pMatrix)
{
    if (pMatrix != nullptr)
        std::copy(pMatrix, pMatrix + 16, m_tInfo.matTransform);
}

void CBone::Update_CombinedTransformationMatrix(const std::vector<CBone*>& vecBone, const float* pPreTransform)
{
    if (m_tInfo.iParentIndex >= 0 && static_cast<size_t>(m_tInfo.iParentIndex) < vecBone.size())
        Multiply(m_tInfo.matTransform, vecBone[m_tInfo.iParentIndex]->m_matCombined, m_matCombined);
    else if (pPreTransform != nullptr)
        Multiply(m_tInfo.matTransform, pPreTransform, m_matCombined);
    else
        std::copy(std::begin(m_tInfo.matTransform), std::end(m_tInfo.matTransform), m_matCombined);
}

CBone* CBone::Create(const Engine::BONE_INFO& tBoneInfo)
{
    CBone* pInstance = new CBone{};
    if (FAILED(pInstance->Initialize(tBoneInfo)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CBone* CBone::Clone() { return new CBone{*this}; }
void CBone::Free() { CBase::Free(); }
