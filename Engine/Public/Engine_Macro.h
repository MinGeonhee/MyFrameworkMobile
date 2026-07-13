#pragma once

#if defined(_WIN32)
#    if defined(ENGINE_EXPORTS)
#        define ENGINE_DLL __declspec(dllexport)
#    else
#        define ENGINE_DLL __declspec(dllimport)
#    endif
#else
#    define ENGINE_DLL __attribute__((visibility("default")))
#endif

#define DECLARE_SINGLETON(ClassName)        \
public:                                     \
    static ClassName* GetInstance();        \
    static void DestroyInstance();          \
                                            \
private:                                    \
    static ClassName* s_pInstance;

#define IMPLEMENT_SINGLETON(ClassName)      \
    ClassName* ClassName::s_pInstance = nullptr; \
    ClassName* ClassName::GetInstance()     \
    {                                       \
        if (s_pInstance == nullptr)          \
            s_pInstance = new ClassName{};  \
        return s_pInstance;                 \
    }                                       \
    void ClassName::DestroyInstance()       \
    {                                       \
        delete s_pInstance;                 \
        s_pInstance = nullptr;              \
    }

