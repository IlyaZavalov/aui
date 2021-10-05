﻿/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include <ft2build.h>
#include <freetype/freetype.h>
#include "FreeType.h"
#include "AFontCharacter.h"
#include "AUI/Platform/AFontManager.h"
#include <fstream>
#include <string>
#include "AUI/Common/AStringVector.h"
#include "AFont.h"


AFont::AFont(AFontManager* fm, const AString& path) :
	ft(fm->mFreeType)
{
	if (FT_New_Face(fm->mFreeType->getFt(), path.toStdString().c_str(), 0, &face)) {
		throw AException(("Could not load font: " + path).toStdString());
	}
}

AFont::AFont(AFontManager *fm, const AUrl& url):
    ft(fm->mFreeType) {
    mFontDataBuffer = AByteBuffer::fromStream(url.open());

    if (FT_New_Memory_Face(fm->mFreeType->getFt(), (const FT_Byte*) mFontDataBuffer.data(), mFontDataBuffer.getSize(), 0, &face)) {
        throw AException(("Could not load font: " + url.getFull()).toStdString());
    }
}

AFont::~AFont() {
	FT_Done_Face(face);
	for (size_t i = 0; i < data.size(); i++) {
		AVector<Character*> c = data[i].chars;
		for (size_t j = 0; j < c.size(); j++) {
			Character* ch = c[j];
			delete ch;
		}
		c.clear();
	}
	data.clear();
}


glm::vec2 AFont::getKerning(wchar_t left, wchar_t right)
{
	FT_Vector vec2;
	FT_Get_Kerning(face, left, right, FT_KERNING_DEFAULT, &vec2);

	return { vec2.x >> 6, vec2.y >> 6 };
}

AFont::FontData& AFont::getCharsetBySize(long size, FontRendering fr) {
	for (size_t i = 0; i < data.size(); i++) {
		if (data[i].size == size && data[i].fontRendering == fr) {
			return data[i];
		}
	}
	FontData s;
	s.size = size;
	s.fontRendering = fr;
	data.push_back(s);
	//renderGlyphs(size);
	return data[data.size() - 1];
}
AFont::Character* AFont::renderGlyph(FontData& fs, long glyph, long size, FontRendering fr) {
	FT_Set_Pixel_Sizes(face, 0, size);

	FT_Int32 flags = FT_LOAD_RENDER;
	if (fr == FontRendering::SUBPIXEL)
		flags |= FT_LOAD_TARGET_LCD;
	if (fr == FontRendering::NEAREST)
	    flags |= FT_LOAD_TARGET_MONO;

	FT_Error e = FT_Load_Char(face, glyph, flags);
	if (e) {
		throw std::runtime_error(("Cannot load char: error code" + AString::number(e)).toStdString());
	}
	FT_GlyphSlot g = face->glyph;
	if (g->bitmap.width && g->bitmap.rows) {
		const float div = 1.f / 64.f;
		Character* c = new Character;
		c->c = glyph;
		c->width = g->bitmap.width;
		
		if (fr == FontRendering::SUBPIXEL)
			c->width /= 3;
		
		c->height = g->bitmap.rows;
		c->bearingX = float(g->bitmap_left);
		c->advanceX = g->metrics.horiAdvance * div;
		c->advanceY = -(g->metrics.horiBearingY * div) + size;

		AVector<uint8_t> data;

		if (fr == FontRendering::NEAREST) {
		    // when nearest, freetype renders glyphs into the 1bit-depth image but OpenGL required at least8bit-depth,
		    // so we will convert it here
            data.resize(g->bitmap.rows * g->bitmap.width);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                for (unsigned c = 0; c < g->bitmap.width; ++c) {
                    data[c + r * g->bitmap.width] = (bufPtr[c / 8] & (0b10000000 >> (c % 8))) ? 255 : 0;
                }
            }
        } else {
            data.reserve(g->bitmap.rows * g->bitmap.pitch);

            for (unsigned r = 0; r < g->bitmap.rows; ++r) {
                unsigned char* bufPtr = g->bitmap.buffer + r * g->bitmap.pitch;
                data.insert(data.end(), bufPtr, bufPtr + g->bitmap.width);
            }
		}

		int imageFormat = AImage::BYTE;
		if (fr == FontRendering::SUBPIXEL)
			imageFormat |= AImage::RGB;
		else
			imageFormat |= AImage::R;
		
		auto img = _new<AImage>(data, c->width, c->height, imageFormat);

		c->uv = fs.tp->insert(img);
		
		fs.isDirty = true;
		return c;
	}
	return nullptr;
}

_<GL::Texture2D> AFont::textureOf(long size, FontRendering fr) {
	FontData& chars = getCharsetBySize(size, fr);
	if (!chars.texture)
	{
		chars.texture = _new<GL::Texture2D>();
		chars.texture->setupNearest();
	}
	if (chars.isDirty)
	{
		chars.isDirty = false;
		chars.texture->tex2D(chars.tp->getImage());
		/*
		if (size == 14) {
			std::ofstream fos(std::to_string(size) + ".txt");
			for (uint16_t y = 0; y < chars.tp->getImage()->getHeight(); ++y) {
				for (uint16_t x = 0; x < chars.tp->getImage()->getWidth(); ++x) {
					char c = chars.tp->getImage()->getData()[y * chars.tp->getImage()->getWidth() + x];;
					unsigned char u = *reinterpret_cast<unsigned char*>(&c);
					if (u > 200)
						fos << "#";
					else if (u > 100)
						fos << ".";
					else
						fos << " ";
				}
				fos << std::endl;
			}
			fos.close();
		}*/
	}
	return chars.texture;
}
AFont::Character* AFont::getCharacter(long id, long size, FontRendering fr) {
	FontData& chars = getCharsetBySize(size, fr);
	if (chars.chars.size() > id && chars.chars[id]) {
		return chars.chars[id];
	}
	else {
		if (chars.chars.size() <= id)
			chars.chars.resize(id + 1, nullptr);
		Character* d = renderGlyph(chars, id, size, fr);
		chars.chars[id] = d;
		return d;
	}
	return nullptr;
}

float AFont::length(const AString& text, long size, FontRendering fr)
{
	float advance = 0;

	for (AString::const_iterator i = text.begin(); i != text.end(); i++) {
		if (*i == ' ')
			advance += size / 2.3f;
		else if (*i == '\n')
		    advance = 0;
		else {
			Character* ch = getCharacter(*i, size, fr);
			if (ch) {
                advance += ch->advanceX;
                advance = glm::floor(advance);
            }
			else
				advance += size / 2.3f;
		}
	}
	return advance;
}

AString AFont::trimStringToWidth(const AString& text, size_t width, long size, const FontRendering& fr) {
	return trimStringToWidth(text.begin(), text.end(), width, size, fr);
}
AString AFont::trimStringToWidth(AString::const_iterator begin, AString::const_iterator end, size_t width, long size, const FontRendering& fr) {
	AString s;
	size_t advance = 0;
	size_t length = 0;
	size_t space_width = AFont::length(" ", size, fr);

	for (auto i = begin; i != end;) {
		// find next space
		AString::const_iterator wordEnd = i;
		for (; wordEnd != end && *wordEnd != ' '; ++wordEnd) {

		}
		AString word = {i, wordEnd};
		if (wordEnd == end) {
			i = end;
		} else {
			i = wordEnd + 1;
		}

		if (word.empty()) {
			if (!s.empty())
				s += " ";
			continue;
		}
		size_t l = AFont::length(word, size, fr);
		if (s.size())
			l += size;

		if (length + l > width && !(l > width))
			return s;

		if (!s.empty()) {
			s += AString(" ");
			length += space_width;
		}
		for (size_t j = 0; j < word.size(); j++) {
			if (length > width)
			{
				return s;
			}
			wchar_t c = word[j];
			if (c == '\n') {
				return s;
			}
			AString str = c;
			s += str;
			length += AFont::length(str, size, fr);
		}
	}
	return s;
}

size_t AFont::indexOfX(const AString& text, size_t width, long size, FontRendering fr) {
    float advance = 0;
    size_t index = 0;

    for (auto c : text) {
        float widthOfChar;
        if (c == ' ') {
            widthOfChar = size / 2.3f;
        } else {
            auto characterInfo = getCharacter(c, size, fr);
            if (!characterInfo) {
                continue;
            }

            widthOfChar = characterInfo->advanceX;
        }

        if (advance + widthOfChar / 2.f > width) {
            break;
        }
        advance += widthOfChar;
        advance = glm::floor(advance);
        index += 1;
    }

    return index;
}

AStringVector AFont::trimStringToMultiline(const AString& text, int width, long scale, FontRendering fr) {
	AStringVector v;
	AString currentText = text;
	while (currentText.length()) {
		AString l = trimStringToWidth(currentText, width, scale, fr);
		v.push_back(l);
		size_t len = l.length();
		if (len + 1 >= currentText.length())
			break;
		currentText = currentText.mid(len + 1);
	}
	return v;
}

bool AFont::isHasKerning()
{
	return FT_HAS_KERNING(face);
}

int AFont::getAscenderHeight(long size) const
{
	return int(face->ascender) * size / face->height;
}


int AFont::getDescenderHeight(long size) const
{
	return -int(face->descender) * size / face->height;
}

_<Util::SimpleTexturePacker> AFont::texturePackerOf(long size, FontRendering fr) {
	return getCharsetBySize(size, fr).tp;
}