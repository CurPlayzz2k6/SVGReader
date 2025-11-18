#include "Function.h"
#include "FillSVG.h"

using namespace Gdiplus;
using namespace std;

void FillSVG::setFillColor(Color fillColor) {
	this->fillColor = fillColor;
}

void FillSVG::setFillOpacity(float fillOpacity) {
	this->fillOpacity = fillOpacity;
}

Color FillSVG::getFillColor() {
	return this->fillColor;
}

float FillSVG::getFillOpacity() {
	return this->fillOpacity;
}

FillSVG::~FillSVG() {}