#ifndef SHAPESVG_H
#define SHAPESVG_H
#include "Function.h"
#include "StrokeSVG.h"
#include "FillSVG.h"
#include "OpacitySVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class ShapeSVG {
protected:
	StrokeSVG stroke;
	OpacitySVG opacity;

public:
	virtual void read(xml_node<>* node) = 0;
	virtual void draw(Graphics& graphics) = 0;
};
#endif SHAPESVG_H