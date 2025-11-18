#include "Function.h"
#include "ParserSVG.h"

using namespace Gdiplus;
using namespace std;

ParserSVG::ParserSVG() {
	svgDocument.clear();
}

void ParserSVG::readSVG(vector<char>& xmlBuffer) {
	// Parse the buffer using the xml file parsing library into doc 
	svgDocument.parse<0>(&xmlBuffer[0]);
	return;
}

xml_node<>* ParserSVG::getSVGDocumentFirstNode(){
	return svgDocument.first_node();
}

ParserSVG::~ParserSVG() {}