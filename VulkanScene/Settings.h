#pragma once
#include <vulkan/vulkan.h>
#include <string>

struct Settings
{
    VkSampleCountFlagBits sampleCount;

    uint32_t shadowsDim;

    float shadowsDistance;

    bool ssaoEnabled;

    std::string scenePath;
};
