#pragma once

#include "Framework.h"

//responsible for creating & managing Windows

class WindowFramework : public Framework
{
public:
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void StartUpdate(const double delta) override;
    virtual void FinishUpdate() override;
};
