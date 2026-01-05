#include "EllipseSVG.h"
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

EllipseSVG::EllipseSVG() {
    this->cx = 0;
    this->cy = 0;
    this->rx = 0;
    this->ry = 0;
}

void EllipseSVG::read(xml_node<>* node) {
    hasFillColor = false;
    hasFillOpacity = false;
    hasStrokeColor = false;
    hasStrokeWidth = false;
    hasStrokeOpacity = false;
    hasOpacity = false;
    fillGradientId = "";
    className = "";
    xml_attribute<>* classAttribute = node->first_attribute("class");
    if (classAttribute) {
        className = classAttribute->value();
    }

    xml_attribute<>* styleAttribute = node->first_attribute("style");
    string inlineStyle = styleAttribute ? styleAttribute->value() : "";

    for (xml_attribute<>* attribute = node->first_attribute(); attribute; attribute = attribute->next_attribute()) {
        string attributeName(attribute->name());
        string attributeValue = attribute->value();

        if (attributeName == "cx") {
            cx = stof(attributeValue);
        }
        else if (attributeName == "cy") {
            cy = stof(attributeValue);
        }
        else if (attributeName == "rx") {
            rx = stof(attributeValue);
        }
        else if (attributeName == "ry") {
            ry = stof(attributeValue);
        }
        else if (attributeName == "opacity") {
            opacity.setOpacity(stof(attributeValue));
            hasOpacity = true;
        }
        else if (attributeName == "fill") {
            if (attributeValue.find("url(") != string::npos) {
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
        else if (attributeName == "transform") {
            parseTransform(attributeValue);
        }
    }

    if (!inlineStyle.empty()) {
        ElementSVG::parseStyle(inlineStyle, fill, stroke, opacity,
            hasFillColor, hasFillOpacity, hasStrokeColor, hasStrokeWidth, hasStrokeOpacity, fillGradientId);
    }
}

void EllipseSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    GraphicsState graphicsState = graphics.Save();
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetPixelOffsetMode(PixelOffsetModeHalf);
    graphics.SetCompositingMode(CompositingModeSourceOver);

    if (!className.empty()) {
        string styleString = defs.getStyleByClass(className);
        FillSVG tempFill = fill;
        StrokeSVG tempStroke = stroke;
        OpacitySVG tempOpacity = opacity;
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
    RectF localBounds(cx - rx, cy - ry, 2 * rx, 2 * ry);
    if (localBounds.Width <= 0) {
        localBounds.Width = 0.1f;
    }
    if (localBounds.Height <= 0) {
        localBounds.Height = 0.1f;
    }
    GraphicsPath path;
    path.AddEllipse(localBounds);
    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);

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

                size_t index = 0;
                for (size_t k = 0; k < positions.size() - 1; ++k) {
                    if (pos >= positions[k]) {
                        index = k;
                    }
                }

                if (pos <= positions.front()) {
                    colorTable[i] = colors.front();
                    continue;
                }
                if (pos >= positions.back()) {
                    colorTable[i] = colors.back();
                    continue;
                }

                float t = (pos - positions[index]) / (positions[index + 1] - positions[index]);
                Color c1 = colors[index];
                Color c2 = colors[index + 1];

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

EllipseSVG::~EllipseSVG() {}