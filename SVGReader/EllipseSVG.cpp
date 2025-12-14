#include "EllipseSVG.h"
#include "DefinitionsSVG.h"
#include <iostream>

using namespace Gdiplus;
using namespace std;

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

    for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
        string type(attr->name());
        string value = attr->value();

        if (type == "cx") cx = stof(value);
        else if (type == "cy") cy = stof(value);
        else if (type == "rx") rx = stof(value);
        else if (type == "ry") ry = stof(value);
        else if (type == "opacity") { 
            opacity.setOpacity(stof(value)); hasOpacity = true; 
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
void EllipseSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    GraphicsState state = graphics.Save();
    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    float opacityAll = opacity.getOpacity() * 255.0f;

    // SỬA ĐỔI: Dùng defs.getGradient
    const LinearGradientData* gradData = nullptr;
    if (!fillGradientId.empty()) {
        gradData = defs.getGradient(fillGradientId);
    }

    Brush* fillBrush = nullptr;
    LinearGradientBrush* gradBrush = nullptr;
    SolidBrush* solidBrush = nullptr;

    if (gradData) {
        RectF bounds(cx - rx, cy - ry, 2 * rx, 2 * ry);
        if (bounds.Width <= 0) bounds.Width = 0.1f;
        if (bounds.Height <= 0) bounds.Height = 0.1f;

        PointF start(bounds.X + gradData->startPoint.X * bounds.Width, bounds.Y + gradData->startPoint.Y * bounds.Height);
        PointF end(bounds.X + gradData->endPoint.X * bounds.Width, bounds.Y + gradData->endPoint.Y * bounds.Height);

        if (abs(start.X - end.X) < 0.001f && abs(start.Y - end.Y) < 0.001f) end.X += 0.1f;

        gradBrush = new LinearGradientBrush(start, end, Color::Black, Color::White);
        int count = gradData->stops.size();

        if (count > 0) {
            Color* colors = new Color[count];
            float* positions = new float[count];

            for (int i = 0; i < count; ++i) {
                Color c = gradData->stops[i].color;
                float stopAlpha = c.GetA();
                float fillOp = fill.getFillOpacity();
                float finalA = stopAlpha * (opacityAll / 255.0f) * fillOp;
                if (finalA > 255.0f) finalA = 255.0f;

                colors[i] = Color((float)finalA, c.GetR(), c.GetG(), c.GetB());
                positions[i] = gradData->stops[i].offset;
            }
            gradBrush->SetInterpolationColors(colors, positions, count);
            delete[] colors;
            delete[] positions;
        }
        fillBrush = gradBrush;
    }
    else {
        Color rgbFill = fill.getFillColor();
        if (rgbFill.GetA() > 0) {
            float calculatedFillAlpha = opacityAll * fill.getFillOpacity();
            float opFill = minValue(calculatedFillAlpha, (float)rgbFill.GetA());
            if (opFill > 0) {
                Color finalFill((float)opFill, rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
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
        if (opStroke > 0 && currentStrokeWidth <= 0.0f) currentStrokeWidth = 1.0f;

        if (opStroke > 0 && currentStrokeWidth > 0) {
            Color finalStroke((float)opStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
            strokePen = new Pen(finalStroke, currentStrokeWidth);
        }
    }

    if (fillBrush) graphics.FillEllipse(fillBrush, cx - rx, cy - ry, 2 * rx, 2 * ry);
    if (strokePen) graphics.DrawEllipse(strokePen, cx - rx, cy - ry, 2 * rx, 2 * ry);

    if (gradBrush) delete gradBrush;
    if (solidBrush) delete solidBrush;
    if (strokePen) delete strokePen;

    graphics.Restore(state);
}

EllipseSVG::~EllipseSVG() {}