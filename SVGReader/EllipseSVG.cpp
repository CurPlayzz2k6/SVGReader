#include "EllipseSVG.h"

using namespace Gdiplus;
using namespace std;

EllipseSVG::EllipseSVG() {
	cx = 0;
	cy = 0;
	rx = 0;
	ry = 0;
}

void EllipseSVG::read(xml_node<>* node) {
	for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
		string type(attr->name());
		if (type == "cx")
			cx = stof(attr->value());
		else if (type == "cy")
			cy = stof(attr->value());
		else if (type == "rx")
			rx = stof(attr->value());
		else if (type == "ry")
			ry = stof(attr->value());
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

void EllipseSVG::draw(Graphics& graphics) {
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
	float opacityAll = ((float)opacity.getOpacity() * 255);
	Color rgbFill = fill.getFillColor();
	float opacityFill = minValue(opacityAll, ((float)fill.getFillOpacity() * 255));
	Color rgbStroke = stroke.getStrokeColor();
	float opacityStroke = minValue(opacityAll, ((float)stroke.getStrokeOpacity() * 255));
	rgbFill = Color(opacityFill, ((int) rgbFill.GetR()), ((int) rgbFill.GetG()), ((int) rgbFill.GetB()));
	rgbStroke = Color(opacityStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
	Pen penStroke(rgbStroke, stroke.getStrokeWidth());
	SolidBrush penFill(rgbFill); 
	if (fill.getFillColor().GetA() > 0 || fill.getFillColor().GetR() > 0 || fill.getFillColor().GetG() > 0 || fill.getFillColor().GetB() > 0) graphics.FillEllipse(&penFill, this->cx - this->rx, this->cy - this->ry, 2 * this->rx, 2 * this->ry);
	if (stroke.getStrokeColor().GetA() > 0 || stroke.getStrokeColor().GetR() > 0 || stroke.getStrokeColor().GetG() > 0 || stroke.getStrokeColor().GetB() > 0) graphics.DrawEllipse(&penStroke, this->cx - this->rx, this->cy - this->ry, 2 * this->rx, 2 * this->ry);
}

EllipseSVG::~EllipseSVG() {}