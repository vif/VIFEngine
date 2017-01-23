#include "stdafx.h"
#include <MainFramework/MainFramework.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    std::wstring wArgs = pCmdLine;
    return MainFramework::Run(ToUTF8(wArgs));
}