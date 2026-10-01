#include "CLight.h"
#include "MyImGuiHeader.h"


CLight::CLight(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice) , m_pDeviceContext(pContext) , m_eLightType(LIGHT_TYPE::END)
	, m_tLightBufferData{}  , m_isActive(true)
{
}

HRESULT CLight::Light_Initialize(const LIGHT_DESC& tDesc)
{
	m_tLightBufferData = tDesc.tLightBufferData;
	m_eLightType = tDesc.eLightType;

	return S_OK;
}

bool CLight::Is_CameraInRange(const Float4& vRenderCamAreaXZ)
{
	float MinX = vRenderCamAreaXZ.x;
	float MaxX = vRenderCamAreaXZ.y;

	float MinZ = vRenderCamAreaXZ.z;
	float MaxZ = vRenderCamAreaXZ.w;

	float fLightX = m_tLightBufferData.vPosition.x;
	float fLightZ = m_tLightBufferData.vPosition.z;
	float fRangeSq = m_tLightBufferData.fRange  * m_tLightBufferData.fRange;

	/* 제일가까운점? */
	float fX = fLightX < MinX ? MinX : (fLightX > MaxX) ? MaxX : fLightX;
	float fZ = fLightZ < MinZ ? MinZ : (fLightX > MaxZ) ? MaxZ : fLightZ;
	

	float fDistX = fLightX - fX;
	float fDistZ = fLightZ - fZ;

	return (fDistX * fDistX + fDistZ * fDistZ) < fRangeSq;

}

CLight* CLight::Create(const LIGHT_DESC& tDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CLight* pLight = new CLight(pDevice, pContext);

	if (FAILED(pLight->Light_Initialize(tDesc)))
	{
		Safe_Release(pLight);
		MSG_BOX("Light is failed to Create");
		return nullptr;
	}
	return pLight;
}


HRESULT	CLight::Render(CCameraObjectBase* pRenderCam)
{
	uint32 iPassIndex{ 0 };

	if (m_eLightType == LIGHT_TYPE::DIRECTIONAL)
	{
		/* 각기 바인딩 함수가 다르다 */




	}
	else if (m_eLightType == LIGHT_TYPE::POINT)
	{

	}







	return S_OK;
}




void CLight::Free()
{
	__super::Free();
}



#ifdef _DEBUG
void	CLight::ImGuiUpdate()
{
	MYIMGUI::EnumRadioButton<LIGHT_TYPE> Light_Type_BT(m_eLightType);
	Light_Type_BT.Render();

	MYIMGUI::Slider_Float RangeSlider("Light Range", &m_tLightBufferData.fRange, 0.f, 1000.f, false);

	RangeSlider.Render();


	ImGui::Text(" Pos X : %f" , m_tLightBufferData.vPosition.x);
	if (ImGui::Button("+ ##1", ImVec2(20,20)))
		m_tLightBufferData.vPosition.x += 1.f;
	ImGui::SameLine();
	if (ImGui::Button("- ##1", ImVec2(20, 20)))
		m_tLightBufferData.vPosition.x -= 1.f;

	ImGui::Text(" Pos Y : %f", m_tLightBufferData.vPosition.y);
	if (ImGui::Button("+ ##2", ImVec2(20, 20)))
		m_tLightBufferData.vPosition.y += 1.f;
	ImGui::SameLine();
	if (ImGui::Button("- ##2", ImVec2(20, 20)))
		m_tLightBufferData.vPosition.y -= 1.f;

	ImGui::Text(" Pos Z : %f", m_tLightBufferData.vPosition.z);
	if (ImGui::Button("+ ##3", ImVec2(20, 20)))
		m_tLightBufferData.vPosition.z += 1.f;
	ImGui::SameLine();
	if (ImGui::Button("- ##3", ImVec2(20, 20)))
		m_tLightBufferData.vPosition.z -= 1.f;


	ImGui::DragFloat3("Diffuse" , &m_tLightBufferData.vDiffuse.x , 0.01f , 0.f , 100.f);
	ImGui::DragFloat3("Ambient", &m_tLightBufferData.vAmbient.x, 0.01f, 0.f, 100.f);
	ImGui::DragFloat3("Specular", &m_tLightBufferData.vSpecular.x, 0.01f, 0.f, 100.f);

}
#endif // _DEBUG
