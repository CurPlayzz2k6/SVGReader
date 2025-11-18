#ifndef STROKESVG_H
#define STROKESVG_H
#include "Function.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class StrokeSVG {
protected:
	Color strokeColor;
	float strokeWidth;
	float strokeOpacity;

public:
	void setStrokeColor(Color strokeColor);
	void setStrokeWidth(float strokeWidth);
	void setStrokeOpacity(float strokeOpacity);
	Color getStrokeColor();
	float getStrokeWidth();
	float getStrokeOpacity();
	~StrokeSVG();
};
#endif STROKESVG_H