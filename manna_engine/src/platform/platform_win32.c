#include "platform/platform.h"

#if M_PLATFORM_WINDOWS

#include "core/logger.h"

#include <windows.h>
#include <windowsx.h> //for input 
#include <stdlib.h>

typedef struct internal_state {
	HINSTANCE instance;
	HWND window;
} internal_state;

static f64 clock_frequency;
static LARGE_INTEGER start_time;

LRESULT CALLBACK win32_window_event_handler(HWND window, UINT message, WPARAM w_param, LPARAM l_param); //forward declare window event handler

b8 platform_startup(
	platform_state* platform_state,
	const char* title,
	i32 x,
	i32 y,
	i32 width,
	i32 height) {

	platform_state->internal_state = malloc(sizeof(internal_state)); //instantiate internal state
	if(!platform_state->internal_state) { return FALSE; }

	internal_state *state = (internal_state *)platform_state->internal_state; //state variable = internal_state variable casted to windows specific internal_state type

	state->instance = GetModuleHandle(NULL);					//returns handle to currently executing program

	//configure window class
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

	//register window class
	if (!RegisterClassA(&wc)) {
		MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK); //error popup
		return FALSE;
	}

	//Create Window
	u32 window_style = WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
	u32 window_style_ex = WS_EX_APPWINDOW;

	//calculate border size
	RECT border_rect = {0};
	AdjustWindowRectEx(&border_rect, window_style, FALSE, window_style_ex);

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
		return FALSE;
	} else {
		state->window = window_handle;
	}

	//set command flags
	b32 should_activivate = 1; //accept input
	i32 show_window_command_flags = should_activivate ? SW_SHOW : SW_SHOWNOACTIVATE;

	ShowWindow(state->window, show_window_command_flags);
	
	//setup clock
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	clock_frequency = 1.0 / (f64)frequency.QuadPart;
	QueryPerformanceCounter(&start_time);

	return TRUE;
}

void platform_shutdown(platform_state* platform_state) {
	internal_state * state = (internal_state *)platform_state->internal_state;

	if (state->window) {
		DestroyWindow(state->window);
		state->window = 0;
	}
}

b8 platform_get_messages (platform_state* platform_state) {
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return TRUE;
}

void* platform_allocate(u64 size, b8 aligned)
{
	return malloc(size);
}

void platform_free(void *ptr, b8 aligned)
{
	free(ptr);
}

void* platform_copy_memory(void* dest, const void* src, u64 size)
{
	return memcpy(dest, src, size);
}

void* platform_zero_memory(void* dest, u64 size)
{
	return memset(dest, 0, size);
}

void* platform_set_memory(void* dest, u8 value, u64 size)
{
	return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 color_index)
{
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };					//color for console text
	SetConsoleTextAttribute(console_handle, levels[color_index]);

	OutputDebugStringA(message);
	u64 length = strlen(message);
	LPDWORD bytes_written = 0;
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, bytes_written, NULL);
}

void platform_console_write_error(const char* message, u8 color_index)
{

	HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);			//write to error stream
	static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };					//color for console text
	SetConsoleTextAttribute(console_handle, levels[color_index]);

	OutputDebugStringA(message);
	u64 length = strlen(message);
	LPDWORD bytes_written = 0;
	WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, bytes_written, NULL);
}

f64 platform_get_time()
{
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	return (f64)current_time.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms)
{
	Sleep(ms);
}

LRESULT CALLBACK win32_window_event_handler(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	switch (message) {
		case WM_ERASEBKGND:
			return 1;	//handle erasing in application
		case WM_CLOSE:
			//TODO: shutdown event launch
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_SIZE: {
			//RECT r;
			//GetClientRect(window, &r);
			//u32 width = r.right - r.left;
			//u32 height = r.bottom - r.top;

			//TODO: resize event launch
		} break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			//b8 pressed = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
			//TODO: input processing
		} break;
		case WM_MOUSEMOVE: {
			//i32 x = GET_X_LPARAM(l_param);
			//i32 y = GET_Y_LPARAM(l_param);
			//TODO: input processing
		} break;
		case WM_MOUSEWHEEL: {
			//i32 delta = GET_WHEEL_DELTA_WPARAM(w_param);
			//if (delta != 0) {
			//	delta = (delta < 0) ? -1 : 1;
			//	//TODO: input processing
			//}
		} break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP: {
			//b8 pressed = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN;
			//TODO: input processing
		} break;
	}

	return DefWindowProcA(window, message, w_param, l_param);
}

#endif // M_PLATFORM_WINDOWS
