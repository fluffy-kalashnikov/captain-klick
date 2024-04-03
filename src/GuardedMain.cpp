#include "pch.h"
#include "Camera.h"
#include "Clock.h"
#include "GraphicsEngine/GraphicsEngine.h"
#include "GraphicsEngine/GraphicsSwapChain.h"
#include "GraphicsEngine/GraphicsGlobals.h"
#include "InputHandler.h"


void GuardedMain()
{
    //Camera camera;
    //camera.Initialize();

    GraphicsDevice graphicsDevice;
    graphicsDevice.Initialize();
    GraphicsEngine graphicsEngine;
    graphicsEngine.Initialize(&graphicsDevice);
    GraphicsSwapChain graphicsSwapChain;
    graphicsSwapChain.Initialize(&graphicsDevice, &graphicsEngine.GetGraphicsQueue());
    
    Clock clock;
    clock.Initialize();
    while (graphicsSwapChain.IsRunning())
    {
        clock.Update();
        //const float deltaSeconds = clock.GetDeltaSeconds(), 
        //            timeSeconds = clock.GetTimeSeconds();

        //camera.Update(deltaSeconds);
        graphicsEngine.Update(&graphicsSwapChain, graphicsSwapChain.GetCamera());
        globalInputHandler.EndFrame();
    }
    graphicsSwapChain.Shutdown();
    graphicsEngine.Shutdown();
}