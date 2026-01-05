#include "TextSVG.h"
#include "DefinitionsSVG.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Gdiplus;
using namespace std;

static void normalizeStopsLinear(const vector<GradientStopData>& inputStops, vector<Color>& outColors, vector<float>& outPositions, float opacityMultiplier) {
    if (inputStops.empty()) {
        return;
    }
    vector<GradientStopData> stops = inputStops;
    stable_sort(stops.begin(), stops.end(), [](const GradientStopData& a, const GradientStopData& b) { 
        return a.offset < b.offset; 
        });

    if (stops.front().offset > 0.001f) {
        GradientStopData s = stops.front();
        s.offset = 0.0f;
        stops.insert(stops.begin(), s);
    }
    if (stops.back().offset < 0.999f) {
        GradientStopData s = stops.back();
        s.offset = 1.0f;
        stops.push_back(s);
    }

    outColors.clear();
    outPositions.clear();
    for (const auto& s : stops) {
        float pos = s.offset;
        if (!outPositions.empty() && pos <= outPositions.back()) {
            pos = outPositions.back() + 0.00001f;
        }
        if (pos > 1.0f) {
            pos = 1.0f;
        }
        outPositions.push_back(pos);
        Color c = s.color;
        outColors.push_back(Color((BYTE)(c.GetA() * opacityMultiplier), c.GetR(), c.GetG(), c.GetB()));
    }
}

static wstring getValidFontFamily(const string& requestedFamily) {
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

static StringAlignment getValidAlignment(const string& anchor) {
    if (anchor == "middle") {
        return StringAlignmentCenter;
    }
    if (anchor == "end") {
        return StringAlignmentFar;
    }
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
    hasFillColor = false;

    hasFillOpacity = false;
    hasStrokeColor = false;
    hasStrokeWidth = false;
    hasStrokeOpacity = false;
    hasOpacity = false;
    fillGradientId = "";

    if (node->value()) {
        string str(node->value());
        text = "";
        bool lastIsSpace = false;
        for (char c : str) {
            if (c == '\n' || c == '\r' || c == '\t') {
                c = ' ';
            }
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
        if (!text.empty() && text.front() == ' ') {
            text.erase(0, 1);
        }
        if (!text.empty() && text.back() == ' ') {
            text.pop_back();
        }
    }

    for (xml_attribute<>* attribute = node->first_attribute(); attribute; attribute = attribute->next_attribute()) {
        string attributeName(attribute->name());
        if (attributeName == "x") {
            x = stof(attribute->value());
        }
        else if (attributeName == "y") {
            y = stof(attribute->value());
        }
        else if (attributeName == "rotate") {
            rotate = stof(attribute->value());
        }
        else if (attributeName == "font-family") {
            fontFamily = attribute->value();
        }
        else if (attributeName == "font-style") {
            fontStyle = attribute->value();
        }
        else if (attributeName == "font-size") {
            fontSize = stof(attribute->value());
        }
        else if (attributeName == "text-anchor") {
            textAnchor = attribute->value();
        }
        else if (attributeName == "opacity") {
            opacity.setOpacity(stof(attribute->value()));
            hasOpacity = true;
        }
        else if (attributeName == "fill") {
            string val = attribute->value();
            if (val.find("url(") != string::npos) {
                fillGradientId = getUrlId(val);
                hasFillColor = true;
            }
            else {
                fill.setFillColor(getRGB(val));
                hasFillColor = true;
            }
        }
        else if (attributeName == "fill-opacity") {
            fill.setFillOpacity(stof(attribute->value()));
            hasFillOpacity = true;
        }
        else if (attributeName == "stroke") {
            stroke.setStrokeColor(getRGB(attribute->value()));
            hasStrokeColor = true;
        }
        else if (attributeName == "stroke-width") {
            stroke.setStrokeWidth(stof(attribute->value()));
            hasStrokeWidth = true;
        }
        else if (attributeName == "stroke-opacity") {
            stroke.setStrokeOpacity(stof(attribute->value()));
            hasStrokeOpacity = true;
        }
        else if (attributeName == "transform") {
            parseTransform(attribute->value());
        }
    }
}

void TextSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    if (text.empty()) {
        return;
    }

    GraphicsState graphicsState = graphics.Save();

    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
    graphics.SetCompositingMode(CompositingModeSourceOver);

    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);

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

    RectF localBounds;
    path.GetBounds(&localBounds);
    if (localBounds.Width <= 0) localBounds.Width = 0.1f;
    if (localBounds.Height <= 0) localBounds.Height = 0.1f;

    float opacityAll = opacity.getOpacity() * 255.0f;

    const LinearGradientData* linearGradient = defs.getLinearGradient(fillGradientId);
    const RadialGradientData* radialGradient = (!linearGradient) ? defs.getRadialGradient(fillGradientId) : nullptr;

    if (linearGradient || radialGradient) {
        Matrix worldMatrix;
        graphics.GetTransform(&worldMatrix);

        int rampWidth = 256;
        vector<Color> colorTable(rampWidth);
        vector<Color> colors;
        vector<float> positions;
        if (linearGradient) {
            normalizeStopsLinear(linearGradient->stops, colors, positions, opacityAll / 255.0f * fill.getFillOpacity());
        }
        else {
            normalizeStopsLinear(radialGradient->stops, colors, positions, opacityAll / 255.0f * fill.getFillOpacity());
        }

        if (!colors.empty()) {
            for (int i = 0; i < rampWidth; ++i) {
                float pos = (float)i / (float)(rampWidth - 1);
                size_t idx = 0;
                for (size_t k = 0; k < positions.size() - 1; ++k) {
                    if (pos >= positions[k]) idx = k;
                }
                if (pos <= positions.front()) {
                    colorTable[i] = colors.front();
                    continue;
                }
                if (pos >= positions.back()) {
                    colorTable[i] = colors.back();
                    continue;
                }
                float t = (pos - positions[idx]) / (positions[idx + 1] - positions[idx]);
                Color c1 = colors[idx];
                Color c2 = colors[idx + 1];
                BYTE a = (BYTE)(c1.GetA() + (c2.GetA() - c1.GetA()) * t);
                BYTE r = (BYTE)(c1.GetR() + (c2.GetR() - c1.GetR()) * t);
                BYTE g = (BYTE)(c1.GetG() + (c2.GetG() - c1.GetG()) * t);
                BYTE b = (BYTE)(c1.GetB() + (c2.GetB() - c1.GetB()) * t);
                colorTable[i] = Color(a, r, g, b);
            }
        }

        GraphicsPath* screenPath = path.Clone();
        screenPath->Transform(&worldMatrix);
        screenPath->Flatten(NULL, 0.5f);
        RectF globalBounds;
        screenPath->GetBounds(&globalBounds);
        delete screenPath;

        int padding = 2;
        int boundsX = (int)floor(globalBounds.X) - padding;
        int boundsY = (int)floor(globalBounds.Y) - padding;
        int boundsWidth = (int)ceil(globalBounds.X + globalBounds.Width) - boundsX + padding;
        int boundsHeight = (int)ceil(globalBounds.Y + globalBounds.Height) - boundsY + padding;

        if (boundsWidth > 20000) boundsWidth = 20000;
        if (boundsHeight > 20000) boundsHeight = 20000;
        if (boundsWidth <= 0) boundsWidth = 1;
        if (boundsHeight <= 0) boundsHeight = 1;

        Bitmap canvas(boundsWidth, boundsHeight, PixelFormat32bppARGB);
        Graphics graphicsCanvas(&canvas);
        graphicsCanvas.Clear(Color(0, 0, 0, 0));

        Matrix totalMatrix;
        if (linearGradient) {
            PointF point1 = linearGradient->startPoint;
            PointF point2 = linearGradient->endPoint;
            float deltaX, deltaY;
            if (linearGradient->units == UnitsObjectBoundingBox) {
                deltaX = (point2.X - point1.X) * localBounds.Width;
                deltaY = (point2.Y - point1.Y) * localBounds.Height;
                totalMatrix.Translate(localBounds.X + point1.X * localBounds.Width, localBounds.Y + point1.Y * localBounds.Height, MatrixOrderAppend);
            }
            else {
                deltaX = point2.X - point1.X;
                deltaY = point2.Y - point1.Y;
                totalMatrix.Translate(point1.X, point1.Y, MatrixOrderAppend);
            }
            float length = sqrt(deltaX * deltaX + deltaY * deltaY);
            float angle = atan2(deltaY, deltaX) * 180.0f / (float)M_PI;
            Matrix forwardMatrix;
            forwardMatrix.Scale(length, 1.0f, MatrixOrderAppend);
            forwardMatrix.Rotate(angle, MatrixOrderAppend);
            REAL matrixElements[6];
            totalMatrix.GetElements(matrixElements);
            forwardMatrix.Translate(matrixElements[4], matrixElements[5], MatrixOrderAppend);
            forwardMatrix.Multiply(&const_cast<Matrix&>(linearGradient->transform), MatrixOrderAppend);
            forwardMatrix.Multiply(&worldMatrix, MatrixOrderAppend);
            forwardMatrix.GetElements(matrixElements);
            totalMatrix.SetElements(matrixElements[0], matrixElements[1], matrixElements[2], matrixElements[3], matrixElements[4], matrixElements[5]);
            totalMatrix.Invert();
        }
        else {
            totalMatrix.Multiply(&const_cast<Matrix&>(radialGradient->transform), MatrixOrderAppend);
            if (radialGradient->units == UnitsObjectBoundingBox) {
                totalMatrix.Scale(localBounds.Width, localBounds.Height, MatrixOrderAppend);
                totalMatrix.Translate(localBounds.X, localBounds.Y, MatrixOrderAppend);
            }
            totalMatrix.Multiply(&worldMatrix, MatrixOrderAppend);
            totalMatrix.Invert();
        }

        BitmapData canvasData;
        Rect canvasRect(0, 0, boundsWidth, boundsHeight);
        canvas.LockBits(&canvasRect, ImageLockModeWrite, PixelFormat32bppARGB, &canvasData);
        UINT* outPixels = (UINT*)canvasData.Scan0;
        int stride = canvasData.Stride / 4;

        float radialCx = 0, radialCy = 0, radialR = 0, radialFx = 0, radialFy = 0, radialRSq = 0, radialDistF = 0, radialFxC = 0, radialFyC = 0, radialCVal = 0;
        if (radialGradient) {
            radialCx = radialGradient->cx;
            radialCy = radialGradient->cy;
            radialR = radialGradient->r;
            radialFx = radialGradient->fx;
            radialFy = radialGradient->fy;
            radialRSq = radialR * radialR;
            if (radialRSq < 1e-6f) radialRSq = 1e-6f;
            radialDistF = sqrt(pow(radialFx - radialCx, 2) + pow(radialFy - radialCy, 2));
            if (radialDistF >= radialR * 0.99f) {
                float s = (radialR * 0.99f) / radialDistF;
                radialFx = radialCx + (radialFx - radialCx) * s;
                radialFy = radialCy + (radialFy - radialCy) * s;
            }
            radialFxC = radialFx - radialCx;
            radialFyC = radialFy - radialCy;
            radialCVal = radialFxC * radialFxC + radialFyC * radialFyC - radialRSq;
        }
        GradientSpreadMethod spreadMethod = linearGradient ? linearGradient->spreadMethod : radialGradient->spreadMethod;

        for (int y = 0; y < boundsHeight; y++) {
            for (int x = 0; x < boundsWidth; x++) {
                PointF point((float)(boundsX + x), (float)(boundsY + y));
                totalMatrix.TransformPoints(&point);
                float t = -1.0f;
                if (linearGradient) t = point.X;
                else {
                    if (abs(radialFx - radialCx) < 1e-5f && abs(radialFy - radialCy) < 1e-5f) {
                        float distSq = pow(point.X - radialCx, 2) + pow(point.Y - radialCy, 2);
                        t = sqrt(distSq / radialRSq);
                    }
                    else {
                        float dx = point.X - radialFx;
                        float dy = point.Y - radialFy;
                        float termA = dx * dx + dy * dy;
                        float termB = 2 * (radialFxC * dx + radialFyC * dy);
                        if (termA >= 1e-6f) {
                            float delta = termB * termB - 4 * termA * radialCVal;
                            if (delta >= 0) t = 1.0f / ((-termB + sqrt(delta)) / (2 * termA));
                        }
                    }
                }
                if (spreadMethod == SpreadMethodPad) {
                    if (t < 0.0f) t = 0.0f;
                    if (t > 1.0f) t = 1.0f;
                }
                else if (spreadMethod == SpreadMethodRepeat) {
                    t = t - floor(t);
                }
                else if (spreadMethod == SpreadMethodReflect) {
                    t = 2.0f * fabs(round(0.5f * t) - 0.5f * t);
                }
                if (t >= 0.0f && t <= 1.0f) {
                    int index = (int)(t * (rampWidth - 1));
                    if (index < 0) index = 0;
                    if (index >= rampWidth) index = rampWidth - 1;
                    outPixels[y * stride + x] = colorTable[index].GetValue();
                }
            }
        }
        canvas.UnlockBits(&canvasData);

        Matrix oldMatrix;
        graphics.GetTransform(&oldMatrix);
        graphics.ResetTransform();
        TextureBrush textureBrush(&canvas);
        textureBrush.TranslateTransform((float)boundsX, (float)boundsY);
        GraphicsPath* renderPath = path.Clone();
        renderPath->Transform(&worldMatrix);
        graphics.FillPath(&textureBrush, renderPath);
        delete renderPath;
        graphics.SetTransform(&oldMatrix);
    }
    else {
        Color rgbFill = fill.getFillColor();
        if (rgbFill.GetA() != 0) {
            BYTE finalAlpha = (BYTE)((rgbFill.GetA() * opacityAll * fill.getFillOpacity()) / 255.0f);
            Color finalFillColor(finalAlpha, rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
            SolidBrush brush(finalFillColor);
            graphics.FillPath(&brush, &path);
        }
    }

    Color rgbStroke = stroke.getStrokeColor();
    if (rgbStroke.GetA() > 0) {
        float calculatedStrokeAlpha = opacityAll * stroke.getStrokeOpacity();
        float opStroke = minValue(calculatedStrokeAlpha, (float)rgbStroke.GetA());
        float currentStrokeWidth = stroke.getStrokeWidth();

        if (opStroke > 0 && currentStrokeWidth <= 0.0f) {
            currentStrokeWidth = 1.0f;
        }

        if (opStroke > 0 && currentStrokeWidth > 0) {
            Color finalStrokeColor((BYTE)opStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
            Pen pen(finalStrokeColor, currentStrokeWidth);
            graphics.DrawPath(&pen, &path);
        }
    }

    graphics.Restore(graphicsState);
}

TextSVG::~TextSVG() {}