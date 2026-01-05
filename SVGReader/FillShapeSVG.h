#ifndef FILLSHAPESVG_H
#define FILLSHAPESVG_H
#include "ElementSVG.h"
#include "FillSVG.h"
#include "StrokeSVG.h"
#include <string>

using namespace std;

class FillShapeSVG : public ElementSVG {
protected:
	FillSVG fill;
	string fillGradientId;
	bool hasFillColor;
	bool hasFillOpacity;
	bool hasStrokeColor;
	bool hasStrokeWidth;
	bool hasStrokeOpacity;
	bool hasOpacity;

public:
	FillShapeSVG();
	virtual void inheritStyle(FillSVG& pFill, StrokeSVG& pStroke, OpacitySVG& pOpacity);
	virtual void read(xml_node<>* node) = 0;
	virtual void draw(Graphics& graphics, const DefinitionsSVG& defs) = 0;
	void setGradientId(string id);
	string getGradientId();
	~FillShapeSVG();
};

#endif FILLSHAPESVG_H