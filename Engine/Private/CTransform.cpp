#include "pch.h"
#include "CTransform.h"
CTransform::CTransform(Device* d,Context* c):CComponent{d,c}{}
CTransform::CTransform(const CTransform& p):CComponent{p},m_Position{p.m_Position},m_Rotation{p.m_Rotation},m_Scale{p.m_Scale},m_WorldMatrix{p.m_WorldMatrix},m_fSpeedPerSecond{p.m_fSpeedPerSecond},m_fRotationPerSecond{p.m_fRotationPerSecond}{}
HRESULT CTransform::Proto_Initialize(){Update_WorldMatrix();return S_OK;}
HRESULT CTransform::Clone_Initialize(void* pArg){if(FAILED(CComponent::Clone_Initialize(pArg)))return E_FAIL;if(pArg){auto& d=*static_cast<Engine::TRANSFORM_DESC*>(pArg);m_Position={d.vPosition[0],d.vPosition[1],d.vPosition[2]};m_Rotation={d.vRotationRadian[0],d.vRotationRadian[1],d.vRotationRadian[2]};m_Scale={d.vScale[0],d.vScale[1],d.vScale[2]};m_fSpeedPerSecond=d.fSpeedPerSecond;m_fRotationPerSecond=d.fRotationPerSecond;}Update_WorldMatrix();return S_OK;}
void CTransform::Update_WorldMatrix(){using M=Diligent::float4x4;m_WorldMatrix=M::Scale(m_Scale)*M::RotationX(m_Rotation.x)*M::RotationY(m_Rotation.y)*M::RotationZ(m_Rotation.z)*M::Translation(m_Position);}
void CTransform::Set_Position(const Diligent::float3& v){m_Position=v;Update_WorldMatrix();}
void CTransform::Set_Rotation(const Diligent::float3& v){m_Rotation=v;Update_WorldMatrix();}
void CTransform::Add_Rotation(float p,float y,float r){m_Rotation.x=std::clamp(m_Rotation.x+p,-1.55334f,1.55334f);m_Rotation.y+=y;m_Rotation.z+=r;Update_WorldMatrix();}
Diligent::float3 CTransform::Get_Right()const{return Diligent::normalize(Diligent::float3{m_WorldMatrix._11,m_WorldMatrix._12,m_WorldMatrix._13});}
Diligent::float3 CTransform::Get_Up()const{return Diligent::normalize(Diligent::float3{m_WorldMatrix._21,m_WorldMatrix._22,m_WorldMatrix._23});}
Diligent::float3 CTransform::Get_Look()const{return Diligent::normalize(Diligent::float3{m_WorldMatrix._31,m_WorldMatrix._32,m_WorldMatrix._33});}
void CTransform::Move_Local(float r,float u,float f,float dt){m_Position+=(Get_Right()*r+Get_Up()*u+Get_Look()*f)*(m_fSpeedPerSecond*dt);Update_WorldMatrix();}
CTransform* CTransform::Create(Device*d,Context*c){auto*p=new CTransform{d,c};if(FAILED(p->Proto_Initialize())){p->Release();return nullptr;}return p;}
CComponent* CTransform::Clone(void*a){auto*p=new CTransform{*this};if(FAILED(p->Clone_Initialize(a))){p->Release();return nullptr;}return p;}
void CTransform::Free(){CComponent::Free();}
