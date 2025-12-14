#ifndef NONFILLSHAPESVG_H
#define NONFILLSHAPESVG_H
#include "Function.h"
#include "ElementSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class NonFillShapeSVG : public ElementSVG {
protected:
	bool hasStrokeColor;
	bool hasStrokeWidth;
	bool hasStrokeOpacity;
	bool hasOpacity;

public:
	NonFillShapeSVG();
	void inheritStyle(StrokeSVG& pStroke, OpacitySVG& pOpacity);
	virtual void read(xml_node<>* node) = 0;
	virtual void draw(Graphics& graphics, const DefinitionsSVG& defs) = 0;
	~NonFillShapeSVG();
};
#endif NONFILLSHAPESVG_H