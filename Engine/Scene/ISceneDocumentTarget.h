#pragma once

#include <string>

class ISceneDocumentTarget
{
public:
    virtual ~ISceneDocumentTarget() = default;

    virtual bool SaveSceneDocument(
        const std::wstring& path
    ) = 0;

    virtual bool LoadSceneDocument(
        const std::wstring& path
    ) = 0;
};