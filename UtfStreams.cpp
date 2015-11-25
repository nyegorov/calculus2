#include "stdafx.h"

#ifdef  _MSC_VER
#   pragma warning( disable: 4373 )     // "Your override overrides"
#endif

#include <assert.h>         // assert
#include <codecvt>          // std::codecvt_utf8 (C++11)
#include <stdexcept>        // std::exception
#include <stdlib.h>         // EXIT_SUCCESS, EXIT_FAILURE
#include <streambuf>        // std::basic_streambuf
#include <string>           // wstring
#include <iostream>         // std::cout, std::endl
#include <locale>           // std::locale
#include <memory>           // std::unique_ptr (C++11)
using namespace std;

#include    <io.h>      // _setmode, _fileno
#include    <fcntl.h>   // _O_U8TEXT

#undef  UNICODE
#define UNICODE
#undef  STRICT
#define STRING
#include <windows.h>    // MultiByteToWideChar


template< class CharType >
class AbstractOutputBuffer
	: public basic_streambuf< CharType >
{
public:
	typedef basic_streambuf< CharType >     Base;
	typedef typename Base::traits_type      Traits;

	typedef Base                            StreamBuffer;

protected:
	virtual streamsize xsputn(char_type const* const s, streamsize const n) = 0;

	virtual int_type overflow(int_type const c)
	{
		bool const cIsEOF = Traits::eq_int_type(c, Traits::eof());
		int_type const  failureValue = Traits::eof();
		int_type const  successValue = (cIsEOF ? Traits::not_eof(c) : c);

		if(!cIsEOF) {
			char_type const     ch = Traits::to_char_type(c);
			streamsize const    nCharactersWritten = xsputn(&ch, 1);

			return (nCharactersWritten == 1 ? successValue : failureValue);
		}
		return successValue;
	}

public:
	AbstractOutputBuffer()
	{}

	AbstractOutputBuffer(StreamBuffer& existingBuffer)
		: Base(existingBuffer)
	{}
};


class OutputForwarderBuffer
	: public AbstractOutputBuffer< char >
{
public:
	typedef AbstractOutputBuffer< char >    Base;
	typedef Base::Traits                    Traits;

	typedef Base::StreamBuffer              StreamBuffer;
	typedef basic_streambuf<wchar_t>        WideStreamBuffer;

private:
	WideStreamBuffer*       pWideStreamBuffer_;
	wstring                 wideCharBuffer_;

	OutputForwarderBuffer(OutputForwarderBuffer const&);      // No such.
	void operator=(OutputForwarderBuffer const&);             // No such.

protected:
	virtual streamsize xsputn(char const* const s, streamsize const n)
	{
		if(n == 0) { return 0; }

		int const   nAsInt = static_cast<int>(n);    //  Visual C++ sillywarnings.
		wideCharBuffer_.resize(nAsInt);
		int const nWideCharacters = MultiByteToWideChar(
			CP_ACP,             // Windows ANSI
			MB_PRECOMPOSED,     // Always precompose characters (this is the default).
			s,                  // Narrow character string.
			nAsInt,             // Number of bytes in narrow character string.
			&wideCharBuffer_[0],
			nAsInt              // Wide char buffer size.
			);
		assert(nWideCharacters > 0);
		return pWideStreamBuffer_->sputn(&wideCharBuffer_[0], nWideCharacters);
	}

public:
	OutputForwarderBuffer(
		StreamBuffer&       existingBuffer,
		WideStreamBuffer*   pWideStreamBuffer
		)
		: Base(existingBuffer)
		, pWideStreamBuffer_(pWideStreamBuffer)
	{}
};


class DirectOutputBuffer
	: public AbstractOutputBuffer< wchar_t >
{
public:
	enum StreamId { outputStreamId, errorStreamId, logStreamId };

private:
	StreamId    streamId_;

protected:
	virtual streamsize xsputn(wchar_t const* const s, streamsize const n)
	{
		static HANDLE const outputStreamHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		static HANDLE const errorStreamHandle = GetStdHandle(STD_ERROR_HANDLE);

		HANDLE const    streamHandle =
			(streamId_ == outputStreamId ? outputStreamHandle : errorStreamHandle);

		DWORD nCharactersWritten = 0;
		bool writeSucceeded = !!WriteConsole(
			streamHandle, s, static_cast< DWORD >(n), &nCharactersWritten, 0
			);
		return (writeSucceeded ? static_cast< streamsize >(nCharactersWritten) : 0);
	}

public:
	DirectOutputBuffer(StreamId streamId = outputStreamId)
		: streamId_(streamId)
	{}
};


void setUtf8Conversion(wostream& stream)
{
	typedef codecvt_utf8< wchar_t >     CodeCvt;

	unique_ptr< CodeCvt > pCodeCvt(new CodeCvt);
	locale  utf8Locale(locale(), pCodeCvt.get());
	pCodeCvt.release();
	stream.imbue(utf8Locale);
}

bool isConsole(HANDLE streamHandle)
{
	DWORD consoleMode;
	return !!GetConsoleMode(streamHandle, &consoleMode);
}

bool isConsole(DWORD stdStreamId)
{
	return isConsole(GetStdHandle(stdStreamId));
}

void setDirectOutputSupport(wostream& stream)
{
	typedef DirectOutputBuffer  DOB;

	if(&stream == &wcout) {
		if(isConsole(STD_OUTPUT_HANDLE)) {
			static DOB  outputStreamBuffer(DOB::outputStreamId);
			stream.rdbuf(&outputStreamBuffer);
		}
	} else if(&stream == &wcerr) {
		if(isConsole(STD_ERROR_HANDLE)) {
			static DOB errorStreamBuffer(DOB::errorStreamId);
			stream.rdbuf(&errorStreamBuffer);
		}
	} else if(&stream == &wclog) {
		if(isConsole(STD_ERROR_HANDLE)) {
			static DOB logStreamBuffer(DOB::logStreamId);
			stream.rdbuf(&logStreamBuffer);
		}
	} else {
		assert(("setDirectOutputSupport: unsupported stream", false));
	}
}

void initStreams()
{
	// Set up UTF-8 conversions &  direct console output:
	_setmode(_fileno(stdin), _O_U8TEXT);
	_setmode(_fileno(stdout), _O_U8TEXT);
	setUtf8Conversion(wcerr);  setDirectOutputSupport(wcerr);
	setUtf8Conversion(wclog);  setDirectOutputSupport(wclog);

	// Forward narrow character output to the wide streams:
	static OutputForwarderBuffer    coutBuffer(*cout.rdbuf(), wcout.rdbuf());
	static OutputForwarderBuffer    cerrBuffer(*cerr.rdbuf(), wcerr.rdbuf());
	static OutputForwarderBuffer    clogBuffer(*clog.rdbuf(), wclog.rdbuf());

	cout.rdbuf(&coutBuffer);
	cerr.rdbuf(&cerrBuffer);
	clog.rdbuf(&clogBuffer);
}
