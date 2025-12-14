#ifndef RENDERSVG_H
#define RENDERSVG_H

#include "rapidxml.hpp"
#include "Function.h"
#include "ParserSVG.h"
#include <vector>
#include <gdiplus.h>

// Include DefinitionsSVG để có thể khai báo biến thành viên
#include "DefinitionsSVG.h"

#include "RectangleSVG.h"
#include "CircleSVG.h"
#include "LineSVG.h"
#include "PolygonSVG.h"
#include "PolylineSVG.h"
#include "EllipseSVG.h"
#include "TextSVG.h"
#include "GroupSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class RenderSVG {
private:
    vector<ElementSVG*> groups;

    // CẬP NHẬT: RenderSVG sở hữu DefinitionsSVG
    DefinitionsSVG defs;

public:
    RenderSVG();
    void readAll(ParserSVG& fileXML);
    void drawAll(Graphics* graphics);
    void clearAll();
    ~RenderSVG();
};

#endif RENDERSVG_H