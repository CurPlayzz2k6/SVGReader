#ifndef TEXTSVG_H
#define TEXTSVG_H

#include "Function.h"
#include "FillShapeSVG.h"
#include <gdiplus.h>

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class TextSVG : public FillShapeSVG {
private:
    float x, y;
    float rotate;
    string text;
    string fontFamily;
    string fontStyle;
    float fontSize;
    string textAnchor;

public:
    TextSVG();
    void read(xml_node<>* node);
    void draw(Graphics& graphics, const DefinitionsSVG& defs) override;
    ~TextSVG();
};
#endif TEXTSVG_H