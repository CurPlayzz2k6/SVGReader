#include "CircleSVG.h"

using namespace Gdiplus;
using namespace std;

CircleSVG::CircleSVG() {
	cx = 0;
	cy = 0;
	r = 0;
}

void CircleSVG::read(xml_node<>* node) {
	for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
		string type(attr->name());
		if (type == "cx")
			cx = stof(attr->value());
		else if (type == "cy")
			cy = stof(attr->value());
		else if (type == "r")
			rx = ry = stof(attr->value());
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

CircleSVG::~CircleSVG() {}