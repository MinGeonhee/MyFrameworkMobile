#include "pch.h"
#include "CSceneManager.h"
#include "CScene.h"
CSceneManager* CSceneManager::Create(){return new CSceneManager{};}
HRESULT CSceneManager::Register_SceneFactory(uint32 id,const SceneFactory& f){if(!f||m_mapSceneFactory.count(id))return E_FAIL;m_mapSceneFactory.emplace(id,f);return S_OK;}
void CSceneManager::RequestChangeScene(const Engine::CREATE_SCENE_DESC& d){m_queueSceneChange.push(d);}
CScene* CSceneManager::MakeSceneByFactory(const Engine::CREATE_SCENE_DESC& d)const{auto it=m_mapSceneFactory.find(d.iMakeSceneID);return it!=m_mapSceneFactory.end()?it->second(d):nullptr;}
HRESULT CSceneManager::ChangeScene(const Engine::CREATE_SCENE_DESC& d)
{
 if(d.eIsPop==SCENE_ISPOP::YES){if(m_stackScene.empty())return E_FAIL;m_stackScene.back()->Release();m_stackScene.pop_back();if(!m_stackScene.empty())m_stackScene.back()->OnResume();return S_OK;}
 CScene* s=MakeSceneByFactory(d);if(!s)return E_FAIL;auto type=s->GetSceneInfo().eStackType;
 if(type==SCENE_STACK_TYPE::PUSH&&!m_stackScene.empty())m_stackScene.back()->OnPause();
 else if(type==SCENE_STACK_TYPE::CLEAR){for(auto* old:m_stackScene)old->Release();m_stackScene.clear();}
 else if(type==SCENE_STACK_TYPE::REPLACE&&!m_stackScene.empty()){m_stackScene.back()->Release();m_stackScene.pop_back();}
 m_stackScene.push_back(s);s->OnEnter();return S_OK;
}
HRESULT CSceneManager::ExecutePendingSceneChanges(){while(!m_queueSceneChange.empty()){auto d=m_queueSceneChange.front();m_queueSceneChange.pop();if(FAILED(ChangeScene(d)))return E_FAIL;}return S_OK;}
uint32 CSceneManager::GetCurSceneID()const{return GetCurScene()?GetCurScene()->GetSceneID():Engine::GLOBAL_LEVEL;}
CScene* CSceneManager::GetCurScene()const{return m_stackScene.empty()?nullptr:m_stackScene.back();}
void CSceneManager::Pre_Update(float dt){if(GetCurScene()&&!GetCurScene()->Get_IsPause())GetCurScene()->Pre_Update(dt);}
void CSceneManager::Update(float dt){if(GetCurScene()&&!GetCurScene()->Get_IsPause())GetCurScene()->Update(dt);}
void CSceneManager::Late_Update(float dt){if(GetCurScene()&&!GetCurScene()->Get_IsPause())GetCurScene()->Late_Update(dt);}
void CSceneManager::Add_RenderGroup(){if(GetCurScene())GetCurScene()->Add_RenderGroup();}
HRESULT CSceneManager::Render(){if(m_stackScene.empty())return S_OK;size_t b=m_stackScene.size()-1;while(b>0&&m_stackScene[b]->GetSceneInfo().isRenderWithPreviousScene)--b;for(size_t i=b;i<m_stackScene.size();++i)if(FAILED(m_stackScene[i]->Render()))return E_FAIL;return S_OK;}
void CSceneManager::Free(){while(!m_queueSceneChange.empty())m_queueSceneChange.pop();for(auto* s:m_stackScene)s->Release();m_stackScene.clear();m_mapSceneFactory.clear();CBase::Free();}
