#include "sfontplus.h"
#include "utilities.h"
#include "debug.h"

SFontPlus::SFontPlus(const string &font, RGBAColor textColor, RGBAColor outlineColor) {
	this->textColor = textColor;
	this->outlineColor = outlineColor;
	
	if (!TTF_WasInit()) TTF_Init();
	this->font = TTF_OpenFont(font.c_str(), FONTSIZE);
	fontOutline = TTF_OpenFont(font.c_str(), FONTSIZE);
	TTF_SetFontOutline(fontOutline, 1);
	height = 0;
	// Get maximum line height with a sample text
	TTF_SizeUTF8(fontOutline, "AZ|¹0987654321", NULL, &height);
	halfHeight = height/2;
}

SFontPlus::~SFontPlus() {
	free(font);
	free(fontOutline);
}

bool SFontPlus::utf8Code(unsigned char c) {
	return (c>=194 && c<=198) || c==208 || c==209;
}

SFontPlus *SFontPlus::setColor(RGBAColor color) {
	textColor = color;
	return this;
}

SFontPlus *SFontPlus::setOutlineColor(RGBAColor color) {
	outlineColor = color;
	return this;
}
void SFontPlus::write(SDL_Surface *s, const string &text, int x, int y) {
	if (text.empty()) return;
	
	Surface bg;
	bg.raw = TTF_RenderUTF8_Blended(fontOutline, text.c_str(), rgbatosdl(outlineColor));
	
	Surface fg;
	fg.raw = TTF_RenderUTF8_Blended(font, text.c_str(), rgbatosdl(textColor));
	fg.blit(&bg, 1,1);
	
	bg.blit(s, x,y);
}

void SFontPlus::write(SDL_Surface* surface, const string& text, int x, int y, const unsigned short halign, const unsigned short valign) {
	switch (halign) {
		case SFontHAlignCenter:
			x -= getTextWidth(text)/2;
		break;
		case SFontHAlignRight:
			x -= getTextWidth(text);
		break;
	}

	switch (valign) {
		case SFontVAlignMiddle:
			y -= getHalfHeight();
		break;
		case SFontVAlignBottom:
			y -= getHeight();
		break;
	}

	write(surface, text, x, y);
}

void SFontPlus::write(SDL_Surface* surface, vector<string> *text, int x, int y, const unsigned short halign, const unsigned short valign) {
	switch (valign) {
		case SFontVAlignMiddle:
			y -= getHalfHeight()*text->size();
		break;
		case SFontVAlignBottom:
			y -= getHeight()*text->size();
		break;
	}

	for (uint i=0; i<text->size(); i++) {
		int ix = x;
		switch (halign) {
			case SFontHAlignCenter:
				ix -= getTextWidth(text->at(i))/2;
			break;
			case SFontHAlignRight:
				ix -= getTextWidth(text->at(i));
			break;
		}

		write(surface, text->at(i), x, y+getHeight()*i);
	}
}

void SFontPlus::write(Surface* surface, const string& text, int x, int y, const unsigned short halign, const unsigned short valign) {
	if (text.find("\n",0)!=string::npos) {
		vector<string> textArr;
		split(textArr,text,"\n");
		write(surface->raw, &textArr, x, y, halign, valign);
	} else
		write(surface->raw, text, x, y, halign, valign);
}

uint SFontPlus::getLineWidth(const string& text) {
	int width = 0;
	TTF_SizeUTF8(fontOutline, text.c_str(), &width, NULL);
	return width;
}
uint SFontPlus::getTextWidth(const string& text) {
	if (text.find("\n",0)!=string::npos) {
		vector<string> textArr;
		split(textArr,text,"\n");
		return getTextWidth(&textArr);
	} else
		return getLineWidth(text);
}
uint SFontPlus::getTextWidth(vector<string> *text) {
	int w = 0;
	for (uint i=0; i<text->size(); i++)
		w = max( getLineWidth(text->at(i)), w );
	return w;
}
