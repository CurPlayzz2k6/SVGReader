#include "Function.h"
#include "RenderSVG.h"

using namespace Gdiplus;
using namespace std;

RenderSVG::RenderSVG() {}

void RenderSVG::readRecursiveElement(xml_node<>* node) {
	if (!node) return;
	if (node->name()) {
		const string nodeName(node->name());
		if (nodeName == "rect") {
			ShapeSVG* rect = new RectangleSVG();
			rect->read(node);
			shapes.push_back(rect);
		}
		else if (nodeName == "circle") {
			ShapeSVG* circle = new CircleSVG();
			circle->read(node);
			shapes.push_back(circle);
		}
		else if (nodeName == "ellipse") {
			ShapeSVG* ellipse = new EllipseSVG();
			ellipse->read(node);
			shapes.push_back(ellipse);
		}
		else if (nodeName == "text") {
			ShapeSVG* text = new TextSVG();
			text->read(node);
			shapes.push_back(text);
		}
		else if (nodeName == "line") {
			ShapeSVG* line = new LineSVG();
			line->read(node);
			shapes.push_back(line);
		}
		else if (nodeName == "polygon") {
			ShapeSVG* polygon = new PolygonSVG();
			polygon->read(node);
			shapes.push_back(polygon);
		}
		else if (nodeName == "polyline") {
			ShapeSVG* polyline = new PolylineSVG();
			polyline->read(node);
			shapes.push_back(polyline);
		}
	}

	// Đệ quy: Lặp qua tất cả các node con thuộc tag đã đọc và gọi lại hàm này
	for (xml_node<>* child = node->first_node(); child; child = child->next_sibling())
		readRecursiveElement(child);
}

void RenderSVG::readAll(ParserSVG& fileXML) {
	xml_node<>* rootNode = fileXML.getSVGDocumentFirstNode();
	if (rootNode != NULL) {
		xml_node<>* node;
		node = rootNode->first_node();
		if (node == NULL) {
			return;
		}
		while (node != NULL) {
			readRecursiveElement(node);
			node = node->next_sibling();
		}
	}
}

void RenderSVG::drawAll(Graphics* graphics) {
	for (ShapeSVG* shape : shapes)
		shape->draw(*graphics);
	return;
}

void RenderSVG::clearAll() {
	if (!shapes.empty())
		for (ShapeSVG* shape : shapes)
			if (shape != NULL)
				delete shape;
	shapes.clear();
}

RenderSVG::~RenderSVG() {
	clearAll();
}