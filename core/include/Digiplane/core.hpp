#ifndef CORE_HPP
#define CORE_HPP

#include <string>

#include "../types.hpp"
#include "../geometry/mesh.hpp"

// DPAPI is used to declare public API functions for export as shared library.
#if defined(_WIN32) && defined(_DIGIPLANE_BUILD_DLL)
    // Building as a Win32 DLL
    #define DPAPI __declspec(dllexport)
#elif defined(_WIN32) && defined(DIGIPLANE_DLL)
    // Calling Win32 DLL
    #define DPAPI __declspec(dllimport)
#elif defined(__GNUC__) && defined(_DIGIPLANE_BUILD_DLL)
    //Unix shared library
    #define DPAPI __attribute__((visibility("default")))
#else
    #define DPAPI
#endif

#define DIGIPLANE_MAJOR 0
#define DIGIPLANE_MINOR 1

namespace Digiplane {
    struct initInfo {
        std::string title = "New DigiPlane Window";
        int width  = 800;
        int height = 600;
    };

    // The ApplicationContext is a container for managing and storing application-level objects and resources.
    class DPAPI ApplicationContext {

        // mutable
        static bool shouldExit;
    public:
        static const float digiplaneVersion;     // current version of Digiplane
        static const std::string systemLanguage; // system language

        ApplicationContext() {}
        ~ApplicationContext() {}

        // Native application functions
        virtual void init() {}
        virtual void update(float deltaTime) {deltaTime = NULL;}
    };
} // DigiPlane

#endif // CORE_HPP