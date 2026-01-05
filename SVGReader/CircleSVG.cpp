#include "CircleSVG.h"

using namespace Gdiplus;
using namespace std;

CircleSVG::CircleSVG() : EllipseSVG() {
    r = 0;
    rx = ry = 0;
}

void CircleSVG::read(xml_node<>* node) {
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
        else if (attributeName == "r") {
            rx = ry = stof(attributeValue);
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
        else if (attributeName == "transform") {
            parseTransform(attributeValue);
        }
    }

    if (!inlineStyle.empty()) {
        ElementSVG::parseStyle(inlineStyle, fill, stroke, opacity, hasFillColor, hasFillOpacity, hasStrokeColor, hasStrokeWidth, hasStrokeOpacity, fillGradientId);
    }
}

CircleSVG::~CircleSVG() {}