#include "CModel.h"
#include "CBone.h" 
#include "CMesh.h"
#include "CAnimation.h"
#include "EnumUtils.h"
#include "CResource.h"

CModel::CModel(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CResource(pDevice,pContext), m_umapMesh{}, m_vecMesh{}, m_wstrModelName{}, m_eModelType{MODEL_TYPE::NONANIM}, m_pOwner(nullptr)
	, m_vecBone{}, m_matPreTransform{}, m_iCurrentAnimIndex{ 0 }, m_vecAnimation{}, m_iPreAnimIndex{}, m_iIdleIndex{} , m_isAnimEnd{false}
	, m_fLerpAccTime{}, m_fLerpTime{ 0.07f }, m_isBlendAnimEnd{ false }
	, m_vecBlendStartBoneMatrices{}
#ifdef _DEBUG
	,m_iSelectAnim{0}
#endif // _DEBUG

{
	m_eResourceType = RESOURCE_TYPE::MODEL;
	XMStoreFloat4x4(&m_matPreTransform  , XMMatrixIdentity());
}

CModel::CModel(const CModel& Proto)
	: CResource(Proto), m_umapMesh{ Proto.m_umapMesh },m_vecMesh{Proto.m_vecMesh},m_pOwner(Proto.m_pOwner),m_eModelType(Proto.m_eModelType),
	  m_wstrModelName(Proto.m_wstrModelName) , m_matPreTransform(Proto.m_matPreTransform),
	  m_iCurrentAnimIndex(Proto.m_iCurrentAnimIndex) , m_iPreAnimIndex{ Proto.m_iPreAnimIndex}, m_iIdleIndex{Proto.m_iIdleIndex}, m_isAnimEnd{ Proto.m_isAnimEnd } ,
	  m_fLerpAccTime{ Proto.m_fLerpAccTime }, m_fLerpTime{Proto.m_fLerpTime }, m_isBlendAnimEnd(Proto.m_isBlendAnimEnd)
	, m_vecBlendStartBoneMatrices{Proto.m_vecBlendStartBoneMatrices}
#ifdef _DEBUG
	, m_iSelectAnim{ Proto.m_iSelectAnim }
#endif // _DEBUG
{
	
	/* Bone => 깊은 복사해줘야하는 이유 : Bone안에있는 행렬값을 모두가 공유하고 있으면 같은 모델을 사용하는 객체들이 모두 똑같은 애니매이션으로 덮어씌어저버리는 현상 */	
	size_t Count = Proto.m_vecBone.size();
	this->m_vecBone.resize(Count);
	for (size_t i = 0; i < Count; ++i)
		this->m_vecBone[i] = Proto.m_vecBone[i]->Clone();


	/* 애니매이션이 깊은복사가 필요한 이유 => 애니매이션 안에있는 	m_fCurTrackPosition => 현재 이 애니매이션이 재생되고 있는 TrackPosition으로 이거떄문에 필요하다..*/
	Count = Proto.m_vecAnimation.size();
	this->m_vecAnimation.resize(Count);
	for (size_t i = 0; i < Count; ++i)
		this->m_vecAnimation[i] = Proto.m_vecAnimation[i]->Clone();
	

	/* Mesh를 깊은복사하게 되면 문제되는게 Player관련인데...플레이어 모델은 따로나눠서 플레이어 1(Link1) , 플레이어 2(Link2)로 나눠서 로드하면 될거같음.....*/
	for (auto& Mesh : Proto.m_vecMesh)
		Safe_AddRef(Mesh);
	/* Mesh를 이름으로 검색해서 빠르게 들고올 수 있게해줄 컨테이너 복사도 레퍼런스 증가를 시켜준다*/
	for (auto& Mesh : Proto.m_umapMesh)
		Safe_AddRef(Mesh.second);

}

HRESULT CModel::Initailize(const wstring& _wstrModelInfoPath,const XMatrix_1& PreTransformMatrix)
{
	if (_wstrModelInfoPath.empty())
		return E_FAIL;

	wstring wstrPath = _wstrModelInfoPath;

	XMStoreFloat4x4(&m_matPreTransform, PreTransformMatrix);

	CFileStream fs(_wstrModelInfoPath.c_str());
	nlohmann::ordered_json Modeljson;

	if (!fs.is_open())
		return E_FAIL;
	fs >> Modeljson;

	/* Json 추출 후 바로 close */
	fs.close();

	/* 기본으로 이름받기 */
	m_wstrModelName = String_To_WString(Modeljson["Name"].get<string>());
	m_eModelType = EnumUtils::MODEL_TYPE_String_To_Enum(Modeljson["ModelType"].get<string>());

	HRESULT hr{ E_FAIL };
	hr = m_eModelType == MODEL_TYPE::ANIM ? Ready_AnimModel(PreTransformMatrix,Modeljson) : Ready_NonAnimModel(PreTransformMatrix,Modeljson);





	return hr;
}

HRESULT CModel::Copy_Initialize(void* pArg)
{
	/* 나중에 필요한 자료들을 여기서 확인해준다 */

	return S_OK;
}

HRESULT CModel::Ready_AnimModel(const XMatrix_1& PreTransformMatrix,const nlohmann::ordered_json& ModelJson)
{
	/* Bone */
	uint32 siCountBone = ModelJson["Bone"]["Count"].get<uint32>();
	wstring strBonePath = String_To_WString(ModelJson["Bone"]["Path"].get<string>());


	HRESULT hr{ E_FAIL };


	/* Bone 먼저  */
    hr = Ready_Bone(siCountBone, strBonePath,PreTransformMatrix);

	hr = Ready_AnimMesh(ModelJson["MeshesCount"].get<uint32>(), ModelJson["Mesh"]);


	bool isHaveAnim = ModelJson.contains("Anim");

	if (isHaveAnim)
	{
		uint32 iCountAnimation = ModelJson["Anim"]["Count"].get<uint32>();
		string strAnimPath = ModelJson["Anim"]["Path"].get<string>();

		hr = Ready_Animation(iCountAnimation, strAnimPath);
	}	

	m_vecBlendStartBoneMatrices.resize(m_vecBone.size());

	return S_OK;
}

HRESULT CModel::Ready_NonAnimModel(const XMatrix_1& PreTransformMatrix,const nlohmann::ordered_json& ModelJson)
{
	HRESULT hr{ E_FAIL };


	/* 뼈가 있는 NonAniModel이 있을 수있다 */


	if (ModelJson.contains("Bone"))
		hr = Ready_Bone(ModelJson["Bone"]["Count"].get<uint32>(), String_To_WString(ModelJson["Bone"]["Path"].get<string>()) , PreTransformMatrix );
	

	/* Mesh 개수 , Mesh 이름 */
	hr = Ready_NonAnimMesh(PreTransformMatrix,ModelJson["MeshesCount"].get<uint32>(), ModelJson["Mesh"]);





	return S_OK;
}

HRESULT CModel::Ready_AnimMesh(uint32 iCountMesh, const nlohmann::ordered_json& Meshjson)
{
	for (uint32 i = 0; i < iCountMesh; ++i)
	{
		auto& MeshIndexJson = Meshjson[i];

		auto& VIBufferJson = MeshIndexJson["VIBuffer"];
		string strMeshName = MeshIndexJson["Name"].get<string>();

		/* Animation Model에서만 필요한 정보 */
		MeshConnectedBoneInfo tMeshConnectBoneInfo{};
		tMeshConnectBoneInfo.iCountConnectedBone= MeshIndexJson["Count"]["ConnectedBone"];
		/* 연결된 Bone Id 값 , 연결된 Bone에대한 Offset 정보들 */
		if (tMeshConnectBoneInfo.iCountConnectedBone != 0)
		{
			for (uint32 i = 0; i < tMeshConnectBoneInfo.iCountConnectedBone; i++)
			{
				tMeshConnectBoneInfo.vecConnectedBoneID.push_back( MeshIndexJson["ConnectedBoneID"][i].get<uint32>());
				Float4x4 tMatrix{};
				if (MeshIndexJson.contains("OffsetMatrix")) memcpy(&tMatrix, MeshIndexJson["OffsetMatrix"][i].get<vector<float>>().data(), sizeof(XMFLOAT4X4));
				else XMStoreFloat4x4(&tMatrix, XMMatrixIdentity());

				tMeshConnectBoneInfo.vecConnectedOffsetMatrix.push_back(tMatrix);
			}
		}


		auto& VIBufferCountJson = VIBufferJson["Count"];

		VI_INFO tVIInfo{};
		tVIInfo.strVertexDataPath = VIBufferJson["VertexDataPath"].get<string>();
		tVIInfo.strIndexDataPath = VIBufferJson["IndexDataPath"].get<string>();
		tVIInfo.iVertexCnt = VIBufferCountJson["Vertex"].get<uint32>();
		tVIInfo.iIndexCnt = VIBufferCountJson["Index"].get<uint32>();

		auto& MaterialJson = MeshIndexJson["Material"];
		wstring wstrMaterialName = String_To_WString(MaterialJson["Name"]);
		Material_Path tPath{};

		/* 나중에 for문 돌려서 있는것만 돌려서 넣는게 좋을거 같긴해 */
		tPath.wstrDiffusePath = String_To_WString(MaterialJson["Diffuse"].get<string>());
		tPath.wstrAmbientPath = String_To_WString(MaterialJson["Ambient"].get<string>());
		tPath.wstrNormalPath = String_To_WString(MaterialJson["Normal"].get<string>());
		tPath.wstrSpecularPath = String_To_WString(MaterialJson["Specular"].get<string>());
		tPath.wstrEmissivePath = String_To_WString(MaterialJson["Emissive"].get<string>());
		tPath.wstrMetalPath = String_To_WString(MaterialJson["Metalness"].get<string>());


		CMesh* pMesh = CMesh::Create_Anim(strMeshName, tMeshConnectBoneInfo, wstrMaterialName, tPath, tVIInfo, m_pDevice, m_pDeviceContext);
		NULLPTR_RETURN_EFAIL(pMesh);
		m_umapMesh.emplace(pMesh->Get_Name(), pMesh);
		m_vecMesh.push_back(pMesh);
		Safe_AddRef(pMesh);
	}


	return S_OK;
}

HRESULT CModel::Ready_Animation(uint32 iCountAnimation, const string& strAnimPath)
{

	CFileStream fs_Anim(strAnimPath, std::ios::binary);
	if (!fs_Anim.is_open())
		return E_FAIL;

	ANIM_INFO* pAnimInfo = new ANIM_INFO[iCountAnimation];
	NULLPTR_RETURN_EFAIL(pAnimInfo);

	CAnimation* pAnimation{nullptr};

	for (uint32 i = 0; i < iCountAnimation; ++i)
	{
		fs_Anim.read(pAnimInfo[i].szName, sizeof(char) * MAX_PATH);
		fs_Anim.read(reinterpret_cast<char*>(&pAnimInfo[i].fDuration), sizeof(float));
		fs_Anim.read(reinterpret_cast<char*>(&pAnimInfo[i].fTickPerSecond), sizeof(float));
		fs_Anim.read(reinterpret_cast<char*>(&pAnimInfo[i].iCountChannel), sizeof(int32));

		// Channel 만큼반복해서
		for (uint32 j = 0; j < pAnimInfo[i].iCountChannel; ++j)
		{
			Channel_Info tCHInfo{};
			fs_Anim.read(reinterpret_cast<char*>(&tCHInfo.iConnectedBoneIndex), sizeof(int32));
			fs_Anim.read(reinterpret_cast<char*>(&tCHInfo.iCountKeyFrame), sizeof(int32));
			tCHInfo.vecKeyFrame.resize(tCHInfo.iCountKeyFrame);
			fs_Anim.read(reinterpret_cast<char*>(tCHInfo.vecKeyFrame.data()), sizeof(KEY_FRAME) * tCHInfo.iCountKeyFrame);
			pAnimInfo[i].vecChannelInfo.push_back(tCHInfo);

		}

		pAnimation = CAnimation::Create(pAnimInfo[i]);
		NULLPTR_RETURN_EFAIL(pAnimation);
		m_vecAnimation.push_back(pAnimation);
	}

	fs_Anim.close();

	delete[] pAnimInfo; pAnimInfo = nullptr;


	return S_OK;
}

CMesh* CModel::Find_Mesh(const string& _strName)
{
	const auto& pFind = m_umapMesh.find(_strName);

	if (pFind == m_umapMesh.end())
		return nullptr;

	return pFind->second;
}

HRESULT CModel::Ready_NonAnimMesh(const XMatrix_1& PreTransformMatrix , uint32 iCountMesh,const nlohmann::ordered_json& Meshjson)
{
	for (uint32 i = 0; i < iCountMesh; ++i)
	{
		auto& MeshIndexJson = Meshjson[i];
		string strMeshName = MeshIndexJson["Name"].get<string>();

		auto& VIBufferJson = MeshIndexJson["VIBuffer"];

		auto& VIBufferCountJson = VIBufferJson["Count"];
		
		VI_INFO tVIInfo{};
		tVIInfo.strVertexDataPath = VIBufferJson["VertexDataPath"].get<string>();
		tVIInfo.strIndexDataPath= VIBufferJson["IndexDataPath"].get<string>();
		tVIInfo.iVertexCnt = VIBufferCountJson["Vertex"].get<uint32>();
		tVIInfo.iIndexCnt = VIBufferCountJson["Index"].get<uint32>();
	 
		auto& MaterialJson = MeshIndexJson["Material"];
		wstring wstrMaterialName = String_To_WString(MaterialJson["Name"]);
		Material_Path tPath{};
		
		/* 나중에 for문 돌려서 있는것만 돌려서 넣는게 좋을거 같긴해 */
		tPath.wstrDiffusePath = String_To_WString(MaterialJson["Diffuse"].get<string>());
		tPath.wstrAmbientPath = String_To_WString(MaterialJson["Ambient"].get<string>());
		tPath.wstrNormalPath = String_To_WString(MaterialJson["Normal"].get<string>());
		tPath.wstrSpecularPath = String_To_WString(MaterialJson["Specular"].get<string>());
		tPath.wstrEmissivePath = String_To_WString(MaterialJson["Emissive"].get<string>());
		tPath.wstrMetalPath = String_To_WString(MaterialJson["Metalness"].get<string>());


		CMesh* pMesh = CMesh::Create_NonAnim(strMeshName, wstrMaterialName,tPath , tVIInfo, PreTransformMatrix,m_pDevice , m_pDeviceContext );
		NULLPTR_RETURN_EFAIL(pMesh);
		m_umapMesh.emplace(pMesh->Get_Name() , pMesh);
		m_vecMesh.push_back(pMesh);
		Safe_AddRef(pMesh);
	}


	return S_OK;
}

HRESULT CModel::Ready_Bone( uint32 iCountBone ,const wstring& _wstrBonePath, const XMatrix_1& PreTransformMatrix)
{
	CFileStream fs(_wstrBonePath,std::ios::binary);
	if (fs.is_open() == false) return E_FAIL;

	/* Bone의 개수만큼 생성 */
	BONE_INFO* pBoneInfo = new BONE_INFO[iCountBone];

	fs.read(reinterpret_cast<char*>(pBoneInfo),sizeof(BONE_INFO) *  iCountBone);
	

	for (uint32 i = 0; i < iCountBone; ++i)
	{
		CBone* pBone = CBone::Create(pBoneInfo[i]);
		NULLPTR_RETURN_EFAIL(pBone);
		m_vecBone.push_back(pBone);
	}

	fs.close();

	delete[]  pBoneInfo; pBoneInfo = nullptr;

	return S_OK;
}

const Float4x4* CModel::Get_BoneMatrix(const string& strName)const
{
	const auto& pFindBone = std::find_if(m_vecBone.begin(), m_vecBone.end(), [=](CBone* pBone)
		{
			if (pBone->Get_BoneInfo().szName == strName)
				return true;
			else
				return false;
		});
	
	if (pFindBone == m_vecBone.end())
		return nullptr;


	return (*pFindBone)->Get_BoneMatrixPtr();
}

const Float4x4* CModel::Get_CombinedBoneMatrix(const string& strName) const
{
	const auto& pFindBone = std::find_if(m_vecBone.begin(), m_vecBone.end(), [=](CBone* pBone)
		{
			if (pBone->Get_BoneInfo().szName == strName)
				return true;
			else
				return false;
		});

	if (pFindBone == m_vecBone.end())
		return nullptr;


	return (*pFindBone)->Get_CombinedTransformationMatrixPtr();
}

int32 CModel::Get_BoneIndex(const string& strName)const
{
	int32 iIndex{0};

	const auto& iter = std::find_if(m_vecBone.begin(), m_vecBone.end(),[&iIndex, strName](CBone*  pBone)->bool {
		if (strName == string(pBone->Get_BoneInfo().szName)) return true;
		else
			iIndex++;

		return false;
		});

	/* 없다는뜻 */
	if (iter == m_vecBone.end())
		iIndex = -1;

	return iIndex;
}

HRESULT CModel::Set_GPU_Material(CShader* pShader, int32 iMeshIndex)
{
	if (iMeshIndex > m_vecMesh.size())
		return E_FAIL;

		return m_vecMesh[iMeshIndex]->Set_GPU_Material(pShader);
}

HRESULT CModel::Set_GPU_ConnectBoneMatrix(CShader* pShader, const string& strName, int32 iMeshIndex)
{
	NULLPTR_RETURN_EFAIL(pShader);


	return m_vecMesh[iMeshIndex]->Set_GPU_ConnectedBoneMatrix(pShader, strName, m_vecBone);
}

HRESULT CModel::Set_Material_Index(const string& _strMeshName, MATERIAL_TYPE _eType, uint32 _iIndex)
{
	CMesh* pMesh = Find_Mesh(_strMeshName);

	NULLPTR_RETURN_EFAIL(pMesh);

	return pMesh->Set_Material_Index(_eType, _iIndex);
}

wstring CModel::Get_Material_Name(const string& _strMeshName) const
{
	return m_umapMesh.at(_strMeshName)->Get_MaterialName();
}

void CModel::Reset_Animation(uint32 iAnimIndex)
{
	m_vecAnimation[iAnimIndex]->ResetAnimation();
}

void CModel::Set_IsAnimationEnd(uint32 iAnimIndex, bool isEnd)
{
	m_vecAnimation[iAnimIndex]->Set_IsAnimationEnd(isEnd);
}

void CModel::Set_MeshActive(const string& _strMeshName, bool isActive)
{
	m_umapMesh[_strMeshName]->Set_Active(isActive);
}

void CModel::Set_MeshActive(uint32 iMeshIndex, bool isActive)
{
	m_vecMesh[iMeshIndex]->Set_Active(isActive);
}

void CModel::Set_Owner(FD CGameObject* pOwner)
{
	for (auto& Mesh : m_vecMesh)
		Mesh->Set_Owner(pOwner);

	m_pOwner = pOwner;
}

bool CModel::Get_MeshActive(const string& _strMeshName)
{
	return m_umapMesh[_strMeshName]->Get_IsActive();
}

CMesh* CModel::Get_Mesh(const string& strMeshName)
{
	return m_umapMesh[strMeshName];
}

bool CModel::Get_IsAnimationEnd(uint32 iAnimIndex) const
{
	return m_vecAnimation[iAnimIndex]->Get_IsAnimationEnd();
}

float CModel::Get_AnimProgress()
{
	return m_vecAnimation[m_iCurrentAnimIndex]->Get_AnimProgress();
}

uint32 CModel::Get_AnimCurFrameIndex()
{
	return m_vecAnimation[m_iCurrentAnimIndex]->Get_CurAnimKeyFrameIndex();
}

#ifdef _DEBUG

vector<string> CModel::Get_MeshNames()
{
	vector<string> strNames{};

	for (auto& Mesh : m_vecMesh)
	{
		strNames.push_back(Mesh->Get_Name());
	}

	return strNames;
}

void CModel::ImGui_Update()
{
	if (ImGui::TreeNode("Model Info"))
	{

		if (ImGui::TreeNode("Animation Info"))
		{
			vector<string> strAnimationName{};

			for (auto& Anim : m_vecAnimation)
				strAnimationName.push_back(Anim->Get_AnimationName());

			if (ImGui::BeginCombo(" List##1 ", strAnimationName[m_iSelectAnim].c_str()))
			{
				for (uint32 i = 0; i < static_cast<uint32>(strAnimationName.size()); ++i)
				{
					bool isSelectItem = (m_iSelectAnim == i);
					if (ImGui::Selectable(strAnimationName[i].c_str(), isSelectItem))
						m_iSelectAnim = i;
					if (isSelectItem)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}


			if (ImGui::Button("Play Animaiton"))
			{
				/* Chnage_Animation */
				Change_Animation(m_iSelectAnim,ANIMATION_PLAYTYPE::LOOP);
			}



		
			ImGui::TreePop();
		}


		ImGui::TreePop();
	}
}
#endif // _DEBUG


bool CModel::Play_Animation(float fTickPerSecond,float DT)
{
	if (m_eModelType == MODEL_TYPE::NONANIM)
		return false;

	if (!m_isBlendAnimEnd)
	{
		m_fLerpAccTime += DT;
		if (m_fLerpAccTime < m_fLerpTime)
			m_vecAnimation[m_iCurrentAnimIndex]->Update_BlendAnimation(m_vecBone, m_vecBlendStartBoneMatrices ,m_fLerpAccTime / m_fLerpTime);
		else
		{
            m_fLerpAccTime = 0.f;
			m_isBlendAnimEnd = true;
		}
	}
	else
		m_vecAnimation[m_iCurrentAnimIndex]->Update_TransformMatrix(fTickPerSecond,DT,m_vecBone, m_isAnimEnd);

	for (auto& Bone : m_vecBone)
		Bone->Update_CombinedTransformationMatrix(m_vecBone,XMLoadFloat4x4(&m_matPreTransform));

	return (m_isAnimEnd == true) && (m_isBlendAnimEnd == true);
}

void CModel::Change_Animation(uint32 iSetAnimIndex, ANIMATION_PLAYTYPE ePlayType)
{
	if (iSetAnimIndex >= Get_AnimationCount()) { return; }
	if (m_iCurrentAnimIndex == iSetAnimIndex) return;


	m_iPreAnimIndex = m_iCurrentAnimIndex;
	
	m_iCurrentAnimIndex = iSetAnimIndex;

	m_fLerpAccTime = 0.f;

	m_isBlendAnimEnd = false;
	m_isAnimEnd = false;
	m_vecAnimation[iSetAnimIndex]->Set_IsAnimationEnd(false);

	for (int32 i = 0; i < m_vecBone.size(); ++i)
	{
		assert(m_vecBone[i] != nullptr);
		m_vecBlendStartBoneMatrices[i] = *m_vecBone[i]->Get_BoneMatrixPtr();
	}

	m_vecAnimation[m_iCurrentAnimIndex]->SetPlayType(ePlayType);
}

void CModel::Update_BlendAnimation(float DT)
{
	/* 이게 끝났는지 안끝났는지를 Bool값으로 표기해야할듯? */
	m_vecAnimation[m_iCurrentAnimIndex]->Update_BlendAnimation(m_vecBone,m_vecBlendStartBoneMatrices , DT );
}

void CModel::Apply_BoneCombineTrasnform(const string& strStartBoneName, const string& strEndBoneName, const XMatrix_1& CombineMatrix)
{
	int32 iStartBoneIndex = Get_BoneIndex(strStartBoneName);
	int32 iEndBoneIndex = Get_BoneIndex(strEndBoneName);


	for (int32 i = iStartBoneIndex; i < iEndBoneIndex; ++i)
	{
		m_vecBone[i]->Apply_BoneCombineTrasnform(CombineMatrix);
	}
}

HRESULT CModel::Draw(CShader* _pShader, const string& strName, uint32 iMeshIndex)
{
	if (iMeshIndex >= m_vecMesh.size()) return E_FAIL;


	return m_vecMesh[iMeshIndex]->Draw(_pShader,strName);
}

CResource* CModel::Copy_Resource(void* pArg)
{
	CModel* pCopyModel = new CModel(*this);
	
	if (FAILED(pCopyModel->Copy_Initialize(pArg)))
	{
		Safe_Release(pCopyModel);
		MSG_BOX("Copy Resource is failed To Model");
		return nullptr;
	}

	return pCopyModel;
}

CModel* CModel::Create(const wstring& _wstrModelInfoPath, const XMatrix_1& PreTransformMatrix, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CModel* pModel = new CModel(pDevice, pContext);

	if (FAILED(pModel->Initailize(_wstrModelInfoPath, PreTransformMatrix)))
	{
		Safe_Release(pModel);
		MSG_BOX("Model is failed to Create");
		return nullptr;
	}

	return pModel;
}

void CModel::Free()
{
	__super::Free();

	/* CModel::Free() */

	/* 접근용 지우기 */
	for (auto& Mesh : m_umapMesh)
		Safe_Release(Mesh.second);

	/* 순회용 지우기 */
	for (auto& Mesh : m_vecMesh)
		Safe_Release(Mesh);

	/* Bone 지우기 */
	for (auto& Bone : m_vecBone)
		Safe_Release(Bone);

	/* Animation 지우기 */
	for (auto& Animation: m_vecAnimation)
		Safe_Release(Animation);


	return;
}
