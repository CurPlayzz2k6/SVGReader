#ifndef FUNCTION_H
#define FUNCTION_H
#include <windows.h>
#include <gdiplus.h>
#include <commdlg.h>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <map>
#include <locale>
#include <codecvt>
#include "rapidxml.hpp"

using namespace std;
using namespace rapidxml;
using namespace Gdiplus;

// Hàm format: Biến dấu phẩy, ngoặc thành khoảng trắng để stringstream dễ đọc
string getUrlId(string value);
string formatTransformString(string s);
string toLowerStr(string str);
int convertHexToDec(string hex);
Color getRGB(string rgbStr);
wstring ConvertStringToWstring(const string& str);
vector<PointF> getPolyPoints(string points);
float minValue(float a, float b);

#endif FUNCTION_H