

newoption {
   trigger     = "device",
   value       = "DESKTOP|HANDHELD|IOT",
   description = "target device",
   default     = "DESKTOP",
   allowed = {
      { "DESKTOP",   "Desktop machine" },
      { "HANDHELD",  "Mobile device" },
      { "IOT",       "IoT device" }
   }
}

defines { "DEVICE=" .. _OPTIONS["device"] }
defines { "DEVICE_" .. _OPTIONS["device"] }


function osabbr() 
  return _TARGET_OS
end

workspace "sciter.sdk"

  configurations { "Debug", "Release" }

  cppdialect "C++17" 
  cdialect "C11"


  staticruntime "On"

  -- -- location "build"
  filter "system:windows"
    defines "WINDOWS"  
    location ("build/" .. osabbr())
    links { "shell32", "advapi32", "ole32", "oleaut32", "comdlg32" }
    platforms { "x86", "x64", "arm64" }
    systemversion "latest"
    buildoptions { "/experimental:c11atomics" }

  filter "system:macosx"
    defines "OSX"  
    location ("build/" .. osabbr())
    targetdir ("bin/" .. osabbr())
    links { "CoreFoundation.framework", "Cocoa.framework", "IOKit.framework" }
    platforms { "x64" }
      
  filter { "system:linux" }
    defines "LINUX"  
    location("build/" .. osabbr())
    platforms { "x64", "arm64" }
    defines { "_GNU_SOURCE" }
    buildoptions {
     "-fPIC",
     "-Wno-unknown-pragmas",
     "-Wno-write-strings",
     "-Wno-nonnull",     
     "-ldl",
    }
    linkoptions { 
      "-fPIC",
      "-pthread",
    }


  filter {}

  includedirs { "include" }  

  --flags { "MultiProcessorCompile" }

  filter "platforms:x86"
    architecture "x86"
  filter "platforms:x64"
    architecture "x86_64"  
  filter "platforms:arm64"
    architecture "ARM64"  
  filter "platforms:arm32"
    architecture "ARM"  


  filter {"platforms:x86", "system:windows"}
    defines { "WIN32" }
  filter {"platforms:x64", "system:windows"}
    defines { "WIN32","WIN64" }      
  filter {"platforms:arm64", "system:windows"}
    defines { "WIN32","WIN64", "ARM64" }      

  filter "configurations:Debug*"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"

  filter "configurations:Release*"
    defines { "NDEBUG"}  
    optimize "Size"
    symbols "Off"
--    flags { "LinkTimeOptimization" }

  filter {"system:windows"}
    defines { "_CRT_SECURE_NO_WARNINGS" } 
 
  filter {}

project "usciter"
  kind "WindowedApp"
  entrypoint "wmainCRTStartup"

  targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")

  language "C++"

  dpiawareness "HighPerMonitor"

  files { "include/sciter-main.cpp" }

  files { "demos/usciter/usciter.cpp",
          "include/behaviors/behavior_drawing.cpp",
          "include/behaviors/behavior_drawing_opengl.cpp",
          "include/behaviors/behavior_video_generator.cpp",
          "include/behaviors/behavior_video_generator_full.cpp",
          "include/behaviors/behavior_video_generator_direct.cpp",
          "sciter-sqlite/*.h",
          "sciter-sqlite/*.cpp",
          "sciter-sqlite/sqlite-wrap.c",
          "include/behaviors/behavior_video_generator.cpp",
          "include/behaviors/behavior_video_generator_full.cpp",
          "include/behaviors/behavior_video_generator_direct.cpp",
          "include/gles/sciter-gles.*"
        }

  filter "system:windows"
    files {"include/sciter-*.h",
           "include/sciter-*.hpp",
           "include/aux-*.*",
           "include/behaviors/behavior_native_textarea.cpp",
           "include/behaviors/behavior_camera_capture.cpp",
           "include/behaviors/behavior_tabs.cpp",
           "demos/usciter/win-res/usciter.rc",
           "demos/usciter/win-res/dpi-aware.manifest" }
    links { "shell32", "advapi32", "ole32", "oleaut32", "gdi32", "comdlg32" }
    --prebuildcommands { 
    --  "\"%{prj.location}..\\..\\bin\\".. osabbr() .. "\\packfolder.exe\" \"%{prj.location}..\\..\\demos\\usciter\\res\" \"%{prj.location}..\\..\\demos\\usciter\\resources.cpp\" -v \"resources\""
    --}

  filter "system:macosx"
    xcodebuildsettings { ["GENERATE_INFOPLIST_FILE"] = "YES" }
    xcodebuildsettings { ["MACOSX_DEPLOYMENT_TARGET"] = "10.13" }
    targetdir ("bin/" .. osabbr())

  filter { "system:linux" }
    linkoptions { 
       "-fPIC",
       "-pthread",
       "-Wl,--no-undefined",
       "-ldl",
       "-no-pie"
    }

  filter {}

project "gsciter"
  kind "WindowedApp"
  language "C++"
  entrypoint "wmainCRTStartup"

  dpiawareness "HighPerMonitor"

  files { "demos/gsciter/gsciter.cpp" }

  targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")

  files { "include/sciter-main.cpp" }

  files {"include/sciter-*.h",
         "include/sciter-*.hpp",
         "include/aux-*.*",
         "include/behaviors/behavior_video_generator_full.cpp",
         "demos/gsciter/**.h",
         "demos/gsciter/**.cpp",
    }

  filter "system:windows"

    files {"demos/gsciter/win-res/gsciter.rc",
           "demos/gsciter/win-res/dpi-aware.manifest" }

    links { "shell32", "advapi32", "ole32", "oleaut32", "gdi32", "comdlg32" }
    --prebuildcommands { 
    --  "\"%{prj.location}..\\..\\bin\\".. osabbr() .. "\\packfolder.exe\" \"%{prj.location}..\\..\\demos\\gsciter\\res\" \"%{prj.location}..\\..\\demos\\usciter\\resources.cpp\" -v \"resources\""
    --}

  filter "system:macosx"
    xcodebuildsettings { ["GENERATE_INFOPLIST_FILE"] = "YES" }
    xcodebuildsettings { ["MACOSX_DEPLOYMENT_TARGET"] = "10.13" }
    targetdir ("bin/" .. osabbr())
    
  filter "system:linux"
    linkoptions { 
       "-fPIC",
       "-pthread",
       "-Wl,--no-undefined",
       "-ldl",
       "-no-pie"
    }
  filter {}


project "inspector"
  kind "WindowedApp"
  language "C++"

  targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")
  entrypoint "wmainCRTStartup"

  dpiawareness "HighPerMonitor"

  files { "include/sciter-main.cpp" }
  files { "demos/inspector/inspector.cpp" }

  filter "system:windows"
    removeconfigurations { "*skia" }
    files {"include/sciter-*.h",
           "include/sciter-*.hpp",
           "include/aux-*.*",
           "demos/inspector/win-res/inspector.rc",
           "demos/inspector/win-res/dpi-aware.manifest" }
    --prebuildcommands { 
    --  "\"%{prj.location}..\\..\\bin\\".. osabbr() .. "\\packfolder.exe\" \"%{prj.location}..\\..\\demos\\inspector\\res\" \"%{prj.location}..\\..\\demos\\inspector\\resources.cpp\" -v \"resources\""
    --}

  filter "system:macosx"
    xcodebuildsettings { ["GENERATE_INFOPLIST_FILE"] = "YES" }
    xcodebuildsettings { ["MACOSX_DEPLOYMENT_TARGET"] = "10.13" }
    targetdir ("bin/" .. osabbr())
  filter { "system:linux" }
    linkoptions { 
       "-fPIC",
       "-pthread",
       "-Wl,--no-undefined",
       "-ldl",
       "-no-pie"
    }

  filter {}

project "integration"
  kind "WindowedApp"
  language "C++"

  targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")
  dpiawareness "HighPerMonitor"

  entrypoint "wmainCRTStartup"
  files { "include/sciter-main.cpp" }

  files { "demos/integration/frame.cpp" }

  filter "system:windows"
    files {"include/sciter-*.h",
           "include/sciter-*.hpp",
           "include/aux-*.*",
           "demos/integration/win-res/integration.rc",
           "demos/integration/win-res/dpi-aware.manifest" }
    --prebuildcommands { 
    --  "\"%{prj.location}..\\..\\bin\\".. osabbr() .. "\\packfolder.exe\" \"%{prj.location}..\\..\\demos\\integration\\res\" \"%{prj.location}..\\..\\demos\\integration\\resources.cpp\" -v \"resources\""
    --}

  filter "system:macosx"
    xcodebuildsettings { ["GENERATE_INFOPLIST_FILE"] = "YES" }
    xcodebuildsettings { ["MACOSX_DEPLOYMENT_TARGET"] = "10.13" }
    targetdir ("bin/" .. osabbr())

  filter { "system:linux", "options:gtk=v3" }
    linkoptions { 
       "-fPIC",
       "-pthread",
       "-Wl,--no-undefined",
       "-ldl",
    }

    filter {}

-- sciter extension library - SQLite
project "sciter-sqlite"

  kind "SharedLib"
  language "C++"

  targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")

  targetprefix "" -- do not prepend it with "lib..."

  files { "sciter-sqlite/*.h",
          "sciter-sqlite/*.cpp",
          "sciter-sqlite/sqlite-wrap.c" }

  filter "system:windows"
    files {"sciter-sqlite/sciter-sqlite.def" }
  filter {}

  filter "system:macosx"
    targetdir ("bin/" .. osabbr())


-- sciter extension behavior library - native UI component
project "sciter-component"

  kind "SharedLib"
  language "C++"

  targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")
  targetprefix "" -- do not prepend it with "lib..."

  files { "demos/sciter-component/*.h",
          "demos/sciter-component/*.cpp"}

  filter "system:windows"
    files {"demos/sciter-component/exports.def" }
  filter {}

--end

-- sciter extension library - WebView
project "sciter-webview"

  kind "SharedLib"
  language "C++"

  targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")

  targetprefix "" -- do not prepend it with "lib..."

  defines "SCITERWEBVIEW_EXPORTS"

  files { 
    "include/sciter-*.h",
    "include/sciter-*.hpp",
    "include/aux-*.*",
    "sciter-webview/behavior_webview.cpp"
   }

  filter "system:windows"
    files {
      "sciter-webview/webview/win/exports.def", 
      "sciter-webview/webview/win/dllmain.cpp", 
      "sciter-webview/webview/sciter_winwebview.cpp",
      "sciter-webview/webview/sciter_iewebview.cpp",
      "sciter-webview/webview/sciter_edgewebview.cpp",
    }

  filter "system:macosx"
    xcodebuildsettings { ["CLANG_ENABLE_OBJC_ARC"] = "YES" }

    files {
      "sciter-webview/webview/sciter_wkwebview.*", 
    } 
    links { "WebKit.framework",
            "AppKit.framework" }
    targetdir ("bin/" .. osabbr())


  filter { "system:linux" } 
    files {
      "sciter-webview/webview/sciter_webkitgtk.*", 
    }
    buildoptions {
       "`pkg-config gtk+-3.0 --cflags`",
       "`pkg-config webkit2gtk-4.0 --cflags`"
    }
    linkoptions { 
       "`pkg-config gtk+-3.0 --libs`",
       "`pkg-config webkit2gtk-4.0 --libs`",
       "-fPIC",
       "-pthread",
       "-Wl,--no-undefined",
       "-ldl"
    }
  filter {}    


if _TARGET_OS == "windows" then

  project "window-mixin"

    kind "WindowedApp"
    language "C++"

    dpiawareness "HighPerMonitor"

    targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")

    files { "demos/window-mixin/*.h",
            "demos/window-mixin/*.cpp",
            "demos/window-mixin/*.rc",
            "demos/window-mixin/*.ico",
            "demos/window-mixin/dpi-aware.manifest",
            "demos/window-mixin/res/*.*" }

    files {"include/sciter-*.h",
           "include/sciter-*.hpp",
           "include/aux-*.*"}

end

project "lite-sciter"
  --kind "ConsoleApp"
  kind "WindowedApp"
  language "C++"

  defines "WINDOWLESS"

  cppdialect "C++17" 

  targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")

  dpiawareness "HighPerMonitor"

  filter "system:windows"

    --externalincludedirs { os.getenv("DXSDK_DIR") .. "Include" }
    --libdirs { os.getenv("DXSDK_DIR") .. "Lib/x64" }

    prebuildcommands { 
      "\"%{prj.location}..\\..\\bin\\windows\\packfolder.exe\" \"%{prj.location}..\\..\\demos.lite\\lite-sciter\\facade\" \"%{prj.location}..\\..\\demos.lite\\lite-sciter\\facade-resources.cpp\" -v \"resources\""
    }
  filter {}

  -- ours:
  files { 
    "include/*.h", 
    "include/*.hpp",
    "demos.lite/lite-sciter/*.h",
    "demos.lite/lite-sciter/main.cpp",
    "include/behaviors/behavior_drawing.cpp"
  }
  
  -- theirs, GLFW:
  includedirs { 
    "demos.lite/lite-sciter",
    "demos.lite/lite-sciter/glfw/include",
    "demos.lite/lite-sciter/glfw/deps" }  

  files { 
    "demos.lite/lite-sciter/glfw/src/context.c",
    "demos.lite/lite-sciter/glfw/src/init.c",
    "demos.lite/lite-sciter/glfw/src/input.c",
    "demos.lite/lite-sciter/glfw/src/monitor.c",
    "demos.lite/lite-sciter/glfw/src/vulkan.c",
    "demos.lite/lite-sciter/glfw/src/window.c",
    "demos.lite/lite-sciter/glfw/deps/glad.c",
  }

  filter "system:windows"
    entrypoint "mainCRTStartup"
    dpiawareness "HighPerMonitor"
    defines "_GLFW_WIN32"
    files {
      "demos.lite/lite-sciter/glfw/src/win32_init.c",
      "demos.lite/lite-sciter/glfw/src/win32_joystick.c",
      "demos.lite/lite-sciter/glfw/src/win32_monitor.c",
      "demos.lite/lite-sciter/glfw/src/win32_time.c",
      "demos.lite/lite-sciter/glfw/src/win32_thread.c",
      "demos.lite/lite-sciter/glfw/src/win32_window.c",
      "demos.lite/lite-sciter/glfw/src/wgl_context.c",
      "demos.lite/lite-sciter/glfw/src/egl_context.c",
      "demos.lite/lite-sciter/glfw/src/osmesa_context.c",
    }
    links "shlwapi"
  filter "system:macosx"
    defines "_GLFW_COCOA"
    files {
      "demos.lite/lite-sciter/glfw/src/cocoa_platform.h",
      "demos.lite/lite-sciter/glfw/src/cocoa_joystick.h",
      "demos.lite/lite-sciter/glfw/src/posix_thread.h",
      "demos.lite/lite-sciter/glfw/src/nsgl_context.h", 
      "demos.lite/lite-sciter/glfw/src/egl_context.h",
      "demos.lite/lite-sciter/glfw/src/osmesa_context.h",
      "demos.lite/lite-sciter/glfw/src/cocoa_init.m",
      "demos.lite/lite-sciter/glfw/src/cocoa_joystick.m",
      "demos.lite/lite-sciter/glfw/src/cocoa_monitor.m",
      "demos.lite/lite-sciter/glfw/src/cocoa_window.m",
      "demos.lite/lite-sciter/glfw/src/cocoa_time.c",
      "demos.lite/lite-sciter/glfw/src/posix_thread.c",
      "demos.lite/lite-sciter/glfw/src/nsgl_context.m",
      "demos.lite/lite-sciter/glfw/src/egl_context.c",
      "demos.lite/lite-sciter/glfw/src/osmesa_context.c",
    } 
    links { "CoreVideo.framework" }
    targetdir ("bin/" .. osabbr())   
    
  filter "system:linux"  
    linkoptions { 
      "-Wall", 
      "-pthread", "-lm", 
      "-lX11","-lXrandr","-lXinerama", "-lXcursor",
      "-lGL", "-lGLU", 
      "-lGLESv2","-lEGL",
      "-ldl" }
      
    defines { "_GLFW_X11", "_GLFW_WAYLAND" } -- "_GLFW_MIR"
    
    includedirs { "demos.lite/lite-sciter/glfw/src/wayland" }  
    
    files {
      "demos.lite/lite-sciter/glfw/src/xkb_unicode.c",
      "demos.lite/lite-sciter/glfw/src/glx_context.c",
      "demos.lite/lite-sciter/glfw/src/egl_context.c",
      "demos.lite/lite-sciter/glfw/src/osmesa_context.c",
      "demos.lite/lite-sciter/glfw/src/posix_thread.*",
      "demos.lite/lite-sciter/glfw/src/posix_time.*",
      "demos.lite/lite-sciter/glfw/src/linux_*.*",
      "demos.lite/lite-sciter/glfw/src/x11_*.*",
      "demos.lite/lite-sciter/glfw/src/wayland/*.h",
      "demos.lite/lite-sciter/glfw/src/wl_*.h*",
      "demos.lite/lite-sciter/glfw/src/wl_*.cpp",
    }
    
    
  filter {}

if _TARGET_OS == "windows" then
  project "lite-bitmap"
    --kind "ConsoleApp"
    kind "WindowedApp"
    language "C++"

    defines "WINDOWLESS"

    cppdialect "C++17" 

    targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")

    dpiawareness "HighPerMonitor"

    files { 
      "include/*.h", 
      "include/*.hpp",
      "demos.lite/lite-bitmap/*.h",
      "demos.lite/lite-bitmap/main.cpp",
    }
    
    includedirs { 
      "demos.lite/lite-bitmap",
    }

end

include "demos.lite/lite-sdl/premake-sdl.lua"

project "lite-sciter-sdl"
  --kind "ConsoleApp"
  kind "WindowedApp"
  language "C++"

  defines "WINDOWLESS"

  cppdialect "C++17" 

  targetdir ("bin/" .. osabbr() .. "/%{cfg.platform}")

  dpiawareness "HighPerMonitor"

  filter {}

  -- ours:
  files { 
    "include/*.h", 
    "include/*.hpp",
    "demos.lite/lite-sdl/*.h",
    "demos.lite/lite-sdl/raster/*.h",
    "demos.lite/lite-sdl/raster/main.cpp",
  }
  
  -- theirs, SDL:
  include_files_sdl() 

--include "ffmpeg/premake5.lua"
