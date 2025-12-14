#ifndef PATHSVG_H
#define PATHSVG_H

#include "FillShapeSVG.h"
#include "DefinitionsSVG.h"
#include <vector>
#include <sstream>

class PathSVG : public FillShapeSVG {
private:
	GraphicsPath path;

public:
	PathSVG();
	static string formatPathString(string d);
	void read(xml_node<>* node) override;
	void draw(Graphics& graphics, const DefinitionsSVG& defs) override;
	void parsePathData(string d);
	~PathSVG();
};

#endif PATHSVG_H