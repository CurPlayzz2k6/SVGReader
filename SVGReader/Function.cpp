#include "Function.h"

// Bảng màu HTML cơ bản
const map<string, string> htmlColor = {
    {"none", "#00000000"},
    {"aliceblue", "#FFF0F8FF"},
    {"antiquewhite", "#FFFAEBD7"},
    {"aqua", "#FF00FFFF"},
    {"aquamarine", "#FF7FFFD4"},
    {"azure", "#FFF0FFFF"},
    {"beige", "#FFF5F5DC"},
    {"bisque", "#FFFFE4C4"},
    {"black", "#FF000000"},
    {"blanchedalmond", "#FFFFEBCD"},
    {"blue", "#FF0000FF"},
    {"blueviolet", "#FF8A2BE2"},
    {"brown", "#FFA52A2A"},
    {"burlywood", "#FFDEB887"},
    {"cadetblue", "#FF5F9EA0"},
    {"chartreuse", "#FF7FFF00"},
    {"chocolate", "#FFD2691E"},
    {"coral", "#FFFF7F50"},
    {"cornflowerblue", "#FF6495ED"},
    {"cornsilk", "#FFFFF8DC"},
    {"crimson", "#FFDC143C"},
    {"cyan", "#FF00FFFF"},
    {"darkblue", "#FF00008B"},
    {"darkcyan", "#FF008B8B"},
    {"darkgoldenrod", "#FFB8860B"},
    {"darkgray", "#FFA9A9A9"},
    {"darkgreen", "#FF006400"},
    {"darkgrey", "#FFA9A9A9"},
    {"darkkhaki", "#FFBDB76B"},
    {"darkmagenta", "#FF8B008B"},
    {"darkolivegreen", "#FF556B2F"},
    {"darkorange", "#FFFF8C00"},
    {"darkorchid", "#FF9932CC"},
    {"darkred", "#FF8B0000"},
    {"darksalmon", "#FFE9967A"},
    {"darkseagreen", "#FF8FBC8F"},
    {"darkslateblue", "#FF483D8B"},
    {"darkslategray", "#FF2F4F4F"},
    {"darkslategrey", "#FF2F4F4F"},
    {"darkturquoise", "#FF00CED1"},
    {"darkviolet", "#FF9400D3"},
    {"deeppink", "#FFFF1493"},
    {"deepskyblue", "#FF00BFFF"},
    {"dimgray", "#FF696969"},
    {"dimgrey", "#FF696969"},
    {"dodgerblue", "#FF1E90FF"},
    {"firebrick", "#FFB22222"},
    {"floralwhite", "#FFFFFAF0"},
    {"forestgreen", "#FF228B22"},
    {"fuchsia", "#FFFF00FF"},
    {"gainsboro", "#FFDCDCDC"},
    {"ghostwhite", "#FFF8F8FF"},
    {"gold", "#FFFFD700"},
    {"goldenrod", "#FFDAA520"},
    {"gray", "#FF808080"},
    {"green", "#FF008000"},
    {"greenyellow", "#FFADFF2F"},
    {"grey", "#FF808080"},
    {"honeydew", "#FFF0FFF0"},
    {"hotpink", "#FFFF69B4"},
    {"indianred", "#FFCD5C5C"},
    {"indigo", "#FF4B0082"},
    {"ivory", "#FFFFFFF0"},
    {"khaki", "#FFF0E68C"},
    {"lavender", "#FFE6E6FA"},
    {"lavenderblush", "#FFFFF0F5"},
    {"lawngreen", "#FF7CFC00"},
    {"lemonchiffon", "#FFFFFACD"},
    {"lightblue", "#FFADD8E6"},
    {"lightcoral", "#FFF08080"},
    {"lightcyan", "#FFE0FFFF"},
    {"lightgoldenrodyellow", "#FFFAFAD2"},
    {"lightgray", "#FFD3D3D3"},
    {"lightgreen", "#FF90EE90"},
    {"lightgrey", "#FFD3D3D3"},
    {"lightpink", "#FFFFB6C1"},
    {"lightsalmon", "#FFFFA07A"},
    {"lightseagreen", "#FF20B2AA"},
    {"lightskyblue", "#FF87CEFA"},
    {"lightslategray", "#FF778899"},
    {"lightslategrey", "#FF778899"},
    {"lightsteelblue", "#FFB0C4DE"},
    {"lightyellow", "#FFFFFFE0"},
    {"lime", "#FF00FF00"},
    {"limegreen", "#FF32CD32"},
    {"linen", "#FFFAF0E6"},
    {"magenta", "#FFFF00FF"},
    {"maroon", "#FF800000"},
    {"mediumaquamarine", "#FF66CDAA"},
    {"mediumblue", "#FF0000CD"},
    {"mediumorchid", "#FFBA55D3"},
    {"mediumpurple", "#FF9370DB"},
    {"mediumseagreen", "#FF3CB371"},
    {"mediumslateblue", "#FF7B68EE"},
    {"mediumspringgreen", "#FF00FA9A"},
    {"mediumturquoise", "#FF48D1CC"},
    {"mediumvioletred", "#FFC71585"},
    {"midnightblue", "#FF191970"},
    {"mintcream", "#FFF5FFFA"},
    {"mistyrose", "#FFFFE4E1"},
    {"moccasin", "#FFFFE4B5"},
    {"navajowhite", "#FFFFDEAD"},
    {"navy", "#FF000080"},
    {"oldlace", "#FFFDF5E6"},
    {"olive", "#FF808000"},
    {"olivedrab", "#FF6B8E23"},
    {"orange", "#FFFFA500"},
    {"orangered", "#FFFF4500"},
    {"orchid", "#FFDA70D6"},
    {"palegoldenrod", "#FFEEE8AA"},
    {"palegreen", "#FF98FB98"},
    {"paleturquoise", "#FFAFEEEE"},
    {"palevioletred", "#FFDB7093"},
    {"papayawhip", "#FFFFEFD5"},
    {"peachpuff", "#FFFFDAB9"},
    {"peru", "#FFCD853F"},
    {"pink", "#FFFFC0CB"},
    {"plum", "#FFDDA0DD"},
    {"powderblue", "#FFB0E0E6"},
    {"purple", "#FF800080"},
    {"rebeccapurple", "#FF663399"},
    {"red", "#FFFF0000"},
    {"rosybrown", "#FFBC8F8F"},
    {"royalblue", "#FF4169E1"},
    {"saddlebrown", "#FF8B4513"},
    {"salmon", "#FFFA8072"},
    {"sandybrown", "#FFF4A460"},
    {"seagreen", "#FF2E8B57"},
    {"seashell", "#FFFFF5EE"},
    {"sienna", "#FFA0522D"},
    {"silver", "#FFC0C0C0"},
    {"skyblue", "#FF87CEEB"},
    {"slateblue", "#FF6A5ACD"},
    {"slategray", "#FF708090"},
    {"slategrey", "#FF708090"},
    {"snow", "#FFFFFAFA"},
    {"springgreen", "#FF00FF7F"},
    {"steelblue", "#FF4682B4"},
    {"tan", "#FFD2B48C"},
    {"teal", "#FF008080"},
    {"thistle", "#FFD8BFD8"},
    {"tomato", "#FFFF6347"},
    {"turquoise", "#FF40E0D0"},
    {"violet", "#FFEE82EE"},
    {"wheat", "#FFF5DEB3"},
    {"white", "#FFFFFFFF"},
    {"whitesmoke", "#FFF5F5F5"},
    {"yellow", "#FFFFFF00"},
    {"yellowgreen", "#FF9ACD32"}
};

string getUrlId(string value) {
    if (value.find("url(") != string::npos) {
        size_t startIndex = value.find("#");
        size_t endIndex = value.find(")");
        if (startIndex != string::npos && endIndex != string::npos) {
            return value.substr(startIndex + 1, endIndex - startIndex - 1);
        }
    }
    return "";
}

string formatTransformString(string s) {
    string res = "";
    for (char c : s) {
        if (c == ',' || c == '(' || c == ')') res += ' ';
        else res += c;
    }
    return res;
}

string toLowerStr(string str) {
    for (size_t i = 0; i < str.size(); i++)
        str[i] = tolower(str[i]);
    return str;
}

int convertHexToDec(string hex) {
    map<char, int> hexToDec = { {'0', 0}, {'1', 1}, {'2', 2}, {'3', 3},
                                {'4', 4}, {'5', 5}, {'6', 6}, {'7', 7},
                                {'8', 8}, {'9', 9}, {'a', 10}, {'b', 11},
                                {'c', 12}, {'d', 13}, {'e', 14}, {'f', 15} };
    return (hexToDec[tolower(hex[0])] * 16) + hexToDec[tolower(hex[1])];
}

Color getRGB(string colorStr) {
    vector<int> rgb(4, 0);
    colorStr = toLowerStr(colorStr);
    string tmpStr = "";
    if (colorStr.size() == 4 && colorStr[0] == '#') {
        tmpStr += colorStr[0];
        tmpStr += "FF";
        tmpStr += colorStr[1];
        tmpStr += colorStr[1];
        tmpStr += colorStr[2];
        tmpStr += colorStr[2];
        tmpStr += colorStr[3];
        tmpStr += colorStr[3];
        colorStr = tmpStr;
    }

    if (colorStr == "#00000000" || colorStr == "none" || colorStr == "transparent") {
        return Color(0, 0, 0, 0);
    }

    if (colorStr[0] == '#' && colorStr.size() == 9) {
        rgb[0] = convertHexToDec(colorStr.substr(1, 2));
        rgb[1] = convertHexToDec(colorStr.substr(3, 2));
        rgb[2] = convertHexToDec(colorStr.substr(5, 2));
        rgb[3] = convertHexToDec(colorStr.substr(7, 2));
        return Color(rgb[0], rgb[1], rgb[2], rgb[3]);
    }
    else if (colorStr[0] == '#' && colorStr.size() == 7) {
        rgb[0] = convertHexToDec(colorStr.substr(1, 2));
        rgb[1] = convertHexToDec(colorStr.substr(3, 2));
        rgb[2] = convertHexToDec(colorStr.substr(5, 2));
        return Color(255, rgb[0], rgb[1], rgb[2]);
    }
    else if (htmlColor.find(colorStr) != htmlColor.end()) {
        auto itrValue = htmlColor.find(colorStr);
        return getRGB(itrValue->second);
    }
    else {
        int id = 0;
        for (size_t i = 0; i < colorStr.size(); i++) {
            if (isdigit(colorStr[i])) {
                rgb[id] = 10 * rgb[id] + (colorStr[i] - '0');
            }
            else if (colorStr[i] == ',') {
                id++;
                if (id >= 4) break;
            }
            else if (colorStr[i] == ')') {
                break;
            }
        }

        for (int i = 0; i < 4; i++) {
            if (rgb[i] > 255) rgb[i] = 255;
        }

        if (id == 2)
            return Color(255, rgb[0], rgb[1], rgb[2]);
        else
            return Color(rgb[0], rgb[1], rgb[2], rgb[3]);
    }
}

wstring ConvertStringToWstring(const string& str)
{
    try {
        wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }
    catch (const range_error&) {
        return wstring();
    }
}

vector<PointF> getPolyPoints(string points) {
    string s = points;
    replace(s.begin(), s.end(), ',', ' ');

    for (char& c : s) {
        if (c == '\n' || c == '\r' || c == '\t') c = ' ';
    }

    stringstream ss(s);
    ss.imbue(std::locale("C"));

    vector<PointF> polyPoints;
    float x, y;

    while (ss >> x && ss >> y) {
        polyPoints.push_back(PointF(x, y));
    }

    return polyPoints;
}

float minValue(float a, float b) {
    return (a < b) ? a : b;
}

string cleanTransformString(string s) {
    string res = "";
    for (char c : s) {
        if (c == ',' || c == '(' || c == ')') res += ' ';
        else res += c;
    }
    return res;
}

float parseFloat(stringstream& ss) {
    float val = 0;
    if (!(ss >> val)) { ss.clear(); return 0; }
    return val;
}

float parseString(const string& s) {
    stringstream ss(s);
    ss.imbue(locale("C"));
    float val = 0;
    ss >> val;
    return val;
}