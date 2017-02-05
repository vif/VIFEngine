#include "stdafx.h"

#include "Renderer/RendererFramework.h"
#include <WindowFramework/WindowFramework.h>

TEST_CASE("Start up and shutdown", "[framework]")
{
    auto&& window_framework = WindowFramework::Create();
    REQUIRE(window_framework);
    auto&& renderer_framework = RendererFramework::Create(*window_framework.get());
    REQUIRE(renderer_framework);

    REQUIRE_NOTHROW(window_framework->Init());
    REQUIRE_NOTHROW(renderer_framework->Init());

    REQUIRE_NOTHROW(renderer_framework->Shutdown());
    REQUIRE_NOTHROW(window_framework->Shutdown());
}
