#include "ElementSVG.h"
#include <iostream>
#include <sstream>

ElementSVG::ElementSVG() {
    opacity.setOpacity(1.0f);
    transformMatrix.Reset(); // Khởi tạo ma trận đơn vị
}

void ElementSVG::parseTransform(string transformStr) {
    string s = formatTransformString(transformStr);
    stringstream transformStream(s);
    string temp;

    transformMatrix.Reset();

    while (transformStream >> temp) {
        if (temp == "translate") {
            float tx, ty = 0;
            transformStream >> tx;

            if (!(transformStream >> ty)) {
                ty = 0;
                transformStream.clear();
            }
            transformMatrix.Translate(tx, ty, MatrixOrderPrepend);
        }
        else if (temp == "rotate") {
            float angle, cx, cy;
            transformStream >> angle;

            streampos oldPos = transformStream.tellg();

            if (transformStream >> cx >> cy) {
                transformMatrix.RotateAt(angle, PointF(cx, cy), MatrixOrderPrepend);
            }
            else {
                transformStream.clear();
                transformMatrix.Rotate(angle, MatrixOrderPrepend);
            }
        }
        else if (temp == "scale") {
            float sx, sy;
            transformStream >> sx;

            if (!(transformStream >> sy)) {
                sy = sx;
                transformStream.clear();
            }
            transformMatrix.Scale(sx, sy, MatrixOrderPrepend);
        }
        else if (temp == "matrix") {
            float m11, m12, m21, m22, dx, dy;
            if (transformStream >> m11 >> m12 >> m21 >> m22 >> dx >> dy) {
                Matrix m(m11, m12, m21, m22, dx, dy);
                transformMatrix.Multiply(&m, MatrixOrderPrepend);
            }
        }
    }
}