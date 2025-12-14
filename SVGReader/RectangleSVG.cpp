#include "RectangleSVG.h"
#include "DefinitionsSVG.h"
#include <iostream>
#include <algorithm>

using namespace Gdiplus;
using namespace std;

RectangleSVG::RectangleSVG() : FillShapeSVG() {
    this->height = 0;
    this->width = 0;
    this->x = 0;
    this->y = 0;
    this->rx = 0;
    this->ry = 0;
}

void RectangleSVG::read(xml_node<>* node) {
    hasFillColor = false; 
    hasFillOpacity = false;
    hasStrokeColor = false; 
    hasStrokeWidth = false; 
    hasStrokeOpacity = false;
    hasOpacity = false;
    fillGradientId = "";

    for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
        string type(attr->name());
        string value = attr->value();

        if (type == "x") 
            x = stof(value);
        else if (type == "y") 
            y = stof(value);
        else if (type == "width") 
            width = stof(value);
        else if (type == "height") 
            height = stof(value);
        else if (type == "rx") 
            rx = stof(value);
        else if (type == "ry") 
            ry = stof(value);
        else if (type == "opacity") { 
            opacity.setOpacity(stof(value)); 
            hasOpacity = true; 
        }
        else if (type == "fill") {
            if (value.find("url(") != string::npos) {
                fillGradientId = getUrlId(value);
                hasFillColor = true;
            }
            else {
                fill.setFillColor(getRGB(value));
                hasFillColor = true;
            }
        }
        else if (type == "fill-opacity") { 
            fill.setFillOpacity(stof(value)); 
            hasFillOpacity = true; 
        }
        else if (type == "stroke") { 
            stroke.setStrokeColor(getRGB(value)); 
            hasStrokeColor = true; 
        }
        else if (type == "stroke-width") { 
            stroke.setStrokeWidth(stof(value)); 
            hasStrokeWidth = true; 
        }
        else if (type == "stroke-opacity") { 
            stroke.setStrokeOpacity(stof(value)); 
            hasStrokeOpacity = true; 
        }
        else if (type == "transform") 
            parseTransform(value);
    }

    if ((width > 0 && height <= 0) || (width <= 0 && height > 0)) {
        width = height = max(width, height);
    }
}

void RectangleSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    GraphicsState state = graphics.Save();
    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    float opacityAll = opacity.getOpacity() * 255.0f;

    const LinearGradientData* gradientData = nullptr;
    if (!fillGradientId.empty())
        gradientData = defs.getGradient(fillGradientId);

    Brush* fillBrush = nullptr;
    LinearGradientBrush* gradientBrush = nullptr;
    SolidBrush* solidBrush = nullptr;

    if (gradientData) {
        RectF bounds(x, y, width, height);
        if (bounds.Width <= 0) 
            bounds.Width = 0.1f;
        if (bounds.Height <= 0) 
            bounds.Height = 0.1f;

        PointF start(bounds.X + gradientData->startPoint.X * bounds.Width, bounds.Y + gradientData->startPoint.Y * bounds.Height);
        PointF end(bounds.X + gradientData->endPoint.X * bounds.Width, bounds.Y + gradientData->endPoint.Y * bounds.Height);

        if (abs(start.X - end.X) < 0.001f && abs(start.Y - end.Y) < 0.001f)
            end.X += 0.1f;

        gradientBrush = new LinearGradientBrush(start, end, Color::Black, Color::White);
        int count = gradientData->stops.size();

        if (count > 0) {
            Color* colors = new Color[count];
            float* positions = new float[count];

            for (int i = 0; i < count; ++i) {
                Color c = gradientData->stops[i].color;
                float stopAlpha = c.GetA();
                float fillOp = fill.getFillOpacity();
                float finalA = stopAlpha * (opacityAll / 255.0f) * fillOp;
                if (finalA > 255.0f) 
                    finalA = 255.0f;

                colors[i] = Color(finalA, c.GetR(), c.GetG(), c.GetB());
                positions[i] = gradientData->stops[i].offset;
            }
            gradientBrush->SetInterpolationColors(colors, positions, count);
            delete[] colors;
            delete[] positions;
        }
        fillBrush = gradientBrush;
    }
    else {
        Color rgbFill = fill.getFillColor();
        if (rgbFill.GetA() > 0) {
            float calculatedFillAlpha = opacityAll * fill.getFillOpacity();
            float opFill = minValue(calculatedFillAlpha, (float)rgbFill.GetA());

            if (opFill > 0) {
                Color finalFill(opFill, rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
                solidBrush = new SolidBrush(finalFill);
                fillBrush = solidBrush;
            }
        }
    }

    Color rgbStroke = stroke.getStrokeColor();
    Pen* strokePen = nullptr;

    if (rgbStroke.GetA() > 0) {
        float calculatedStrokeAlpha = opacityAll * stroke.getStrokeOpacity();
        float opStroke = minValue(calculatedStrokeAlpha, (float)rgbStroke.GetA());
        float currentStrokeWidth = stroke.getStrokeWidth();
        if (opStroke > 0 && currentStrokeWidth <= 0.0f) 
            currentStrokeWidth = 1.0f;

        if (opStroke > 0 && currentStrokeWidth > 0) {
            Color finalStroke(opStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
            strokePen = new Pen(finalStroke, currentStrokeWidth);
        }
    }

    if (rx == 0 && ry == 0) {
        if (fillBrush) 
            graphics.FillRectangle(fillBrush, x, y, width, height);
        if (strokePen) 
            graphics.DrawRectangle(strokePen, x, y, width, height);
    }
    else {
        if (rx > width / 2.0f) 
            rx = width / 2.0f;
        if (ry > height / 2.0f) 
            ry = height / 2.0f;
        if (rx == 0) 
            rx = ry;
        if (ry == 0) 
            ry = rx;

        GraphicsPath path;
        float dX = 2 * rx;
        float dY = 2 * ry;
        path.StartFigure();
        path.AddArc(x, y, dX, dY, 180, 90);
        path.AddArc(x + width - dX, y, dX, dY, 270, 90);
        path.AddArc(x + width - dX, y + height - dY, dX, dY, 0, 90);
        path.AddArc(x, y + height - dY, dX, dY, 90, 90);
        path.CloseFigure();

        if (fillBrush) 
            graphics.FillPath(fillBrush, &path);
        if (strokePen) 
            graphics.DrawPath(strokePen, &path);
    }

    if (gradientBrush) 
        delete gradientBrush;
    if (solidBrush) 
        delete solidBrush;
    if (strokePen) 
        delete strokePen;

    graphics.Restore(state);
}

RectangleSVG::~RectangleSVG() {}