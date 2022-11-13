#include <iostream>
#include "Editor.hpp"

#define EDITOR_NAME "DigiPlane"

/* Global variables */

std::unique_ptr<Application> editorApp;


int main(int argc, char *argv[])
{
    editorApp = std::make_unique<Application>();
    editorApp->Awake();
    

    // Editor funcitonality found at Editor.cpp

    return 0;   
}