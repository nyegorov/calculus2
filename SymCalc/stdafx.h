// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0600
#define _WIN32_WINNT	0x0601
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0500

#include <atlbase.h>
#include <atlapp.h>
#include <atlstr.h>
#include <atlconv.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <atlddx.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlctrlx.h>

#include <atlframe.h>
#include <atldlgs.h>
#include <atlcrack.h>

#include <assert.h>

#include <algorithm>
#include <complex>
#include <functional>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/variant.hpp>
#include <boost/math/constants/constants.hpp>

#include "WtlAero.h"

#pragma comment(lib, "msimg32.lib")

#include "other/libreformath.h"
#pragma comment(lib, "other/reformath/libreformath.lib")

#include "other/nanosvg.h"
#include "other/nanosvgrast.h"
#include "other/entities.h"

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
