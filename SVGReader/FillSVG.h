#ifndef FILLSVG_H
#define FILLSVG_H
#include "Function.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class FillSVG {
protected:
	Color fillColor;
	float fillOpacity;

public:
	void setFillColor(Color fillColor);
	void setFillOpacity(float fillOpacity);
	Color getFillColor();
	float getFillOpacity();
	~FillSVG();
};
#endif FILLSVG_H