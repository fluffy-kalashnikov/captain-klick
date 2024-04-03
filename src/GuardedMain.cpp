#include "pch.h"
#include "Camera.h"
#include "Clock.h"
#include "GraphicsEngine/GraphicsEngine.h"
#include "GraphicsEngine/GraphicsSwapChain.h"
#include "GraphicsEngine/GraphicsGlobals.h"
#include "InputHandler.h"


void GuardedMain()
{
    Camera camera;
    camera.OnInitialize();

    GraphicsDevice graphicsDevice;
    graphicsDevice.Initialize();
    GraphicsEngine graphicsEngine;
    graphicsEngine.Initialize(&graphicsDevice);
    GraphicsSwapChain graphicsSwapChain;
    graphicsSwapChain.Initialize(&graphicsDevice, &graphicsEngine.GetGraphicsQueue());
    
    Clock clock;
    clock.OnInitialize();
    while (graphicsSwapChain.IsRunning())
    {
        clock.OnUpdate();
        const float deltaSeconds = clock.GetDeltaSeconds(), 
                    timeSeconds = clock.GetTimeSeconds();

        camera.OnUpdate(deltaSeconds);
        graphicsEngine.Update(&graphicsSwapChain, deltaSeconds, timeSeconds, camera.ToViewMatrix(), 
            camera.ToProjectionMatrix(graphicsSwapChain.GetAspectRatio()));
        globalInputHandler.EndFrame();
    }
    graphicsSwapChain.Shutdown();
    graphicsEngine.Shutdown();
}