#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static constexpr uint32_t WIDTH{ 800 };
static constexpr uint32_t HEIGHT{ 600 };

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class HelloTriangleApplication 
{

public:
    void run() 
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;

    VkInstance instance;
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };

    std::vector<VkLayerProperties> getSupportedValidationLayers()
    {
        // get validation layer count
        uint32_t validationLayerCount{};
        vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

        // get validation layers
        std::vector<VkLayerProperties> validationLayers{ validationLayerCount };
        vkEnumerateInstanceLayerProperties(&validationLayerCount, validationLayers.data());

        return validationLayers;
    }

    void checkSupportedValidationLayers()
    {
        const std::vector<VkLayerProperties> validationLayers{ getSupportedValidationLayers() };

        std::cout << "available validation layers:\n";

        for (const auto& validationLayer : validationLayers)
            std::cout << '\t' << validationLayer.layerName << '\n';
    }

    // returns false if not every target validation layer is supported
    bool checkTargetValidationLayers(const std::vector<const char*>& targetValidationLayers)
    {
        const std::vector<VkLayerProperties> supportedValidationLayers{
            getSupportedValidationLayers()
        };

        for (const char* targetLayer : targetValidationLayers)
        {
            bool foundLayer{ false };

            for (const VkLayerProperties& supportedLayer : supportedValidationLayers)
            {
                if (std::strcmp(supportedLayer.layerName, targetLayer) == 0)
                {
                    foundLayer = true;
                    break;
                }
            }

            if (!foundLayer)
                return false;
        }

        return true;
    }

    std::vector<VkExtensionProperties> getSupportedExtensions()
    {
        // get extension count
        uint32_t extensionCount{};
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        // get extensions
        std::vector<VkExtensionProperties> extensions{ extensionCount };
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        return extensions;
    }

    void checkSupportedExtensions()
    {
        const std::vector<VkExtensionProperties> extensions{ getSupportedExtensions() };

        std::cout << "available extensions:\n";

        for (const auto& extension : extensions)
            std::cout << '\t' << extension.extensionName << '\n';
    }

    void initWindow() 
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() 
    {
        createInstance();
        pickPhysicalDevice();
    }

    void pickPhysicalDevice()
    {
        uint32_t deviceCount{ 0 };
        vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("Failed to find GPU with Vulkan support.");
    
        std::vector<VkPhysicalDevice> devices{ deviceCount };
        vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());

        for (const VkPhysicalDevice& device : devices)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            printDeviceProperties(deviceProperties);
        }

        for (const VkPhysicalDevice& device : devices)
            if (isDeviceSuitable(device))
            {
                this->physicalDevice = device;
                break;
            }

        if (physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("Failed to find suitable GPU.");
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        return true;
    }

    void printDeviceProperties(const VkPhysicalDeviceProperties& p)
    {
        std::cout << "API Version: \t\t" << p.apiVersion <<
            "\nDriver Version: \t\t" << p.driverVersion <<
            "\nVendor ID: \t\t" << p.vendorID <<
            "\nDevice ID: \t\t" << p.deviceID <<
            "\nDevice Type: \t\t" << toString(p.deviceType) <<
            "\nDevice Name: \t\t" << p.deviceName <<
            "\nPipeline Cache UUID: \t\t" << p.pipelineCacheUUID <<
            "\nLimits: \t\t(This is a giant struct)" <<
            "\nSpace Properties: \t\t(Smaller random struct)\n\n";
    }

    std::string_view toString(VkPhysicalDeviceType type)
    {
        switch (type)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:    return "Integrated GPU";
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:      return "GPU";
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:       return "Virtual GPU";
            case VK_PHYSICAL_DEVICE_TYPE_CPU:               return "CPU";
            default:                                        return "Other";
        }
    }

    void createInstance()
    {
        if (enableValidationLayers && !checkTargetValidationLayers(validationLayers))
            throw std::runtime_error("validation layers requested, but not available!");

        // optional struct which provides more info on instance
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = 0;   // determines global validations layers

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }

        // use GLFW to get extensions needed for windowing
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        const VkResult result{ vkCreateInstance(&createInfo, nullptr, &instance) };

        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void mainLoop() 
    {
        checkSupportedExtensions();
        checkSupportedValidationLayers();

        while (!glfwWindowShouldClose(window)) 
        {
            glfwPollEvents();
        }
    }

    void cleanup() 
    {
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }
};

int main() 
{
    HelloTriangleApplication app;

    try 
    {
        app.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}