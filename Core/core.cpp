// Core.cpp
#include "./pch.hpp"
#include <Digiplane/core.hpp>

namespace _DPCORE {
    // current digiplane version in major.minor format
    constexpr float version = DIGIPLANE_MAJOR + 0.1f * DIGIPLANE_MINOR;
    std::string_view systemLanguage;
    
};

namespace Digiplane {
    /* Application Context definitions */ 

    // ApplicationContext constructor
    ApplicationContext::ApplicationContext(const char* title, int width, int height) {
        m_title = title;
        m_width = width;
        m_height = height;

        lua_State* L = luaL_newstate();
        luaL_openlibs(L);

        // add camera entity
        flecs::entity camera = m_world.entity("Camera").set<Transform>({
            
            {7,-7,5},                                          // vec3 position
            {0.4840999f, -0.2005207f, 0.3259418f, 0.7868931f}, // Quaternion rotation
            {1,1,1}                                            // vec3 scale
        }).add<Viewport>();
    }

    ApplicationContext::~ApplicationContext() {
        if (m_pImmediateContext)
            m_pImmediateContext->Flush();

        m_pSwapChain        = nullptr;
        m_pImmediateContext = nullptr;
        m_pDevice           = nullptr;
        
        if (m_window)
        {
            glfwDestroyWindow(m_window);
            glfwTerminate();
        }
    }

    int ApplicationContext::init() {
        if (!glfwInit())
            return -1;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // create window with "XD" added to the title
        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

        glfwMakeContextCurrent(m_window);
        
        #if PLATFORM_WIN32
             Diligent::Win32NativeWindow Window{glfwGetWin32Window(m_window)};
             if (m_DeviceType == Diligent::RENDER_DEVICE_TYPE_UNDEFINED)
                #if D3D12_SUPPORTED
                m_DeviceType = Diligent::RENDER_DEVICE_TYPE_D3D12;
                #elif VULKAN_SUPPORTED
                m_DeviceType = Diligent::RENDER_DEVICE_TYPE_VULKAN;
                #endif
        #endif
        #if PLATFORM_LINUX
            LinuxNativeWindow Window;
            Window.WindowId = glfwGetX11Window(m_window);
            Window.pDisplay = glfwGetX11Display();
        #endif

        assert(m_DeviceType != Diligent::RENDER_DEVICE_TYPE_UNDEFINED);

        Diligent::SwapChainDesc SCDesc;
        switch (m_DeviceType)
        {
            case Diligent::RENDER_DEVICE_TYPE_D3D12:
            {
                Diligent::EngineD3D12CreateInfo EngineCI;

                auto* pFactoryD3D12 = Diligent::GetEngineFactoryD3D12();
                pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, &m_pImmediateContext);
                pFactoryD3D12->CreateSwapChainD3D12(m_pDevice, m_pImmediateContext, SCDesc, Diligent::FullScreenModeDesc{}, Window, &m_pSwapChain);
            }
            break;
        }
        return 0;
    }

    void ApplicationContext::update() {
        glfwPollEvents();
    }

    bool ApplicationContext::processCmdArg(const char* argv) {
        std::string arg = argv;
        if (arg == "-dp_vulkan") {
            #ifndef VULKAN_SUPPORTED
            return 1; // invalid digiplane command
            #endif
            m_DeviceType = Diligent::RENDER_DEVICE_TYPE_VULKAN;
        } else if (arg == "-dp_dx12") {
            #ifndef D3D12_SUPPORTED
            return 1; // invalid digiplane command
            #endif
            m_DeviceType = Diligent::RENDER_DEVICE_TYPE_D3D12;
        }
        else if (arg.find("-dp") == 0 && arg != "-dp_vulkan" && arg != "-dp_dx12") {return 1;} // invalid digiplane command
        return 0; // ignored/valid command
    }

    /* Core interfacing definitions */

    const float& getVersion() {return _DPCORE::version;}

}
