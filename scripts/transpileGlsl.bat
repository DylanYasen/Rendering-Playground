set outputPath=%1

if not exist %outputPath% MKDIR %outputPath%
%VULKAN_SDK%/Bin/glslc.exe ./src/shader.vert -o %outputPath%/vert.spv
%VULKAN_SDK%/Bin/glslc.exe ./src/shader.frag -o %outputPath%/frag.spv
