#ifndef ELEMENTSVG_H
#define ELEMENTSVG_H

#include "Function.h"
#include "StrokeSVG.h"
#include "FillSVG.h"
#include "OpacitySVG.h"
#include <vector>
#include <string>
#include <sstream>

// Forward declaration để tránh lỗi include vòng tròn
class DefinitionsSVG;

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class ElementSVG {
protected:
    StrokeSVG stroke;
    OpacitySVG opacity;
    Matrix transformMatrix;

public:
    ElementSVG();

    void parseTransform(string transformStr);
    virtual void read(xml_node<>* node) = 0;

    // CẬP NHẬT: Nhận thêm const DefinitionsSVG& defs
    virtual void draw(Graphics& graphics, const DefinitionsSVG& defs) = 0;

    virtual ~ElementSVG() {};
};
#endif ELEMENTSVG_H