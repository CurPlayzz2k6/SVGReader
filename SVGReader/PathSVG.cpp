#include "PathSVG.h"
#include "DefinitionsSVG.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Gdiplus;
using namespace std;

// Hàm hỗ trợ chuyển đổi Quadratic Bezier sang Cubic Bezier (GDI+ chỉ hỗ trợ Cubic)
static void addQuadraticBezier(GraphicsPath& path, float x0, float y0, float x1, float y1, float x2, float y2) {
    float cx1 = x0 + (2.0f / 3.0f) * (x1 - x0);
    float cy1 = y0 + (2.0f / 3.0f) * (y1 - y0);
    float cx2 = x2 + (2.0f / 3.0f) * (x1 - x2);
    float cy2 = y2 + (2.0f / 3.0f) * (y1 - y2);
    path.AddBezier(x0, y0, cx1, cy1, cx2, cy2, x2, y2);
}

// Hàm hỗ trợ vẽ Arc theo chuẩn SVG
static void addSvgArc(GraphicsPath& path, float x0, float y0, float rx, float ry, float angle, bool largeArcFlag, bool sweepFlag, float x, float y) {
    if (rx == 0 || ry == 0) {
        path.AddLine(x0, y0, x, y);
        return;
    }
    if (x0 == x && y0 == y) {
        return;
    }

    rx = abs(rx);
    ry = abs(ry);

    float angleRad = angle * (float)M_PI / 180.0f;
    float cosAngle = cos(angleRad);
    float sinAngle = sin(angleRad);

    float dx2 = (x0 - x) / 2.0f;
    float dy2 = (y0 - y) / 2.0f;
    float x1 = cosAngle * dx2 + sinAngle * dy2;
    float y1 = -sinAngle * dx2 + cosAngle * dy2;

    float radiiCheck = x1 * x1 / (rx * rx) + y1 * y1 / (ry * ry);
    if (radiiCheck > 1) {
        rx *= sqrt(radiiCheck);
        ry *= sqrt(radiiCheck);
    }

    float sign = (largeArcFlag == sweepFlag) ? -1.0f : 1.0f;
    float numer = (rx * rx * ry * ry) - (rx * rx * y1 * y1) - (ry * ry * x1 * x1);
    float denom = (rx * rx * y1 * y1) + (ry * ry * x1 * x1);
    float sq = (denom == 0) ? 0 : numer / denom;
    if (sq < 0) {
        sq = 0;
    }

    float coef = sign * sqrt(sq);
    float cx1 = coef * ((rx * y1) / ry);
    float cy1 = coef * -((ry * x1) / rx);

    float cx = x0 + (x - x0) / 2.0f + cosAngle * cx1 - sinAngle * cy1;
    float cy = y0 + (y - y0) / 2.0f + sinAngle * cx1 + cosAngle * cy1;

    auto angleBetween = [](float ux, float uy, float vx, float vy) {
        float sign = (ux * vy - uy * vx < 0) ? -1.0f : 1.0f;
        float dot = ux * vx + uy * vy;
        float n = sqrt(ux * ux + uy * uy) * sqrt(vx * vx + vy * vy);
        float val = (n == 0) ? 0 : dot / n;
        if (val > 1.0f) {
            val = 1.0f;
        }
        else if (val < -1.0f) {
            val = -1.0f;
        }
        return sign * acos(val);
        };

    float startAngle = angleBetween(1, 0, (x1 - cx1) / rx, (y1 - cy1) / ry);
    float dAngle = angleBetween((x1 - cx1) / rx, (y1 - cy1) / ry, (-x1 - cx1) / rx, (-y1 - cy1) / ry);

    if (!sweepFlag && dAngle > 0) {
        dAngle -= (float)(2 * M_PI);
    }
    else if (sweepFlag && dAngle < 0) {
        dAngle += (float)(2 * M_PI);
    }

    int segments = (int)ceil(abs(dAngle) / (M_PI / 2.0));
    float delta = dAngle / segments;
    float t = 8.0f / 3.0f * sin(delta / 4.0f) * sin(delta / 4.0f) / sin(delta / 2.0f);

    float startX = x0;
    float startY = y0;

    for (int i = 0; i < segments; ++i) {
        float theta2 = startAngle + delta;
        float cosTheta2 = cos(theta2);
        float sinTheta2 = sin(theta2);

        float p2x = cx + cosAngle * rx * cosTheta2 - sinAngle * ry * sinTheta2;
        float p2y = cy + sinAngle * rx * cosTheta2 + cosAngle * ry * sinTheta2;

        float cosTheta1 = cos(startAngle);
        float sinTheta1 = sin(startAngle);

        float dx1 = -rx * sinTheta1 * cosAngle - ry * cosTheta1 * sinAngle;
        float dy1 = -rx * sinTheta1 * sinAngle + ry * cosTheta1 * cosAngle;

        float dx2 = -rx * sinTheta2 * cosAngle - ry * cosTheta2 * sinAngle;
        float dy2 = -rx * sinTheta2 * sinAngle + ry * cosTheta2 * cosAngle;

        float cp1x = startX + t * dx1;
        float cp1y = startY + t * dy1;
        float cp2x = p2x - t * dx2;
        float cp2y = p2y - t * dy2;

        path.AddBezier(startX, startY, cp1x, cp1y, cp2x, cp2y, p2x, p2y);

        startX = p2x;
        startY = p2y;
        startAngle += delta;
    }
}

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

PathSVG::PathSVG() : FillShapeSVG() {
    fill.setFillColor(Color(255, 0, 0, 0));
    path.SetFillMode(FillModeWinding);
}

string PathSVG::formatPathString(string pathData) {
    string result = "";
    for (int i = 0; i < pathData.size(); i++) {
        char c = pathData[i];
        if (c == ',' || c == '\n' || c == '\r' || c == '\t') {
            result += ' ';
        }
        else if (c == '-') {
            if (i > 0 && (pathData[i - 1] == 'e' || pathData[i - 1] == 'E')) {
                result += c;
            }
            else {
                result += " -";
            }
        }
        else if (isalpha(c) && c != 'e' && c != 'E') {
            result += ' ';
            result += c;
            result += ' ';
        }
        else {
            result += c;
        }
    }
    return result;
}

void PathSVG::parsePathData(string pathData) {
    path.Reset();
    path.SetFillMode(FillModeWinding);
    string formattedPathData = formatPathString(pathData);
    stringstream stringStream(formattedPathData);
    stringStream.imbue(locale("C"));

    char command = 0;
    char lastCommand = 0;
    float x = 0, y = 0, x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    PointF currentPoint(0, 0);
    PointF startFigurePoint(0, 0);
    PointF lastControlPoint(0, 0);

    stringStream >> ws;
    while (!stringStream.eof()) {
        char nextChar = stringStream.peek();
        if (nextChar == EOF) {
            break;
        }
        if (isalpha(nextChar)) {
            stringStream >> command;
        }
        else {
            if (lastCommand == 0) {
                break;
            }
            if (lastCommand == 'M') {
                command = 'L';
            }
            else if (lastCommand == 'm') {
                command = 'l';
            }
            else {
                command = lastCommand;
            }
        }

        if (command == 'M' || command == 'm') {
            stringStream >> x >> y;
            if (command == 'm') {
                x += currentPoint.X;
                y += currentPoint.Y;
            }
            path.StartFigure();
            currentPoint = PointF(x, y);
            startFigurePoint = currentPoint;
        }
        else if (command == 'L' || command == 'l') {
            stringStream >> x >> y;
            if (command == 'l') {
                x += currentPoint.X;
                y += currentPoint.Y;
            }
            path.AddLine(currentPoint, PointF(x, y));
            currentPoint = PointF(x, y);
        }
        else if (command == 'H' || command == 'h') {
            stringStream >> x;
            if (command == 'h') {
                x += currentPoint.X;
            }
            path.AddLine(currentPoint, PointF(x, currentPoint.Y));
            currentPoint.X = x;
        }
        else if (command == 'V' || command == 'v') {
            stringStream >> y;
            if (command == 'v') {
                y += currentPoint.Y;
            }
            path.AddLine(currentPoint, PointF(currentPoint.X, y));
            currentPoint.Y = y;
        }
        else if (command == 'C' || command == 'c') {
            stringStream >> x1 >> y1 >> x2 >> y2 >> x >> y;
            if (command == 'c') {
                x1 += currentPoint.X;
                y1 += currentPoint.Y;
                x2 += currentPoint.X;
                y2 += currentPoint.Y;
                x += currentPoint.X;
                y += currentPoint.Y;
            }
            path.AddBezier(currentPoint, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            currentPoint = PointF(x, y);
            lastControlPoint = PointF(x2, y2);
        }
        else if (command == 'S' || command == 's') {
            stringStream >> x2 >> y2 >> x >> y;
            if (command == 's') {
                x2 += currentPoint.X;
                y2 += currentPoint.Y;
                x += currentPoint.X;
                y += currentPoint.Y;
            }
            if (lastCommand == 'C' || lastCommand == 'c' || lastCommand == 'S' || lastCommand == 's') {
                x1 = 2 * currentPoint.X - lastControlPoint.X;
                y1 = 2 * currentPoint.Y - lastControlPoint.Y;
            }
            else {
                x1 = currentPoint.X;
                y1 = currentPoint.Y;
            }
            path.AddBezier(currentPoint, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            currentPoint = PointF(x, y);
            lastControlPoint = PointF(x2, y2);
        }
        else if (command == 'Q' || command == 'q') {
            stringStream >> x1 >> y1 >> x >> y;
            if (command == 'q') {
                x1 += currentPoint.X;
                y1 += currentPoint.Y;
                x += currentPoint.X;
                y += currentPoint.Y;
            }
            addQuadraticBezier(path, currentPoint.X, currentPoint.Y, x1, y1, x, y);
            currentPoint = PointF(x, y);
            lastControlPoint = PointF(x1, y1);
        }
        else if (command == 'T' || command == 't') {
            stringStream >> x >> y;
            if (command == 't') {
                x += currentPoint.X;
                y += currentPoint.Y;
            }
            if (lastCommand == 'Q' || lastCommand == 'q' || lastCommand == 'T' || lastCommand == 't') {
                x1 = 2 * currentPoint.X - lastControlPoint.X;
                y1 = 2 * currentPoint.Y - lastControlPoint.Y;
            }
            else {
                x1 = currentPoint.X;
                y1 = currentPoint.Y;
            }
            addQuadraticBezier(path, currentPoint.X, currentPoint.Y, x1, y1, x, y);
            currentPoint = PointF(x, y);
            lastControlPoint = PointF(x1, y1);
        }
        else if (command == 'A' || command == 'a') {
            float rx, ry, rotation;
            char largeArcFlagChar, sweepFlagChar;
            stringStream >> rx >> ry >> rotation >> largeArcFlagChar >> sweepFlagChar >> x >> y;
            bool largeArcFlag = (largeArcFlagChar == '1');
            bool sweepFlag = (sweepFlagChar == '1');
            if (command == 'a') {
                x += currentPoint.X;
                y += currentPoint.Y;
            }
            addSvgArc(path, currentPoint.X, currentPoint.Y, rx, ry, rotation, largeArcFlag, sweepFlag, x, y);
            currentPoint = PointF(x, y);
        }
        else if (command == 'Z' || command == 'z') {
            path.CloseFigure();
            currentPoint = startFigurePoint;
        }
        lastCommand = command;
        stringStream >> ws;
    }
}

void PathSVG::read(xml_node<>* node) {
    hasFillColor = false;
    hasFillOpacity = false;
    hasStrokeColor = false;
    hasStrokeWidth = false;
    hasStrokeOpacity = false;
    hasOpacity = false;
    fillGradientId = "";
    className = "";
    fill.setFillColor(Color(255, 0, 0, 0));
    path.SetFillMode(FillModeWinding);

    xml_attribute<>* classAttribute = node->first_attribute("class");
    if (classAttribute) {
        className = classAttribute->value();
    }

    xml_attribute<>* styleAttribute = node->first_attribute("style");
    string inlineStyle = styleAttribute ? styleAttribute->value() : "";

    for (xml_attribute<>* attribute = node->first_attribute(); attribute; attribute = attribute->next_attribute()) {
        string attributeName = attribute->name();
        string attributeValue = attribute->value();

        if (attributeName == "d") {
            parsePathData(attributeValue);
        }
        else if (attributeName == "transform") {
            parseTransform(attributeValue);
        }
        else if (attributeName == "fill-rule") {
            if (attributeValue == "evenodd") {
                path.SetFillMode(FillModeAlternate);
            }
            else {
                path.SetFillMode(FillModeWinding);
            }
        }
        else if (attributeName == "opacity") {
            opacity.setOpacity(stof(attributeValue));
            hasOpacity = true;
        }
        else if (attributeName == "fill") {
            if (attributeValue == "none") {
                fill.setFillColor(Color(0, 0, 0, 0));
                hasFillColor = true;
            }
            else if (attributeValue.find("url(") != string::npos) {
                fillGradientId = getUrlId(attributeValue);
                hasFillColor = true;
            }
            else {
                fill.setFillColor(getRGB(attributeValue));
                hasFillColor = true;
            }
        }
        else if (attributeName == "fill-opacity") {
            fill.setFillOpacity(stof(attributeValue));
            hasFillOpacity = true;
        }
        else if (attributeName == "stroke") {
            stroke.setStrokeColor(getRGB(attributeValue));
            hasStrokeColor = true;
        }
        else if (attributeName == "stroke-width") {
            stroke.setStrokeWidth(stof(attributeValue));
            hasStrokeWidth = true;
        }
        else if (attributeName == "stroke-opacity") {
            stroke.setStrokeOpacity(stof(attributeValue));
            hasStrokeOpacity = true;
        }
    }

    if (!inlineStyle.empty()) {
        ElementSVG::parseStyle(inlineStyle, fill, stroke, opacity, hasFillColor, hasFillOpacity, hasStrokeColor, hasStrokeWidth, hasStrokeOpacity, fillGradientId);
        if (inlineStyle.find("fill-rule:nonzero") != string::npos) {
            path.SetFillMode(FillModeWinding);
        }
        else if (inlineStyle.find("fill-rule:evenodd") != string::npos) {
            path.SetFillMode(FillModeAlternate);
        }
    }
}

void PathSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    GraphicsState graphicsState = graphics.Save();

    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetPixelOffsetMode(PixelOffsetModeHalf);
    graphics.SetCompositingMode(CompositingModeSourceOver);

    if (!className.empty()) {
        string styleString = defs.getStyleByClass(className);
        FillSVG tempFill;
        StrokeSVG tempStroke;
        OpacitySVG tempOpacity;
        bool tempHasFill = false, tempHasFillOpacity = false, tempHasStroke = false, tempHasStrokeWidth = false, tempHasStrokeOpacity = false;
        string tempGradientId = "";
        ElementSVG::parseStyle(styleString, tempFill, tempStroke, tempOpacity, tempHasFill, tempHasFillOpacity, tempHasStroke, tempHasStrokeWidth, tempHasStrokeOpacity, tempGradientId);
        if (!hasFillColor && tempHasFill) {
            fill.setFillColor(tempFill.getFillColor());
            if (!tempGradientId.empty()) {
                fillGradientId = tempGradientId;
            }
            hasFillColor = true;
        }
        if (!hasFillOpacity && tempHasFillOpacity) {
            fill.setFillOpacity(tempFill.getFillOpacity());
        }
        if (!hasStrokeColor && tempHasStroke) {
            stroke.setStrokeColor(tempStroke.getStrokeColor());
            hasStrokeColor = true;
        }
        if (!hasStrokeWidth && tempHasStrokeWidth) {
            stroke.setStrokeWidth(tempStroke.getStrokeWidth());
        }
        if (!hasStrokeOpacity && tempHasStrokeOpacity) {
            stroke.setStrokeOpacity(tempStroke.getStrokeOpacity());
        }
    }

    float opacityAll = opacity.getOpacity() * 255.0f;

    GraphicsPath* localTempPath = path.Clone();
    localTempPath->Flatten(NULL, 0.5f);
    RectF localBounds;
    localTempPath->GetBounds(&localBounds);
    delete localTempPath;

    if (localBounds.Width <= 0) {
        localBounds.Width = 0.1f;
    }
    if (localBounds.Height <= 0) {
        localBounds.Height = 0.1f;
    }

    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);

    const LinearGradientData* linearGradient = defs.getLinearGradient(fillGradientId);
    const RadialGradientData* radialGradient = (!linearGradient) ? defs.getRadialGradient(fillGradientId) : nullptr;

    if (linearGradient || radialGradient) {
        Matrix worldMatrix;
        graphics.GetTransform(&worldMatrix);

        int rampWidth = 256;
        Bitmap rampBitmap(rampWidth, 1, PixelFormat32bppARGB);
        Graphics graphicsRamp(&rampBitmap);
        LinearGradientBrush linearBrush(Point(0, 0), Point(rampWidth, 0), Color::Black, Color::White);
        vector<Color> colors;
        vector<float> positions;
        if (linearGradient) {
            normalizeStopsLinear(linearGradient->stops, colors, positions, opacityAll / 255.0f * fill.getFillOpacity());
        }
        else {
            normalizeStopsLinear(radialGradient->stops, colors, positions, opacityAll / 255.0f * fill.getFillOpacity());
        }

        if (!colors.empty()) {
            linearBrush.SetInterpolationColors(colors.data(), positions.data(), (int)colors.size());
        }
        graphicsRamp.FillRectangle(&linearBrush, 0, 0, rampWidth, 1);

        vector<Color> colorTable(rampWidth);
        BitmapData rampData;
        Rect rampRect(0, 0, rampWidth, 1);
        rampBitmap.LockBits(&rampRect, ImageLockModeRead, PixelFormat32bppARGB, &rampData);
        UINT* pixels = (UINT*)rampData.Scan0;
        for (int i = 0; i < rampWidth; i++) {
            colorTable[i].SetValue(pixels[i]);
        }
        rampBitmap.UnlockBits(&rampData);

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
                float startX = localBounds.X + point1.X * localBounds.Width;
                float startY = localBounds.Y + point1.Y * localBounds.Height;
                totalMatrix.Translate(startX, startY, MatrixOrderAppend);
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
            if (radialRSq < 1e-6f) {
                radialRSq = 1e-6f;
            }
            radialDistF = sqrt(pow(radialFx - radialCx, 2) + pow(radialFy - radialCy, 2));
            if (radialDistF >= radialR * 0.99f) {
                float scale = (radialR * 0.99f) / radialDistF;
                radialFx = radialCx + (radialFx - radialCx) * scale;
                radialFy = radialCy + (radialFy - radialCy) * scale;
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

                if (linearGradient) {
                    t = point.X;
                }
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
                            if (delta >= 0) {
                                t = 1.0f / ((-termB + sqrt(delta)) / (2 * termA));
                            }
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
                    if (index < 0) {
                        index = 0;
                    }
                    if (index >= rampWidth) {
                        index = rampWidth - 1;
                    }
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
        // Solid Fill
        Color rgbFill = fill.getFillColor();
        if (rgbFill.GetA() != 0) {
            Color finalFill((BYTE)(opacityAll * fill.getFillOpacity()), rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
            SolidBrush brush(finalFill);
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
            Color finalStroke((BYTE)opStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
            Pen strokePen(finalStroke, currentStrokeWidth);
            graphics.DrawPath(&strokePen, &path);
        }
    }

    graphics.Restore(graphicsState);
}

PathSVG::~PathSVG() {}