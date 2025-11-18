#ifndef PARSERSVGFILE_H
#define PARSERSVGFILE_H
#include "rapidxml.hpp"
#include "Function.h"
#include "RectangleSVG.h"
#include "CircleSVG.h"
#include "LineSVG.h"
#include "PolygonSVG.h"
#include "PolylineSVG.h"
#include "EllipseSVG.h"
#include "TextSVG.h"

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class ParserSVG {
private:
	xml_document<> svgDocument;

public:
	ParserSVG();
	void readSVG(vector<char>& xmlBuffer);
	xml_node<>* getSVGDocumentFirstNode();
	~ParserSVG();
};
#endif PARSERSVGFILE_H