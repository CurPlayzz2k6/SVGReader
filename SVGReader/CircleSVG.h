#ifndef CIRCLESVG_H
#define CIRCLESVG_H
#include "Function.h"
#include "EllipseSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class CircleSVG : public EllipseSVG {
private:
	float r;

public:
	CircleSVG();
	void read(xml_node<>* node);
	~CircleSVG();
};

#endif CIRCLESVG_H