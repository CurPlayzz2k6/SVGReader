#include "PolygonSVG.h"

using namespace Gdiplus;
using namespace std;

PolygonSVG::PolygonSVG() {}

void PolygonSVG::read(xml_node<>* node) {
	for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
		string type(attr->name());
		if (type == "points")
			points = attr->value();
		else if (type == "opacity")
			opacity.setOpacity(stof(attr->value()));
		else if (type == "fill")
			fill.setFillColor(getRGB(attr->value()));
		else if (type == "fill-opacity")
			fill.setFillOpacity(stof(attr->value()));
		else if (type == "stroke")
			stroke.setStrokeColor(getRGB(attr->value()));
		else if (type == "stroke-width")
			stroke.setStrokeWidth(stof(attr->value()));
		else if (type == "stroke-opacity")
			stroke.setStrokeOpacity(stof(attr->value()));
	}
}

void PolygonSVG::draw(Graphics& graphics) {
	float opacityAll = ((float)opacity.getOpacity() * 255);
	Color rgbFill = fill.getFillColor();
	float opacityFill = minValue(opacityAll, ((float)fill.getFillOpacity() * 255));
	Color rgbStroke = stroke.getStrokeColor();
	float opacityStroke = minValue(opacityAll, ((float)stroke.getStrokeOpacity() * 255));
	rgbFill = Color(opacityFill, rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
	rgbStroke = Color(opacityStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
	Pen penStroke(rgbStroke, stroke.getStrokeWidth());
	SolidBrush penFill(rgbFill);
	vector<PointF> polyPoints = getPolyPoints(points);
	if (fill.getFillColor().GetA() > 0 || fill.getFillColor().GetR() > 0 || fill.getFillColor().GetG() > 0 || fill.getFillColor().GetB() > 0) graphics.FillPolygon(&penFill, polyPoints.data(), (int)polyPoints.size());
	if (stroke.getStrokeColor().GetA() > 0 || stroke.getStrokeColor().GetR() > 0 || stroke.getStrokeColor().GetG() > 0 || stroke.getStrokeColor().GetB() > 0) graphics.DrawPolygon(&penStroke, polyPoints.data(), (int)polyPoints.size());
}

PolygonSVG::~PolygonSVG() {}