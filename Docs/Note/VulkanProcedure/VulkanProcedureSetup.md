# Vulkan Procedure之Setup
一些Setup涉及的基本流程及概念, 参考[Vulkan Tutorial](https://vulkan-tutorial.com/)

![总览](https://pic1.zhimg.com/v2-adcf46b99591a0e498c0dd345042f3c8_r.jpg)
## 1 Instance
``
VkInstance instance;
``

连接我们的应用和Vulkan库, 用以初始化Vulkan库
### 1.1 VkApplicationInfo(optional)
提供一些我们应用程序的基本信息,其结构体包含：
```C++
// Provided by VK_VERSION_1_0
typedef struct VkApplicationInfo {
    VkStructureType    sType;//VK_STRUCTURE_TYPE_APPLICATION_INFO
    const void*        pNext;//nullptr或指向这个结构体的扩展结构体的指针
    const char*        pApplicationName;//UTF8 string
    uint32_t           applicationVersion;
    const char*        pEngineName;
    uint32_t           engineVersion;
    uint32_t           apiVersion;//必须是该应用使用的最高的Vulkan版本
} VkApplicationInfo;
```
### 1.2 VkInstanceCreateInfo
创建Instance需要用到的参数(Vulkan很多参数都是用结构体的方式传递)
```C++
typedef struct VkInstanceCreateInfo {
    VkStructureType             sType;
    const void*                 pNext;
    VkInstanceCreateFlags       flags;//bitmask indicating the behavior of the instance?
    const VkApplicationInfo*    pApplicationInfo;
    uint32_t                    enabledLayerCount;
    const char* const*          ppEnabledLayerNames;
    uint32_t                    enabledExtensionCount;
    const char* const*          ppEnabledExtensionNames;
} VkInstanceCreateInfo;
```
其中，我们可以通过如下方式获取支持的extensions
```C++
uint32_t extension_count = 0;
vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
// VkResult vkEnumerateInstanceExtensionProperties(
//     const char*                                 pLayerName,
//     uint32_t*                                   pPropertyCount,
//     VkExtensionProperties*                      pProperties);
// 有了count之后就可以分配相应的容器
std::vector<VkExtensionProperties> extensions(extension_count);
vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
```
***
上述步骤之后，我们就可以创建Instance了：  
``
VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
``
***
## 2 Validation layers
正如其名，用来validate的一个可选组件（因为Vulkan默认的错误检查被降至最少），该层常见的操作有：
- 根据要求检查参数的值，防止误传
- 跟踪object的创建和析构，检查资源泄漏
- 检查线程安全
- 记录每次的call和其参数
- 记录Vulkan calls
一般来说，启用VK_LAYER_KHRONOS_validation就足够了
```C++
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
//按需开启Validation layer
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
```
和之前的检查extension类似，我们可以通过``VkResult vkEnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties);``来检查layer，从而检查用到的validationLayers是否包含在可用的layers中。

### 2.1 Message callback
如果只想看到特定的错误信息，我们可以使用VK_EXT_debug_utils扩展来建立一个callback
***
## 3 Physical device
选择物理设备放在``VkPhysicalDevice``句柄里，当Instance被销毁，其也一并隐式销毁，故不需要对其特意处理。
和之前extension、layer类似，我们同样可以使用``VkResult vkEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices);``来获取可用物理设备。  
之后，我们可用通过各种机制（选第一个，打分等）选择合适的物理设备，其中我们可能会用到的函数及宏有：
```C++
void vkGetPhysicalDeviceFeatures(//The support for optional features like texture compression, 64 bit floats and multi viewport rendering (useful for VR)
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures);
void vkGetPhysicalDeviceProperties(//Basic device properties like the name, type and supported Vulkan version
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties);
//features中一定要包含geometryShader
```
***
## 4 Queue families
Vulkan中几乎所有的操作（anything from drawing to uploading textures），都需要将命令先提交到队列中。**Vulkan 有多种不同类型的队列，它们属于不同的队列族，每个队列族的队列只允许执行特定的一部分指令**。 比如，可能存在只允许执行计算相关指令的队列族和只允许执行内存传输的队列族。  
### 4.1 获取所有可用队列族
我们使用``vkGetPhysicalDeviceQueueFamilyProperties``来获取队列族列表（参数老三样）。
```C++
void vkGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties);

uint32_t queueFamilyCount = 0;
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

typedef struct VkQueueFamilyProperties {
    VkQueueFlags    queueFlags;//VkQueueFlagBits的位掩码，此队列族中的队列的能力。
    uint32_t        queueCount;//该队列族的队列数目
    uint32_t        timestampValidBits;//写入时间戳有意义的位数
    VkExtent3D      minImageTransferGranularity;//此队列族中的队列上的图像传输操作支持的最小粒度
} VkQueueFamilyProperties;

std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
```
### 4.2 按需返回队列族索引
基本逻辑：遍历之前获取的队列族列表，根据``VkQueueFamilyProperties.queueflags``判断是否符合需求，返回该索引（因为是uint的关系，包括0在内的任何值都可能是索引，所以可用使用optional库下的has_value()判断该索引存在与否）。

每次创建一个对象都要“填表”的
```C++
typedef struct VkDeviceQueueCreateInfo {
    VkStructureType             sType;
    const void*                 pNext;
    VkDeviceQueueCreateFlags    flags;
    uint32_t                    queueFamilyIndex;
    uint32_t                    queueCount;//该队列族要创建的队列数
    const float*                pQueuePriorities;
} VkDeviceQueueCreateInfo;

//example
uint32_t queue_family_index = xxx;//你需要的队列族的索引
VkDeviceQueueCreateInfo queueCreateInfo{};
queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
queueCreateInfo.queueFamilyIndex = index;
//目前的驱动只允许创建小部分的队列，而且基本上你也不需要超过一个的队列
//因为你可以用多线程创建很多command buffers（见上图）然后一次性提交
queueCreateInfo.queueCount = 1;
//指定队列的优先级，0.0f-1.0f
float queuePriority = 1.0f;
queueCreateInfo.pQueuePriorities = &queuePriority;
```

***
## 5 Logical device
可以先看有关队列族的内容  
``VkDevice device``  是对于物理设备的抽象（a interface)，建立过程与Instance类似。在此过程中，我们需要指定需要的功能和要创建的队列。并且如果有不同的需求，我们甚至可用从同一个物理设备中创建出多个逻辑设备。  
```C++
typedef struct VkDeviceCreateInfo {
    VkStructureType                    sType;//同Instance
    const void*                        pNext;//同Instance
    VkDeviceCreateFlags                flags;//reserved
    uint32_t                           queueCreateInfoCount;//size of the pQueueCreateInfos array
    const VkDeviceQueueCreateInfo*     pQueueCreateInfos;//见队列族
    uint32_t                           enabledLayerCount;
    const char* const*                 ppEnabledLayerNames;
    uint32_t                           enabledExtensionCount;
    const char* const*                 ppEnabledExtensionNames;
    const VkPhysicalDeviceFeatures*    pEnabledFeatures;//null or ptr to a VkPhysicalDeviceFeatures structure
} VkDeviceCreateInfo;
```
### 5.1 创建队列族
见队列族有关内容
### 5.2 指定需要的特性
还记得我们在物理设备中提及的``vkGetPhysicalDeviceFeatures``吗，我们可以选择其获取到的可用的特性创建一个``VkPhysicalDeviceFeatures deviceFeatures{}``

现在我们可以创建Device对象了！还是先“填表”！
```C++
VkDeviceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
createInfo.pQueueCreateInfos = &queueCreateInfo;
createInfo.queueCreateInfoCount = 1;

createInfo.pEnabledFeatures = &deviceFeatures;
//****************其余内容和InstanceCreateInfo类似**********************
//!!!但是，这里要根据具体的设备来，其中一个例子就是extension中的VK_KHR_swapchain
//EnabledLayerCount and ppEnabledLayerNames fields of VkDeviceCreateInfo 
//are ignored by up-to-date implementations
//However, it is still a good idea to set them anyway to be compatible with older implementations:
createInfo.enabledExtensionCount = 0;//0 for now

if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
} else {
    createInfo.enabledLayerCount = 0;
}

```
然后调用``vkCreateDevice创建Device
```C++
VkResult vkCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice);
if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
}
```
device对象要通过调用``void vkDestroyDevice(
    VkDevice                                    device,
    const VkAllocationCallbacks*                pAllocator);``进行销毁。  
### 5.3获取逻辑设备的队列
调用如下函数：
```C++
void vkGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue);

VkQueue graphicsQueue;
vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
//因为之前的例子里我们在逻辑设备的队列族中只创建了一条队列，所以这里queueIndex是0
```
<font size=5>自此我们就完成了对Vulkan的简单setup了，可以着手做些什么了！</font>
