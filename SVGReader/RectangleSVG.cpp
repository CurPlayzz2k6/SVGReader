#include "RectangleSVG.h"

using namespace Gdiplus;
using namespace std;

RectangleSVG::RectangleSVG() {
	this->height = 0;
	this->width = 0;
	this->x = 0;
	this->y = 0;
	this->rx = 0;
	this->ry = 0;
}

void RectangleSVG::read(xml_node<>* node) {
	for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
		string type(attr->name());
		if (type == "x")
			x = stof(attr->value());
		else if (type == "y")
			y = stof(attr->value());
		else if (type == "width")
			width = stof(attr->value());
		else if (type == "height")
			height = stof(attr->value());
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

void RectangleSVG::draw(Graphics& graphics) {
	if (rx == 0 && ry == 0) {
		float opacityAll = ((float)opacity.getOpacity() * 255);
		Color rgbFill = fill.getFillColor();
		float opacityFill = min(opacityAll, ((float)fill.getFillOpacity() * 255), ((float)fill.getFillColor().GetA()));
		Color rgbStroke = stroke.getStrokeColor();
		float opacityStroke = min(opacityAll, ((float)stroke.getStrokeOpacity() * 255), ((float)stroke.getStrokeColor().GetA()));
		rgbFill = Color(opacityFill, rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
		rgbStroke = Color(opacityStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
		Pen penStroke(rgbStroke, stroke.getStrokeWidth());
		SolidBrush penFill(rgbFill);
		if (fill.getFillColor().GetA() > 0 || fill.getFillColor().GetR() > 0 || fill.getFillColor().GetG() > 0 || fill.getFillColor().GetB() > 0) graphics.FillRectangle(&penFill, this->x, this->y, this->width, this->height);
		if (stroke.getStrokeColor().GetA() > 0 || stroke.getStrokeColor().GetR() > 0 || stroke.getStrokeColor().GetG() > 0 || stroke.getStrokeColor().GetB() > 0) graphics.DrawRectangle(&penStroke, this->x, this->y, this->width, this->height);
	}
	else {
		if (this->rx > this->width / 2.0f) this->rx = this->width / 2.0f;
		if (this->ry > this->height / 2.0f) this->ry = this->height / 2.0f;
		if (this->rx == 0) this->rx = this->ry;
		if (this->ry == 0) this->ry = this->rx;
		GraphicsPath path;
		RectF rect(this->x, this->y, this->width, this->height);
		float dx = 2 * this->rx;
		float dy = 2 * this->ry;

		path.StartFigure();
		path.AddArc(RectF(this->x, this->y, dx, dy), 180, 90);
		path.AddArc(RectF(this->x + this->width - dx, this->y, dx, dy), 270, 90);
		path.AddArc(RectF(this->x + this->width - dx, this->y + this->height - dy, dx, dy), 0, 90);
		path.AddArc(RectF(this->x, this->y + this->height - dy, dx, dy), 90, 90);
		path.CloseFigure();

		float opacityAll = ((float)opacity.getOpacity() * 255);
		Color rgbFill = fill.getFillColor();
		float opacityFill = minValue(opacityAll, ((float)fill.getFillOpacity() * 255));
		Color rgbStroke = stroke.getStrokeColor();
		float opacityStroke = minValue(opacityAll, ((float)stroke.getStrokeOpacity() * 255));
		rgbFill = Color(opacityFill, rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
		rgbStroke = Color(opacityStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
		Pen penStroke(rgbStroke, stroke.getStrokeWidth());
		SolidBrush penFill(rgbFill);
		if (fill.getFillColor().GetA() > 0 || fill.getFillColor().GetR() > 0 || fill.getFillColor().GetG() > 0 || fill.getFillColor().GetB() > 0) graphics.FillPath(&penFill, &path);
		if (stroke.getStrokeColor().GetA() > 0 || stroke.getStrokeColor().GetR() > 0 || stroke.getStrokeColor().GetG() > 0 || stroke.getStrokeColor().GetB() > 0) graphics.DrawPath(&penStroke, &path);
	}
}

RectangleSVG::~RectangleSVG() {}