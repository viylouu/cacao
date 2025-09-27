#include "renderer.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>

u32 cc_vk_renderer_draw_calls;

typedef struct {
    VkInstance instance;
} VKrendererState;

//
// INSTANCE
//


void vk_instanceCreate(VKrendererState* state, const char* title) {
    VkApplicationInfo appinfo = {0};
    appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appinfo.pApplicationName = title;
    appinfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appinfo.pEngineName = "cacao";
    appinfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appinfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createinfo = {0};
    createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createinfo.pApplicationInfo = &appinfo;

    u32 extensionCount = 0;
    const char* extensions[] = { "vk" };

    // no macos i dont care

    createinfo.enabledExtensionCount = extensionCount;
    createinfo.ppEnabledExtensionNames = extensions;
    createinfo.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&createinfo, 0, &state->instance);
    if (result != VK_SUCCESS) {
        printf("failed to create vulkan instance!");
        exit(1);
    }
}


//
// QUEUE FAMILIES
//


typedef struct {
    u32 graphics_family;
    b8 has_graphics_family;
} VKqueueFamilyIndices;

VKqueueFamilyIndices vk_getQueueFamilies(VkPhysicalDevice device) {
    VKqueueFamilyIndices indices = {0};

    return indices;
}


//
// PHYSICAL DEVICE
//


b8 vk_isDeviceSuitable(VkPhysicalDevice device) {
    (void)device;

    return 1;
}

void vk_pickPhysicalDevice(VKrendererState* state) {
    VkPhysicalDevice physicaldevice = VK_NULL_HANDLE;

    u32 devicecount = 0;
    vkEnumeratePhysicalDevices(state->instance, &devicecount, 0);

    if (devicecount == 0) {
        printf("failed to find a vulkan supported gpu!\n");
        exit(1);
    }

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * devicecount);
    vkEnumeratePhysicalDevices(state->instance, &devicecount, devices);

    for (u32 i = 0; i < devicecount; ++i)
        if (vk_isDeviceSuitable(devices[i])) {
            physicaldevice = devices[i];
            break;
        }
    
    if (physicaldevice == VK_NULL_HANDLE) {
        printf("failed to find a suitable gpu0\n");
        exit(1);
    }
}


//
// MAIN
//


void* cc_vk_rendererInit(const char* title) {
    VKrendererState* state = malloc(sizeof(VKrendererState));

    vk_instanceCreate(state, title);
    vk_pickPhysicalDevice(state);

    return state;
}

void cc_vk_rendererDeinit(void* client) {
    VKrendererState* state = client;

    vkDestroyInstance(state->instance, 0);
}
