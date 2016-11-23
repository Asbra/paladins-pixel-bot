#include <Windows.h>
#include <stdio.h>
#include <math.h>

COLORREF targetColors[3] = { 0x4A4DFF, 0x4343E3, 0x4581EB };
BYTE colorDeviation = 0x10;// 64;

BYTE* bitData = NULL;

BOOL ScanPixel(HWND hwnd, PLONG pixelX, PLONG pixelY, RECT scanArea, COLORREF* targetColors, BYTE deviation)
{
	HDC hdc = GetWindowDC(hwnd);
	HDC cdc = CreateCompatibleDC(hdc);

	LONG scanWidth = scanArea.right - scanArea.left;
	LONG scanHeight = scanArea.bottom - scanArea.top;

	HBITMAP bmp = CreateCompatibleBitmap(hdc, scanWidth, scanHeight);
	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 24;
	bmi.biWidth = scanWidth;
	bmi.biHeight = -scanHeight;
	bmi.biCompression = BI_RGB;
	SelectObject(cdc, bmp);
	BitBlt(cdc, 0, 0, scanWidth, scanHeight, hdc, scanArea.left, scanArea.top, SRCCOPY);

	GetDIBits(hdc, bmp, 0, scanHeight, bitData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
	DeleteObject(bmp);
	DeleteDC(cdc);
	ReleaseDC(NULL, hdc);

	for (int y = scanHeight; y >= 0; y--) {
		for (int x = 0; x < scanWidth; x++) {
			BYTE r = bitData[3 * ((y * scanWidth) + x) + 2];
			BYTE g = bitData[3 * ((y * scanWidth) + x) + 1];
			BYTE b = bitData[3 * ((y * scanWidth) + x) + 0];

			for (int i = 0; i < 3; i++) {
				UINT targetR = GetRValue(targetColors[i]);
				UINT targetG = GetGValue(targetColors[i]);
				UINT targetB = GetBValue(targetColors[i]);

				if (r >= targetR - deviation && r <= targetR + deviation &&
					g >= targetG - deviation && g <= targetG + deviation &&
					b >= targetB - deviation && b <= targetB + deviation)
				{
					*pixelX = scanArea.left + x;
					*pixelY = scanArea.top + y;
					return TRUE;
				}
			}
		}
	}

	// delete bitData;

	return FALSE;
}

// int WINAPI WinMain(HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
int main()
{
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	LPCWSTR windowClass = TEXT("LaunchUnrealUWindowsClient");
	HWND hwnd = NULL;
	while(!hwnd) hwnd = FindWindow(windowClass, NULL);
	RECT rekt;
	GetWindowRect(hwnd, &rekt);
	LONG width = rekt.right;
	LONG height = rekt.bottom;
	printf("Screen size %dx%d\n", width, height);

	LONG centerX = width / 2;
	LONG centerY = height / 2;
	LONG nearAimX = width / 80;
	LONG nearAimY = height / 64;
	LONG fovX = width / 11;
	LONG fovY = height / 4;
	LONG scanL = centerX - fovX;
	LONG scanT = centerY;
	LONG scanR = centerX + fovX;
	LONG scanB = centerY + fovY;
	RECT scanArea = { centerX - fovX, centerY - fovY, centerX + fovX, centerY + fovY };
	printf("Scan area %d,%d,%d,%d\n", scanArea.left, scanArea.top, scanArea.right, scanArea.bottom);
	RECT nearAimScanArea = { centerX - nearAimX, centerY - nearAimY, centerX + nearAimX, centerY + nearAimY };
	printf("Near aim scan area %d,%d,%d,%d\n", nearAimScanArea.left, nearAimScanArea.top, nearAimScanArea.right, nearAimScanArea.bottom);
	LONG pixelX = 0;
	LONG pixelY = 0;

	LONG scanWidth = scanArea.right - scanArea.left;
	LONG scanHeight = scanArea.bottom - scanArea.top;
	bitData = new BYTE[3 * scanWidth * scanHeight];

	while (1)
	{
		Sleep(10);

		if (GetAsyncKeyState(VK_END)) break;
		bool aim = (GetAsyncKeyState(VK_MENU));
		bool trigger = (GetAsyncKeyState(VK_SHIFT));

		if (GetForegroundWindow() == hwnd && ScanPixel(hwnd, &pixelX, &pixelY, scanArea, targetColors, colorDeviation))
		{
			LONG aimX = pixelX - centerX;
			LONG aimY = pixelY - centerY;

			// Aim
			if (aim && abs(aimX) <= 30) {
				mouse_event(MOUSEEVENTF_MOVE, aimX / 3.f + 2, aimY / 3.f - 6, 0, 0);
			}

			// Trigger
			if (trigger && abs(aimX) <= 30 && !GetAsyncKeyState(VK_LBUTTON)) {
				// mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				// SendMessage(hwnd, BM_CLICK, 0, 0);
				SendMessage(hwnd, WM_LBUTTONDOWN, 0, 0);
				Sleep(1);
				SendMessage(hwnd, WM_LBUTTONUP, 0, 0);
			}
		}
	}

	system("pause");
}