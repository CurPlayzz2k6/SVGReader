#include "PathSVG.h"
#include "DefinitionsSVG.h"
#include <iostream>
#include <algorithm>

using namespace Gdiplus;
using namespace std;

PathSVG::PathSVG() : FillShapeSVG() {}

string PathSVG::formatPathString(string d) {
    string result = "";
    for (int i = 0; i < d.size(); ++i) {
        char c = d[i];
        if (c == ',' || c == '\n' || c == '\r' || c == '\t' || c == '\r\n' || c == '\n\r') {
            result += ' ';
        }
        else if (c == '-') {
            if (i > 0 && (d[i - 1] == 'e' || d[i - 1] == 'E')) result += c;
            else result += " -";
        }
        else if (isalpha(c) && c != 'e' && c != 'E') {
            result += ' '; result += c; result += ' ';
        }
        else result += c;
    }
    return result;
}

void PathSVG::read(xml_node<>* node) {
    hasFillColor = false; hasFillOpacity = false;
    hasStrokeColor = false; hasStrokeWidth = false; hasStrokeOpacity = false;
    hasOpacity = false;

    for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
        string type = attr->name();
        string value = attr->value();

        if (type == "d") 
            parsePathData(value);
        else if (type == "opacity") { 
            opacity.setOpacity(stof(attr->value())); 
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
            fill.setFillOpacity(stof(attr->value())); 
            hasFillOpacity = true; 
        }
        else if (type == "stroke") { 
            stroke.setStrokeColor(getRGB(attr->value())); 
            hasStrokeColor = true; 
        }
        else if (type == "stroke-width") { 
            stroke.setStrokeWidth(stof(attr->value())); 
            hasStrokeWidth = true; 
        }
        else if (type == "stroke-opacity") { 
            stroke.setStrokeOpacity(stof(attr->value())); 
            hasStrokeOpacity = true; 
        }
        else if (type == "transform") 
            parseTransform(value);
    }
}

void PathSVG::parsePathData(string d) {
    string formattedD = formatPathString(d);
    stringstream pathStream(formattedD);
    char command = 0, lastCommand = 0;
    float x, y, x1, y1, x2, y2, dx, dy;
    PointF currentPoint(0, 0), lastControlPoint(0, 0), startFigurePoint(0, 0);

    pathStream >> ws;
    while (pathStream.peek() != EOF) {
        char nextChar = pathStream.peek();
        if (isalpha(nextChar)) 
            pathStream >> command;
        else {
            if (command == 0) 
                break;
            if (command == 'M') 
                command = 'L';
            if (command == 'm') 
                command = 'l';
        }

        if (command == 'M' || command == 'm') {
            float moveX, moveY; 
            pathStream >> moveX >> moveY;
            if (command == 'M') 
                currentPoint = PointF(moveX, moveY);
            else currentPoint = PointF(currentPoint.X + moveX, currentPoint.Y + moveY);
            path.StartFigure(); 
            startFigurePoint = currentPoint;
            if (command == 'M') 
                command = 'L'; 
            if (command == 'm') 
                command = 'l';
            lastCommand = command;
        }
        else if (command == 'L' || command == 'l') {
            pathStream >> dx >> dy;
            if (command == 'L') { 
                x = dx; 
                y = dy; 
            }
            else { 
                x = currentPoint.X + dx; 
                y = currentPoint.Y + dy; 
            }
            path.AddLine(currentPoint, PointF(x, y)); 
            currentPoint = PointF(x, y);
        }
        else if (command == 'H') { 
            pathStream >> x; 
            path.AddLine(currentPoint, PointF(x, currentPoint.Y)); 
            currentPoint.X = x; 
        }
        else if (command == 'h') { 
            pathStream >> dx; 
            currentPoint.X += dx; 
            path.AddLine(currentPoint, PointF(currentPoint.X, currentPoint.Y)); 
        }
        else if (command == 'V') { 
            pathStream >> y; 
            path.AddLine(currentPoint, PointF(currentPoint.X, y)); 
            currentPoint.Y = y; 
        }
        else if (command == 'v') { 
            pathStream >> dy; 
            currentPoint.Y += dy; 
            path.AddLine(currentPoint, PointF(currentPoint.X, currentPoint.Y)); 
        }
        else if (command == 'C' || command == 'c') {
            pathStream >> x1 >> y1 >> x2 >> y2 >> x >> y;
            if (command == 'c') {
                x1 += currentPoint.X; 
                y1 += currentPoint.Y; 
                x2 += currentPoint.X; 
                y2 += currentPoint.Y; 
                x += currentPoint.X; 
                y += currentPoint.Y;
            }
            path.AddBezier(currentPoint, PointF(x1, y1), PointF(x2, y2), PointF(x, y));
            currentPoint = PointF(x, y); 
            lastControlPoint = PointF(x2, y2);
        }
        else if (command == 'S' || command == 's') {
            float nextX, nextY, ctrl2X, ctrl2Y;
            if (command == 'S') 
                pathStream >> ctrl2X >> ctrl2Y >> nextX >> nextY;
            else { 
                float dx2, dy2, dx3, dy3; 
                pathStream >> dx2 >> dy2 >> dx3 >> dy3; 
                ctrl2X = currentPoint.X + dx2; 
                ctrl2Y = currentPoint.Y + dy2; 
                nextX = currentPoint.X + dx3; 
                nextY = currentPoint.Y + dy3; 
            }

            if (lastCommand == 'C' || lastCommand == 'c' || lastCommand == 'S' || lastCommand == 's') {
                x1 = 2 * currentPoint.X - lastControlPoint.X; 
                y1 = 2 * currentPoint.Y - lastControlPoint.Y;
            }
            else { 
                x1 = currentPoint.X; 
                y1 = currentPoint.Y; 
            }

            path.AddBezier(currentPoint, PointF(x1, y1), PointF(ctrl2X, ctrl2Y), PointF(nextX, nextY));
            currentPoint = PointF(nextX, nextY); 
            lastControlPoint = PointF(ctrl2X, ctrl2Y);
        }
        else if (command == 'Z' || command == 'z') { 
            path.CloseFigure(); 
            currentPoint = startFigurePoint; 
        }

        if (command != 'M' && command != 'm') 
            lastCommand = command;
        pathStream >> ws;
    }
}

void PathSVG::draw(Graphics& graphics, const DefinitionsSVG& defs) {
    GraphicsState state = graphics.Save();
    graphics.MultiplyTransform(&transformMatrix, MatrixOrderPrepend);

    float opacityAll = opacity.getOpacity() * 255.0f;
    if (hasStrokeColor && !hasStrokeWidth) {
        stroke.setStrokeWidth(1.0f);
    }
    const LinearGradientData* gradientData = nullptr;
    if (!fillGradientId.empty()) {
        gradientData = defs.getGradient(fillGradientId);
    }

    if (gradientData) {
        RectF bounds;
        path.GetBounds(&bounds);

        PointF start(bounds.X + gradientData->startPoint.X * bounds.Width, bounds.Y + gradientData->startPoint.Y * bounds.Height);
        PointF end(bounds.X + gradientData->endPoint.X * bounds.Width, bounds.Y + gradientData->endPoint.Y * bounds.Height);

        if (abs(start.X - end.X) < 0.001f && abs(start.Y - end.Y) < 0.001f)
            end.X += 0.1f;

        LinearGradientBrush gradientBrush(start, end, Color::Black, Color::White);
        int count = gradientData->stops.size();

        if (count > 0) {
            vector<Color> colors(count);
            vector<float> positions(count);

            for (int i = 0; i < count; ++i) {
                Color c = gradientData->stops[i].color;
                float stopAlpha = c.GetA();
                float fillOp = fill.getFillOpacity();
                float finalA = stopAlpha * (opacityAll / 255.0f) * fillOp;
                if (finalA > 255.0f) 
                    finalA = 255.0f;
                if (finalA < 0.0f) 
                    finalA = 0.0f;

                colors[i] = Color(finalA, c.GetR(), c.GetG(), c.GetB());
                positions[i] = gradientData->stops[i].offset;
            }
            gradientBrush.SetInterpolationColors(colors.data(), positions.data(), count);
            graphics.FillPath(&gradientBrush, &path);
        }
    }
    else {
        Color rgbFill = fill.getFillColor();
        float opFill = min(opacityAll, fill.getFillOpacity() * 255.0f);
        if (fill.getFillColor().GetA() == 0) 
            opFill = 0;

        if (opFill > 0) {
            Color finalFill(opFill, rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
            SolidBrush brush(finalFill);
            graphics.FillPath(&brush, &path);
        }
    }

    Color rgbStroke = stroke.getStrokeColor();
    float opStroke = min(opacityAll, stroke.getStrokeOpacity() * 255.0f);
    if (stroke.getStrokeColor().GetA() == 0) 
        opStroke = 0;

    if (opStroke > 0 && stroke.getStrokeWidth() > 0) {
        Color finalStroke(opStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
        Pen pen(finalStroke, stroke.getStrokeWidth());
        graphics.DrawPath(&pen, &path);
    }

    graphics.Restore(state);
}

PathSVG::~PathSVG() {}