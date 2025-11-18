#include "Function.h"
#include "FillShapeSVG.h"

using namespace Gdiplus;
using namespace std;

FillShapeSVG::FillShapeSVG() {
	fill.setFillColor(Color(0, 0, 0, 0));
	fill.setFillOpacity(1.0f);
	opacity.setOpacity(1000.0f); // Giá trị flag để đánh dấu có opacity hay không?
	stroke.setStrokeColor(Color(0, 0, 0, 0));
	stroke.setStrokeWidth(0);
	stroke.setStrokeOpacity(1.0f);
}

FillShapeSVG::~FillShapeSVG() {}