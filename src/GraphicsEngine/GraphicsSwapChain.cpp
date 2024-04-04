#include "pch.h"
#include "Exception.h"
#include "GraphicsEngine/GraphicsDevice.h"
#include "GraphicsEngine/GraphicsGlobals.h"
#include "GraphicsEngine/GraphicsSwapChain.h"
#include "GraphicsEngine/Vertex.h"
#include "hlsl/includeConstantBuffers.hlsli.h"
#include "InputHandler.h"
#include "TimerScope.h"


using namespace GraphicsGlobals;


LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	GraphicsSwapChain* swapChain = nullptr;
	
	if (Msg == WM_CREATE)
	{
		CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);

		swapChain = reinterpret_cast<GraphicsSwapChain*>(createStruct->lpCreateParams);
		swapChain->myHwnd = hWnd;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(swapChain));
		return 0;
	}

	swapChain = reinterpret_cast<GraphicsSwapChain*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	if (swapChain)
	{
		return swapChain->WndProc(Msg, wParam, lParam);
	}
	else
	{
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	}
}

GraphicsSwapChain::GraphicsSwapChain()
	: myHwnd(nullptr)
	, myWidth(0)
	, myHeight(0)
	, myAspectRatio(0)
	, myIsResizing(false)
	, myIsRunning(true)
	, myFrameIndex(0)
{
}

void GraphicsSwapChain::Initialize(GraphicsDevice* aDevice, GraphicsQueue* aQueueToWaitFor)
{
	myThread = std::thread(&GraphicsSwapChain::Thread, this, aDevice, aQueueToWaitFor);
}

void GraphicsSwapChain::Shutdown()
{
	myThread.join();
	myQueue.Signal();
	myQueue.WaitForIdle();
}

void GraphicsSwapChain::Thread(GraphicsDevice* aDevice, GraphicsQueue* aQueueToWaitFor)
{
	try
	{
		GuardedThread(aDevice, aQueueToWaitFor);
	}
	catch (std::exception& e)
	{
		PrintExceptionStack(e);
	}
}

void GraphicsSwapChain::GuardedThread(GraphicsDevice* aDevice, GraphicsQueue* aQueueToWaitFor)
{
	/*
	TODO
		- [ ] test
			- [ ] bind vertex buffer of plane
			- [ ] bind index buffer of plane
			- [ ] try to render with the camera perspective of game except rotation only
			- [ ] try to move sprite away
		- [ ] proper
			- [ ] swap 2 textures between display and reading
			- [ ] render scene buffer on plane covering screen in world, not screenspace
	*/
	{
		const HINSTANCE hInstance = GetModuleHandleW(NULL);

		WNDCLASSEXW wcex{};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = &::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = NULL;
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = L"comrade-klick";
		wcex.hIconSm = NULL;
		RegisterClassExW(&wcex);

		HWND hwnd = CreateWindowExW(0L, L"comrade-klick",
			L"comrade-klick", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL,
			NULL, hInstance, this);
		ShowWindow(hwnd, SW_SHOWMAXIMIZED);
		UpdateWindow(hwnd);

		enum {
			HID_USAGE_PAGE_GENERIC = 0x01,
			HID_USAGE_GENERIC_MOUSE = 0x02
		};
		RAWINPUTDEVICE device = {};
		device.dwFlags = RIDEV_INPUTSINK;
		device.hwndTarget = hwnd;
		device.usUsage = HID_USAGE_GENERIC_MOUSE;
		device.usUsagePage = HID_USAGE_PAGE_GENERIC;
		ThrowIfFailed(::RegisterRawInputDevices(&device, 1, sizeof(device)));

		myQueue.Initialize(aDevice, L"GraphicsSwapChain::myQueue");
		mySwapChain = aDevice->CreateSwapChain(myQueue.GetID3D12CommandQueue(), hwnd, FRAME_COUNT);

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.NumDescriptors = FRAME_COUNT;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		myRtvHeap = aDevice->CreateDescriptorHeap(L"GraphicsSwapChain::myRtvHeap", rtvHeapDesc);

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.NumDescriptors = FRAME_COUNT;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		myDsvHeap = aDevice->CreateDescriptorHeap(L"GraphicsSwapChain::myDsvHeap", dsvHeapDesc);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(myRtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < FRAME_COUNT; n++)
		{
			ThrowIfFailed(mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myBackBuffers[n])));
			ThrowIfFailed(myBackBuffers[n]->SetName(std::wstring(std::wstring(L"myBackBuffers[") + std::to_wstring(n) + L"]").c_str()));
			aDevice->CreateRenderTargetView(myBackBuffers[n].Get(), rtvHeapHandle);

			rtvHeapHandle.Offset(1, aDevice->RtvDescriptorSize());
		}
	}

	Buffer passConstantBuffer = aDevice->CreateUploadBuffer<cbPassStruct>(L"SwapChain cbPass");
	Buffer instanceConstantBuffer = aDevice->CreateUploadBuffer<cbInstanceStruct>(L"SwapChain cbInstance");

	myQueue.InsertWaitForQueue(*aQueueToWaitFor);
	myCommandAllocator = aDevice->CreateCommandAllocator(L"GraphicsSwapChain::myCommandAllocator");
	myCommandList = aDevice->CreateGraphicsCommandList(L"GraphicsSwapChain::myCommandList", globalModelPipeline.Get(), myCommandAllocator);
	while (myIsRunning)
	{
		MSG msg{};
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			myIsRunning &= (msg.message != WM_QUIT && msg.message != WM_CLOSE && msg.message != WM_DESTROY);
		}
		if (myWidth < 4 && myHeight < 4)
		{
			continue;
		}
		if (myIsResizing)
		{
			myScissorRect.right = myWidth;
			myScissorRect.bottom = myHeight;
			myViewport.Width = static_cast<float>(myWidth);
			myViewport.Height = static_cast<float>(myHeight);
			myViewport.MinDepth = 0.0f;
			myViewport.MaxDepth = 1.0f;

			for (ComPtr<ID3D12Resource>& backBuffer : myBackBuffers)
			{
				backBuffer.Reset();
			}
			myQueue.Signal();
			myQueue.WaitForIdle();
			mySwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
			myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(myRtvHeap->GetCPUDescriptorHandleForHeapStart());
			for (UINT n = 0; n < FRAME_COUNT; n++)
			{
				ThrowIfFailed(mySwapChain->GetBuffer(n, IID_PPV_ARGS(&myBackBuffers[n])));
				ThrowIfFailed(myBackBuffers[n]->SetName(std::wstring(std::wstring(L"myBackBuffers[") + std::to_wstring(n) + L"]").c_str()));
				aDevice->CreateRenderTargetView(myBackBuffers[n].Get(), rtvHeapHandle);

				rtvHeapHandle.Offset(1, aDevice->RtvDescriptorSize());
			}

			myDepthStencilBuffer = aDevice->CreateTexture2D(L"GraphicsSwapChain::myDepthStencilBuffer", myWidth, myHeight,
				DXGI_FORMAT_D32_FLOAT, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
			aDevice->CreateDepthStencilView(myDepthStencilBuffer.Get(), myDsvHeap->GetCPUDescriptorHandleForHeapStart());
		}

		myCamera.Update(0.01f, { static_cast<float>(myWidth), static_cast<float>(myHeight) });

		cbInstanceStruct cbInstance;
		cbInstance.transform = Mat4::TranslationMatrix(0, 0, 200);
		cbInstance.color = Vec4(1, 1, 1, 1);
		instanceConstantBuffer.Upload(cbInstance);

		cbPassStruct cbPass; //TODO: separate rotation and translation matrix
		cbPass.cameraV = myCamera.GetViewMatrix();
		cbPass.cameraP = myCamera.GetProjectionMatrix();
		cbPass.cameraVP = cbPass.cameraV * cbPass.cameraP;
		passConstantBuffer.Upload(cbPass);



		D3D12_RESOURCE_BARRIER rtvPresentToTarget = CD3DX12_RESOURCE_BARRIER::Transition(myBackBuffers[myFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
		                       rtvTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(myBackBuffers[myFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		

		const float clearColor[4] = { 0.f, 0.f, 0.f, 0.f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(myDsvHeap->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(myRtvHeap->GetCPUDescriptorHandleForHeapStart());
		rtvHandle.Offset(myFrameIndex, aDevice->RtvDescriptorSize());
		myCommandList->ResourceBarrier(1, &rtvPresentToTarget);
		myCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		myCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
		myCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		myCommandList->RSSetScissorRects(1, &myScissorRect);
		myCommandList->RSSetViewports(1, &myViewport);
		myCommandList->SetGraphicsRootSignature(globalRootSignature.Get());
		myCommandList->SetGraphicsRootConstantBufferView(0, instanceConstantBuffer.GetGPUVirtualAddress());
		myCommandList->SetGraphicsRootConstantBufferView(1, passConstantBuffer.GetGPUVirtualAddress());
		myCommandList->IASetVertexBuffers(0, 1, &globalPlaneMesh.VertexBufferView());
		myCommandList->IASetIndexBuffer(&globalPlaneMesh.IndexBufferView());
		myCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		myCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		myCommandList->ResourceBarrier(1, &rtvTargetToPresent);

		ThrowIfFailed(myCommandList->Close());
		myQueue.ExecuteCommandList(myCommandList);
		{
			TimerScope timerScope("GraphicsSwapChain");
			myQueue.WaitForIdle();
		}
		ThrowIfFailed(mySwapChain->Present(1, 0));
		ThrowIfFailed(myCommandAllocator->Reset());
		ThrowIfFailed(myCommandList->Reset(myCommandAllocator.Get(), globalModelPipeline.Get()));
		myFrameIndex = mySwapChain->GetCurrentBackBufferIndex();

		myIsResizing = false;
		globalInputHandler.EndFrame();
	}
}

LRESULT GraphicsSwapChain::WndProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	globalInputHandler.WndProcHandler(*this, Msg, wParam, lParam);

	switch (Msg)
	{
		case WM_QUIT:
		case WM_CLOSE:
		{
			myIsRunning = false;
			break;
		}
		case WM_SIZE:
		{
			myWidth = static_cast<UINT>(GET_X_LPARAM(lParam));
			myHeight = static_cast<UINT>(GET_Y_LPARAM(lParam));
			myAspectRatio = myWidth / static_cast<float>(myHeight);
			myIsResizing = true;
			break;
		}
		default:
		{
			return DefWindowProcW(myHwnd, Msg, wParam, lParam);
		}
	}
	return 0;
}