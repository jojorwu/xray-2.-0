#pragma once

#include "../../xrEngine/stdafx.h"

#define R_VULKAN 5
#define RENDER R_VULKAN
#define USE_VULKAN

#include "../../xrParticles/psystem.h"

#include "../../xrEngine/vis_common.h"
#include "../../xrEngine/Render.h"
#include "../../xrEngine/IGame_Level.h"

#ifndef _WIN32
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include "../../3rd party/volk/volk.h"
#include "vulkan_render.h"
