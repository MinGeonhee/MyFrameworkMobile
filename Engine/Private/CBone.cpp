#include "CBone.h"
#include "CModel.h"

CBone::CBone()
    : m_tInfo{}
{
}

CBone::CBone(const CBone& Proto)
    : m_tInfo(Proto.m_tInfo)
{

}

CBone::~CBone()
{
}

HRESULT CBone::Initialize(const BONE_INFO& tBoneInfo)
{
    m_tInfo = tBoneInfo;

    return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const vector<CBone*>& vecBone, const XMatrix_1& PreTransformMatrix)
{
    if (vecBone.empty())
        return;

    /* 이러면 이게 RootNode라는뜻이다 */ // -1 이면 부모
    if (m_tInfo.iParentIndex == -1)
    {
        /* 내 로컬 매트릭스에 변환 행렬을 곱해준다 */
        XMStoreFloat4x4(&m_tInfo.matCombinedTransformation, PreTransformMatrix * XMLoadFloat4x4(&m_tInfo.matTransform));
    }
    else
    {
        /* 내가가진 부모의 뼈의 정보에 접근해서 이믹 곱해졌을 Combined된 트랜스폼을 곱해준다. */
        XMStoreFloat4x4(&m_tInfo.matCombinedTransformation, XMLoadFloat4x4(&m_tInfo.matTransform) * XMLoadFloat4x4(&vecBone[m_tInfo.iParentIndex]->m_tInfo.matCombinedTransformation));
    }
}

void CBone::Apply_BoneCombineTrasnform(const XMatrix_1& CombineMatrix)
{
    return XMStoreFloat4x4(&m_tInfo.matCombinedTransformation, XMLoadFloat4x4(&m_tInfo.matCombinedTransformation) * CombineMatrix);
}

CBone* CBone::Create(const BONE_INFO& tBoneInfo )
{
    CBone* pBone = new CBone();

    if ( FAILED(pBone->Initialize(tBoneInfo)))
    {
        Safe_Release(pBone);
        MSG_BOX("Bone is failed to Create");

        return nullptr;
    }
    return pBone;
}

CBone* CBone::Clone()
{
    return new CBone(*this);
}

void CBone::Free()
{
    __super::Free();

    /* CBone::Free() */

}
