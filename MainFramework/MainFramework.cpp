#include "stdafx.h"

#include "MainFramework.h"
#include <chrono>

#include <WindowFramework/Window.h>
#include <WindowFramework/WindowFramework.h>

StartupConf ParseArgs(const std::string& args)
{
    StartupConf ret{};
    return ret;
}

int MainFramework::Run(const std::string& args)
{
    StartupConf conf = ParseArgs(args);

    //creation
    auto&& windowFramework = WindowFramework::Create();

    //push into the vector so we can iterate easily
    std::vector<Framework*> frameworks;
    frameworks.emplace_back(windowFramework.get());

    //initialization
    for(auto&& framework : frameworks)
    {
        framework->Init();
    }

    //random stuff
    auto&& window = windowFramework->CreateWindow("Game", nullptr);
    window->Show();

    //main loop
    {
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
    }

    //shutdown
    for(auto&& it = std::rbegin(frameworks); it != std::rend(frameworks); ++it)
    {
        (*it)->Shutdown();
    }

    return 0;
}
