#include "PolylineSVG.h"
#include "DefinitionsSVG.h"
#include <iostream>
#include <vector>

using namespace Gdiplus;
using namespace std;

PolylineSVG::PolylineSVG() : FillShapeSVG() {}

void PolylineSVG::read(xml_node<>* node) {
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

        if (type == "points") 
            points = value;
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
}

// CẬP NHẬT: Thêm tham số defs
void PolylineSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    GraphicsState state = graphics.Save();
    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    float opacityAll = opacity.getOpacity() * 255.0f;
    vector<PointF> polyPoints = getPolyPoints(points);

    if (polyPoints.size() < 2) {
        graphics.Restore(state);
        return;
    }

    // SỬA ĐỔI: Dùng defs.getGradient
    const LinearGradientData* gradientData = nullptr;
    if (!fillGradientId.empty())
        gradientData = defs.getGradient(fillGradientId);

    Brush* fillBrush = nullptr;
    LinearGradientBrush* gradientBrush = nullptr;
    SolidBrush* solidBrush = nullptr;

    if (gradientData) {
        float minX = polyPoints[0].X, maxX = polyPoints[0].X;
        float minY = polyPoints[0].Y, maxY = polyPoints[0].Y;
        for (const auto& p : polyPoints) {
            if (p.X < minX) 
                minX = p.X; 
            if (p.X > maxX) 
                maxX = p.X;
            if (p.Y < minY) 
                minY = p.Y; 
            if (p.Y > maxY) 
                maxY = p.Y;
        }
        RectF bounds(minX, minY, maxX - minX, maxY - minY);
        if (bounds.Width <= 0) bounds.Width = 0.1f;
        if (bounds.Height <= 0) bounds.Height = 0.1f;

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
                float finalA = c.GetA() * (opacityAll / 255.0f) * fill.getFillOpacity();
                if (finalA > 255.0f) finalA = 255.0f;
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
            float opFill = minValue(opacityAll * fill.getFillOpacity(), (float)rgbFill.GetA());
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
        float opStroke = minValue(opacityAll * stroke.getStrokeOpacity(), (float)rgbStroke.GetA());
        float currentStrokeWidth = stroke.getStrokeWidth();
        if (opStroke > 0 && currentStrokeWidth <= 0.0f)
            currentStrokeWidth = 1.0f;

        if (opStroke > 0 && currentStrokeWidth > 0) {
            Color finalStroke(opStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
            strokePen = new Pen(finalStroke, currentStrokeWidth);
        }
    }

    if (fillBrush) 
        graphics.FillPolygon(fillBrush, polyPoints.data(), (int)polyPoints.size());
    if (strokePen) 
        graphics.DrawLines(strokePen, polyPoints.data(), (int)polyPoints.size());

    if (gradientBrush) 
        delete gradientBrush;
    if (solidBrush) 
        delete solidBrush;
    if (strokePen) 
        delete strokePen;

    graphics.Restore(state);
}

PolylineSVG::~PolylineSVG() {}