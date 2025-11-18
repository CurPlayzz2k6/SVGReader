#ifndef POLYLINESVG_H
#define POLYLINESVG_H
#include "Function.h"
#include "FillShapeSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class PolylineSVG : public FillShapeSVG {
private:
	string points;

public:
	PolylineSVG();
	void read(xml_node<>* node);
	void draw(Graphics& graphics) override;
	~PolylineSVG();
};
#endif POLYLINESVG_H