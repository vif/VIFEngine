#pragma once

struct StartupConf
{
    StartupConf() = delete;
};

class Framework
{
public:
    virtual ~Framework() = default;
    virtual void Init() = 0;
    virtual void Shutdown() = 0;

    virtual void StartUpdate(const double delta) = 0;
    virtual void FinishUpdate() = 0;

    virtual bool ShouldExit() { return false; }
};
