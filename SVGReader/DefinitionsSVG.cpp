#include "DefinitionsSVG.h"
#include "Function.h" // Đảm bảo có hàm getRGB
#include <iostream>

DefinitionsSVG::DefinitionsSVG() {}

DefinitionsSVG::~DefinitionsSVG() {
    clear();
}

void DefinitionsSVG::clear() {
    maplinearGradients.clear();
}

GradientStopData DefinitionsSVG::readGradientStop(xml_node<>* stopNode) {
    GradientStopData data;
    Color tempColor = Color(255, 255, 255, 255);
    float stopOpacity = 1.0f;

    for (xml_attribute<>* attr = stopNode->first_attribute(); attr; attr = attr->next_attribute()) {
        string name = attr->name();
        string value = attr->value();

        try {
            if (name == "offset") {
                if (!value.empty() && value.back() == '%')
                    data.offset = stof(value.substr(0, value.size() - 1)) / 100.0f;
                else
                    data.offset = stof(value);
            }
            else if (name == "stop-color")
                tempColor = getRGB(value);
            else if (name == "stop-opacity")
                stopOpacity = stof(value);
        }
        catch (...) {}
    }

    float finalAlpha = (float)(tempColor.GetA() * stopOpacity);
    data.color = Color(finalAlpha, tempColor.GetR(), tempColor.GetG(), tempColor.GetB());
    return data;
}

LinearGradientData DefinitionsSVG::readLinearGradient(xml_node<>* gradientNode) {
    LinearGradientData data;

    for (xml_attribute<>* attr = gradientNode->first_attribute(); attr; attr = attr->next_attribute()) {
        string name = attr->name();
        string value = attr->value();

        try {
            if (name == "id") data.id = value;
            else if (name == "x1") 
                data.startPoint.X = (value.back() == '%') ? stof(value) / 100.0f : stof(value);
            else if (name == "y1") 
                data.startPoint.Y = (value.back() == '%') ? stof(value) / 100.0f : stof(value);
            else if (name == "x2") 
                data.endPoint.X = (value.back() == '%') ? stof(value) / 100.0f : stof(value);
            else if (name == "y2") 
                data.endPoint.Y = (value.back() == '%') ? stof(value) / 100.0f : stof(value);
        }
        catch (...) {}
    }

    for (xml_node<>* stop_node = gradientNode->first_node("stop"); stop_node; stop_node = stop_node->next_sibling("stop"))
        data.stops.push_back(readGradientStop(stop_node));

    sort(data.stops.begin(), data.stops.end(),
        [](const GradientStopData& a, const GradientStopData& b) {
            return a.offset < b.offset;
        });

    return data;
}

void DefinitionsSVG::readDefs(xml_node<>* defsNode) {
    if (!defsNode) return;
    for (xml_node<>* node = defsNode->first_node(); node; node = node->next_sibling()) {
        string name = node->name();
        if (name == "linearGradient") {
            LinearGradientData gradient = readLinearGradient(node);
            if (!gradient.id.empty())
                maplinearGradients[gradient.id] = gradient;
        }
    }
}

const LinearGradientData* DefinitionsSVG::getGradient(const string& id) const {
    auto itr = maplinearGradients.find(id);
    if (itr != maplinearGradients.end())
        return &(itr->second);
    return nullptr;
}