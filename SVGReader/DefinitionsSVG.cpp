#include "DefinitionsSVG.h"
#include <stack>
#include <iostream>
#include <cmath>
#include <regex>
#include <sstream>
#include <algorithm>
#include <vector>
#include <locale>
#include <functional>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;
using namespace Gdiplus;

DefinitionsSVG::DefinitionsSVG() {}
DefinitionsSVG::~DefinitionsSVG() {
    clear();
}

void DefinitionsSVG::clear() {
    mapLinearGradients.clear();
    mapRadialGradients.clear();
    mapStyles.clear();
}

void parseGradientTransform(Matrix& matrix, string transformString) {
    string cleanString = cleanTransformString(transformString);
    stringstream stringStream(cleanString);
    stringStream.imbue(std::locale("C"));

    string tempString;
    matrix.Reset();

    while (stringStream >> tempString) {
        if (tempString == "matrix") {
            float matrix11 = parseFloat(stringStream);
            float matrix12 = parseFloat(stringStream);
            float matrix21 = parseFloat(stringStream);
            float matrix22 = parseFloat(stringStream);
            float dx = parseFloat(stringStream);
            float dy = parseFloat(stringStream);
            Matrix tempMatrix(matrix11, matrix12, matrix21, matrix22, dx, dy);
            matrix.Multiply(&tempMatrix, MatrixOrderAppend);
        }
        else if (tempString == "scale") {
            float scaleX = parseFloat(stringStream);
            float scaleY = scaleX;
            streampos oldPosition = stringStream.tellg();
            if (!(stringStream >> scaleY)) {
                stringStream.clear();
                stringStream.seekg(oldPosition);
            }
            matrix.Scale(scaleX, scaleY, MatrixOrderAppend);
        }
        else if (tempString == "rotate") {
            float angle = parseFloat(stringStream);
            float centerX = 0, centerY = 0;
            streampos oldPosition = stringStream.tellg();
            if (stringStream >> centerX >> centerY) {
                matrix.RotateAt(angle, PointF(centerX, centerY), MatrixOrderAppend);
            }
            else {
                stringStream.clear();
                stringStream.seekg(oldPosition);
                matrix.Rotate(angle, MatrixOrderAppend);
            }
        }
        else if (tempString == "translate") {
            float translateX = parseFloat(stringStream);
            float translateY = 0;
            streampos oldPosition = stringStream.tellg();
            if (!(stringStream >> translateY)) {
                stringStream.clear();
                stringStream.seekg(oldPosition);
            }
            matrix.Translate(translateX, translateY, MatrixOrderAppend);
        }
        else if (tempString == "skewX") {
            float angle = parseFloat(stringStream);
            float tanValue = (float)tan(angle * M_PI / 180.0f);
            matrix.Shear(tanValue, 0, MatrixOrderAppend);
        }
        else if (tempString == "skewY") {
            float angle = parseFloat(stringStream);
            float tanValue = (float)tan(angle * M_PI / 180.0f);
            matrix.Shear(0, tanValue, MatrixOrderAppend);
        }
    }
}

GradientStopData DefinitionsSVG::readGradientStop(xml_node<>* stopNode) {
    GradientStopData data;
    Color temporaryColor = Color::Black;
    float stopOpacity = 1.0f;

    for (xml_attribute<>* attribute = stopNode->first_attribute(); attribute; attribute = attribute->next_attribute()) {
        string attributeName = attribute->name();
        string attributeValue = attribute->value();
        if (attributeName == "offset") {
            if (!attributeValue.empty() && attributeValue.back() == '%') {
                data.offset = parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f;
            }
            else {
                data.offset = parseString(attributeValue);
            }
        }
        else if (attributeName == "stop-color") {
            temporaryColor = getRGB(attributeValue);
        }
        else if (attributeName == "stop-opacity") {
            stopOpacity = parseString(attributeValue);
        }
    }

    xml_attribute<>* styleAttribute = stopNode->first_attribute("style");
    if (styleAttribute) {
        string styleString = styleAttribute->value();
        if (styleString.find("stop-color") != string::npos) {
            int position = styleString.find("stop-color");
            int colonIndex = styleString.find(":", position);
            int semiColonIndex = styleString.find(";", colonIndex);
            if (semiColonIndex == string::npos) {
                semiColonIndex = styleString.length();
            }
            if (colonIndex != string::npos) {
                temporaryColor = getRGB(styleString.substr(colonIndex + 1, semiColonIndex - colonIndex - 1));
            }
        }
        if (styleString.find("stop-opacity") != string::npos) {
            int position = styleString.find("stop-opacity");
            int colonIndex = styleString.find(":", position);
            int semiColonIndex = styleString.find(";", colonIndex);
            if (semiColonIndex == string::npos) {
                semiColonIndex = styleString.length();
            }
            if (colonIndex != string::npos) {
                stopOpacity = parseString(styleString.substr(colonIndex + 1, semiColonIndex - colonIndex - 1));
            }
        }
    }

    if (data.offset < 0.0f) {
        data.offset = 0.0f;
    }
    if (data.offset > 1.0f) {
        data.offset = 1.0f;
    }

    data.color = Color((BYTE)(temporaryColor.GetA() * stopOpacity), temporaryColor.GetR(), temporaryColor.GetG(), temporaryColor.GetB());
    return data;
}

LinearGradientData DefinitionsSVG::readLinearGradient(xml_node<>* gradientNode) {
    LinearGradientData data;
    for (xml_attribute<>* attribute = gradientNode->first_attribute(); attribute; attribute = attribute->next_attribute()) {
        string attributeName = attribute->name();
        string attributeValue = attribute->value();

        if (attributeName == "id") {
            data.id = attributeValue;
        }
        else if (attributeName == "x1") {
            data.startPoint.X = (attributeValue.back() == '%') ? parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f : parseString(attributeValue);
        }
        else if (attributeName == "y1") {
            data.startPoint.Y = (attributeValue.back() == '%') ? parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f : parseString(attributeValue);
        }
        else if (attributeName == "x2") {
            data.endPoint.X = (attributeValue.back() == '%') ? parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f : parseString(attributeValue);
        }
        else if (attributeName == "y2") {
            data.endPoint.Y = (attributeValue.back() == '%') ? parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f : parseString(attributeValue);
        }
        else if (attributeName == "gradientUnits" && attributeValue == "userSpaceOnUse") {
            data.units = UnitsUserSpaceOnUse;
        }
        else if (attributeName == "gradientTransform") {
            parseGradientTransform(data.transform, attributeValue);
        }
        else if (attributeName == "href" || attributeName == "xlink:href") {
            if (!attributeValue.empty() && attributeValue[0] == '#') {
                data.href = attributeValue.substr(1);
            }
        }
        else if (attributeName == "spreadMethod") {
            if (attributeValue == "reflect") {
                data.spreadMethod = SpreadMethodReflect;
            }
            else if (attributeValue == "repeat") {
                data.spreadMethod = SpreadMethodRepeat;
            }
            else {
                data.spreadMethod = SpreadMethodPad;
            }
        }
    }
    for (xml_node<>* stop = gradientNode->first_node("stop"); stop; stop = stop->next_sibling("stop")) {
        data.stops.push_back(readGradientStop(stop));
    }

    sort(data.stops.begin(), data.stops.end(), [](const GradientStopData& a, const GradientStopData& b) {
        return a.offset < b.offset;
        });

    return data;
}

RadialGradientData DefinitionsSVG::readRadialGradient(xml_node<>* gradientNode) {
    RadialGradientData data;
    bool hasFx = false, hasFy = false;
    for (xml_attribute<>* attribute = gradientNode->first_attribute(); attribute; attribute = attribute->next_attribute()) {
        string attributeName = attribute->name();
        string attributeValue = attribute->value();

        if (attributeName == "id") {
            data.id = attributeValue;
        }
        else if (attributeName == "cx") {
            data.cx = (attributeValue.back() == '%') ? parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f : parseString(attributeValue);
        }
        else if (attributeName == "cy") {
            data.cy = (attributeValue.back() == '%') ? parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f : parseString(attributeValue);
        }
        else if (attributeName == "r") {
            data.r = (attributeValue.back() == '%') ? parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f : parseString(attributeValue);
        }
        else if (attributeName == "fx") {
            data.fx = (attributeValue.back() == '%') ? parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f : parseString(attributeValue);
            hasFx = true;
        }
        else if (attributeName == "fy") {
            data.fy = (attributeValue.back() == '%') ? parseString(attributeValue.substr(0, attributeValue.size() - 1)) / 100.0f : parseString(attributeValue);
            hasFy = true;
        }
        else if (attributeName == "gradientUnits" && attributeValue == "userSpaceOnUse") {
            data.units = UnitsUserSpaceOnUse;
        }
        else if (attributeName == "gradientTransform") {
            parseGradientTransform(data.transform, attributeValue);
        }
        else if (attributeName == "href" || attributeName == "xlink:href") {
            if (!attributeValue.empty() && attributeValue[0] == '#') {
                data.href = attributeValue.substr(1);
            }
        }
        else if (attributeName == "spreadMethod") {
            if (attributeValue == "reflect") {
                data.spreadMethod = SpreadMethodReflect;
            }
            else if (attributeValue == "repeat") {
                data.spreadMethod = SpreadMethodRepeat;
            }
            else {
                data.spreadMethod = SpreadMethodPad;
            }
        }
    }
    if (!hasFx) {
        data.fx = data.cx;
    }
    if (!hasFy) {
        data.fy = data.cy;
    }
    for (xml_node<>* stop = gradientNode->first_node("stop"); stop; stop = stop->next_sibling("stop")) {
        data.stops.push_back(readGradientStop(stop));
    }

    sort(data.stops.begin(), data.stops.end(), [](const GradientStopData& a, const GradientStopData& b) {
        return a.offset < b.offset;
        });

    return data;
}

void DefinitionsSVG::parseStyleBlock(const string& content) {
    string processedString = content;
    replace(processedString.begin(), processedString.end(), '\n', ' ');
    replace(processedString.begin(), processedString.end(), '\r', ' ');
    int position = 0;
    while (position < processedString.length()) {
        int braceOpenIndex = processedString.find('{', position);
        if (braceOpenIndex == string::npos) {
            break;
        }
        string selector = processedString.substr(position, braceOpenIndex - position);
        int braceCloseIndex = processedString.find('}', braceOpenIndex);
        if (braceCloseIndex == string::npos) {
            break;
        }
        string properties = processedString.substr(braceOpenIndex + 1, braceCloseIndex - braceOpenIndex - 1);
        selector.erase(0, selector.find_first_not_of(" \t"));
        int lastIndex = selector.find_last_not_of(" \t");
        if (lastIndex != string::npos) {
            selector.erase(lastIndex + 1);
        }
        if (!selector.empty() && selector[0] == '.') {
            string className = selector.substr(1);
            mapStyles[className] = properties;
        }
        position = braceCloseIndex + 1;
    }
}

void DefinitionsSVG::resolveInheritance() {
    for (auto& pair : mapLinearGradients) {
        if (pair.second.stops.empty() && !pair.second.href.empty()) {
            string hrefID = pair.second.href;
            auto linearIterator = mapLinearGradients.find(hrefID);
            if (linearIterator != mapLinearGradients.end()) {
                pair.second.stops = linearIterator->second.stops;
            }
            else {
                auto radialIterator = mapRadialGradients.find(hrefID);
                if (radialIterator != mapRadialGradients.end()) {
                    pair.second.stops = radialIterator->second.stops;
                }
            }
        }
    }

    for (auto& pair : mapRadialGradients) {
        if (pair.second.stops.empty() && !pair.second.href.empty()) {
            string hrefID = pair.second.href;
            auto linearIterator = mapLinearGradients.find(hrefID);
            if (linearIterator != mapLinearGradients.end()) {
                pair.second.stops = linearIterator->second.stops;
            }
            else {
                auto radialIterator = mapRadialGradients.find(hrefID);
                if (radialIterator != mapRadialGradients.end()) {
                    pair.second.stops = radialIterator->second.stops;
                }
            }
        }
    }
}

void DefinitionsSVG::readDefs(xml_node<>* rootNode) {
    if (!rootNode) {
        return;
    }
    stack<xml_node<>*> nodeStack;
    nodeStack.push(rootNode);

    while (!nodeStack.empty()) {
        xml_node<>* node = nodeStack.top();
        nodeStack.pop();

        string name = node->name();

        if (name == "linearGradient") {
            LinearGradientData gradient = readLinearGradient(node);
            if (!gradient.id.empty()) {
                mapLinearGradients[gradient.id] = gradient;
            }
            continue;
        }
        else if (name == "radialGradient") {
            RadialGradientData gradient = readRadialGradient(node);
            if (!gradient.id.empty()) {
                mapRadialGradients[gradient.id] = gradient;
            }
            continue;
        }
        else if (name == "style") {
            if (node->value()) {
                parseStyleBlock(node->value());
            }
            continue;
        }
        vector<xml_node<>*> children;
        for (xml_node<>* child = node->first_node(); child; child = child->next_sibling()) {
            children.push_back(child);
        }
        for (auto iterator = children.rbegin(); iterator != children.rend(); ++iterator) {
            nodeStack.push(*iterator);
        }
    }

    resolveInheritance();
}

const LinearGradientData* DefinitionsSVG::getLinearGradient(const string& id) const {
    auto iterator = mapLinearGradients.find(id);
    if (iterator != mapLinearGradients.end()) {
        return &(iterator->second);
    }
    return nullptr;
}

const RadialGradientData* DefinitionsSVG::getRadialGradient(const string& id) const {
    auto iterator = mapRadialGradients.find(id);
    if (iterator != mapRadialGradients.end()) {
        return &(iterator->second);
    }
    return nullptr;
}

string DefinitionsSVG::getStyleByClass(const string& className) const {
    auto iterator = mapStyles.find(className);
    if (iterator != mapStyles.end()) {
        return iterator->second;
    }
    return "";
}