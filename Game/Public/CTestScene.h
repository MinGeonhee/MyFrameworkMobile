#pragma once

#include "Game_Define.h"
#include "CScene.h"

class CTestObject;

/* ------------------------------------------------------------------------------------------- */
/* 엔진 이식 확인용 테스트 씬                                                                      */
/*  - 자유 카메라 (CDefaultCameraObject)                                                          */
/*  - 디퍼드 렌더링 : 방향광 + 점광원 , G-Buffer → Light → Combine                                 */
/*  - 애니메이션 모델 (Link) + 정적 모델 (Tile / Rock / TreasureBox)                              */
/*  - 폰트 출력 (SpriteFont)                                                                      */
/* ------------------------------------------------------------------------------------------- */
class CTestScene final : public CScene
{
private:
	explicit CTestScene(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CTestScene() = default;

protected:
	virtual HRESULT			Initialize(const CreateScene_Desc& tCreateSceneDesc) override;

private:
	HRESULT					Ready_Camera();
	HRESULT					Ready_Object();

public:
	virtual void			Update(float DT) override;
	virtual HRESULT			Render() override;
	virtual void			Ready_CollisionTable() override;
	virtual void			Ready_Light() override;
	virtual string			GetSceneName() override { return "TestScene"; }

private:
	CTestObject*			m_pLink = nullptr;
	uint32					m_iAnimIndex = 0;
	float					m_fAccTime = 0.f;
	float					m_fLightTime = 0.f;

public:
	static CTestScene*		Create(const CreateScene_Desc& tCreateSceneDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void			Free() override;
};
