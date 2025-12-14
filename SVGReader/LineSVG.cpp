#pragma once
#include "LineSVG.h"
#include "DefinitionsSVG.h" // Thêm include để nhận kiểu dữ liệu

using namespace Gdiplus;
using namespace std;

LineSVG::LineSVG() {
    this->x1 = 0;
    this->y1 = 0;
    this->x2 = 0;
    this->y2 = 0;
}

void LineSVG::read(xml_node<>* node) {
    for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
        string type(attr->name());
        if (type == "x1") 
            x1 = stof(attr->value());
        else if (type == "y1") 
            y1 = stof(attr->value());
        else if (type == "x2") 
            x2 = stof(attr->value());
        else if (type == "y2") 
            y2 = stof(attr->value());
        else if (type == "opacity") { 
            opacity.setOpacity(stof(attr->value())); 
            hasOpacity = true; 
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

void LineSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    GraphicsState state = graphics.Save();
    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    float opacityAll = ((float)opacity.getOpacity() * 255);
    Color rgbStroke = stroke.getStrokeColor();
    float opacityStroke = minValue(opacityAll, ((float)stroke.getStrokeOpacity() * 255));
    if (stroke.getStrokeColor().GetA() == 0) opacityStroke = 0;

    rgbStroke = Color(opacityStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
    Pen penStroke(rgbStroke, stroke.getStrokeWidth());

    if (stroke.getStrokeColor().GetA() > 0 || stroke.getStrokeColor().GetR() > 0 || stroke.getStrokeColor().GetG() > 0 || stroke.getStrokeColor().GetB() > 0)
        graphics.DrawLine(&penStroke, this->x1, this->y1, this->x2, this->y2);

    graphics.Restore(state);
}

LineSVG::~LineSVG() {}