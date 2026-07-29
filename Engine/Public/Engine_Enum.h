#pragma once

enum class GRAPHIC_API
{
    D3D11,
    VULKAN
};

enum class PROTOTYPE_TYPE
{
    GAMEOBJECT,
    COMPONENT,
    END
};

// 원본 MyFramework의 리소스 분류 순서를 유지한다.
enum class RESOURCE_TYPE
{
    TEXTURE,
    SHADER,
    MODEL,
    MATERIAL,
    END
};

enum class MAKE_TYPE
{
    PROTOTYPE,
    CLONE,
    END
};

enum class MATERIAL_TYPE
{
    DIFFUSE,
    AMBIENT,
    NORMAL,
    SPECULAR,
    EMISSIVE,
    METAL,
    END
};

enum class MODEL_TYPE
{
    NONANIM,
    ANIM,
    END
};

enum class ANIMATION_PLAYTYPE
{
    ONCE,
    LOOP,
    LASTSTOP,
    END
};

// 공개 Shader 설정에서 Diligent 열거형을 직접 노출하지 않기 위한 엔진 타입이다.
enum class SHADER_VALUE_TYPE
{
    FLOAT32,
    UINT16,
    UINT32
};

enum class SHADER_PRIMITIVE_TOPOLOGY
{
    TRIANGLE_LIST,
    LINE_LIST,
    POINT_LIST
};

enum class SHADER_CULL_MODE
{
    NONE,
    FRONT,
    BACK
};

enum class SHADER_TEXTURE_FORMAT
{
    UNKNOWN,
    RGBA8_UNORM,
    RGBA8_UNORM_SRGB,
    D32_FLOAT
};

enum class UPDATE_PHASE
{
    PreUpdate,
    Update,
    LateUpdate,
    Add_RenderGroup,
    Render,
    END
};

enum class UPDATE_MODE
{
    STATIC,
    DYNAMIC,
    END
};

enum class RENDER_GROUP
{
    SKYBOX,
    PRIORITY,
    NONBLEND,
    NONLIGHT,
    BLEND,
    END
};

enum class SCENE_STACK_TYPE { NONE, PUSH, REPLACE, CLEAR, END };
enum class SCENE_ISPOP { YES, NO };

enum class KEY_STATE { NONE, PRESS, DOWN, UP, END };
enum class KEY_TYPE : uint32
{
    LBUTTON=0x01, RBUTTON=0x02, SHIFT=0x10, CONTROL=0x11, ESCAPE=0x1B, SPACE=0x20,
    LEFT=0x25, UP=0x26, RIGHT=0x27, DOWN=0x28,
    NUM0='0',NUM1='1',NUM2='2',NUM3='3',NUM4='4',NUM5='5',NUM6='6',NUM7='7',NUM8='8',NUM9='9',
    A='A',B='B',C='C',D='D',E='E',F='F',G='G',H='H',I='I',J='J',K='K',L='L',M='M',
    N='N',O='O',P='P',Q='Q',R='R',S='S',T='T',U='U',V='V',W='W',X='X',Y='Y',Z='Z'
};

enum class LIGHT_TYPE { DIRECTIONAL, POINT, END };
enum class COLLIDER_TYPE { SPHERE, AABB, END };
enum class UI_FADE_STATE { NONE, FADE_IN, FADE_OUT };
enum class RENDER_STATE_TYPE { RASTERIZER, DEPTH_STENCIL, BLEND, END };
enum class SPRITE_TEXTURE_MODE { ATLAS_GRID, TEXTURE_SEQUENCE };
enum class PROJECTION_TYPE { PERSPECTIVE, ORTHOGRAPHIC, END };
enum class CAMERA_TYPE { FIRSTPERSON, THIRDPERSON, FREE, FIXED, END };
enum class MOVE_TYPE { NORMAL, SMOOTH, END };
enum class CAMERA_UPDATE_DEPTH : uint32 { DEPTH_0, DEPTH_1, DEPTH_2, END };
enum class CAMERA_RENDER_DEPTH : uint32 { DEPTH_0, DEPTH_1, DEPTH_2, END };
