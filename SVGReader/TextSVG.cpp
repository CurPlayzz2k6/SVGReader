#include "TextSVG.h"

using namespace Gdiplus;
using namespace std;

TextSVG::TextSVG() {
	this->x = 0;
	this->y = 0;
	this->rotate = 0;
	this->fontSize = 0;
	this->fontStyle = "normal";
	this->fontFamily = "Times New Roman";
}


void TextSVG::read(xml_node<>* node) {
	string str(node->value());
	text = str;
	for (xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
		string type(attr->name());
		if (type == "x")
			x = stof(attr->value());
		else if (type == "y")
			y = stof(attr->value());
		else if (type == "rotate")
			rotate = stof(attr->value());
		else if (type == "font-family")
			fontFamily = attr->value();
		else if (type == "font-style")
			fontStyle = attr->value();
		else if (type == "font-size")
			fontSize = stof(attr->value());
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

void TextSVG::draw(Graphics& graphics) {
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
	if (this->rotate == 0) {
		GraphicsPath path;
		StringFormat form;
		form.SetAlignment(StringAlignmentNear);
		form.SetLineAlignment(StringAlignmentNear);
		wstring strText = ConvertStringToWstring(this->text);
		wstring strFontFamily = ConvertStringToWstring(this->fontFamily);
		const WCHAR* textWChar = strText.c_str();
		FontFamily fontF(strFontFamily.c_str());
		int fontStyle = FontStyleRegular;
		if (this->fontStyle == "italic")
			fontStyle = FontStyleItalic;
		else if (this->fontStyle == "oblique") {
			fontStyle = FontStyleItalic;
			rotate += 10; // Oblique có độ nghiêng 10 + this->rotate ban đầu
			draw(graphics); // Gọi đệ quy hàm drawText để vẽ text có điều kiện rotate > 0
			return;
		}
		int asUnits = fontF.GetCellAscent(fontStyle);
		int emUnits = fontF.GetEmHeight(fontStyle); // Tổng chiều cao

		// Chuyển đổi Ascent sang Pixels
		// (Tỷ lệ ascent / Tổng chiều cao) * Kích thước font bằng pixel
		float heightTextInPixel = ((float)asUnits / emUnits) * fontSize;

		// Tính GdiX, GdiY (Top-Left)
		float gdiX = this->x; // Điều chỉnh sai số để giống trên các trình duyệt web
		float gdiY = this->y - heightTextInPixel;

		PointF startPoint(gdiX, gdiY);
		path.AddString(textWChar, -1, &fontF, fontStyle, this->fontSize, startPoint, &form);
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
	else {
		wstring strFontFamily = ConvertStringToWstring(this->fontFamily);
		FontFamily fontF(strFontFamily.c_str());
		int fontStyle = FontStyleRegular;
		if (this->fontStyle == "italic" || this->fontStyle == "oblique")
			fontStyle = FontStyleItalic;
		Font font(&fontF, this->fontSize, fontStyle, UnitPixel);
		int asUnits = fontF.GetCellAscent(fontStyle);
		int emUnits = fontF.GetEmHeight(fontStyle); // Tổng chiều cao
		float heightTextInPixel = ((float)asUnits / emUnits) * fontSize;
		float gdiX = this->x - 0.2f * heightTextInPixel;
		float gdiY = this->y - 0.04f * heightTextInPixel;
		float curX = gdiX;
		float curY = gdiY; // Tọa độ baseline
		for (int i = 0; i < this->text.size(); i++) {
			StringFormat form;
			Region chrRegion;
			RectF layout;
			form.SetAlignment(StringAlignmentNear);
			form.SetLineAlignment(StringAlignmentNear);
			form.SetFormatFlags(form.GetFormatFlags() | StringFormatFlagsNoClip);
			CharacterRange chrRange(0, 1); // Đo 1 ký tự từ index = 0
			form.SetMeasurableCharacterRanges(1, &chrRange);
			string charStr(1, this->text[i]);
			wstring strText = ConvertStringToWstring(charStr);
			const WCHAR* wCharText = strText.c_str();
			GraphicsState state = graphics.Save();
			graphics.TranslateTransform(curX, curY);
			graphics.RotateTransform((float)this->rotate);
			PointF curPoint(0.0f, -heightTextInPixel);
			float opacityAll = ((float)opacity.getOpacity() * 255);
			Color rgbFill = fill.getFillColor();
			float opacityFill = minValue(opacityAll, ((float)fill.getFillOpacity() * 255));
			Color rgbStroke = stroke.getStrokeColor();
			float opacityStroke = minValue(opacityAll, ((float)stroke.getStrokeOpacity() * 255));
			rgbFill = Color(opacityFill, rgbFill.GetR(), rgbFill.GetG(), rgbFill.GetB());
			rgbStroke = Color(opacityStroke, rgbStroke.GetR(), rgbStroke.GetG(), rgbStroke.GetB());
			Pen penStroke(rgbStroke, stroke.getStrokeWidth());
			SolidBrush penFill(rgbFill);
			graphics.DrawString(wCharText, -1, &font, PointF(0.0f, -heightTextInPixel), &form, &penFill);
			graphics.Restore(state);

			GraphicsContainer cont = graphics.BeginContainer();
			graphics.ResetTransform();
			graphics.MeasureString(wCharText, -1, &font, PointF(0, 0), &form, &layout);
			graphics.EndContainer(cont);

			curX += layout.Width - layout.Width / 3.0f; // Dịch chuyển 1 khoảng sau khi vẽ xong 1 ký tự, /3.0f để tránh giãn chữ quá xa
		}
	}
}

TextSVG::~TextSVG() {}