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