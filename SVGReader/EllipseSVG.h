#ifndef ELLIPSESVG_H
#define ELLIPSESVG_H
#include "Function.h"
#include "FillShapeSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class EllipseSVG : public FillShapeSVG {
protected:
	float rx, ry;
	float cx, cy;

public:
	EllipseSVG();
	void read(xml_node<>* node);
	void draw(Graphics& graphics, const DefinitionsSVG& defs) override;
	~EllipseSVG();
};
#endif ELLIPSESVG_H