SET( LIBRARY_VERSION "0.1.0" )

SET( HEADER_PATH ${PROJECT_SOURCE_DIR}/include/osre/ )
add_definitions( -DOSRE_BUILD_EXPORT )

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
    ${HEADER_PATH}/include
    ../../3dparty/cppcore/include
    ../../3dparty/glew/include
    ../../3dparty/glm/
    ../../3dparty/unzip/
    ../../3dparty/zlib/
    ../../3dparty/assimp/include
)

# For Vulkan support
if( NOT USE_PLATFORM MATCHES "VK_USE_PLATFORM_.*" )
	if( WIN32 )
		set( USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR" )
	elseif( UNIX )
		set( USE_PLATFORM "VK_USE_PLATFORM_XCB_KHR" )
	endif()
endif()

IF( WIN32 )
    SET( platform_libs comctl32.lib Winmm.lib opengl32.lib glu32.lib SDL2 )
ELSE( WIN32 )
    SET( platform_libs SDL2 )
ENDIF( WIN32 )

#==============================================================================
# App
#==============================================================================
SET( app_inc
    ${HEADER_PATH}/App/AppCommon.h
    ${HEADER_PATH}/App/AppBase.h
    ${HEADER_PATH}/App/ServiceProvider.h
    ${HEADER_PATH}/App/TAbstractCtrlBase.h
)

SET( app_src
    App/AppBase.cpp
    App/ServiceProvider.cpp
)

#==============================================================================
# Assets
#==============================================================================
SET( assets_inc
    ${HEADER_PATH}/Assets/AssetsCommon.h
    ${HEADER_PATH}/Assets/AssetRegistry.h
    ${HEADER_PATH}/Assets/AssetDataArchive.h
    ${HEADER_PATH}/Assets/AssimpWrapper.h
    ${HEADER_PATH}/Assets/Model.h
)
SET( assets_src
    Assets/AssetRegistry.cpp
    Assets/AssetDataArchive.cpp
    Assets/AssimpWrapper.cpp
    Assets/Model.cpp
)

#==============================================================================
# Common
#==============================================================================
SET( common_inc
	${HEADER_PATH}/Common/Common.h
    ${HEADER_PATH}/Common/TCommand.h
    ${HEADER_PATH}/Common/AbstractCodec.h
    ${HEADER_PATH}/Common/AbstractEventHandler.h
    ${HEADER_PATH}/Common/AbstractProcessor.h
    ${HEADER_PATH}/Common/ArgumentParser.h
    ${HEADER_PATH}/Common/AbstractService.h
    ${HEADER_PATH}/Common/CodecRegistry.h
    ${HEADER_PATH}/Common/ColorRGBA.h
    ${HEADER_PATH}/Common/DateTime.h
    ${HEADER_PATH}/Common/Event.h
    ${HEADER_PATH}/Common/EventTriggerer.h
    ${HEADER_PATH}/Common/Ids.h
    ${HEADER_PATH}/Common/Logger.h
    ${HEADER_PATH}/Common/Object.h
    ${HEADER_PATH}/Common/StringUtils.h
    ${HEADER_PATH}/Common/TFunctor.h
	${HEADER_PATH}/Common/TResourceCache.h
    ${HEADER_PATH}/Common/TObjPtr.h
    ${HEADER_PATH}/Common/Tokenizer.h
    ${HEADER_PATH}/Common/osre_common.h
)
SET( common_src
    Common/AbstractCodec.cpp
    Common/ArgumentParser.cpp
    Common/CodecRegistry.cpp
    Common/DateTime.cpp
    Common/Event.cpp
    Common/EventTriggerer.cpp
    Common/Ids.cpp
    Common/Logger.cpp
    Common/Object.cpp
    Common/Tokenizer.cpp
)

#==============================================================================
# Collision
#==============================================================================
SET( collision_inc
    ${HEADER_PATH}/Collision/CollisionCommon.h
    ${HEADER_PATH}/Collision/GeometryProcessor.h
    ${HEADER_PATH}/Collision/TAABB.h
    ${HEADER_PATH}/Collision/TQuadTree.h
    ${HEADER_PATH}/Collision/TRay.h
)
SET( collision_src
    Collision/GeometryProcessor.cpp
)

#==============================================================================
# Debugging
#==============================================================================
SET( debugging_inc
    ${HEADER_PATH}/Debugging/AssertHandler.h
    ${HEADER_PATH}/Debugging/osre_debugging.h
)
SET( debugging_src
    Debugging/AssertHandler.cpp
)

#==============================================================================
# Documentation
#==============================================================================
SET( doc_inc
    ${CMAKE_HOME_DIRECTORY}/doc/Doxygen/osre_doc.h
)

#==============================================================================
# IO
#==============================================================================
SET( io_src
    IO/Directory.cpp
    IO/FileStream.cpp
    IO/FileStream.h
    IO/ImageCodec.h
    IO/ImageCodec.cpp
    IO/IOService.cpp
    IO/LocaleFileSystem.cpp
    IO/LocaleFileSystem.h
    IO/MemoryStream.h
    IO/Stream.cpp
    IO/Uri.cpp
    IO/ZipFileSystem.cpp
    IO/ZipFileSystem.h
    IO/ZipFileStream.cpp
    IO/ZipFileStream.h
    IO/IOSystemInfo.cpp
)
SET( io_inc
    ${HEADER_PATH}/IO/IOCommon.h
    ${HEADER_PATH}/IO/Directory.h
    ${HEADER_PATH}/IO/Stream.h
    ${HEADER_PATH}/IO/AbstractFileSystem.h
    ${HEADER_PATH}/IO/IOService.h
    ${HEADER_PATH}/IO/IOSystemInfo.h
    ${HEADER_PATH}/IO/Uri.h
)

#==============================================================================
# Math
#==============================================================================
SET( math_src
    Math/BaseMath.cpp
)
SET( math_inc
    ${HEADER_PATH}/Math/BaseMath.h
)

#==============================================================================
# Platform
#==============================================================================
SET( platform_inc
    ${HEADER_PATH}/Platform/PlatformCommon.h
    ${HEADER_PATH}/Platform/AbstractOSService.h
    ${HEADER_PATH}/Platform/AbstractThread.h
    ${HEADER_PATH}/Platform/AbstractThreadEvent.h
    ${HEADER_PATH}/Platform/AbstractThreadFactory.h
    ${HEADER_PATH}/Platform/AbstractThreadLocalStorage.h
    ${HEADER_PATH}/Platform/AbstractCriticalSection.h
    ${HEADER_PATH}/Platform/AtomicInt.h
    ${HEADER_PATH}/Platform/AbstractPlatformEventQueue.h
    ${HEADER_PATH}/Platform/AbstractRenderContext.h
    ${HEADER_PATH}/Platform/AbstractWindow.h
    ${HEADER_PATH}/Platform/AbstractTimer.h
	${HEADER_PATH}/Platform/AbstractSystemInfo.h
    ${HEADER_PATH}/Platform/CPUInfo.h
    ${HEADER_PATH}/Platform/SystemInfo.h
    ${HEADER_PATH}/Platform/SystemUtils.h
    ${HEADER_PATH}/Platform/KeyTypes.h
    ${HEADER_PATH}/Platform/PluginType.h
    ${HEADER_PATH}/Platform/PlatformInterface.h
    ${HEADER_PATH}/Platform/PlatformOperations.h
)
SET( platform_win_inc
	${HEADER_PATH}/Platform/Windows/MinWindows.h
)
SET( platform_src
    Platform/CPUInfo.cpp
    Platform/SystemInfo.cpp
    Platform/SystemUtils.cpp
    Platform/AtomicInt.cpp
    Platform/AbstractWindow.cpp
    Platform/PlatformInterface.cpp
    Platform/PlatformPluginFactory.cpp
    Platform/PlatformPluginFactory.h
    Platform/AbstractDynamicLoader.cpp
    Platform/PlatformOperations.cpp
)
SET( platform_win32_src
	Platform/win32/Win32SystemInfo.h
    Platform/win32/Win32Atomic.h
    Platform/win32/Win32DbgLogStream.h
    Platform/win32/Win32Window.cpp
    Platform/win32/Win32Window.h
    Platform/win32/Win32EventQueue.cpp
    Platform/win32/Win32EventQueue.h
    Platform/win32/Win32RenderContext.cpp
    Platform/win32/Win32RenderContext.h
    Platform/win32/Win32Timer.cpp
    Platform/win32/Win32Timer.h
    Platform/win32/Win32CriticalSection.h
    Platform/win32/Win32Thread.cpp
    Platform/win32/Win32Thread.h
    Platform/win32/Win32ThreadEvent.cpp
    Platform/win32/Win32ThreadEvent.h
    Platform/win32/Win32ThreadFactory.cpp
    Platform/win32/Win32ThreadFactory.h
    Platform/win32/Win32Timer.cpp
    Platform/win32/Win32Timer.h
    Platform/win32/Win32DynamicLoader.cpp
    Platform/win32/Win32DynamicLoader.h
    Platform/win32/Win32OSService.cpp
    Platform/win32/Win32OSService.h
)
SET( platform_sdl2_src
    Platform/sdl2/SDL2Atomic.h
    Platform/sdl2/SDL2CriticalSection.h
    Platform/sdl2/SDL2EventQueue.cpp
    Platform/sdl2/SDL2EventQueue.h
    Platform/sdl2/SDL2Initializer.cpp
    Platform/sdl2/SDL2Initializer.h
    Platform/sdl2/SDL2Window.cpp
    Platform/sdl2/SDL2Window.h
    Platform/sdl2/SDL2Timer.cpp
    Platform/sdl2/SDL2Timer.h
    Platform/sdl2/SDL2RenderContext.cpp
    Platform/sdl2/SDL2RenderContext.h
    Platform/sdl2/SDL2Thread.cpp
    Platform/sdl2/SDL2Thread.h
    Platform/sdl2/SDL2ThreadEvent.cpp
    Platform/sdl2/SDL2ThreadEvent.h
    Platform/sdl2/SDL2ThreadFactory.cpp
    Platform/sdl2/SDL2ThreadFactory.h
    Platform/sdl2/SDL2ThreadLocalStorage.h
    Platform/sdl2/SDL2DynamicLoader.cpp
    Platform/sdl2/SDL2DynamicLoader.h
	Platform/sdl2/SDL2SystemInfo.h
    Platform/sdl2/SDL2OSService.h
    Platform/sdl2/SDL2OSService.cpp
)

IF( WIN32 )
    SET( platform_impl_src ${platform_win32_src} )
ENDIF( WIN32 )

#==============================================================================
# Profiling
#==============================================================================
SET( profiling_inc
    ${HEADER_PATH}/Profiling/ProfilingCommon.h
    ${HEADER_PATH}/Profiling/FPSCounter.h
    ${HEADER_PATH}/Profiling/PerformanceCounterRegistry.h
)
SET( profiling_src
    Profiling/FPSCounter.cpp
    Profiling/PerformanceCounterRegistry.cpp
)

#==============================================================================
# Properties
#==============================================================================
SET ( properties_inc
    ${HEADER_PATH}/Properties/PropertiesCommon.h
    ${HEADER_PATH}/Properties/BasePropertyMap.h
    ${HEADER_PATH}/Properties/AbstractPropertyMap.h
    ${HEADER_PATH}/Properties/Property.h
    ${HEADER_PATH}/Properties/Settings.h
)
SET ( properties_src
    Properties/BasePropertyMap.cpp
    Properties/Settings.cpp
    Properties/Property.cpp
)

#==============================================================================
# Scene
#==============================================================================
SET( scene_inc
    ${HEADER_PATH}/Scene/SceneCommon.h
    ${HEADER_PATH}/Scene/DbgRenderer.h
    ${HEADER_PATH}/Scene/Component.h
    ${HEADER_PATH}/Scene/GeometryBuilder.h
    ${HEADER_PATH}/Scene/MaterialBuilder.h
    ${HEADER_PATH}/Scene/Node.h
    ${HEADER_PATH}/Scene/Stage.h
    ${HEADER_PATH}/Scene/TrackBall.h
    ${HEADER_PATH}/Scene/View.h
    ${HEADER_PATH}/Scene/World.h
    ${HEADER_PATH}/Scene/LineBuilder.h
)
SET( scene_src
    Scene/DbgRenderer.cpp
    Scene/Component.cpp
    Scene/LineBuilder.cpp
    Scene/GeometryBuilder.cpp
    Scene/MaterialBuilder.cpp
    Scene/Node.cpp
    Scene/Stage.cpp
    Scene/TrackBall.cpp
    Scene/View.cpp
    Scene/World.cpp
)

#==============================================================================
# RenderBackend
#==============================================================================
SET( renderbackend_inc
    ${HEADER_PATH}/RenderBackend/RenderCommon.h
    ${HEADER_PATH}/RenderBackend/Mesh.h
    ${HEADER_PATH}/RenderBackend/FontBase.h
    ${HEADER_PATH}/RenderBackend/HWBufferManager.h
    ${HEADER_PATH}/RenderBackend/FontBase.h
    ${HEADER_PATH}/RenderBackend/Parameter.h
    ${HEADER_PATH}/RenderBackend/ParticleGenerator.h
    ${HEADER_PATH}/RenderBackend/Pipeline.h
    ${HEADER_PATH}/RenderBackend/RenderBackendService.h
	${HEADER_PATH}/RenderBackend/RenderStates.h
    ${HEADER_PATH}/RenderBackend/Shader.h
)
SET( renderbackend_src
    RenderBackend/Mesh.cpp
    RenderBackend/FontBase.cpp
    RenderBackend/RenderBackendService.cpp
    RenderBackend/RenderCommon.cpp
    RenderBackend/Parameter.cpp
    RenderBackend/Pipeline.cpp
    RenderBackend/ParticleGenerator.cpp
    RenderBackend/HWBufferManager.cpp
    RenderBackend/Shader.cpp
)
SET( renderbackend_oglrenderer_src
    RenderBackend/OGLRenderer/OGLCommon.h
    RenderBackend/OGLRenderer/OGLCommon.cpp
    RenderBackend/OGLRenderer/OGLEnum.cpp
    RenderBackend/OGLRenderer/OGLEnum.h
    RenderBackend/OGLRenderer/OGLRenderBackend.cpp
    RenderBackend/OGLRenderer/OGLRenderBackend.h
    RenderBackend/OGLRenderer/RenderCmdBuffer.cpp
    RenderBackend/OGLRenderer/RenderCmdBuffer.h
    RenderBackend/OGLRenderer/OGLRenderEventHandler.cpp
    RenderBackend/OGLRenderer/OGLRenderEventHandler.h
    RenderBackend/OGLRenderer/OGLShader.cpp
    RenderBackend/OGLRenderer/OGLShader.h
)
SET( renderbackend_vulkanrenderer_src
    RenderBackend/VulkanRenderer/VlkFunctions.h
    RenderBackend/VulkanRenderer/VlkFunctions.cpp
    RenderBackend/VulkanRenderer/VlkExportedFunctions.inl
    RenderBackend/VulkanRenderer/VlkCommon.h
    RenderBackend/VulkanRenderer/VlkRenderBackend.cpp
    RenderBackend/VulkanRenderer/VlkRenderBackend.h
    RenderBackend/VulkanRenderer/VlkRenderEventHandler.h
    RenderBackend/VulkanRenderer/VlkRenderEventHandler.cpp
)
IF( WIN32 )
    SET( renderbackend_dx11renderer_src
        RenderBackend/DX11Renderer/DX11Common.h
        RenderBackend/DX11Renderer/DX11Renderer.h
        RenderBackend/DX11Renderer/DX11Renderer.cpp
        RenderBackend/DX11Renderer/DX11RenderVEventHandler.h
        RenderBackend/DX11Renderer/DX11RenderVEventHandler.cpp
    )
ENDIF( WIN32 )

#==============================================================================
# Scripting with Mono
#==============================================================================
SET( scripting_mono_src
)
SET( scripting_mono_inc
	${HEADER_PATH}/Scripting/osre_scripting.h
)

#==============================================================================
# UI
#==============================================================================
SET( ui_src
    UI/FontManager.cpp
    UI/Image.cpp
    UI/Widget.cpp
    UI/UiRenderer.cpp
    UI/UIRenderUtils.cpp
    UI/UIRenderUtils.h
    UI/ButtonBase.cpp
    UI/Panel.cpp
    UI/Screen.cpp
    UI/TextBase.cpp
    UI/UiItemFactory.cpp
)
SET( ui_inc
    ${HEADER_PATH}/UI/UICommon.h
    ${HEADER_PATH}/UI/FontManager.h
	${HEADER_PATH}/UI/Image.h
    ${HEADER_PATH}/UI/Widget.h
    ${HEADER_PATH}/UI/ButtonBase.h
    ${HEADER_PATH}/UI/Panel.h
    ${HEADER_PATH}/UI/Screen.h
	${HEADER_PATH}/UI/TextBase.h
	${HEADER_PATH}/UI/UiRenderer.h
	${HEADER_PATH}/UI/UiItemFactory.h
)

#==============================================================================
# Threading
#==============================================================================
SET( threading_inc
    ${HEADER_PATH}/Threading/ThreadingCommon.h
    ${HEADER_PATH}/Threading/AbstractTask.h
    ${HEADER_PATH}/Threading/SystemTask.h
    ${HEADER_PATH}/Threading/TaskJob.h
    ${HEADER_PATH}/Threading/TAsyncQueue.h
)
SET( threading_src
    Threading/AbstractTask.cpp
    Threading/AbstractThreadFactory.cpp
    Threading/SystemTask.cpp
)

if( NOT USE_PLATFORM MATCHES "VK_USE_PLATFORM_.*" )
	if( WIN32 )
		set( USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR" )
	elseif( UNIX )
		set( USE_PLATFORM "VK_USE_PLATFORM_XCB_KHR" )
	endif()
endif()

#==============================================================================
# Defined Source groups
#==============================================================================
SOURCE_GROUP( App                 FILES ${app_src} )
SOURCE_GROUP( Assets              FILES ${assets_src} )
SOURCE_GROUP( Common              FILES ${common_src} )
SOURCE_GROUP( Components          FILES ${components_src} )
SOURCE_GROUP( Collision           FILES ${collision_src} )
SOURCE_GROUP( Debugging           FILES ${debugging_src} )
SOURCE_GROUP( Debugging\\Win32    FILES ${debugging_windows_src} )
SOURCE_GROUP( IO                  FILES ${io_src} )
SOURCE_GROUP( Math                FILES ${math_src} )
SOURCE_GROUP( Memory              FILES ${memory_src} )
SOURCE_GROUP( Platform            FILES ${platform_src} )
SOURCE_GROUP( Platform\\Win32     FILES ${platform_impl_src} )
SOURCE_GROUP( Platform\\sdl2      FILES ${platform_sdl2_src} )
SOURCE_GROUP( Profiling           FILES ${profiling_src} )
SOURCE_GROUP( Properties          FILES ${properties_src} )
SOURCE_GROUP( RenderBackend       FILES ${renderbackend_src} )
SOURCE_GROUP( RenderBackend\\OGLRenderer    FILES ${renderbackend_oglrenderer_src} )
SOURCE_GROUP( RenderBackend\\VulkanRenderer FILES ${renderbackend_vulkanrenderer_src} )
SOURCE_GROUP( RenderBackend\\DX11Renderer FILES ${renderbackend_dx11renderer_src} )
SOURCE_GROUP( Resources           FILES ${resources_src} )
SOURCE_GROUP( Scene               FILES ${scene_src} )
SOURCE_GROUP( Threading           FILES ${threading_src} )
SOURCE_GROUP( UI                  FILES ${ui_src} )
SOURCE_GROUP( Include\\App        FILES ${app_inc} )
SOURCE_GROUP( Include\\Assets     FILES ${assets_inc} )
SOURCE_GROUP( Include\\Debugging  FILES ${debugging_inc} )
SOURCE_GROUP( Include\\Common     FILES ${common_inc} )
SOURCE_GROUP( Include\\Components FILES ${components_inc} )
SOURCE_GROUP( Include\\Collision  FILES ${collision_inc} )
SOURCE_GROUP( Include\\Debugging  FILES ${debugging_inc} )
SOURCE_GROUP( Include\\System     FILES ${system_inc} )
SOURCE_GROUP( Include\\Utils      FILES ${utils_inc} )
SOURCE_GROUP( Include\\IO         FILES ${io_inc} )
SOURCE_GROUP( Include\\Math          FILES ${math_inc} )
SOURCE_GROUP( Include\\Memory        FILES ${memory_inc} )
SOURCE_GROUP( Include\\Properties    FILES ${properties_inc} )
SOURCE_GROUP( Include\\Platform      FILES ${platform_inc} )
SOURCE_GROUP( Include\\Platform\\Windows FILES ${platform_win_inc} )
SOURCE_GROUP( Include\\Profiling     FILES ${profiling_inc} )
SOURCE_GROUP( Include\\Resources     FILES ${resources_inc} )
SOURCE_GROUP( Include\\RenderBackend FILES ${renderbackend_inc} )
SOURCE_GROUP( Include\\Scene         FILES ${scene_inc} )
SOURCE_GROUP( Include\\Scripting     FILES ${scripting_mono_inc} )
SOURCE_GROUP( Include\\Threading     FILES ${threading_inc} )
SOURCE_GROUP( Include\\UI            FILES ${ui_inc} )

SOURCE_GROUP( Documentation          FILES ${doc_inc} )

add_definitions( -D${USE_PLATFORM} )

#==============================================================================
# Define the Infrastructure library
#==============================================================================
ADD_LIBRARY( osre SHARED
    ../../README.md
    ${app_inc}
    ${assets_inc}
    ${debugging_inc}
    ${doc_inc}
    ${common_inc}
    ${common_src}
    ${collision_inc}
    ${collision_src}
    ${components_inc}
    ${debugging_inc}
    ${system_inc}
    ${io_inc}
    ${math_inc}
    ${memory_inc}
    ${utils_inc}
    ${properties_inc}
    ${platform_inc}
	${platform_win_inc}
    ${profiling_inc}
    ${resources_inc}
    ${scene_inc}
	${scripting_mono_inc}
    ${threading_inc}
    ${renderbackend_inc}
    ${ui_inc}
    ${math_inc}

    ${app_src}
    ${assets_src}
    ${Common_src}
    ${components_src}
    ${debugging_src}
        ${debugging_windows_src}
    ${io_src}
    ${math_src}
    ${memory_src}
    ${system_src}
        ${system_impl_src}
    ${properties_src}
    ${profiling_src}
    ${platform_src}
        ${platform_impl_src}
        ${platform_sdl2_src}
    ${utils_src}
    ${resources_src}
    ${renderbackend_src}
        ${renderbackend_oglrenderer_src}
        ${renderbackend_vulkanrenderer_src}
        ${renderbackend_dx11renderer_src}
    ${scene_src}
    ${threading_src}
        ${threading_impl_src}
    ${ui_src}
)

target_link_libraries ( osre cppcore zlib Unzip glew soil jsoncpp_lib_static ${platform_libs} assimp )
set_target_properties(  osre PROPERTIES FOLDER Engine )
