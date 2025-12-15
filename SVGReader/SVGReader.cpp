#pragma comment (lib, "Gdiplus.lib")
#pragma comment (lib, "Comdlg32.lib") // Cho GetOpenFileName
#define UNICODE
#define _UNICODE
#define PI 3.1415926535f

#include <stdio.h>
#include <iostream>
#include <io.h> 
#include <fcntl.h>
#include <cmath>
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <commdlg.h> // Cho hộp thoại mở tệp
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "GlobalVariables.h"
#include "rapidxml.hpp"
#include "Function.h"
#include "RectangleSVG.h"
#include "CircleSVG.h"
#include "LineSVG.h"
#include "PolygonSVG.h"
#include "PolylineSVG.h"
#include "EllipseSVG.h"
#include "TextSVG.h"
#include "RenderSVG.h"
#include "GroupSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

// ID cho menu
#define IDM_FILE_OPEN 101
#define IDM_FILE_EXIT 103

ULONG_PTR gdiplusToken; // Token GDI+
wstring fileContent; // Nội dung văn bản (đã chuyển sang WCHAR)


// Kích thước cửa sổ ban đầu
const int xWindow = 1200;
const int yWindow = 600;

// Góc độ quay hiện tại
float curRotate = 0;

// Biến đọc file RapidXML
ParserSVG fileXML;
vector<char> xmlBuffer;

// RenderXML
RenderSVG screen;

// Biến trạng thái của thu phóng camera
float fScale = 1.0f;     // Mức zoom (1.0 = 100%)
float fPanX = 0.0f;      // Vị trí pan X (offset của camera)
float fPanY = 0.0f;      // Vị trí pan Y (offset của camera)

// Biến trạng thái để kéo (panning)
bool isPanning = false;  // Đang nhấn chuột giữa để kéo?
POINT panningStart;      // Điểm trỏ chuột lúc bắt đầu kéo
float panningStartX, panningStartY; // Vị trí x, y của trỏ chuột pan lúc bắt đầu kéo

// Khai báo hàm
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeGDIPlus();
void ShutdownGDIPlus();
bool OpenFile(HWND hwnd);
void drawNote(Graphics& graphics, int windowWidth, int windowHeight);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {

	//EnableConsole();

	InitializeGDIPlus();

	// Đăng ký lớp cửa sổ
	const wchar_t CLASS_NAME[] = L"SVGReader";

	WNDCLASSW wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	RegisterClassW(&wc);

	// Tạo Menu
	HMENU hMenu = CreateMenu();
	HMENU hSubMenu = CreateMenu();
	AppendMenuW(hSubMenu, MF_STRING, IDM_FILE_OPEN, L"&Open a SVG file");
	AppendMenuW(hSubMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(hSubMenu, MF_STRING, IDM_FILE_EXIT, L"&Exit");
	AppendMenuW(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, L"&File");

	// 4. Tạo cửa sổ
	HWND hwnd = CreateWindowExW(
		0,
		CLASS_NAME,
		L"SVG Reader",
		WS_OVERLAPPEDWINDOW, // Note: Thêm WS_VSCROLL nếu cần thanh cuộn
		CW_USEDEFAULT, CW_USEDEFAULT, xWindow, yWindow,
		NULL,
		hMenu, // Gắn menu Tệp vào cửa sổ
		hInstance,
		NULL
	);

	if (hwnd == NULL) {
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Dọn GDI+
	ShutdownGDIPlus();
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
	{
		// Xử lý sự kiện click menu
		switch (LOWORD(wParam)) {
		case IDM_FILE_OPEN:
			if (OpenFile(hwnd)) {
				InvalidateRect(hwnd, NULL, TRUE); // Vẽ lại cửa sổ
			}
			break;
		case IDM_FILE_EXIT:
			DestroyWindow(hwnd); // Đóng ứng dụng
			break;
		}
	}

	case WM_ERASEBKGND:
		return 1; // 1 để chặn xoá nền để sử dụng double buffering

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		// Dùng double buffering để chống nháy màn hình
		RECT rcClient;
		GetClientRect(hwnd, &rcClient);
		float centerX = rcClient.right / 2.0f;
		float centerY = rcClient.bottom / 2.0f;
		int width = rcClient.right - rcClient.left;
		int height = rcClient.bottom - rcClient.top;

		Bitmap memBitmap(width, height, PixelFormat32bppARGB);
		Graphics memG(&memBitmap);

		memG.Clear(Color(255, 255, 255, 255)); // Xoá nền trắng
		memG.SetSmoothingMode(SmoothingModeAntiAlias);
		memG.SetTextRenderingHint(TextRenderingHintAntiAlias);
		memG.ScaleTransform(fScale, fScale); // Áp dụng Zoom
		memG.TranslateTransform(fPanX, fPanY); // Áp dụng Pan
		memG.RotateTransform(curRotate); // Áp dụng Rotate
		Matrix transformMatrix;

		// Tạo ma trận để biến đổi
		// Xoay một góc [curRotate] xung quanh điểm (centerX, centerY)
		transformMatrix.RotateAt(curRotate, PointF(centerX, centerY)); // Áp dụng Rotate (RotateAt)
		transformMatrix.Scale(fScale, fScale, MatrixOrderAppend); // Áp dụng Scale (Pan)
		transformMatrix.Translate(fPanX, fPanY, MatrixOrderAppend); // Áp dụng Pan (Translate)
		memG.SetTransform(&transformMatrix);
		screen.drawAll(&memG);
		drawNote(memG, width, height);

		Graphics graphics(hdc);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

		// Áp dụng ma trận sau khi xoay vào graphics
		graphics.DrawImage(&memBitmap, 0, 0);
		graphics.ResetTransform();
		EndPaint(hwnd, &ps);
		break;
	}

	// Nhấn phím mũi tên để rotate
	case WM_KEYDOWN:
		if (wParam == VK_RIGHT)
		{
			curRotate += 10.0f; // Xoay sang phải 10 độ
			if (curRotate >= 360) curRotate = 0; // Tránh sai số khi xoay 360 độ
		}
		else if (wParam == VK_LEFT)
		{
			curRotate -= 10.0f; // Xoay sang trái 10 độ
			if (curRotate < 0) curRotate += 360; // Tránh sai số khi xoay 360 độ
		}

		// Bắt cửa sổ vẽ lại
		InvalidateRect(hwnd, NULL, TRUE);
		break;

	// Lăn chuột để thu phóng
	case WM_MOUSEWHEEL:
	{
		// Lấy vị trí con trỏ (chuột trái) (tọa độ client)
		POINT pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		ScreenToClient(hwnd, &pt); // Chuyển về tọa độ client

		int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		float fZoomFactor = 1.1f;
		float fMouseWorldX_Before = (pt.x - fPanX) / fScale;
		float fMouseWorldY_Before = (pt.y - fPanY) / fScale;

		// Tính mức zoom mới
		float fOldScale = fScale;
		if (delta > 0) fScale *= fZoomFactor; // Zoom in
		else fScale /= fZoomFactor; // Zoom out

		// Giới hạn zoom
		if (fScale < 0.1f) fScale = 0.1f;
		if (fScale > 20.0f) fScale = 20.0f;

		// Tính toán lại pan
		fPanX = pt.x - (fMouseWorldX_Before * fScale);
		fPanY = pt.y - (fMouseWorldY_Before * fScale);

		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	}
	break;

	case WM_MBUTTONDOWN:
	{
		isPanning = true;
		panningStart.x = GET_X_LPARAM(lParam);
		panningStart.y = GET_Y_LPARAM(lParam);

		// Lưu lại vị trí Pan gốc khi bắt đầu
		panningStartX = fPanX;
		panningStartY = fPanY;

		SetCapture(hwnd); // Bắt chuột
		return 0;
	}
	break;

	case WM_MBUTTONUP:
	{
		isPanning = false;
		ReleaseCapture(); // Nhả chuột
		return 0;
	}
	break;

	case WM_MOUSEMOVE:
	{
		if (isPanning)
		{
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);

			// Tính delta màn hình - khoảng cách kéo thả chuột (Screen Space)
			int deltaX = mouseX - panningStart.x;
			int deltaY = mouseY - panningStart.y;

			// Áp dụng Pan (Screen Space)
			// công thức tính x, y để di chuyển theo hướng chuột
			fPanX = panningStartX + deltaX;
			fPanY = panningStartY + deltaY;

			InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;
	}
	break;

	// Kéo di chuyển ảnh trên màn hình
	case WM_LBUTTONDOWN:
	{
		isPanning = true;
		panningStart.x = GET_X_LPARAM(lParam);
		panningStart.y = GET_Y_LPARAM(lParam);

		// Lưu lại vị trí Pan gốc khi bắt đầu
		panningStartX = fPanX;
		panningStartY = fPanY;

		SetCapture(hwnd); // Bắt chuột
		return 0;
	}
	break;

	case WM_LBUTTONUP:
	{
		isPanning = false;
		ReleaseCapture(); // Nhả chuột
		return 0;
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Khởi tạo GDI+
void InitializeGDIPlus() {
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

// Tắt Application
void ShutdownGDIPlus() {
	screen.clearAll();
	GdiplusShutdown(gdiplusToken);
}

// Hàm mở hộp thoại, đọc tệp và lưu vào fileContent
bool OpenFile(HWND hwnd) {
	try {
		OPENFILENAMEW ofn = {};
		WCHAR szFile[260] = { 0 };

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
		ofn.lpstrFilter = L"SVG Files (*.svg)\0*.svg\0All Files (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameW(&ofn) == TRUE) {
			ifstream file(ofn.lpstrFile, ios::binary);
			if (!file.is_open()) {
				MessageBoxW(hwnd, L"Cannot open file!", L"Error", MB_OK | MB_ICONERROR);
				return false;
			}
			xmlBuffer.clear();
			xmlBuffer.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
			xmlBuffer.push_back('\0');
			file.close();
			fileXML.readSVG(xmlBuffer);
			screen.clearAll(); // Xoá các phần tử cũ lưu trong shapes
			screen.readAll(fileXML);
			curRotate = 0;
			fScale = 1.0f;     // Mức zoom (1.0 = 100%)
			fPanX = 0.0f;      // Vị trí pan X (offset của camera)
			fPanY = 0.0f;      // Vị trí pan Y (offset của camera)
			InvalidateRect(hwnd, NULL, TRUE);
			return true;
		}
		return false;

	}
	catch (rapidxml::parse_error& err) {
		string errorMessage = "System error: ";
		errorMessage += err.what();
		errorMessage += "\nDouble check the SVG file structure (XML) or valid file type (SVG).";

		MessageBoxA(hwnd, errorMessage.c_str(), "Error loading SVG file", MB_ICONERROR | MB_OK);
		return false;

	}
	catch (exception& err) {
		// Bắt lỗi chung (ví dụ: lỗi đọc file)
		MessageBoxA(hwnd, err.what(), "Syntax error in SVG file", MB_ICONERROR | MB_OK);
		return false;
	}

}

void drawNote(Graphics& graphics, int windowWidth, int windowHeight) {
	Matrix oldMatrix;
	graphics.GetTransform(&oldMatrix);
	graphics.ResetTransform();
	FontFamily fontFamily(L"Arial");
	Font font(&fontFamily, 10, FontStyleRegular, UnitPoint);
	SolidBrush textBrush(Color(255, 255, 255, 255)); // Chữ màu trắng
	SolidBrush bgBrush(Color(180, 0, 0, 0));         // Nền đen bán trong suốt (Alpha = 180)
	vector<wstring> lines = {
		L"Kéo thả chuột: Panning (Di chuyển)",
		L"Lăn chuột: Phóng to / Thu nhỏ",
		L"Phím ← : Xoay trái 10°",
		L"Phím → : Xoay phải 10°"
	};
	float lineHeight = 20.0f;
	float padding = 10.0f;
	float boxWidth = 230.0f;
	float boxHeight = (lines.size() * lineHeight) + (padding * 2);
	float margin = 10.0f;
	float x = (float)windowWidth - boxWidth - margin;
	float y = (float)windowHeight - boxHeight - margin;
	graphics.FillRectangle(&bgBrush, x, y, boxWidth, boxHeight);
	PointF textPos(x + padding, y + padding);
	for (const auto& line : lines) {
		graphics.DrawString(line.c_str(), -1, &font, textPos, &textBrush);
		textPos.Y += lineHeight;
	}
	graphics.SetTransform(&oldMatrix);
}