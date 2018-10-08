#include "Window.h"

// public:

Window::Window(HINSTANCE hInstance, int width, int height)
{
	this->hInstance = hInstance;

	registerWindowClass();

	createWindow(width, height);
}

void Window::setUserPointer(void * pointer)
{
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pointer);
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
		(uint32_t)(rect.left - rect.right),  
		(uint32_t)(rect.bottom - rect.top) 
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

int Window::mainLoop()
{
	showWindow();

	MSG msg;
	BOOL result;

	while (result = GetMessage(&msg, hWnd, 0, 0))
	{
		if (result == -1)
		{
			throw std::runtime_error("Failed to get window message");
		}

		Vulkan *pVulkan = reinterpret_cast<Vulkan*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		pVulkan->drawFrame();

		DispatchMessage(&msg);
	}
}

// private:

ATOM Window::registerWindowClass()
{
	WNDCLASSEX cex{
		sizeof(WNDCLASSEX),				// cbSize 
		CS_HREDRAW | CS_VREDRAW,		// style         
		WndProc,						// lpfnWndProc   
		0,								// cbClsExtra    
		0,								// cbWndExtra    
		hInstance,							// hInstance     
		0,								// hIcon         
		LoadCursor(nullptr, IDC_ARROW),	// hCursor       
		(HBRUSH)(COLOR_WINDOW + 1),		// hbrBackground 
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

	if (!hWnd)
	{
		throw std::runtime_error("Failed to create window");
	}
}

void Window::showWindow()
{
	SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
	SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
	ShowWindow(hWnd, SW_SHOWMAXIMIZED);

	UpdateWindow(hWnd);
}

LRESULT Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Vulkan *pVulkan = reinterpret_cast<Vulkan*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_SIZE:
		pVulkan->minimized = wParam == SIZE_MINIMIZED;
		pVulkan->resize(getClientExtent(hWnd));
		break;

	case WM_KEYDOWN:
		pVulkan->keyDownCallback(wParam);
		break;

	case WM_KEYUP:
		pVulkan->keyUpCallback(wParam);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}