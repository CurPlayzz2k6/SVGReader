#ifndef DEFINITIONSSVG_H
#define DEFINITIONSSVG_H

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Function.h" // Đảm bảo file này có chứa hàm getRGB hoặc các tiện ích chung
#include <gdiplus.h>
#include "rapidxml.hpp"
#include <vector>
#include <string>
#include <map>
#include <algorithm>

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

enum GradientUnits {
    UnitsObjectBoundingBox,
    UnitsUserSpaceOnUse
};

enum GradientSpreadMethod {
    SpreadMethodPad,
    SpreadMethodReflect,
    SpreadMethodRepeat
};

struct GradientStopData {
    float offset;
    Color color;
};

struct LinearGradientData {
    string id;
    PointF startPoint;
    PointF endPoint;
    vector<GradientStopData> stops;
    GradientUnits units;
    GradientSpreadMethod spreadMethod;
    Matrix transform;
    string href;

    bool hasStart, hasEnd, hasUnits, hasSpread, hasTransform, hasStops;

    LinearGradientData() : startPoint(0.0f, 0.0f), endPoint(1.0f, 0.0f),
        units(UnitsObjectBoundingBox), spreadMethod(SpreadMethodPad),
        hasStart(false), hasEnd(false), hasUnits(false), hasSpread(false), hasTransform(false), hasStops(false) {
    }

    LinearGradientData(const LinearGradientData& other) { 
        *this = other; 
    }

    LinearGradientData& operator=(const LinearGradientData& other) {
        if (this != &other) {
            id = other.id;
            startPoint = other.startPoint;
            endPoint = other.endPoint;
            stops = other.stops;
            units = other.units;
            spreadMethod = other.spreadMethod;
            href = other.href;

            hasStart = other.hasStart;
            hasEnd = other.hasEnd;
            hasUnits = other.hasUnits;
            hasSpread = other.hasSpread;
            hasTransform = other.hasTransform;
            hasStops = other.hasStops;

            REAL m[6];
            other.transform.GetElements(m);
            transform.SetElements(m[0], m[1], m[2], m[3], m[4], m[5]);
        }
        return *this;
    }
};

struct RadialGradientData {
    string id;
    float cx, cy, r, fx, fy;
    vector<GradientStopData> stops;
    GradientUnits units;
    GradientSpreadMethod spreadMethod;
    Matrix transform;
    string href;

    bool hasCx, hasCy, hasR, hasFx, hasFy, hasUnits, hasSpread, hasTransform, hasStops;

    RadialGradientData() : cx(0.5f), cy(0.5f), r(0.5f), fx(0.5f), fy(0.5f),
        units(UnitsObjectBoundingBox), spreadMethod(SpreadMethodPad),
        hasCx(false), hasCy(false), hasR(false), hasFx(false), hasFy(false),
        hasUnits(false), hasSpread(false), hasTransform(false), hasStops(false) {
    }

    RadialGradientData(const RadialGradientData& other) { 
        *this = other; 
    }

    RadialGradientData& operator=(const RadialGradientData& other) {
        if (this != &other) {
            id = other.id;
            cx = other.cx; cy = other.cy; r = other.r;
            fx = other.fx; fy = other.fy;
            stops = other.stops;
            units = other.units;
            spreadMethod = other.spreadMethod;
            href = other.href;

            hasCx = other.hasCx; hasCy = other.hasCy; hasR = other.hasR;
            hasFx = other.hasFx; hasFy = other.hasFy;
            hasUnits = other.hasUnits; hasSpread = other.hasSpread;
            hasTransform = other.hasTransform; hasStops = other.hasStops;

            REAL m[6];
            other.transform.GetElements(m);
            transform.SetElements(m[0], m[1], m[2], m[3], m[4], m[5]);
        }
        return *this;
    }
};

class DefinitionsSVG {
private:
    map<string, LinearGradientData> mapLinearGradients;
    map<string, RadialGradientData> mapRadialGradients;
    map<string, string> mapStyles;

public:
    DefinitionsSVG();
    void readDefs(xml_node<>* defsNode);
    GradientStopData readGradientStop(xml_node<>* stopNode);
    LinearGradientData readLinearGradient(xml_node<>* gradientNode);
    RadialGradientData readRadialGradient(xml_node<>* gradientNode);
    const LinearGradientData* getLinearGradient(const string& id) const;
    const RadialGradientData* getRadialGradient(const string& id) const;
    void parseStyleBlock(const string& content);
    void resolveInheritance();
    string getStyleByClass(const string& className) const;
    void clear();
    ~DefinitionsSVG();
};

#endif DEFINITIONSSVG_H