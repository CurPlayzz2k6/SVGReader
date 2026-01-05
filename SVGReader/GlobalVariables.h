#pragma once
#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H
#include "ParserSVG.h"
#include "RenderSVG.h"
#include <windows.h>
#include <vector>

using namespace std;
using namespace rapidxml;

extern ULONG_PTR gdiplusToken; // Token GDI+
extern wstring fileContent; // Nội dung văn bản (đã chuyển sang WCHAR)
extern HDC curHDC; // Lưu HDC hiện tại trong WM_PAINT

// Biến trạng thái góc rotate hiện tại
extern float fScale;     // Mức zoom
extern float fPanX;      // Vị trí pan X (offset của camera)
extern float fPanY;      // Vị trí pan Y (offset của camera)

// Biến trạng thái để kéo (panning)
extern bool isPanning;  // Đang nhấn chuột giữa để kéo?
extern POINT panningStart;  // Vị trí chuột lúc bắt đầu kéo
extern float panningStartX, panningStartY; // Vị trí pan lúc bắt đầu kéo
extern float curRotate;

// Hỗ trợ cho lấy tâm rotate
extern float svgWidth;
extern float svgHeight;

// Biến đọc file RapidXML
extern ParserSVG fileXML;
extern vector<char> xmlBuffer;

// RenderXML
extern RenderSVG screen;
#endif GLOBALVARIABLES_H
