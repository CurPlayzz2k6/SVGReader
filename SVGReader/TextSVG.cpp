#include "TextSVG.h"
#include "DefinitionsSVG.h"
#include <iostream>
#include <algorithm>

using namespace Gdiplus;
using namespace std;

wstring getValidFontFamily(const string& requestedFamily) {
    wstring wRequestedFamily = ConvertStringToWstring(requestedFamily);
    FontFamily testFont(wRequestedFamily.c_str());

    WCHAR outputName[LF_FACESIZE];
    if (testFont.GetFamilyName(outputName) == Ok) {
        if (wcscmp(wRequestedFamily.c_str(), outputName) == 0) {
            return wRequestedFamily;
        }
    }
    return L"Times New Roman";
}

StringAlignment getValidAlignment(const string& anchor) {
    if (anchor == "middle") 
        return StringAlignmentCenter;
    if (anchor == "end") 
        return StringAlignmentFar;
    return StringAlignmentNear;
}

TextSVG::TextSVG() {
    this->x = 0;
    this->y = 0;
    this->rotate = 0;
    this->fontSize = 36.0f;
    this->fontFamily = "Times New Roman";
    this->fontStyle = "normal";
    this->text = "";
    this->textAnchor = "start";
}

void TextSVG::read(xml_node<>* node) {
    if (node->value()) {
        string str(node->value());
        text = "";
        bool lastIsSpace = false;
        for (char c : str) {
            if (c == '\n' || c == '\r' || c == '\t') 
                c = ' ';
            if (c == ' ') {
                if (!lastIsSpace) {
                    text += ' ';
                    lastIsSpace = true;
                }
            }
            else {
                text += c;
                lastIsSpace = false;
            }
        }
        if (!text.empty() && text.front() == ' ') 
            text.erase(0, 1);
        if (!text.empty() && text.back() == ' ') 
            text.pop_back();
    }

    for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
        string type(attr->name());
        if (type == "x") 
            x = stof(attr->value());
        else if (type == "y") 
            y = stof(attr->value());
        else if (type == "rotate") 
            rotate = stof(attr->value());
        else if (type == "font-family") 
            fontFamily = attr->value();
        else if (type == "font-style") 
            fontStyle = attr->value();
        else if (type == "font-size") 
            fontSize = stof(attr->value());
        else if (type == "text-anchor") 
            textAnchor = attr->value();
        else if (type == "opacity") { 
            opacity.setOpacity(stof(attr->value())); 
            hasOpacity = true; 
        }
        else if (type == "fill") { 
            fill.setFillColor(getRGB(attr->value())); 
            hasFillColor = true; 
        }
        else if (type == "fill-opacity") { 
            fill.setFillOpacity(stof(attr->value())); 
            hasFillOpacity = true; 
        }
        else if (type == "stroke") { 
            stroke.setStrokeColor(getRGB(attr->value())); 
            hasStrokeColor = true; 
        }
        else if (type == "stroke-width") { 
            stroke.setStrokeWidth(stof(attr->value())); 
            hasStrokeWidth = true; 
        }
        else if (type == "stroke-opacity") { 
            stroke.setStrokeOpacity(stof(attr->value())); 
            hasStrokeOpacity = true; 
        }
        else if (type == "transform") 
            parseTransform(attr->value());
    }
}

void TextSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    if (text.empty()) 
        return;

    GraphicsState state = graphics.Save();
    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

    wstring wFamily = getValidFontFamily(this->fontFamily);
    FontFamily fontFamilyObj(wFamily.c_str());

    int style = FontStyleRegular;
    if (this->fontStyle == "italic" || this->fontStyle == "oblique") style = FontStyleItalic;
    if (this->fontStyle == "bold") style = FontStyleBold;

    UINT16 ascent = fontFamilyObj.GetCellAscent(style);
    UINT16 emHeight = fontFamilyObj.GetEmHeight(style);
    float ascentPixel = (float)ascent / emHeight * this->fontSize;
    float gdiY = this->y - ascentPixel;
    PointF origin(this->x, gdiY);

    if (this->rotate != 0) {
        graphics.TranslateTransform(this->x, this->y);
        graphics.RotateTransform(this->rotate);
        graphics.TranslateTransform(-this->x, -this->y);
    }

    GraphicsPath path;
    StringFormat format;
    format.SetAlignment(getValidAlignment(this->textAnchor));
    format.SetLineAlignment(StringAlignmentNear);
    format.SetFormatFlags(StringFormatFlagsNoWrap | StringFormatFlagsMeasureTrailingSpaces);

    wstring wText = ConvertStringToWstring(this->text);
    path.AddString(wText.c_str(), -1, &fontFamilyObj, style, this->fontSize, origin, &format);

    float globalOpacity = opacity.getOpacity() * 255.0f;

    Color rgbFill = fill.getFillColor();
    float finalFillAlpha = globalOpacity * fill.getFillOpacity();
    if (finalFillAlpha > rgbFill.GetA()) finalFillAlpha = (float)rgbFill.GetA();

    Color finalFillColor((BYTE)finalFillAlpha, rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
    SolidBrush brush(finalFillColor);

    Color rgbStroke = stroke.getStrokeColor();
    float finalStrokeAlpha = globalOpacity * stroke.getStrokeOpacity();
    if (finalStrokeAlpha > rgbStroke.GetA()) finalStrokeAlpha = (float)rgbStroke.GetA();

    Color finalStrokeColor((BYTE)finalStrokeAlpha, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
    float currentStrokeWidth = stroke.getStrokeWidth();
    if (finalStrokeAlpha > 0 && currentStrokeWidth <= 0.0f)
        currentStrokeWidth = 1.0f;
    Pen pen(finalStrokeColor, currentStrokeWidth);

    if (finalFillAlpha > 0)
        graphics.FillPath(&brush, &path);
    if (finalStrokeAlpha > 0 && currentStrokeWidth > 0)
        graphics.DrawPath(&pen, &path);

    graphics.Restore(state);
}

TextSVG::~TextSVG() {}