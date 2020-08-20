#include <vulkan/vulkan.h>
#include "./UserInterface.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>

const std::string root_PATH = "D:\\git\\VulkanPlayground\\data";
const std::string MODEL_PATH = root_PATH + "\\viking_room.obj";
const std::string TEXTURE_PATH = root_PATH + "\\viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


struct Instance {

};

#include <Windows.h>
#include <GLFW/glfw3.h>

struct VulkanDevice {
   VkDevice handle;
   VkPhysicalDevice physicalHandle;

   uint32_t graphicFamilyIndex;
   uint32_t displayFamilyIndex;
};

struct VulkanPhysicalDeviceInfos : VkPhysicalDeviceProperties {
   VkPhysicalDevice handle;
   VkPhysicalDeviceFeatures features;

   struct QueueFamilyInfos : VkQueueFamilyProperties {
      std::string getTag() const {
         std::string tag = "";
         char tmp[12];
         if (this->queueFlags & VK_QUEUE_GRAPHICS_BIT) tag += "graphics,";
         if (this->queueFlags & VK_QUEUE_COMPUTE_BIT) tag += "compute,";
         if (this->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) tag += "sparse-binding,";
         if (this->queueFlags & VK_QUEUE_PROTECTED_BIT) tag += "protected,";
         tag += "timestamp=" + std::string(itoa(this->timestampValidBits, tmp, 10)) + ",";
         tag += "im.align="
            + std::string(itoa(this->minImageTransferGranularity.width, tmp, 10)) + "x"
            + std::string(itoa(this->minImageTransferGranularity.height, tmp, 10)) + "x"
            + std::string(itoa(this->minImageTransferGranularity.depth, tmp, 10)) + ",";
         tag += std::string(itoa(this->queueCount, tmp, 10));
         return tag;
      }
   };

   VulkanPhysicalDeviceInfos(VkPhysicalDevice handle) {
      this->handle = handle;
      vkGetPhysicalDeviceProperties(handle, this);
      vkGetPhysicalDeviceFeatures(handle, &features);
   }
   std::vector<QueueFamilyInfos> getQueueFamilies() const {
      uint32_t queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCount, nullptr);
      std::vector<QueueFamilyInfos> queueFamilies(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCount, queueFamilies.data());
      return queueFamilies;
   }
   int getRate() const {
      int score = 0;
      if (!this->features.geometryShader) return 0;
      if (this->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
      score += this->limits.maxImageDimension2D;
      return score;
   }
   VulkanDevice* createGraphicDevice() const {
      float queuePriority = 1.0f;

      int queueFamilyIndex = 0;
      bool queueFamilyFound = false;
      for (const auto& qfamily : this->getQueueFamilies()) {
         if (qfamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyFound = true;
            break;
         }
         queueFamilyIndex++;
      }
      if (!queueFamilyFound) return 0;

      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;

      VkDeviceCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      createInfo.pQueueCreateInfos = &queueCreateInfo;
      createInfo.queueCreateInfoCount = 1;
      createInfo.pEnabledFeatures = &features;
      createInfo.enabledExtensionCount = 0;
      createInfo.enabledLayerCount = 0;
#ifndef NDEBUG
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
#endif
      auto device = new VulkanDevice();
      if (vkCreateDevice(handle, &createInfo, nullptr, &device->handle) != VK_SUCCESS) {
         throw std::runtime_error("failed to create logical device!");
      }
      device->physicalHandle = this->handle;
      device->graphicFamilyIndex = queueFamilyIndex;
      device->displayFamilyIndex = queueFamilyIndex;
      return device;
   }
};

struct VulkanInstance {
   VkInstance instance = 0;

   VulkanInstance(const char* applicationName) {
      VkApplicationInfo appInfo{};
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = applicationName;
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.pEngineName = "No Engine";
      appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.apiVersion = VK_API_VERSION_1_0;

      std::vector<const char*> extensions;
      UserDisplaySurface::setRequiredExtensions(extensions);

      VkInstanceCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      createInfo.pApplicationInfo = &appInfo;
      createInfo.enabledLayerCount = 0;
#ifndef NDEBUG
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
      createInfo.enabledExtensionCount = extensions.size();
      createInfo.ppEnabledExtensionNames = extensions.data();
      if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
         throw std::runtime_error("failed to create instance!");
      }

#ifndef NDEBUG
      auto CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
      if (CreateDebugUtilsMessengerEXT) {
         VkDebugUtilsMessengerCreateInfoEXT createInfo{};
         createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
         createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
         createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
         createInfo.pfnUserCallback = debugCallback;
         createInfo.pUserData = nullptr; // Optional
         if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
         }
      }
#endif
   }
   ~VulkanInstance() {
      if (debugMessenger) {
         auto DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
         DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
      }
      vkDestroyInstance(instance, nullptr);
   }
   std::vector<VulkanPhysicalDeviceInfos> getPhysicalDevices() {
      uint32_t deviceCount = 0;
      vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
      std::vector<VkPhysicalDevice> handles(deviceCount);
      vkEnumeratePhysicalDevices(instance, &deviceCount, handles.data());
      std::vector<VulkanPhysicalDeviceInfos> devices;
      for (auto handle : handles) devices.push_back(VulkanPhysicalDeviceInfos(handle));
      return devices;
   }
   std::vector<VkExtensionProperties> getExtensions() {
      uint32_t extensionCount = 0;
      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
      std::vector<VkExtensionProperties> extensions(extensionCount);
      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
      return extensions;
   }
   void showInfos() {
      std::cout << "[available extensions]\n";
      for (const auto& extension : this->getExtensions()) {
         std::cout << "  " << extension.extensionName << "\n";
      }
      std::cout << "[available devices]\n";
      for (const auto& deviceInfos : this->getPhysicalDevices()) {
         std::cout << "  " << deviceInfos.deviceName << "\n";
         std::cout << "    > rate: " << deviceInfos.getRate() << "\n";
         for (const auto& qfamily : deviceInfos.getQueueFamilies()) {
            std::cout << "    > queue: " << qfamily.getTag() << "\n";
         }
      }
   }
private:
#ifndef NDEBUG
   const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
   };
   VkDebugUtilsMessengerEXT debugMessenger = 0;
   static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData) {

      std::cerr << "[vulkan] " << pCallbackData->pMessage << std::endl;

      return VK_FALSE;
   }
#endif
};

void main() {
   VulkanInstance instance("render3d");
   instance.showInfos();

   auto display = new UserDisplaySurface(instance.instance);

   VulkanDevice* device = instance.getPhysicalDevices()[0].createGraphicDevice();
   VkQueue graphicsQueue;
   vkGetDeviceQueue(device->handle, 0, 0, &graphicsQueue);

   VkBool32 presentSupport = 0;
   vkGetPhysicalDeviceSurfaceSupportKHR(device->physicalHandle, device->displayFamilyIndex, display->surface, &presentSupport);
   if (!presentSupport) {
      throw "no presentSupport";
   }

   while (display->animate()) {
      Sleep(10);
   }
}
