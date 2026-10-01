#pragma once

#include "Game_Define.h"
#include "CGameObject.h"

namespace Engine
{
	class CShader;
	class CModel;
}

/* ------------------------------------------------------------------------------------------- */
/* 테스트용 모델 오브젝트 (원본 Client CObject 와 같은 방식)                                        */
/* CModel(정적 / 애니메이션) 을 Shader_VtxModel / Shader_VtxAnimModel 로 디퍼드 G-Buffer 에 그린다.  */
/* ------------------------------------------------------------------------------------------- */
class CTestObject final : public CGameObject
{
public:
	typedef struct TestObject_Desc : public GAMEOBJECT_DESC
	{
		std::wstring	wstrModelKey{};
		std::wstring	wstrShaderKey{};
		std::string		strPass{ "Default" };
		bool			isAnim{ false };
		uint32			iAnimIndex{ 0 };
		float			fAnimSpeed{ 1.f };
		float			fTurnSpeed{ 0.f };		/* Y 축 회전 속도 (라디안/초) */
		bool			isCollider{ false };
	}TESTOBJECT_DESC;

private:
	explicit CTestObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit CTestObject(const CTestObject& Prototype);
	virtual ~CTestObject() = default;

private:
	HRESULT					Proto_Initialize();
	virtual HRESULT			Clone_Initialize(void* pArg) override;
	virtual HRESULT			Ready_Component(void* pArg) override;

public:
	virtual void			Update(float DT) override;
	virtual void			Late_Update(float DT) override;
	virtual HRESULT			Add_RenderGroup() override;
	virtual HRESULT			Render(CCameraObjectBase* pRenderCam) override;
	virtual void			Add_ColliderGroup() override {}

	void					Change_Animation(uint32 iAnimIndex);
	uint32					Get_AnimationCount() const;

private:
	CShader*				m_pShader = nullptr;
	CModel*					m_pModel = nullptr;
	std::string				m_strPass;
	bool					m_isAnim = false;
	uint32					m_iAnimIndex = 0;
	float					m_fAnimSpeed = 1.f;
	float					m_fTurnSpeed = 0.f;

public:
	static CTestObject*		Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};
