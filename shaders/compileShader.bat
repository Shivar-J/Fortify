@echo off
set PROJECT_DIR=%~1

echo Compiling shaders...

if not exist "%PROJECT_DIR%spv" mkdir "%PROJECT_DIR%spv"

del "%PROJECT_DIR%spv\frag.spv"
del "%PROJECT_DIR%spv\vert.spv"
del "%PROJECT_DIR%spv\textureMapFrag.spv"
del "%PROJECT_DIR%spv\textureMapVert.spv"
del "%PROJECT_DIR%spv\primitiveFrag.spv"
del "%PROJECT_DIR%spv\primitiveVert.spv"
del "%PROJECT_DIR%spv\lightFrag.spv"
del "%PROJECT_DIR%spv\lightVert.spv"
del "%PROJECT_DIR%spv\skyboxFrag.spv"
del "%PROJECT_DIR%spv\skyboxVert.spv"
del "%PROJECT_DIR%spv\raytraceRaygen.spv"
del "%PROJECT_DIR%spv\raytraceMiss.spv"
del "%PROJECT_DIR%spv\raytraceChit.spv"
del "%PROJECT_DIR%spv\raytraceAhit.spv"
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%shader.vert" -o "%PROJECT_DIR%spv\vert.spv" || (echo failed to compile "shader.vert" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%shader.frag" -o "%PROJECT_DIR%spv\frag.spv" || (echo failed to compile "shader.frag" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%textureMapVert.vert" -o "%PROJECT_DIR%spv\textureMapVert.spv" || (echo failed to compile "textureMapVert.vert" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%textureMapFrag.frag" -o "%PROJECT_DIR%spv\textureMapFrag.spv" || (echo failed to compile "textureMapFrag.frag" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%primitive.vert" -o "%PROJECT_DIR%spv\primitiveVert.spv" || (echo failed to compile "primitive.vert" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%primitive.frag" -o "%PROJECT_DIR%spv\primitiveFrag.spv" || (echo failed to compile "primitive.frag" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%light.vert" -o "%PROJECT_DIR%spv\lightVert.spv" || (echo failed to compile "light.vert" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%light.frag" -o "%PROJECT_DIR%spv\lightFrag.spv" || (echo failed to compile "light.frag" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%skyboxVert.vert" -o "%PROJECT_DIR%spv\skyboxVert.spv" || (echo failed to compile "skyboxVert.vert" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe "%PROJECT_DIR%skyboxFrag.frag" -o "%PROJECT_DIR%spv\skyboxFrag.spv" || (echo failed to compile "skyboxFrag.frag" && exit)

C:\VulkanSDK\1.4.304.1\Bin\glslc.exe --target-env=vulkan1.3 "%PROJECT_DIR%raytrace.rgen" -o "%PROJECT_DIR%spv\raytraceRaygen.spv" || (echo failed to compile "raytrace.rgen" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe --target-env=vulkan1.3 "%PROJECT_DIR%raytrace.rmiss" -o "%PROJECT_DIR%spv\raytraceMiss.spv" || (echo failed to compile "raytrace.rmiss" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe --target-env=vulkan1.3 "%PROJECT_DIR%raytrace.rchit" -o "%PROJECT_DIR%spv\raytraceChit.spv" || (echo failed to compile "raytrace.rchit" && exit)
C:\VulkanSDK\1.4.304.1\Bin\glslc.exe --target-env=vulkan1.3 "%PROJECT_DIR%raytrace.rahit" -o "%PROJECT_DIR%spv\raytraceAhit.spv" || (echo failed to compile "raytrace.rany" && exit)

echo Shaders compiled
pause