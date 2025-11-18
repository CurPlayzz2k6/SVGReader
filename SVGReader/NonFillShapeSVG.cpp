#include "Function.h"
#include "NonFillShapeSVG.h"

using namespace Gdiplus;
using namespace std;

NonFillShapeSVG::NonFillShapeSVG() {
	opacity.setOpacity(1.0f);
	stroke.setStrokeColor(Color(0, 0, 0, 0));
	stroke.setStrokeWidth(0);
	stroke.setStrokeOpacity(1.0f);
}

NonFillShapeSVG::~NonFillShapeSVG() {}