#ifndef ELEMENTSVG_H
#define ELEMENTSVG_H

#include "Function.h"
#include "StrokeSVG.h"
#include "FillSVG.h"
#include "OpacitySVG.h"
#include "DefinitionsSVG.h"
#include <vector>
#include <string>
#include <sstream>

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class ElementSVG {
protected:
    StrokeSVG stroke;
    OpacitySVG opacity;
    FillSVG fill;
    string fillGradientId;
    string className;
    Matrix transformMatrix;

    static void parseStyle(string styleStr, FillSVG& fill, StrokeSVG& stroke, OpacitySVG& opacity, bool& hasFill, bool& hasFillOp, bool& hasStroke, bool& hasStrokeW, bool& hasStrokeOp, string& gradId);

public:
    ElementSVG();
    void parseTransform(string transformStr);
    virtual void read(xml_node<>* node) = 0;
    virtual void draw(Graphics& graphics, const DefinitionsSVG& defs) = 0;
    virtual ~ElementSVG() {};
};
#endif ELEMENTSVG_H