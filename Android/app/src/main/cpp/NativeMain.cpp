#include <android/input.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window.h>

#include <array>
#include <atomic>
#include <chrono>
#include <thread>

#include "Engine_Platform.h"

#include "CGameManager.h"
#include "Engine_Android.h"
#include "Game_Entry.h"

namespace
{
    constexpr const char* LOG_TAG = "MyFrameworkMobile";
    constexpr uint32 TEST_SCENE_ID = 1;
    constexpr int32 INPUT_LOOPER_ID = 1;

    class CAndroidClient final
    {
    public:
        ~CAndroidClient()
        {
            StopInput();
            Stop();
        }

        void Start(ANativeWindow* pWindow, AAssetManager* pAssetManager)
        {
            Stop();
            if (pWindow == nullptr)
                return;

            m_iWindowWidth = static_cast<uint32>(ANativeWindow_getWidth(pWindow));
            m_isRunning = true;
            m_RenderThread = std::thread([this, pWindow, pAssetManager]()
            {
                CGameManager* pGameManager = CGameManager::GetInstance();

                Engine::Set_Android_AssetManager(pAssetManager);

                Engine::Engine_Desc Desc{};
                Desc.eGraphicAPI   = GRAPHIC_API::VULKAN;
                Desc.pNativeWindow = pWindow;
                Desc.iWinSizeX     = static_cast<uint32>(ANativeWindow_getWidth(pWindow));
                Desc.iWinSizeY     = static_cast<uint32>(ANativeWindow_getHeight(pWindow));

                if (FAILED(pGameManager->Initialize_Engine(Desc)))
                {
                    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Engine initialization failed");
                    m_isRunning = false;
                    return;
                }

                if (FAILED(pGameManager->SceneManager_Register_Factory(
                        TEST_SCENE_ID,
                        [](const Engine::CREATE_SCENE_DESC& SceneDesc) -> CScene*
                        {
                            return Game::Create_TestScene(SceneDesc);
                        })))
                {
                    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Game scene factory registration failed");
                    pGameManager->Release_Engine();
                    m_isRunning = false;
                    return;
                }

                Engine::CREATE_SCENE_DESC TestSceneDesc{};
                TestSceneDesc.iMakeSceneID = TEST_SCENE_ID;
                pGameManager->SceneManager_Request_Change(TestSceneDesc);

                __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Engine and Game initialized with Vulkan");

                while (m_isRunning)
                {
                    pGameManager->TimerManager_Compute_TimeDelta(L"Timer_Default");
                    const float fTimeDelta = pGameManager->TimerManager_Get_TimeDelta(L"Timer_Default");

                    const uint32 WindowWidth = static_cast<uint32>(ANativeWindow_getWidth(pWindow));
                    const uint32 WindowHeight = static_cast<uint32>(ANativeWindow_getHeight(pWindow));
                    m_iWindowWidth = WindowWidth;

                    pGameManager->Resize_Engine(WindowWidth, WindowHeight);
                    pGameManager->Pre_Update_Engine(fTimeDelta);
                    pGameManager->Update_Engine(fTimeDelta);
                    pGameManager->Late_Update_Engine(fTimeDelta);
                    pGameManager->Draw();

                    std::this_thread::sleep_for(std::chrono::milliseconds{1});
                }

                pGameManager->Release_Engine();
            });
        }

        void Stop()
        {
            m_isRunning = false;
            if (m_RenderThread.joinable())
                m_RenderThread.join();
        }

        void StartInput(AInputQueue* pInputQueue)
        {
            StopInput();
            if (pInputQueue == nullptr)
                return;

            m_pInputQueue = pInputQueue;
            m_isInputRunning = true;
            m_InputThread = std::thread([this]()
            {
                ALooper* pLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
                AInputQueue_attachLooper(m_pInputQueue, pLooper, INPUT_LOOPER_ID, nullptr, nullptr);
                __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Android input queue attached");

                while (m_isInputRunning)
                {
                    const int32 EventId = ALooper_pollOnce(16, nullptr, nullptr, nullptr);
                    if (EventId != INPUT_LOOPER_ID)
                        continue;

                    AInputEvent* pEvent = nullptr;
                    while (AInputQueue_getEvent(m_pInputQueue, &pEvent) >= 0)
                    {
                        if (AInputQueue_preDispatchEvent(m_pInputQueue, pEvent))
                            continue;

                        const bool isHandled = ProcessInputEvent(pEvent);
                        AInputQueue_finishEvent(m_pInputQueue, pEvent, isHandled ? 1 : 0);
                    }
                }

                Release_AllTouch();
                AInputQueue_detachLooper(m_pInputQueue);
                m_pInputQueue = nullptr;
                __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Android input queue detached");
            });
        }

        void StopInput()
        {
            m_isInputRunning = false;
            if (m_InputThread.joinable())
                m_InputThread.join();
        }

    private:
        bool ProcessInputEvent(AInputEvent* pEvent)
        {
            if (pEvent == nullptr || AInputEvent_getType(pEvent) != AINPUT_EVENT_TYPE_MOTION)
                return false;

            const int32 Action = AMotionEvent_getAction(pEvent);
            const int32 ActionMasked = Action & AMOTION_EVENT_ACTION_MASK;
            const int32 ActionIndex = (Action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            const int32 PointerCount = AMotionEvent_getPointerCount(pEvent);

            switch (ActionMasked)
            {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                Set_TouchByPointer(pEvent, ActionIndex, true);
                return true;

            case AMOTION_EVENT_ACTION_MOVE:
                for (int32 i = 0; i < PointerCount; ++i)
                    Set_TouchByPointer(pEvent, i, true);
                return true;

            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                Set_TouchByPointer(pEvent, ActionIndex, false);
                return true;

            case AMOTION_EVENT_ACTION_CANCEL:
                Release_AllTouch();
                return true;

            default:
                return false;
            }
        }

        void Set_TouchByPointer(AInputEvent* pEvent, int32 iPointerIndex, bool isPressed)
        {
            if (pEvent == nullptr || iPointerIndex < 0)
                return;

            const int32 PointerId = AMotionEvent_getPointerId(pEvent, iPointerIndex);
            const float X = AMotionEvent_getX(pEvent, iPointerIndex);
            const float Y = AMotionEvent_getY(pEvent, iPointerIndex);

            int32 Slot = Find_TouchSlot(PointerId);
            if (Slot < 0 && isPressed)
                Slot = X < (m_iWindowWidth * 0.5f) ? 0 : 1;
            if (Slot < 0 || Slot >= static_cast<int32>(m_TouchPointerIds.size()))
                return;

            m_TouchPointerIds[Slot] = isPressed ? PointerId : -1;
            CGameManager::GetInstance()->InputManager_SetNativeTouchState(
                static_cast<uint32>(Slot),
                isPressed,
                X,
                Y);
        }

        int32 Find_TouchSlot(int32 iPointerId) const
        {
            for (uint32 i = 0; i < m_TouchPointerIds.size(); ++i)
            {
                if (m_TouchPointerIds[i] == iPointerId)
                    return static_cast<int32>(i);
            }
            return -1;
        }

        void Release_AllTouch()
        {
            for (uint32 i = 0; i < m_TouchPointerIds.size(); ++i)
            {
                m_TouchPointerIds[i] = -1;
                CGameManager::GetInstance()->InputManager_SetNativeTouchState(i, false, 0.f, 0.f);
            }
        }

    private:
        std::atomic_bool m_isRunning{false};
        std::atomic_bool m_isInputRunning{false};
        std::thread      m_RenderThread;
        std::thread      m_InputThread;
        AInputQueue*     m_pInputQueue = nullptr;
        uint32           m_iWindowWidth = 1;
        std::array<int32, 2> m_TouchPointerIds{-1, -1};
    };

    CAndroidClient* GetClient(ANativeActivity* pActivity)
    {
        return pActivity != nullptr
            ? static_cast<CAndroidClient*>(pActivity->instance)
            : nullptr;
    }

    void OnNativeWindowCreated(ANativeActivity* pActivity, ANativeWindow* pWindow)
    {
        if (auto* pClient = GetClient(pActivity))
            pClient->Start(pWindow, pActivity->assetManager);
    }

    void OnNativeWindowDestroyed(ANativeActivity* pActivity, ANativeWindow*)
    {
        if (auto* pClient = GetClient(pActivity))
            pClient->Stop();
    }

    void OnInputQueueCreated(ANativeActivity* pActivity, AInputQueue* pInputQueue)
    {
        if (auto* pClient = GetClient(pActivity))
            pClient->StartInput(pInputQueue);
    }

    void OnInputQueueDestroyed(ANativeActivity* pActivity, AInputQueue*)
    {
        if (auto* pClient = GetClient(pActivity))
            pClient->StopInput();
    }

    void OnDestroy(ANativeActivity* pActivity)
    {
        delete GetClient(pActivity);
        pActivity->instance = nullptr;
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "NativeActivity destroyed");
    }
}

extern "C" __attribute__((visibility("default")))
void ANativeActivity_onCreate(ANativeActivity* pActivity, void*, size_t)
{
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "NativeActivity created");

    if (pActivity == nullptr || pActivity->callbacks == nullptr)
        return;

    pActivity->instance = new CAndroidClient{};
    pActivity->callbacks->onNativeWindowCreated   = OnNativeWindowCreated;
    pActivity->callbacks->onNativeWindowDestroyed = OnNativeWindowDestroyed;
    pActivity->callbacks->onInputQueueCreated     = OnInputQueueCreated;
    pActivity->callbacks->onInputQueueDestroyed   = OnInputQueueDestroyed;
    pActivity->callbacks->onDestroy               = OnDestroy;
}
