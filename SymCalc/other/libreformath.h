/*
* ******************************DO NOT ALTER ANYTHING IN THIS FILE!!!!******************************
*
* Copyright (c) 2011 Peter Frane, Jr. All Rights Reserved
* Emails: reformath@hotmail.com (inquries, bug reports) and pfranejr@hotmail.com (donations via PayPal)
* Website: reformath.weebly.com (temporary)
*
*
* LIBREFORMATH is a library for converting MathML to other file formats such as PNG, PDF, SVG, XAML.
* The current version is 1.1.
* 
* Use of this software is STRICTLY for evaluation purposes only. The author will not be responsible for any damage to your system/data as a result of using this code.
*  
*										
*/

#ifdef LIBREFORMATH_EXPORTS
#define LIBREFORMATH_API __declspec(dllexport)
#else
#define LIBREFORMATH_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum mml_file_type 
{
    mml_file_type_pdf,
    mml_file_type_png,
    mml_file_type_svg,
    mml_file_type_xaml
};

struct mml_image_metrics
{
	int baseline, width, height, text_height, text_depth;
	double fwidth, fheight;
};

struct reformath_handle;

typedef reformath_handle *mml_handle;

LIBREFORMATH_API mml_handle mml_create_handle(void);

LIBREFORMATH_API void mml_free_handle( mml_handle handle );

LIBREFORMATH_API bool mml_convert( mml_handle handle, const char *mathml_text );

LIBREFORMATH_API bool mml_save_to_file( mml_handle handle, const char *filename, mml_file_type file_type );

LIBREFORMATH_API bool mml_save_to_file_w( mml_handle handle, const wchar_t *filename, mml_file_type file_type );

LIBREFORMATH_API bool mml_save_to_stream( mml_handle handle, unsigned char **ptr, int *block_size, mml_file_type file_type );

LIBREFORMATH_API double mml_set_scale( mml_handle handle, double xy );

LIBREFORMATH_API double mml_get_scale( mml_handle handle );

LIBREFORMATH_API bool mml_can_render( mml_handle handle );

LIBREFORMATH_API void mml_clear( mml_handle handle );

LIBREFORMATH_API void mml_destroy_stream( unsigned char **ptr );

LIBREFORMATH_API bool mml_get_image_size( mml_handle handle, mml_image_metrics * size );

#ifdef WIN32
#include <Windows.h>

LIBREFORMATH_API bool mml_copy_to_clipboard( mml_handle handle, HWND hwnd );

LIBREFORMATH_API HBITMAP mml_create_bitmap( mml_handle handle, HDC hdc, mml_image_metrics * size );
#endif

#ifdef __cplusplus
}
#endif