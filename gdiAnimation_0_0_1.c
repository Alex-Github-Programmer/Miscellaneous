#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#define cyan 0xccff66
#define yellow 0x00ffff
#define red 0x0000ff
#define lengthOfSide 100
const char *CLASS_NAME = "WND_CLS";
RECT square, screen;
LONG velocity, score;
LONG directX, directY;
LONG offsetX, offsetY;
COLORREF color;
BOOL paused, dragging;
clock_t lastTime;
UINT_PTR lastKey;
HDC hdc;
char *dest, *begin;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	dest = (char*) malloc(1024);
	memcpy(dest, "SCORE: 00000000", 16);
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);
	HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Bounce!", 
	WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
	CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if(hwnd == NULL) return 0;
	ShowWindow(hwnd, nShowCmd);
	MSG msg={};
	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_DESTROY: PostQuitMessage(0); return 0;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			hdc = BeginPaint(hwnd, &ps);
			FillRect(hdc, &ps.rcPaint, CreateSolidBrush(cyan));
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_CREATE: {
			square.left = 150;
			square.top = 350;
			square.right = 250;
			square.bottom = 450;
			velocity = 10;
			directX = 1;
			directY = -1;
			color = yellow;
			paused = FALSE;
			dragging = FALSE;
			score = 0;
			SetTimer(hwnd, 1, 1, NULL); 
			return 0;
		}
		case WM_TIMER: {
			if(paused||dragging) return 0;
			hdc = GetDC(hwnd);
			GetClientRect(hwnd, &screen);
			SetBkColor(hdc, cyan);
			SetTextColor(hdc, red);
			FillRect(hdc, &square, CreateSolidBrush(cyan));
			TextOut(hdc, 10, 10, dest, strlen(dest));
			if(square.left <= screen.left) {
				directX = 1;
				square.left = screen.left;
				square.right = screen.left + lengthOfSide;
			}
			if(square.top <= screen.top) {
				directY = 1;
				square.top = screen.top;
				square.bottom = screen.top + lengthOfSide;
			}
			if(square.right >= screen.right) {
				directX = -1;
				square.right = screen.right;
				square.left = screen.right - lengthOfSide;
			}
			if(square.bottom >= screen.bottom) {
				directY = -1;
				square.bottom = screen.bottom;
				square.top = screen.bottom - lengthOfSide;
			}
			square.left += velocity * directX;
			square.top += velocity * directY;
			square.right += velocity * directX;
			square.bottom += velocity * directY;
			FillRect(hdc, &square, CreateSolidBrush(color));
			ReleaseDC(hwnd, hdc);
			return 0;
		}
		case WM_LBUTTONDOWN: {
			int posX = GET_X_LPARAM(lParam);
			int posY = GET_Y_LPARAM(lParam);
			if(square.left <= posX && posX < square.right &&
			square.top <= posY && posY < square.bottom) {
				dragging = TRUE;
				offsetX = posX - square.left;
				offsetY = posY - square.top;
				if(!paused) {
					score += velocity == 10 ? 1 : 5;
					sprintf(dest + 7, "%05ld000", score);
				}
				return 0;
			}
			if(!paused) color = red;
			return 0;
		}
		case WM_LBUTTONUP: {
			dragging = FALSE;
			color = yellow;
			return 0;
		}
		case WM_MOUSEMOVE: {
			if(!dragging) return 0;
			int posX = GET_X_LPARAM(lParam);
			int posY = GET_Y_LPARAM(lParam);
			hdc = GetDC(hwnd);
			SetBkColor(hdc, cyan);
			SetTextColor(hdc, red);
			FillRect(hdc, &square, CreateSolidBrush(cyan));
			TextOut(hdc, 10, 10, dest, strlen(dest));
			square.top = posY - offsetY;
			square.left = posX - offsetX;
			square.bottom = square.top + lengthOfSide;
			square.right = square.left + lengthOfSide;
			FillRect(hdc, &square, CreateSolidBrush(color));
			ReleaseDC(hwnd, hdc);
			return 0;
		}
		case WM_KEYDOWN: {
			if(dragging) return 0;
			switch(wParam) {
				case VK_ESCAPE: DestroyWindow(hwnd); return 0;
				case VK_SPACE: paused ^= 1; return 0;
				case 'Q': directX = -1; directY = -1; goto ARROW;
				case 'E': directX = 1; directY = -1; goto ARROW;
				case 'C': directX = 1; directY = 1; goto ARROW;
				case 'Z': directX = -1; directY = 1; goto ARROW;
				ARROW: {
					clock_t recTime = clock();
					if(recTime - lastTime < CLOCKS_PER_SEC && lastKey == wParam) {
						if(velocity == 10) velocity = 20;
						else velocity = 10;
						lastTime = 0;
						lastKey = 0;
					} else {
						lastTime = recTime;
						lastKey = wParam;
					}
					return 0;
				}
			}
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
