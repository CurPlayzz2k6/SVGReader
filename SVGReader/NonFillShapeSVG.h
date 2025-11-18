#ifndef NONFILLSHAPESVG_H
#define NONFILLSHAPESVG_H
#include "Function.h"
#include "ShapeSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class NonFillShapeSVG : public ShapeSVG {
public:
	NonFillShapeSVG();
	virtual void read(xml_node<>* node) = 0;
	virtual void draw(Graphics& graphics) = 0;
	~NonFillShapeSVG();
};
#endif NONFILLSHAPESVG_H