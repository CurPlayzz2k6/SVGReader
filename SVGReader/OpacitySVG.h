#ifndef OPACITYSVG_H
#define OPACITYSVG_H
#include "Function.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class OpacitySVG {
protected:
	float opacity;

public:
	void setOpacity(float opacity);
	float getOpacity();
	~OpacitySVG();
};
#endif OPACITYSVG_H