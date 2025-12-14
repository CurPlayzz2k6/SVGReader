#include "CircleSVG.h"

using namespace Gdiplus;
using namespace std;

CircleSVG::CircleSVG() : EllipseSVG() {
	r = 0;
}

void CircleSVG::read(xml_node<>* node) {
	hasFillColor = false; 
	hasFillOpacity = false;
	hasStrokeColor = false; 
	hasStrokeWidth = false; 
	hasStrokeOpacity = false;
	hasOpacity = false;
	fillGradientId = "";

	for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
		string type(attr->name());
		string value = attr->value();

		if (type == "cx") 
			cx = stof(value);
		else if (type == "cy") 
			cy = stof(value);
		else if (type == "r")
			rx = ry = stof(value);
		else if (type == "opacity") {
			opacity.setOpacity(stof(value));
			hasOpacity = true;
		}
		else if (type == "fill") {
			if (value.find("url(") != string::npos) {
				fillGradientId = getUrlId(value);
				hasFillColor = true;
			}
			else {
				fill.setFillColor(getRGB(value));
				hasFillColor = true;
			}
		}
		else if (type == "fill-opacity") {
			fill.setFillOpacity(stof(value));
			hasFillOpacity = true;
		}
		else if (type == "stroke") {
			stroke.setStrokeColor(getRGB(value));
			hasStrokeColor = true;
		}
		else if (type == "stroke-width") {
			stroke.setStrokeWidth(stof(value));
			hasStrokeWidth = true;
		}
		else if (type == "stroke-opacity") {
			stroke.setStrokeOpacity(stof(value));
			hasStrokeOpacity = true;
		}
		else if (type == "transform")
			parseTransform(value);
	}
}

CircleSVG::~CircleSVG() {}