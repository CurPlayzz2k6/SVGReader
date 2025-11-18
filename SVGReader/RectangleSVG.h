#ifndef RECTANGLESVG_H
#define RECTANGLESVG_H
#include "Function.h"
#include "FillShapeSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class RectangleSVG : public FillShapeSVG {
private:
	float width;
	float height;
	float x, y;
	float rx, ry;

public:
	RectangleSVG();
	void read(xml_node<>* node);
	void draw(Graphics& graphics) override;
	~RectangleSVG();
};
#endif RECTANGLESVG_H