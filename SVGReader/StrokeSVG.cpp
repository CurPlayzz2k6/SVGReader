#include "Function.h"
#include "StrokeSVG.h"

using namespace Gdiplus;
using namespace std;

void StrokeSVG::setStrokeColor(Color strokeColor) {
	this->strokeColor = strokeColor;
}

void StrokeSVG::setStrokeWidth(float strokeWidth) {
	this->strokeWidth = strokeWidth;
}

void StrokeSVG::setStrokeOpacity(float strokeOpacity) {
	this->strokeOpacity = strokeOpacity;
}

Color StrokeSVG::getStrokeColor() {
	return this->strokeColor;
}

float StrokeSVG::getStrokeWidth() {
	return this->strokeWidth;
}

float StrokeSVG::getStrokeOpacity() {
	return this->strokeOpacity;
}

StrokeSVG::~StrokeSVG() {}