#include "LineSVG.h"
#include "DefinitionsSVG.h"
#include <iostream>

using namespace Gdiplus;
using namespace std;

LineSVG::LineSVG() {
    this->x1 = 0;
    this->y1 = 0;
    this->x2 = 0;
    this->y2 = 0;
}

void LineSVG::read(xml_node<>* node) {
    hasStrokeColor = false;
    hasStrokeWidth = false;
    hasStrokeOpacity = false;
    hasOpacity = false;
    className = "";

    xml_attribute<>* classAttribute = node->first_attribute("class");
    if (classAttribute) {
        className = classAttribute->value();
    }

    xml_attribute<>* styleAttribute = node->first_attribute("style");
    string inlineStyle = styleAttribute ? styleAttribute->value() : "";

    for (xml_attribute<>* attribute = node->first_attribute(); attribute; attribute = attribute->next_attribute()) {
        string attributeName(attribute->name());
        if (attributeName == "x1") {
            x1 = stof(attribute->value());
        }
        else if (attributeName == "y1") {
            y1 = stof(attribute->value());
        }
        else if (attributeName == "x2") {
            x2 = stof(attribute->value());
        }
        else if (attributeName == "y2") {
            y2 = stof(attribute->value());
        }
        else if (attributeName == "opacity") {
            opacity.setOpacity(stof(attribute->value()));
            hasOpacity = true;
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

    if (!inlineStyle.empty()) {
        FillSVG unusedFill;
        bool tempFill = false, tempFillOpacity = false;
        string tempGradientId;
        ElementSVG::parseStyle(inlineStyle, unusedFill, stroke, opacity, tempFill, tempFillOpacity, hasStrokeColor, hasStrokeWidth, hasStrokeOpacity, tempGradientId);
    }
}

void LineSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    GraphicsState graphicsState = graphics.Save();
    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    if (!className.empty()) {
        string styleString = defs.getStyleByClass(className);
        FillSVG unusedFill;
        StrokeSVG tempStroke = stroke;
        OpacitySVG tempOpacity = opacity;
        bool tempFill = false, tempFillOpacity = false, tempHasStroke = false, tempHasStrokeWidth = false, tempHasStrokeOpacity = false;
        string tempGradientId = "";
        ElementSVG::parseStyle(styleString, unusedFill, tempStroke, tempOpacity, tempFill, tempFillOpacity, tempHasStroke, tempHasStrokeWidth, tempHasStrokeOpacity, tempGradientId);

        if (!hasStrokeColor && tempHasStroke) {
            stroke.setStrokeColor(tempStroke.getStrokeColor());
        }
        if (!hasStrokeWidth && tempHasStrokeWidth) {
            stroke.setStrokeWidth(tempStroke.getStrokeWidth());
        }
        if (!hasStrokeOpacity && tempHasStrokeOpacity) {
            stroke.setStrokeOpacity(tempStroke.getStrokeOpacity());
        }
    }

    float opacityAll = opacity.getOpacity() * 255.0f;
    Color rgbStroke = stroke.getStrokeColor();
    if (rgbStroke.GetA() > 0 && stroke.getStrokeWidth() > 0) {
        float finalAlpha = opacityAll * stroke.getStrokeOpacity();
        Color finalStroke((BYTE)finalAlpha, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
        Pen penStroke(finalStroke, stroke.getStrokeWidth());
        graphics.DrawLine(&penStroke, x1, y1, x2, y2);
    }

    graphics.Restore(graphicsState);
}

LineSVG::~LineSVG() {}