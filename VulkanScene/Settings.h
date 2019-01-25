#pragma once
#include <vulkan/vulkan.h>
#include <string>

struct Settings
{
    VkSampleCountFlagBits sampleCount;

    uint32_t shadowsDim;

    std::string scenePath;
};
