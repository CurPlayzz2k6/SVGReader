#ifndef GROUPSVG_H
#define GROUPSVG_H

#include "rapidxml.hpp"
#include "Function.h"
#include "ElementSVG.h"
#include "DefinitionsSVG.h" 
#include <vector>
#include <gdiplus.h>

class DefinitionsSVG;

#include "RectangleSVG.h"
#include "CircleSVG.h"
#include "EllipseSVG.h"
#include "LineSVG.h"
#include "PolygonSVG.h"
#include "PolylineSVG.h"
#include "TextSVG.h"
#include "PathSVG.h" 
#include "FillShapeSVG.h" 

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class GroupSVG : public ElementSVG {
private:
    vector<ElementSVG*> children;
    FillSVG fill;
    StrokeSVG stroke;

    bool hasFillColor;
    bool hasFillOpacity;
    bool hasStrokeColor;
    bool hasStrokeWidth;
    bool hasStrokeOpacity;
    bool hasOpacity;

public:
    GroupSVG();
    void inheritStyle(FillSVG& pFill, StrokeSVG& pStroke, OpacitySVG& pOpacity);
    void read(xml_node<>* node) override;
    void draw(Graphics& graphics, const DefinitionsSVG& defs) override;
    void clear();
    ~GroupSVG();
};

#endif GROUPSVG_H