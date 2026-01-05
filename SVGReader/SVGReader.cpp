#pragma comment (lib, "Gdiplus.lib")
#pragma comment (lib, "Comdlg32.lib")

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
#include <commdlg.h>
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

#define IDM_FILE_OPEN 101
#define IDM_FILE_EXIT 102
#define IDM_FILE_SAVE 103

ULONG_PTR gdiplusToken;
wstring fileContent;

const int xWindow = 1200;
const int yWindow = 800;

float curRotate = 0;

ParserSVG fileXML;
vector<char> xmlBuffer;
RenderSVG screen;

float fScale = 1.0f;
float fPanX = 0.0f;
float fPanY = 0.0f;

bool isPanning = false;
POINT panningStart;
float panningStartX, panningStartY;

float svgWidth = 0.0f;
float svgHeight = 0.0f;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeGDIPlus();
void ShutdownGDIPlus();
bool OpenFile(HWND hwnd);
void SaveImage(HWND hwnd);
bool LoadFileSVG(HWND hwnd, const wstring& filePath);
void drawNote(Graphics& graphics, int windowWidth, int windowHeight);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
    InitializeGDIPlus();

    const wchar_t CLASS_NAME[] = L"SVGReader";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreateMenu();
    AppendMenuW(hSubMenu, MF_STRING, IDM_FILE_OPEN, L"&Open a SVG file");
    AppendMenuW(hSubMenu, MF_STRING, IDM_FILE_SAVE, L"&Save image as...");
    AppendMenuW(hSubMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hSubMenu, MF_STRING, IDM_FILE_EXIT, L"&Exit");
    AppendMenuW(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, L"&File");

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"SVG Reader", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, xWindow, yWindow,
        NULL, hMenu, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    if (__argc > 1) {
        wstring cmdFilePath = __wargv[1];
        LoadFileSVG(hwnd, cmdFilePath);
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ShutdownGDIPlus();
    return 0;
}

bool LoadFileSVG(HWND hwnd, const wstring& filePath) {
    try {
        ifstream file(filePath, ios::binary);
        if (!file.is_open()) {
            MessageBoxW(hwnd, L"Cannot open file!", L"Error", MB_OK | MB_ICONERROR);
            return false;
        }
        xmlBuffer.clear();
        xmlBuffer.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        xmlBuffer.push_back('\0');
        vector<char> xmlBuffer2 = xmlBuffer;
        file.close();

        fileXML.readSVG(xmlBuffer);
        screen.clearAll();
        screen.readAll(fileXML);

        svgWidth = 800.0f;
        svgHeight = 600.0f;

        try {
            xml_document<> doc;
            doc.parse<0>(&xmlBuffer2[0]);
            xml_node<>* root = doc.first_node();
            if (root) {
                xml_attribute<>* attrWidth = root->first_attribute("width");
                xml_attribute<>* attrHeight = root->first_attribute("height");
                if (attrWidth) svgWidth = (float)atof(attrWidth->value());
                if (attrHeight) svgHeight = (float)atof(attrHeight->value());

                if (svgWidth <= 0) svgWidth = 800.0f;
                if (svgHeight <= 0) svgHeight = 600.0f;
            }
        }
        catch (...) {}
        
        curRotate = 0;
        fScale = 1.0f;
        fPanX = 0.0f;
        fPanY = 0.0f;

        InvalidateRect(hwnd, NULL, TRUE);
        return true;
    }
    catch (rapidxml::parse_error& err) {
        MessageBoxA(hwnd, err.what(), "XML Parse Error", MB_ICONERROR | MB_OK);
        return false;
    }
    catch (exception& err) {
        MessageBoxA(hwnd, err.what(), "Error", MB_ICONERROR | MB_OK);
        return false;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_OPEN:
            OpenFile(hwnd);
            break;
        case IDM_FILE_SAVE:
            SaveImage(hwnd);
            break;
        case IDM_FILE_EXIT:
            DestroyWindow(hwnd);
            break;
        }
        break;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);

        int width = rcClient.right - rcClient.left;
        int height = rcClient.bottom - rcClient.top;
        if (width <= 0) width = 1;
        if (height <= 0) height = 1;

        Bitmap memBitmap(width, height, PixelFormat32bppARGB);
        Graphics memG(&memBitmap);

        memG.Clear(Color(255, 255, 255, 255));
        memG.SetSmoothingMode(SmoothingModeAntiAlias);
        memG.SetTextRenderingHint(TextRenderingHintAntiAlias);

        Matrix transformMatrix;

        float svgCenterX = svgWidth / 2.0f;
        float svgCenterY = svgHeight / 2.0f;

        // Quay quanh tâm của chính hình đó
        transformMatrix.RotateAt(curRotate, PointF(svgCenterX, svgCenterY), MatrixOrderAppend);

        // Zoom
        transformMatrix.Scale(fScale, fScale, MatrixOrderAppend);

        // Pan
        transformMatrix.Translate(fPanX, fPanY, MatrixOrderAppend);

        memG.SetTransform(&transformMatrix);

        screen.drawAll(&memG);

        drawNote(memG, width, height);

        Graphics graphics(hdc);
        graphics.DrawImage(&memBitmap, 0, 0);

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_RIGHT) {
            curRotate += 10.0f;
            if (curRotate >= 360) curRotate = 0;
        }
        else if (wParam == VK_LEFT) {
            curRotate -= 10.0f;
            if (curRotate < 0) curRotate += 360;
        }
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_MOUSEWHEEL:
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(hwnd, &pt);

        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        float fZoomFactor = 1.1f;

        float fMouseWorldX_Before = (pt.x - fPanX) / fScale;
        float fMouseWorldY_Before = (pt.y - fPanY) / fScale;

        if (delta > 0) fScale *= fZoomFactor;
        else fScale /= fZoomFactor;

        if (fScale < 0.1f) fScale = 0.1f;
        if (fScale > 20.0f) fScale = 20.0f;

        fPanX = pt.x - (fMouseWorldX_Before * fScale);
        fPanY = pt.y - (fMouseWorldY_Before * fScale);

        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    break;

    case WM_MBUTTONDOWN:
        isPanning = true;
        panningStart.x = GET_X_LPARAM(lParam);
        panningStart.y = GET_Y_LPARAM(lParam);
        panningStartX = fPanX;
        panningStartY = fPanY;
        SetCapture(hwnd);
        return 0;

    case WM_MBUTTONUP:
        isPanning = false;
        ReleaseCapture();
        return 0;

    case WM_MOUSEMOVE:
        if (isPanning) {
            int mouseX = GET_X_LPARAM(lParam);
            int mouseY = GET_Y_LPARAM(lParam);
            fPanX = panningStartX + (mouseX - panningStart.x);
            fPanY = panningStartY + (mouseY - panningStart.y);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;

    case WM_LBUTTONDOWN: // Hỗ trợ pan bằng chuột trái
        isPanning = true;
        panningStart.x = GET_X_LPARAM(lParam);
        panningStart.y = GET_Y_LPARAM(lParam);
        panningStartX = fPanX;
        panningStartY = fPanY;
        SetCapture(hwnd);
        return 0;

    case WM_LBUTTONUP:
        isPanning = false;
        ReleaseCapture();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void SaveImage(HWND hwnd) {
    OPENFILENAMEW ofn = {};
    WCHAR szFile[260] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = L"PNG Image (*.png)\0*.png\0JPEG Image (*.jpg)\0*.jpg\0Bitmap (*.bmp)\0*.bmp\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"png";

    if (GetSaveFileNameW(&ofn) == TRUE) {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        int width = rcClient.right - rcClient.left;
        int height = rcClient.bottom - rcClient.top;
        if (width <= 0) width = 1;
        if (height <= 0) height = 1;

        Bitmap saveBitmap(width, height, PixelFormat32bppARGB);
        Graphics saveG(&saveBitmap);

        saveG.Clear(Color(255, 255, 255, 255));
        saveG.SetSmoothingMode(SmoothingModeAntiAlias);
        saveG.SetTextRenderingHint(TextRenderingHintAntiAlias);

        Matrix transformMatrix;
        float svgCenterX = svgWidth / 2.0f;
        float svgCenterY = svgHeight / 2.0f;

        transformMatrix.RotateAt(curRotate, PointF(svgCenterX, svgCenterY), MatrixOrderAppend);
        transformMatrix.Scale(fScale, fScale, MatrixOrderAppend);
        transformMatrix.Translate(fPanX, fPanY, MatrixOrderAppend);

        saveG.SetTransform(&transformMatrix);

        screen.drawAll(&saveG);

        CLSID encoderClsid;
        if (ofn.nFilterIndex == 1)
            GetEncoderClsid(L"image/png", &encoderClsid);
        else if (ofn.nFilterIndex == 2) 
            GetEncoderClsid(L"image/jpeg", &encoderClsid);
        else GetEncoderClsid(L"image/bmp", &encoderClsid);

        saveBitmap.Save(ofn.lpstrFile, &encoderClsid, NULL);
    }
}

bool OpenFile(HWND hwnd) {
    OPENFILENAMEW ofn = {};
    WCHAR szFile[260] = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = L"SVG Files (*.svg)\0*.svg\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE) {
        return LoadFileSVG(hwnd, ofn.lpstrFile);
    }
    return false;
}

void InitializeGDIPlus() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

void ShutdownGDIPlus() {
    screen.clearAll();
    GdiplusShutdown(gdiplusToken);
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT  num = 0;
    UINT  size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;
    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;
    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    free(pImageCodecInfo);
    return -1;
}

void drawNote(Graphics& graphics, int windowWidth, int windowHeight) {
    Matrix oldMatrix;
    graphics.GetTransform(&oldMatrix);
    graphics.ResetTransform();

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 10, FontStyleRegular, UnitPoint);
    SolidBrush textBrush(Color(255, 255, 255, 255));
    SolidBrush bgBrush(Color(180, 0, 0, 0));

    vector<wstring> lines = {
        L"Open file: Use Command line or File -> Open a SVG file",
        L"Save file as PNG/JPG/BMP: File -> Save image as...",
        L"Drag and drop: Panning",
        L"Scroll: Zoom in / Zoom out",
        L"Press <-: Rotate left",
        L"Press ->: Rotate right"
    };

    float lineHeight = 20.0f;
    float padding = 10.0f;
    float boxWidth = 365.0f;
    float boxHeight = (lines.size() * lineHeight) + (padding * 2);

    float x = (float)windowWidth - boxWidth - 10.0f;
    float y = (float)windowHeight - boxHeight - 10.0f;

    graphics.FillRectangle(&bgBrush, x, y, boxWidth, boxHeight);

    PointF textPos(x + padding, y + padding);
    for (const auto& line : lines) {
        graphics.DrawString(line.c_str(), -1, &font, textPos, &textBrush);
        textPos.Y += lineHeight;
    }
    graphics.SetTransform(&oldMatrix);
}