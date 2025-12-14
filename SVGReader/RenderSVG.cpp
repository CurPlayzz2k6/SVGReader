#include "RenderSVG.h"
// #include "DefinitionsSVG.h" // Đã include trong .h

using namespace Gdiplus;
using namespace std;

RenderSVG::RenderSVG() {}

void RenderSVG::readAll(ParserSVG& fileXML) {
    xml_node<>* rootNode = fileXML.getSVGDocumentFirstNode();
    if (rootNode == NULL) return;

    // CẬP NHẬT: Gọi readDefs thông qua biến thành viên defs
    for (xml_node<>* node = rootNode->first_node(); node; node = node->next_sibling())
        if (string(node->name()) == "defs")
            defs.readDefs(node);

    xml_node<>* node = rootNode->first_node();
    while (node != NULL) {
        const string nodeName = node->name();

        if (nodeName == "defs") {
            node = node->next_sibling();
            continue;
        }

        ElementSVG* element = nullptr;

        if (nodeName == "rect")
            element = new RectangleSVG();
        else if (nodeName == "circle")
            element = new CircleSVG();
        else if (nodeName == "ellipse")
            element = new EllipseSVG();
        else if (nodeName == "line")
            element = new LineSVG();
        else if (nodeName == "polygon")
            element = new PolygonSVG();
        else if (nodeName == "polyline")
            element = new PolylineSVG();
        else if (nodeName == "text")
            element = new TextSVG();
        else if (nodeName == "path")
            element = new PathSVG();
        else if (nodeName == "g")
            element = new GroupSVG();

        if (element != nullptr) {
            element->read(node);
            groups.push_back(element);
        }

        node = node->next_sibling();
    }
}

void RenderSVG::drawAll(Graphics* graphics) {
    for (ElementSVG* element : groups)
        if (element != NULL)
            // CẬP NHẬT: Truyền defs vào hàm draw
            element->draw(*graphics, defs);
}

void RenderSVG::clearAll() {
    if (!groups.empty()) {
        for (ElementSVG* element : groups) {
            if (element != NULL) delete element;
        }
    }
    groups.clear();
    // Có thể gọi defs.clear() nếu cần reset cho lần đọc sau
    defs.clear();
}

RenderSVG::~RenderSVG() {
    clearAll();
}