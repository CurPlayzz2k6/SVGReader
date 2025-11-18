#ifndef RENDERSVG_H
#define RENDERSVG_H
#include "rapidxml.hpp"
#include "Function.h"
#include "RectangleSVG.h"
#include "CircleSVG.h"
#include "LineSVG.h"
#include "PolygonSVG.h"
#include "PolylineSVG.h"
#include "EllipseSVG.h"
#include "TextSVG.h"
#include "ParserSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class RenderSVG {
private:
	vector<ShapeSVG*> shapes;

public:
	RenderSVG();
	void readRecursiveElement(xml_node<>* node);
	void readAll(ParserSVG& fileXML);
	void drawAll(Graphics* graphics);
	void clearAll();
	~RenderSVG();
};

#endif RENDERSVG_H