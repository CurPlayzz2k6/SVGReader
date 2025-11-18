#include "Function.h"
#include "OpacitySVG.h"

using namespace Gdiplus;
using namespace std;

void OpacitySVG::setOpacity(float opacity) {
	this->opacity = opacity;
}

float OpacitySVG::getOpacity() {
	return this->opacity;
}

OpacitySVG::~OpacitySVG(){}
