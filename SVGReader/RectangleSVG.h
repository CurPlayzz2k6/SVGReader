#ifndef RECTANGLESVG_H
#define RECTANGLESVG_H

#include "Function.h"
#include "FillShapeSVG.h"
// Lưu ý: Không include "DefinitionsSVG.h" ở đây nếu chỉ dùng trong tham số hàm (đã forward declare ở ElementSVG),
// nhưng include để chắc chắn cũng không sao. Tốt nhất là forward declare trong .h và include trong .cpp.

using namespace Gdiplus;
using namespace rapidxml;
using namespace std;

class RectangleSVG : public FillShapeSVG {
private:
    float width;
    float height;
    float x, y;
    float rx, ry;

public:
    RectangleSVG();
    void read(xml_node<>* node) override;

    // CẬP NHẬT
    void draw(Graphics& graphics, const DefinitionsSVG& defs) override;

    ~RectangleSVG();
};
#endif RECTANGLESVG_H