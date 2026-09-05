#pragma once

class IPlayModeSnapshotTarget
{
public:
    virtual ~IPlayModeSnapshotTarget() = default;

    virtual bool CapturePlaySnapshot() = 0;

    virtual bool RestorePlaySnapshot() = 0;

    virtual void DiscardPlaySnapshot()
        noexcept = 0;

    [[nodiscard]]
    virtual bool HasPlaySnapshot()
        const noexcept = 0;
};