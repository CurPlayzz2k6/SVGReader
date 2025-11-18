#ifndef LINESVG_H
#define LINESVG_H
#include "Function.h"
#include "NonFillShapeSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class LineSVG : public NonFillShapeSVG {
private:
	float x1, y1;
	float x2, y2;

public:
	LineSVG();
	void read(xml_node<>* node);
	void draw(Graphics& graphics) override;
	~LineSVG();
};
#endif LINESVG_H