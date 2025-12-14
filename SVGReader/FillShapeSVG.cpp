#include "Function.h"
#include "FillShapeSVG.h"

using namespace Gdiplus;
using namespace std;

FillShapeSVG::FillShapeSVG() {
	fill.setFillColor(Color(255, 0, 0, 0)); // Đen
	fill.setFillOpacity(1.0f);

	stroke.setStrokeColor(Color(0, 0, 0, 0)); // Trong suốt
	stroke.setStrokeWidth(0);
	stroke.setStrokeOpacity(1.0f);

	opacity.setOpacity(1.0f);

	hasFillColor = false;
	hasFillOpacity = false;
	hasStrokeColor = false;
	hasStrokeWidth = false;
	hasStrokeOpacity = false;
	hasOpacity = false;
	fillGradientId = "";
}

void FillShapeSVG::setGradientId(string id) { 
	this->fillGradientId = id; 
}

string FillShapeSVG::getGradientId() { 
	return this->fillGradientId; 
}

void FillShapeSVG::inheritStyle(FillSVG& pFill, StrokeSVG& pStroke, OpacitySVG& pOpacity) {
	if (!hasFillColor)
		this->fill.setFillColor(pFill.getFillColor());
	if (!hasFillOpacity) 
		this->fill.setFillOpacity(pFill.getFillOpacity());
	if (!hasStrokeColor) 
		this->stroke.setStrokeColor(pStroke.getStrokeColor());
	if (!hasStrokeWidth) 
		this->stroke.setStrokeWidth(pStroke.getStrokeWidth());
	if (!hasStrokeOpacity) 
		this->stroke.setStrokeOpacity(pStroke.getStrokeOpacity());

	// Xử lý Opacity (Nhân dồn)
	float parentOp = pOpacity.getOpacity();
	if (!hasOpacity) {
		this->opacity.setOpacity(parentOp);
	}
	else {
		float currentOp = this->opacity.getOpacity();
		this->opacity.setOpacity(currentOp * parentOp);
	}
}

FillShapeSVG::~FillShapeSVG() {}