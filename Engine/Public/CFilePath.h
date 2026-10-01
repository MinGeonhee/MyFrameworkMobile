#pragma once
#include "CBase.h"
class CFilePath :
    public CBase
{
public:
    /* 파일 이름 , 파일 확장자명 기입  */
    inline  static wstring  ColliderData_Path_Json = L"../../Resource/Data/ModelData/ColliderData.json";

    /* World Scene Data JsonPath */
    inline  static wstring  WordlSceneData_Path_Json = L"../../Resource/Data/SceneData/WorldData/World.json";
};

