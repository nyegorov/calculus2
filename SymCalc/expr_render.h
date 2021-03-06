#pragma once

#include "../calculus.h"
#include "../parser.h"

using std::string;

using image_ptr		= std::unique_ptr<NSVGimage, void (*)(NSVGimage*)>;
using rasterizer_ptr= std::unique_ptr<NSVGrasterizer, void (*)(NSVGrasterizer*)>;
using bitmap_ptr	= std::unique_ptr<std::remove_pointer_t<HBITMAP>, std::function<BOOL(HBITMAP)>>;
using mhandle_ptr	= std::unique_ptr<std::remove_pointer_t<mml_handle>, void (*)(mml_handle)>;

struct expr_info
{
	expr		source;
	expr		result;
	string		text;
	string		mml;
	string		svg;
	image_ptr 	pimage{nullptr, nsvgDelete};
	unsigned width()  const { return pimage ? (unsigned)pimage->width : 0; }
	unsigned height() const { return pimage ? (unsigned)pimage->height : 0; }
};

class expr_renderer
{
	rasterizer_ptr	_prasterizer;
	mhandle_ptr		_mhandle;
	NScript			_parser;

	string exp2mml(expr e)
	{
		std::ostringstream oss;
		oss << mml(true) << e;
		return oss.str();
	}

	string mml2svg(string mml)
	{
		string svg;
		if(mml_convert(_mhandle.get(), mml.c_str()) && mml_can_render(_mhandle.get())) {
			unsigned char *buf = nullptr;
			int size;
			if(mml_save_to_stream(_mhandle.get(), &buf, &size, mml_file_type_svg) && size) {
				svg.assign((char *)buf, (size_t)size);
				mml_destroy_stream(&buf);
			}
		}
		return svg;
	}
	NSVGimage* svg2img(string svg) { return nsvgParse(const_cast<char *>(svg.c_str()), "px", 96.0f); }

	void render_text(NSVGrasterizer* r, NSVGimage* image, HDC hdc, int x, int y)
	{
		NSVGshape *shape = NULL;
		wchar_t unicode_text[1'024];
		char decoded_text[NSVG_MAX_TEXT];

		for(shape = image->shapes; shape != NULL; shape = shape->next) {
			if(!(shape->flags & NSVG_FLAGS_VISIBLE) || !shape->text)
				continue;

			LOGFONT lf = {0};
			lf.lfHeight = -LONG(shape->fontSize + 0.5);
			lf.lfWeight = FW_NORMAL;
			lf.lfQuality = CLEARTYPE_QUALITY;
			MultiByteToWideChar(CP_ACP, 0, shape->fontFamily, strlen(shape->fontFamily), lf.lfFaceName, LF_FACESIZE);
			HFONT hFont = CreateFontIndirect(&lf);
			int decoded_size = decode_html_entities_utf8(decoded_text, shape->text);
			int unicode_size = MultiByteToWideChar(CP_UTF8, 0, decoded_text, decoded_size, unicode_text, sizeof(unicode_text) / sizeof(unicode_text[0]));

			TEXTMETRIC tm;
			SelectObject(hdc, hFont);
			SetTextColor(hdc, shape->fill.color);
			SetBkMode(hdc, TRANSPARENT);
			GetTextMetrics(hdc, &tm);
			TextOut(hdc, x + int(shape->paths->pts[0] + 0.5), y + int(shape->paths->pts[1] - tm.tmAscent + 0.5), unicode_text, unicode_size);
			DeleteObject(hFont);
		}
	}


public:
	expr_renderer(double scale) : _parser(false), 
		_prasterizer(nsvgCreateRasterizer(), nsvgDeleteRasterizer),
		_mhandle(mml_create_handle(), mml_free_handle)
	{
		mml_set_scale(_mhandle.get(), scale);
	}

	~expr_renderer()
	{
	}

	expr_info create(string src)
	{
		const char header[] = "<math xmlns='http://www.w3.org/1998/Math/MathML'>", footer[] = "</math>";
		expr_info me;
		expr e = _parser.eval(src);
		me.text = src;
		me.source = e;
		me.result = *e;

		if(src.find('~') != string::npos)	me.result = ~me.result;

		if(is<symbol>(me.result)) {
			expr val = as<symbol>(me.result).value();
			_parser.set(as<symbol>(me.result).name(), is<func>(val) ? val : me.result);
			if(val != empty)	me.result = val;
		}

		auto mml_src = exp2mml(me.source);
		auto mml_res = exp2mml(me.result);
		me.mml = mml_src + (src.find('~') == string::npos ? "<mo mathcolor='red'>&rArr;</mo>" : "<mo>&asymp;</mo>") + mml_res;
		me.svg = mml2svg(me.mml);
		me.pimage.reset(svg2img(me.svg));
		return me;
	}

	void render(const expr_info& me, HDC hdc, int x, int y)
	{
		if(me.pimage->shapes == nullptr)	return;
		unsigned width = me.width();
		unsigned height = me.height();
		std::vector<uint32_t> image_data(width * height);

		HDC hdcMem = CreateCompatibleDC(hdc);
		nsvgRasterize(_prasterizer.get(), me.pimage.get(), 0, 0, 1, (unsigned char *)&image_data.front(), width, height, width * 4);
		auto pbitmap = bitmap_ptr(CreateBitmap(width, height, 1, 32, &image_data.front()), ::DeleteObject);
		HGDIOBJ old_bmp = SelectObject(hdcMem, pbitmap.get());
		BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
		AlphaBlend(hdc, x, y, width, height, hdcMem, 0, 0, width, height, bf);
		render_text(_prasterizer.get(), me.pimage.get(), hdc, x, y);
		SelectObject(hdcMem, old_bmp);
		DeleteDC(hdcMem);
	}
};

