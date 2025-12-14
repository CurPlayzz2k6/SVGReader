#include "Function.h"
#include "NonFillShapeSVG.h"

using namespace Gdiplus;
using namespace std;

NonFillShapeSVG::NonFillShapeSVG() {
	stroke.setStrokeColor(Color(0, 0, 0, 0));
	stroke.setStrokeWidth(0);
	stroke.setStrokeOpacity(1.0f);
	opacity.setOpacity(1.0f);

	hasStrokeColor = false;
	hasStrokeWidth = false;
	hasStrokeOpacity = false;
	hasOpacity = false;
}

void NonFillShapeSVG::inheritStyle(StrokeSVG& pStroke, OpacitySVG& pOpacity) {
	if (!hasStrokeColor) 
		this->stroke.setStrokeColor(pStroke.getStrokeColor());
	if (!hasStrokeWidth) 
		this->stroke.setStrokeWidth(pStroke.getStrokeWidth());
	if (!hasStrokeOpacity) 
		this->stroke.setStrokeOpacity(pStroke.getStrokeOpacity());

	float parentOp = pOpacity.getOpacity();
	if (!hasOpacity)
		this->opacity.setOpacity(parentOp);
	else {
		float currentOp = this->opacity.getOpacity();
		this->opacity.setOpacity(currentOp * parentOp);
	}
}

NonFillShapeSVG::~NonFillShapeSVG() {}