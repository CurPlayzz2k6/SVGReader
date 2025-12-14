#ifndef DEFINITIONSSVG_H
#define DEFINITIONSSVG_H

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Function.h"
#include <gdiplus.h>
#include "rapidxml.hpp"
#include <vector>
#include <string>
#include <map>
#include <algorithm>

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

struct GradientStopData {
    float offset;
    Color color;
};

struct LinearGradientData {
    string id;
    PointF startPoint;
    PointF endPoint;
    vector<GradientStopData> stops;

    LinearGradientData(): startPoint(0.0f, 0.0f), endPoint(1.0f, 0.0f) {}
};

class DefinitionsSVG {
private:
    map<string, LinearGradientData> maplinearGradients;
    GradientStopData readGradientStop(xml_node<>* stopNode);
    LinearGradientData readLinearGradient(xml_node<>* gradientNode);

public:
    DefinitionsSVG();
    void readDefs(xml_node<>* defsNode);
    const LinearGradientData* getGradient(const string& id) const;
    void clear();
    ~DefinitionsSVG();
};

#endif DEFINITIONSSVG_H