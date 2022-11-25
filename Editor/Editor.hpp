#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <iostream>
#include <string>

#include "DigiPlane/dp_core.hpp"

// Application entry point class
// This class is responsible for initializing the editor
class Application {

    bool is_running;
public:
    Application(void);
    ~Application(void);

    // remove copy constructor and assignment operator to avoid multiple instances of the application
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void init(void){
        std::cout << "Initialized\n";
    }
};

#endif // EDITOR_HPP

