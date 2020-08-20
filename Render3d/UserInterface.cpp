#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "./UserInterface.h"

UserDisplaySurface::UserDisplaySurface(VkInstance _instance, const uint32_t WIDTH, const uint32_t HEIGHT) {
   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
   window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
   instance = _instance;
   if (glfwCreateWindowSurface(_instance, window, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface!");
   }
}

UserDisplaySurface::~UserDisplaySurface() {
   if (surface) vkDestroySurfaceKHR(instance, surface, nullptr);
   if (window) glfwDestroyWindow(window);
}

bool UserDisplaySurface::animate() {
   if (window) {
      if (glfwWindowShouldClose(window)) {
         glfwDestroyWindow(window);
         window = 0;
      }
      else {
         glfwPollEvents();
      }
   }
   return !!window;
}

void UserDisplaySurface::setRequiredExtensions(std::vector<const char*>& extensions) {
   glfwInit();
   uint32_t glfwExtensionCount = 0;
   const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
   for (int i = 0; i < glfwExtensionCount; i++) {
      extensions.push_back(glfwExtensions[i]);
   }
}