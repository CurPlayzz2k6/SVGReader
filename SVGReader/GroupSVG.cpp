#include "GroupSVG.h"
#include "Function.h"
#include "NonFillShapeSVG.h" 
#include "FillShapeSVG.h" 
#include "DefinitionsSVG.h" 

using namespace Gdiplus;
using namespace std;

GroupSVG::GroupSVG() {
    opacity.setOpacity(1.0f);

    fill.setFillColor(Color(255, 0, 0, 0)); // Đen
    fill.setFillOpacity(1.0f);

    stroke.setStrokeColor(Color(0, 0, 0, 0)); // Trong suốt
    stroke.setStrokeWidth(0);
    stroke.setStrokeOpacity(1.0f);

    hasFillColor = false;
    hasFillOpacity = false;
    hasStrokeColor = false;
    hasStrokeWidth = false;
    hasStrokeOpacity = false;
    hasOpacity = false;
}

void GroupSVG::inheritStyle(FillSVG& pFill, StrokeSVG& pStroke, OpacitySVG& pOpacity) {
    if (!hasFillColor) 
        this->fill.setFillColor(pFill.getFillColor());
    if (!hasFillOpacity) 
        this->fill.setFillOpacity(pFill.getFillOpacity());
    if (!hasStrokeColor) 
        this->stroke.setStrokeColor(pStroke.getStrokeColor());
    if (!hasStrokeWidth) 
        this->stroke.setStrokeWidth(pStroke.getStrokeWidth());
    if (!hasStrokeOpacity) 
        this->stroke.setStrokeOpacity(pStroke.getStrokeOpacity());

    // Opacity có tính chất nhân dồn
    float parentOp = pOpacity.getOpacity();
    if (!hasOpacity) this->opacity.setOpacity(parentOp);
    else this->opacity.setOpacity(this->opacity.getOpacity() * parentOp);

    for (ElementSVG* child : children) {
        // Nếu con là hình có Fill (Rect, Circle, Ellipse...)
        FillShapeSVG* fillShape = dynamic_cast<FillShapeSVG*>(child);
        if (fillShape) {
            fillShape->inheritStyle(this->fill, this->stroke, this->opacity);
        }

        // Nếu con là hình không Fill (Line, Polyline)
        NonFillShapeSVG* nonFillShape = dynamic_cast<NonFillShapeSVG*>(child);
        if (nonFillShape) {
            nonFillShape->inheritStyle(this->stroke, this->opacity);
        }

        // Nếu con là Group khác (Đệ quy)
        GroupSVG* groupChild = dynamic_cast<GroupSVG*>(child);
        if (groupChild) {
            groupChild->inheritStyle(this->fill, this->stroke, this->opacity);
        }
    }
}

void GroupSVG::read(xml_node<>* node) {
    // Đọc thuộc tính của bản thân Group
    for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
        string name = attr->name();
        string value = attr->value();

        if (name == "transform")
            parseTransform(value);
        else if (name == "opacity") {
            opacity.setOpacity(stof(value));
            hasOpacity = true;
        }
        else if (name == "fill") {
            fill.setFillColor(getRGB(value));
            hasFillColor = true;
        }
        else if (name == "fill-opacity") {
            fill.setFillOpacity(stof(value));
            hasFillOpacity = true;
        }
        else if (name == "stroke") {
            stroke.setStrokeColor(getRGB(value));
            hasStrokeColor = true;
        }
        else if (name == "stroke-width") {
            stroke.setStrokeWidth(stof(value));
            hasStrokeWidth = true;
        }
        else if (name == "stroke-opacity") {
            stroke.setStrokeOpacity(stof(value));
            hasStrokeOpacity = true;
        }
    }

    // Duyệt và tạo các node con
    xml_node<>* child = node->first_node();
    while (child != NULL) {
        string nodeName = child->name();
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
            element->read(child);
            
            FillShapeSVG* fillShape = dynamic_cast<FillShapeSVG*>(element);
            if (fillShape)
                fillShape->inheritStyle(this->fill, this->stroke, this->opacity);

            NonFillShapeSVG* nonFillShape = dynamic_cast<NonFillShapeSVG*>(element);
            if (nonFillShape)
                nonFillShape->inheritStyle(this->stroke, this->opacity);

            GroupSVG* groupChild = dynamic_cast<GroupSVG*>(element);
            if (groupChild)
                groupChild->inheritStyle(this->fill, this->stroke, this->opacity);

            children.push_back(element);
        }
        child = child->next_sibling();
    }
}

void GroupSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    GraphicsState state = graphics.Save();
    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);

    for (ElementSVG* child : children) {
        if (child) {
            child->draw(graphics, defs);
        }
    }

    graphics.Restore(state);
}

void GroupSVG::clear() {
    for (ElementSVG* child : children) {
        if (child) delete child;
    }
    children.clear();
}

GroupSVG::~GroupSVG() {
    clear();
}