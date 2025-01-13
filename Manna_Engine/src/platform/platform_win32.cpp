#include "platform/platform.h"

#if MN_PLATFORM_WINDOWS

#include "core/logger.h"

#include <windows.h>
#include <windowsx.h> //for input 

typedef struct internal_state {
	HINSTANCE instance;
	HWND window;
} internal_state;


LRESULT CALLBACK win32_window_event_handler(HWND window, UINT message, WPARAM w_param, LPARAM l_param); //forward declare window event handler

b8 platform_startup(
	platform_state* platform_state,
	const char* title,
	i32 x,
	i32 y,
	i32 width,
	i32 height) {

	platform_state->internal_state = malloc(sizeof(internal_state));
	internal_state * state = (internal_state *)platform_state->internal_state; //state variable = internal_state variable casted to windows specific internal_state type

	state->instance = GetModuleHandle(NULL);					//returns handle to currently executing program

	HICON icon = LoadIcon(state->instance, IDI_APPLICATION);	//TODO: use (HICON)LoadImageW(instance, MAKEINTRESOURCE(MY_ICON), IMAGE_ICON, 32, 32, LR_LOADFROMFILE); for finer control. 
	WNDCLASSA wc = {0};											//window class ANSI meaning 8 bit characters, as opposed to UNICODE - 16 bit characters
	wc.style = CS_DBLCLKS;										//flags for window performance and style. Accepts double clicks in window
	wc.lpfnWndProc = win32_window_event_handler;				//long pointer to window procedure function, function for handling window events
	wc.cbClsExtra = 0;											//legacy class specific mem allocation shared by all windows from this class
	wc.cbWndExtra = 0;											//legacy window specific mem allocation
	wc.hInstance = state->instance;								//handle to the program containing window procedure
	wc.hIcon = icon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);					//load stock arrow cursor
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "manna_window_class";	

	if (!RegisterClassA(&wc)) {
		MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK); //error popup
		return false;
	}

	//Create Window
	

	u32 window_style = WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
	u32 window_style_ex = WS_EX_APPWINDOW;

	RECT border_rect = {0};
	AdjustWindowRectEx(&border_rect, window_style, false, window_style_ex);

	x += border_rect.left;
	y += border_rect.top;
	width += border_rect.right - border_rect.left;
	height += border_rect.bottom - border_rect.top;

	HWND window_handle = CreateWindowExA(
		window_style_ex,
		"manna_window_class",
		title,
		window_style,
		x,
		y,
		width,
		height,
		NULL,
		NULL,
		state->instance,
		NULL);

	if (!window_handle) {
		MessageBoxA(0, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK); //error popup
		LOG_FATAL("Window creation failed");
		return false;
	} else {
		state->window = window_handle;
	}

	b32 should_activivate = 1; //accept input
	i32 show_window_command_flags = should_activivate ? SW_SHOW : SW_SHOWNOACTIVATE;

	ShowWindow(state->window, show_window_command_flags);
	
	return true;
}

void platform_shutdown(platform_state* platform_state) {
	internal_state * state = (internal_state *)platform_state->internal_state;

	if (state->window) {
		DestroyWindow(state->window);
		state->window = 0;
	}
}

b8 platform_update(platform_state* platform_state) {
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

void * platform_allocate(u64 size, b8 aligned)
{
	return malloc(size); //TODO: implement allocator
}
#endif