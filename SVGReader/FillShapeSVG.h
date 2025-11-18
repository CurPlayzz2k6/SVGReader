#ifndef FILLSHAPESVG_H
#define FILLSHAPESVG_H
#include "Function.h"
#include "FillSVG.h"
#include "ShapeSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class FillShapeSVG : public ShapeSVG {
protected:
	FillSVG fill;

public:
	FillShapeSVG();
	virtual void read(xml_node<>* node) = 0;
	virtual void draw(Graphics& graphics) = 0;
	~FillShapeSVG();
};
#endif FILLSHAPESVG_H