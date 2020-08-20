#include <stdint.h>
#include <vector>

struct GLFWwindow;

struct UserDisplaySurface {
   GLFWwindow* window;
   VkSurfaceKHR surface;
   VkInstance instance;

   UserDisplaySurface(VkInstance instance, const uint32_t WIDTH = 800, const uint32_t HEIGHT = 600);
   ~UserDisplaySurface();
   bool animate();

   static void setRequiredExtensions(std::vector<const char*>& extensions);
};
