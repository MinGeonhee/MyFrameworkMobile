#include "CUILayer.h"
#include "CUIObject.h"

CUILayer::CUILayer()
	: m_vecUIObject{}, m_isActive{ true }, m_eUILayerOrder{UILAYER_ORDER::END}
{
}


HRESULT CUILayer::Initialize(UILAYER_ORDER eOrder)
{
	m_eUILayerOrder = eOrder;

	return S_OK;
}

void CUILayer::Pre_Update(CCanvas* pCanvas, float DT)
{
	for (auto& UIObject : m_vecUIObject)
		if (UIObject)
			if (UIObject->Get_IsActive())
				UIObject->Pre_Update(pCanvas,DT);
}

void CUILayer::Update(CCanvas* pCanvas, float DT)
{
	for (auto& UIObject : m_vecUIObject)
		if (UIObject)
			if (UIObject->Get_IsActive())
				UIObject->Update(pCanvas, DT);
}

void CUILayer::Late_Update(CCanvas* pCanvas, float DT)
{
	for (auto& UIObject : m_vecUIObject)
		if (UIObject)
			if (UIObject->Get_IsActive())
				UIObject->Late_Update(pCanvas, DT);
}

HRESULT CUILayer::Render(CCanvas* pCanvas)
{
	///* UiObject Sort */
	//std::sort(m_vecUIObject.begin(), m_vecUIObject.end(), [](CUIObject* pFirst, CUIObject* pSecond) { return pFirst->Get_UIDepth() < pSecond->Get_UIDepth();});

	for (auto& UIObject : m_vecUIObject)
		if (UIObject)
			if (UIObject->Get_IsActive())
				UIObject->Render(pCanvas);

	return S_OK;
}


HRESULT CUILayer::Add_UIObject(CUIObject* pUIObject)
{
	m_vecUIObject.push_back(pUIObject);

	/* 미리 UI Depth로 내림ㅊ라순 */
	std::sort(m_vecUIObject.begin(), m_vecUIObject.end(), [](const CUIObject* pFirst,const CUIObject* pSecond) { return pFirst->Get_UIDepth() < pSecond->Get_UIDepth();});

	return S_OK;
}

CUIObject* CUILayer::Get_UIObject(const wstring& wstrUIObjectName)
{
	auto iter = std::find_if(m_vecUIObject.begin(), m_vecUIObject.end(), [wstrUIObjectName](const CUIObject* pUIObject) {
		return pUIObject->Get_Name() == wstrUIObjectName;
		});

	if (iter == m_vecUIObject.end()) return nullptr;

	return *iter;
}




CUILayer* CUILayer::Create(UILAYER_ORDER eLayerOrder)
{
	CUILayer* pUILayer = new CUILayer();

	if (FAILED(pUILayer->Initialize(eLayerOrder)))
	{
		Safe_Release(pUILayer);
		MSG_BOX("UI Layer is failed to Create");
		return nullptr;
	}

	return pUILayer;
}

void CUILayer::Free()
{
	__super::Free();


	for (auto& UIObject : m_vecUIObject)
		Safe_Release(UIObject);
}

