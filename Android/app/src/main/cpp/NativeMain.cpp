#include <android/input.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window.h>

#include <array>
#include <atomic>
#include <thread>

#include "Engine_Define.h"
#include "CGameManager.h"
#include "Game_Entry.h"

namespace
{
    constexpr const char* LOG_TAG = "MyFrameworkMobile";
    constexpr int32 INPUT_LOOPER_ID = 1;
    constexpr float JOYSTICK_DEADZONE = 40.f;   // 왼쪽 가상 조이스틱 : 시작점에서 이만큼(px) 움직여야 키 입력

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
                Engine::Set_Platform_AssetManager(pAssetManager);

                /* Windows Client 와 같은 진입점 : 엔진은 고정 해상도(WINCX x WINCY)로 그리고 화면에 맞춰 출력한다 */
                if (FAILED(Game::Initialize(pWindow, nullptr, Game::WINCX, Game::WINCY, GRAPHIC_API::VULKAN, nullptr, nullptr)))
                {
                    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Game initialization failed");
                    Game::Release();
                    m_isRunning = false;
                    return;
                }
                m_isEngineReady = true;
                __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Engine and Game initialized with Vulkan");

                CGameManager* pGameManager = CGameManager::GetInstance();
                while (m_isRunning)
                {
                    m_iWindowWidth = static_cast<uint32>(ANativeWindow_getWidth(pWindow));
                    pGameManager->GraphicDevice_Resize(m_iWindowWidth, static_cast<uint32>(ANativeWindow_getHeight(pWindow)));

                    if (!Game::Frame())
                        std::this_thread::sleep_for(std::chrono::milliseconds{1});
                }

                m_isEngineReady = false;
                Game::Release();
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

                while (m_isInputRunning)
                {
                    if (ALooper_pollOnce(16, nullptr, nullptr, nullptr) != INPUT_LOOPER_ID)
                        continue;

                    AInputEvent* pEvent = nullptr;
                    while (AInputQueue_getEvent(m_pInputQueue, &pEvent) >= 0)
                    {
                        if (AInputQueue_preDispatchEvent(m_pInputQueue, pEvent))
                            continue;

                        const bool isHandled = m_isEngineReady && ProcessInputEvent(pEvent);
                        AInputQueue_finishEvent(m_pInputQueue, pEvent, isHandled ? 1 : 0);
                    }
                }

                if (m_isEngineReady)
                    Release_AllTouch();
                AInputQueue_detachLooper(m_pInputQueue);
                m_pInputQueue = nullptr;
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
            if (AInputEvent_getType(pEvent) != AINPUT_EVENT_TYPE_MOTION)
                return false;

            const int32 Action = AMotionEvent_getAction(pEvent);
            const int32 ActionIndex = (Action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

            switch (Action & AMOTION_EVENT_ACTION_MASK)
            {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                Set_TouchByPointer(pEvent, ActionIndex, true);
                return true;

            case AMOTION_EVENT_ACTION_MOVE:
                for (int32 i = 0; i < static_cast<int32>(AMotionEvent_getPointerCount(pEvent)); ++i)
                    Set_TouchByPointer(pEvent, i, true);
                return true;

            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                Set_TouchByPointer(pEvent, ActionIndex, false);
                return true;

            case AMOTION_EVENT_ACTION_CANCEL:
                Release_AllTouch();
                return true;
            }
            return false;
        }

        /* 슬롯 0 = 왼쪽 화면 가상 조이스틱(W/A/S/D) , 슬롯 1 = 오른쪽 화면 카메라 회전(우클릭 드래그) */
        void Set_TouchByPointer(AInputEvent* pEvent, int32 iPointerIndex, bool isPressed)
        {
            const int32 PointerId = AMotionEvent_getPointerId(pEvent, iPointerIndex);
            const float X = AMotionEvent_getX(pEvent, iPointerIndex);
            const float Y = AMotionEvent_getY(pEvent, iPointerIndex);

            int32 Slot = -1;
            for (int32 i = 0; i < 2; ++i)
                if (m_TouchPointerIds[i] == PointerId) Slot = i;

            if (Slot < 0)
            {
                if (!isPressed)
                    return;
                Slot = X < m_iWindowWidth * 0.5f ? 0 : 1;
                if (m_TouchPointerIds[Slot] != -1)
                    return;
                if (Slot == 0)
                    m_JoystickOrigin = { X, Y };
            }

            m_TouchPointerIds[Slot] = isPressed ? PointerId : -1;

            CGameManager* pGameManager = CGameManager::GetInstance();
            if (Slot == 1)
            {
                pGameManager->InputManager_Set_NativeTouch(1, isPressed, X, Y);
                return;
            }

            const float fDX = X - m_JoystickOrigin[0];
            const float fDY = Y - m_JoystickOrigin[1];
            pGameManager->InputManager_Set_NativeKey(KEY_TYPE::W, isPressed && fDY < -JOYSTICK_DEADZONE);
            pGameManager->InputManager_Set_NativeKey(KEY_TYPE::S, isPressed && fDY > JOYSTICK_DEADZONE);
            pGameManager->InputManager_Set_NativeKey(KEY_TYPE::A, isPressed && fDX < -JOYSTICK_DEADZONE);
            pGameManager->InputManager_Set_NativeKey(KEY_TYPE::D, isPressed && fDX > JOYSTICK_DEADZONE);
        }

        void Release_AllTouch()
        {
            CGameManager* pGameManager = CGameManager::GetInstance();
            m_TouchPointerIds = { -1, -1 };
            pGameManager->InputManager_Set_NativeTouch(1, false, 0.f, 0.f);
            for (KEY_TYPE eKey : { KEY_TYPE::W, KEY_TYPE::A, KEY_TYPE::S, KEY_TYPE::D })
                pGameManager->InputManager_Set_NativeKey(eKey, false);
        }

    private:
        std::atomic_bool        m_isRunning{ false };
        std::atomic_bool        m_isEngineReady{ false };
        std::atomic_bool        m_isInputRunning{ false };
        std::thread             m_RenderThread;
        std::thread             m_InputThread;
        AInputQueue*            m_pInputQueue = nullptr;
        std::atomic<uint32>     m_iWindowWidth{ 1 };
        std::array<int32, 2>    m_TouchPointerIds{ -1, -1 };
        std::array<float, 2>    m_JoystickOrigin{ 0.f, 0.f };
    };

    CAndroidClient* GetClient(ANativeActivity* pActivity)
    {
        return pActivity != nullptr ? static_cast<CAndroidClient*>(pActivity->instance) : nullptr;
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
    }
}

extern "C" __attribute__((visibility("default")))
void ANativeActivity_onCreate(ANativeActivity* pActivity, void*, size_t)
{
    if (pActivity == nullptr || pActivity->callbacks == nullptr)
        return;

    pActivity->instance = new CAndroidClient{};
    pActivity->callbacks->onNativeWindowCreated   = OnNativeWindowCreated;
    pActivity->callbacks->onNativeWindowDestroyed = OnNativeWindowDestroyed;
    pActivity->callbacks->onInputQueueCreated     = OnInputQueueCreated;
    pActivity->callbacks->onInputQueueDestroyed   = OnInputQueueDestroyed;
    pActivity->callbacks->onDestroy               = OnDestroy;
}
