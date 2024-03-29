// Core.cpp
#include "./pch.hpp"
#include "./include/Digiplane/digiplane.hpp"
#include "./core.hpp"

namespace Digiplane {

    //
    // Application Context definitions 
    //

    void ApplicationContext::GLFWResizeCallback(GLFWwindow* wnd, int w, int h) {
        auto* pSelf = static_cast<ApplicationContext*>(glfwGetWindowUserPointer(wnd));
        if (pSelf->m_pSwapChain != nullptr)
            pSelf->m_pSwapChain->Resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
        #if defined(DIGIPLANE_NK_PLUGIN)
        nk_diligent_resize(m_pNkDlgCtx, m_pImmediateContext, static_cast<uint32_t>(w), static_cast<uint32_t>(h));
        #endif
    }

    void ApplicationContext::Draw() {
        Diligent::ITextureView* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
        Diligent::ITextureView* pDSV = m_pSwapChain->GetDepthBufferDSV();
        m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Clear the back buffer
        const float ClearColor[4] = {0.01f, 0.01f, 0.01f, 1.0f};
        m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        m_pImmediateContext->Flush();
        m_pSwapChain->Present();

        #if defined(DIGIPLANE_NK_PLUGIN)
        nk_diligent_render(m_pNkDlgCtx, m_pImmediateContext, NK_ANTI_ALIASING_ON);
        #endif
    }

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

        #if defined(DIGIPLANE_NK_PLUGIN)
        nk_diligent_shutdown(m_pNkDlgCtx);
        #endif
        
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
        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

        glfwMakeContextCurrent(m_window);

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, &GLFWResizeCallback);
        glfwSetWindowSizeLimits(m_window, 320, 240, GLFW_DONT_CARE, GLFW_DONT_CARE);
        
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

        constexpr uint32_t NuklearMaxVBSize = 512 * 1024;
        constexpr uint32_t NuklearMaxIBSize = 128 * 1024;
        m_pNkDlgCtx = nk_diligent_init(m_pDevice, SCDesc.Width, SCDesc.Height, SCDesc.ColorBufferFormat, SCDesc.DepthBufferFormat, NuklearMaxVBSize, NuklearMaxIBSize);
        m_pNkCtx    = nk_diligent_get_nk_ctx(m_pNkDlgCtx);
        nk_font_atlas* atlas = nullptr;
        nk_diligent_font_stash_begin(m_pNkDlgCtx, &atlas);
        nk_diligent_font_stash_end(m_pNkDlgCtx, m_pImmediateContext);
        //set_style(m_pNkCtx, THEME_DARK);

        return 0;
    }

    void ApplicationContext::update() {

        if (glfwWindowShouldClose(m_window))
            return;
            
        glfwPollEvents();

        int w, h;
        glfwGetFramebufferSize(m_window, &w, &h);
        if (w > 0 && h > 0)
            this->Draw();

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

    const float getVersion() {return _DPCORE::version;}

}
