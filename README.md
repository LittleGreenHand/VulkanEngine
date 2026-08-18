# VulkanEngine

一个基于 Vulkan 的个人实时渲染器项目，用于学习、研究和实践现代实时渲染技术。

项目基于 [Sascha Willems Vulkan Samples](https://github.com/SaschaWillems/Vulkan) 的部分基础 Vulkan Framework 进行开发，并在此基础上实现了自己的渲染管线、PBR 光照、延迟渲染、阴影、后处理、材质、Shader 资源布局以及相关渲染功能。

> 本项目的主要目标是在一个稳定的 Vulkan 基础框架之上，持续研究和实现实时渲染技术、物理模拟与引擎架构。


## Features

目前已经实现的主要功能包括：
- Hybrid Deferred
- PBR渲染
- IBL
- 金属度-粗糙度工作流的PBR材质
- 各向异性高光
- DOF
- 定向光与点光源
- 定向光CSM阴影与点光源的Omni阴影
- PCF
- glTF 2.0 model loading
- glTF material system
- KTX texture loading

### Shader System

Shader 使用Slang着色器语言编写，生成Shaders项目时会通过python脚本编译生成SPIR-V，相关编译设置已经集成到 CMake 文件中。