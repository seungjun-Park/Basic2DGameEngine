#pragma once

#include <memory>
#include <string>

struct ProjectConfig;

class ProjectConfigSerializer
{
public:
    static bool Save(
        const ProjectConfig& config,
        const std::wstring& path
    );

    static std::unique_ptr<ProjectConfig>
        Load(
            const std::wstring& path
        );
};