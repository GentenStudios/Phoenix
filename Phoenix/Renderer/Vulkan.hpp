
//#pragma warning(disable : 46812)
#pragma warning(push, 0)
#ifdef _WIN32
//#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__

#endif
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <volk.h>
//#include <volk.h>
#pragma warning(pop)
