#include "ElementSVG.h"
#include <iostream>
#include <sstream>

ElementSVG::ElementSVG() {
    opacity.setOpacity(1.0f);
    transformMatrix.Reset();
    className = "";
}

void ElementSVG::parseTransform(string transformStr) {
    string s = formatTransformString(transformStr);
    stringstream transformStream(s);
    string temp;

    transformMatrix.Reset();

    while (transformStream >> temp) {
        if (temp == "translate") {
            float tx;
            float ty = 0;
            transformStream >> tx;
            if (!(transformStream >> ty)) {
                ty = 0;
                transformStream.clear();
            }
            transformMatrix.Translate(tx, ty, MatrixOrderPrepend);
        }
        else if (temp == "rotate") {
            float angle, cx, cy;
            transformStream >> angle;
            if (transformStream >> cx >> cy) {
                transformMatrix.RotateAt(angle, PointF(cx, cy), MatrixOrderPrepend);
            }
            else {
                transformStream.clear();
                transformMatrix.Rotate(angle, MatrixOrderPrepend);
            }
        }
        else if (temp == "scale") {
            float sx, sy;
            transformStream >> sx;
            if (!(transformStream >> sy)) {
                sy = sx;
                transformStream.clear();
            }
            transformMatrix.Scale(sx, sy, MatrixOrderPrepend);
        }
        else if (temp == "matrix") {
            float m11, m12, m21, m22, dx, dy;
            if (transformStream >> m11 >> m12 >> m21 >> m22 >> dx >> dy) {
                Matrix m(m11, m12, m21, m22, dx, dy);
                transformMatrix.Multiply(&m, MatrixOrderPrepend);
            }
        }
    }
}

void ElementSVG::parseStyle(string styleStr, FillSVG& fill, StrokeSVG& stroke, OpacitySVG& opacity,
    bool& hasFill, bool& hasFillOp, bool& hasStroke, bool& hasStrokeW, bool& hasStrokeOp, string& gradId) {
    if (styleStr.empty()) {
        return;
    }
    stringstream ss(styleStr);
    string segment;
    while (getline(ss, segment, ';')) {
        size_t colon = segment.find(':');
        if (colon == string::npos) {
            continue;
        }
        string key = segment.substr(0, colon);
        string val = segment.substr(colon + 1);

        key.erase(0, key.find_first_not_of(" \t\n\r"));
        key.erase(key.find_last_not_of(" \t\n\r") + 1);
        val.erase(0, val.find_first_not_of(" \t\n\r"));
        val.erase(val.find_last_not_of(" \t\n\r") + 1);

        if (key == "fill") {
            if (val == "none") {
                fill.setFillColor(Color(0, 0, 0, 0));
                hasFill = true;
            }
            else if (val.find("url(") != string::npos) {
                gradId = getUrlId(val);
                hasFill = true;
            }
            else {
                fill.setFillColor(getRGB(val));
                hasFill = true;
            }
        }
        else if (key == "fill-opacity") {
            fill.setFillOpacity(stof(val));
            hasFillOp = true;
        }
        else if (key == "stroke") {
            if (val == "none") {
                stroke.setStrokeColor(Color(0, 0, 0, 0));
                hasStroke = true;
            }
            else {
                stroke.setStrokeColor(getRGB(val));
                hasStroke = true;
            }
        }
        else if (key == "stroke-width") {
            stroke.setStrokeWidth(stof(val));
            hasStrokeW = true;
        }
        else if (key == "stroke-opacity") {
            stroke.setStrokeOpacity(stof(val));
            hasStrokeOp = true;
        }
        else if (key == "opacity") {
            opacity.setOpacity(stof(val));
        }
    }
}