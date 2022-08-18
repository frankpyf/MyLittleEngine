# Vulkan Procedure之Presentation
由于Vulkan是一个跨平台的库，所以它不能直接与window system（注意不是windows操作系统）进行交互，我们需要使用WSI(Window System Integration)扩展。
## VK_KHR_surface/window surface
``VkSurfaceKHR surface``是对于呈现我们渲染图片的“表面”的抽象
它的用法虽然是跨平台的，但是该对象的创建还是需要根据不同平台分别实现的。  
简单来说，直接用glfw提供的glfwCreateWindowSurface函数就可以直接帮你创建一个surface句柄，可以忽略平台细节。   
欲知详情，可以看看该函数的实现。这里不做介绍。
