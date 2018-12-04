#include <iostream>

#include "Window.h"

// public:

Window::Window(): hInstance(nullptr)
{
}

Window::Window(HINSTANCE hInstance, int width, int height)
{
    std::cout << "Creating window..." << std::endl;

	this->hInstance = hInstance;

	registerWindowClass();

	createWindow(width, height);
}

void Window::setUserPointer(void * pointer) const
{
	SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(pointer));
}

VkExtent2D Window::getClientExtent() const
{
	return getClientExtent(hWnd);
}

VkExtent2D Window::getClientExtent(HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);

	return { 
		uint32_t(rect.right - rect.left) ,
		uint32_t(rect.bottom - rect.top)
	};
}

HINSTANCE Window::getHInstance() const
{
	return hInstance;
}

HWND Window::getHWnd() const
{
	return hWnd;
}

int Window::mainLoop() const
{
	showWindow();

	MSG msg;
	BOOL result;

	while ((result = GetMessage(&msg, hWnd, 0, 0)))
	{
		if (result == -1)
		{
			break;
		}

		auto pVulkan = reinterpret_cast<Vulkan*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		pVulkan->drawFrame();

		DispatchMessage(&msg);
	}

	return result;
}

// private:

ATOM Window::registerWindowClass() const
{
	WNDCLASSEX cex{
		sizeof(WNDCLASSEX),				// cbSize 
		CS_HREDRAW | CS_VREDRAW,		// style         
		wndProc,						// lpfnWndProc   
		0,								// cbClsExtra    
		0,								// cbWndExtra    
		hInstance,						// hInstance     
		0,								// hIcon         
		LoadCursor(nullptr, IDC_ARROW),	// hCursor       
		HBRUSH(COLOR_WINDOW + 1),		// hbrBackground 
		0,								// lpszMenuName  
		WINDOW_CLASS.data(),			// lpszClassName 
		0								// hIconSm      
	};

	return RegisterClassEx(&cex);
}

void Window::createWindow(int width, int height)
{
	hWnd = CreateWindow(WINDOW_CLASS.data(), WINDOW_TITLE.data(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, width, height, nullptr, nullptr, hInstance, nullptr);

	assert(hWnd);
}

void Window::showWindow() const
{
	SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
	SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
	ShowWindow(hWnd, SW_SHOWMAXIMIZED);

	UpdateWindow(hWnd);
}

LRESULT Window::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto pVulkan = reinterpret_cast<Vulkan*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_SIZE:
		pVulkan->minimized = wParam == SIZE_MINIMIZED;
		pVulkan->resize(getClientExtent(hWnd));
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
