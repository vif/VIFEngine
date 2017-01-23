#include "stdafx.h"

#include "MainFramework.h"
#include <chrono>

#include "WindowFramework.h"

StartupConf ParseArgs(const std::string& args)
{
    StartupConf ret{};
    return ret;
}

int MainFramework::Run(const std::string& args)
{
    StartupConf conf = ParseArgs(args);

    std::vector<std::unique_ptr<Framework>> frameworks;
    frameworks.emplace_back(std::make_unique<WindowFramework>());

    for(auto&& framework : frameworks)
    {
        framework->Init();
    }

    const auto start_time = std::chrono::high_resolution_clock::now();

    auto current_time = start_time;

    bool keepRunning = true;

    while(keepRunning)
    {
        auto new_time = std::chrono::high_resolution_clock::now();
        double delta = std::chrono::duration<double, std::nano>(new_time - current_time).count();
        current_time = new_time;

        for(auto&& framework : frameworks)
        {
            framework->StartUpdate(delta);
        }

        for(auto&& framework : frameworks)
        {
            framework->FinishUpdate();
        }

        for(auto&& framework : frameworks)
        {
            keepRunning &= !framework->ShouldExit();
        }
    }

    for(auto&& framework : frameworks)
    {
        framework->Shutdown();
    }

    return 0;
}
