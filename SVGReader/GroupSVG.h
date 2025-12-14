#ifndef GROUPSVG_H
#define GROUPSVG_H

#include "ElementSVG.h"
#include "rapidxml.hpp"
#include <vector>
#include <gdiplus.h>

// Forward declaration để tránh lỗi biên dịch nếu header chưa include đủ
class DefinitionsSVG;

// Include đầy đủ các hình con để Factory (trong hàm read) hoạt động
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

    // Các cờ đánh dấu xem Group này có set thuộc tính style không
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