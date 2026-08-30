<img width="2562" height="1486" alt="3d23e144c1b546877bafffcbe29ad2e4" src="https://github.com/user-attachments/assets/5c92ced9-b285-4939-ac2d-d237203e6f3f" /># 

一个基于 Vulkan 的个人实时渲染器项目，用于学习、研究和实践现代实时渲染与物理模拟技术。

项目基于 [Sascha Willems Vulkan Samples](https://github.com/SaschaWillems/Vulkan) 的部分初始化 Vulkan 的 Framework 进行开发，目前已重构其大部分逻辑，并在此基础上实现了自己的渲染管线、PBR 光照、延迟渲染、阴影、后处理、材质、Shader 资源布局以及相关渲染功能。

本项目的主要目标是在一个稳定的 Vulkan 基础框架之上，持续研究和实现Realtime Rendering、物理模拟与引擎架构。

# 环境依赖
- Windows 10/11
- CMake 3.10
- C++ 20
- Vulkan SDK
- Python3
- Git

# 第三方库
- GLFW
- GLM
- ImGui
- tinygltf
- KTX
- gli
- Slang
以上第三方库都已包含在仓库的thirdParty目录下，如果需要更新相关库的版本，可以自行下载替换

# 项目构建
直接使用CMake生成即可

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

### Demo screenshot
<img width="2562" height="1486" alt="3d23e144c1b546877bafffcbe29ad2e4" src="https://github.com/user-attachments/assets/2bda226c-9522-484a-9379-40b49f0704e1" />
<img width="2562" height="1486" alt="3d23e144c1b546877bafffcbe29ad2e4" src="https://github.com/user-attachments/assets/de7200b7-7863-45df-a121-abb6a0f5eba2" />
<img width="2562" height="1486" alt="ee375da9273cde24007740d202b3d283" src="https://github.com/user-attachments/assets/bbbdd5e7-70a4-409b-b0b1-c6f5c0e1cb18" />
