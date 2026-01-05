#ifndef PATHSVG_H
#define PATHSVG_H

#include "FillShapeSVG.h"
#include <vector>
#include <string>

class PathSVG : public FillShapeSVG {
private:
    GraphicsPath path;
    static string formatPathString(string d);
    void parsePathData(string d);

public:
    PathSVG();
    void read(xml_node<>* node) override;
    void draw(Graphics& graphics, const DefinitionsSVG& defs) override;
    ~PathSVG();
};

#endif PATHSVG_H