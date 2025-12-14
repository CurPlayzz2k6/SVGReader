#ifndef POLYGONSVG_H
#define POLYGONSVG_H
#include "Function.h"
#include "FillShapeSVG.h"

using namespace Gdiplus;
using namespace std;
using namespace rapidxml;

class PolygonSVG : public FillShapeSVG {
private:
	string points;

public:
	PolygonSVG();
	void read(xml_node<>* node);
	void draw(Graphics& graphics, const DefinitionsSVG& defs) override;
	~PolygonSVG();
};
#endif POLYGONSVG_H