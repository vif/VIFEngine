#pragma once

#include "DllExport.h"

#include <string>

//dispatcher framework, initializes and calls all the other frameworks

namespace MainFramework
{
    MainFrameWorkEXPORT int Run(const std::string& args);
};
