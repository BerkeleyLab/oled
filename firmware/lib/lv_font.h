#ifndef LV_FONT_H
#define LV_FONT_H
// MIT licence
// Copyright (c) 2020 LVGL LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define LV_ATTRIBUTE_LARGE_CONST
#define lv_font_get_glyph_dsc_fmt_txt NULL
#define lv_font_get_bitmap_fmt_txt NULL
#define LVGL_VERSION_MAJOR 7
#define LVGL_VERSION_MINOR 0
#define LV_VERSION_CHECK(x,y,z) (x == LVGL_VERSION_MAJOR && (y < LVGL_VERSION_MINOR || (y == LVGL_VERSION_MINOR && z <= LVGL_VERSION_PATCH)))

typedef int16_t lv_coord_t;

/** Describes the properties of a glyph. */
typedef struct {
    uint16_t adv_w; /**< The glyph needs this space. Draw the next glyph after this width. 8 bit integer, 4 bit fractional */
    uint16_t box_w;  /**< Width of the glyph's bounding box*/
    uint16_t box_h;  /**< Height of the glyph's bounding box*/
    int16_t ofs_x;   /**< x offset of the bounding box*/
    int16_t ofs_y;  /**< y offset of the bounding box*/
    uint8_t bpp;   /**< Bit-per-pixel: 1, 2, 4, 8*/
} lv_font_glyph_dsc_t;


/** The bitmaps might be upscaled by 3 to achieve subpixel rendering. */
enum {
    LV_FONT_SUBPX_NONE,
    LV_FONT_SUBPX_HOR,
    LV_FONT_SUBPX_VER,
    LV_FONT_SUBPX_BOTH,
};

typedef uint8_t lv_font_subpx_t;

/** Describe the properties of a font*/
typedef struct _lv_font_struct {
    /** Get a glyph's  descriptor from a font*/
    bool (*get_glyph_dsc)(const struct _lv_font_struct *, lv_font_glyph_dsc_t *, uint32_t letter, uint32_t letter_next);

    /** Get a glyph's bitmap from a font*/
    const uint8_t * (*get_glyph_bitmap)(const struct _lv_font_struct *, uint32_t);

    /*Pointer to the font in a font pack (must have the same line height)*/
    lv_coord_t line_height;         /**< The real line height where any text fits*/
    lv_coord_t base_line;           /**< Base line measured from the top of the line_height*/
    uint8_t subpx  : 2;             /**< An element of `lv_font_subpx_t`*/
    void * dsc;                     /**< Store implementation specific or run_time data or caching here*/
} lv_font_t;


/** This describes a glyph. */
typedef struct {
    uint32_t bitmap_index : 20;     /**< Start index of the bitmap. A font can be max 1 MB. */
    uint32_t adv_w : 12;            /**< Draw the next glyph after this width. 8.4 format (real_value * 16 is stored). */
    uint8_t box_w;                  /**< Width of the glyph's bounding box*/
    uint8_t box_h;                  /**< Height of the glyph's bounding box*/
    int8_t ofs_x;                   /**< x offset of the bounding box*/
    int8_t ofs_y;                  /**< y offset of the bounding box. Measured from the top of the line*/
} lv_font_fmt_txt_glyph_dsc_t;


/** Format of font character map. */
enum {
    LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
    LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL,
    LV_FONT_FMT_TXT_CMAP_SPARSE_TINY,
    LV_FONT_FMT_TXT_CMAP_SPARSE_FULL,
};

typedef uint8_t lv_font_fmt_txt_cmap_type_t;


/* Map codepoints to a `glyph_dsc`s
 * Several formats are supported to optimize memory usage
 * See https://github.com/lvgl/lv_font_conv/blob/master/doc/font_spec.md
 */
typedef struct {
    /** First Unicode character for this range */
    uint32_t range_start;

    /** Number of Unicode characters related to this range.
     * Last Unicode character = range_start + range_length - 1*/
    uint16_t range_length;

    /** First glyph ID (array index of `glyph_dsc`) for this range */
    uint16_t glyph_id_start;

    /*
    According the specification there are 4 formats:
        https://github.com/lvgl/lv_font_conv/blob/master/doc/font_spec.md

    For simplicity introduce "relative code point":
        rcp = codepoint - range_start

    and a search function:
        search a "value" in an "array" and returns the index of "value".

    Format 0 tiny
        unicode_list == NULL && glyph_id_ofs_list == NULL
        glyph_id = glyph_id_start + rcp

    Format 0 full
        unicode_list == NULL && glyph_id_ofs_list != NULL
        glyph_id = glyph_id_start + glyph_id_ofs_list[rcp]

    Sparse tiny
        unicode_list != NULL && glyph_id_ofs_list == NULL
        glyph_id = glyph_id_start + search(unicode_list, rcp)

    Sparse full
        unicode_list != NULL && glyph_id_ofs_list != NULL
        glyph_id = glyph_id_start + glyph_id_ofs_list[search(unicode_list, rcp)]
    */

    const uint16_t * unicode_list;

    /** if(type == LV_FONT_FMT_TXT_CMAP_FORMAT0_...) it's `uint8_t *`
     * if(type == LV_FONT_FMT_TXT_CMAP_SPARSE_...)  it's `uint16_t *`
     */
    const void * glyph_id_ofs_list;

    /** Length of `unicode_list` and/or `glyph_id_ofs_list`*/
    uint16_t list_length;

    /** Type of this character map*/
    lv_font_fmt_txt_cmap_type_t type;
} lv_font_fmt_txt_cmap_t;

/** A simple mapping of kern values from pairs*/
typedef struct {
    /*To get a kern value of two code points:
       1. Get the `glyph_id_left` and `glyph_id_right` from `lv_font_fmt_txt_cmap_t
       2  for(i = 0; i < pair_cnt * 2; i+2)
             if(gylph_ids[i] == glyph_id_left &&
                gylph_ids[i+1] == glyph_id_right)
                 return values[i / 2];
     */
    const void * glyph_ids;
    const int8_t * values;
    uint32_t pair_cnt   : 24;
    uint32_t glyph_ids_size : 2;    /*0: `glyph_ids` is stored as `uint8_t`; 1: as `uint16_t`*/
} lv_font_fmt_txt_kern_pair_t;

/** More complex but more optimal class based kern value storage*/
typedef struct {
    /*To get a kern value of two code points:
          1. Get the `glyph_id_left` and `glyph_id_right` from `lv_font_fmt_txt_cmap_t
          2  Get the class of the left and right glyphs as `left_class` and `right_class`
              left_class = left_class_mapping[glyph_id_left];
              right_class = right_class_mapping[glyph_id_right];
          3. value = class_pair_values[(left_class-1)*right_class_cnt + (right_class-1)]
        */

    const int8_t * class_pair_values;    /*left_class_num * right_class_num value*/
    const uint8_t * left_class_mapping;   /*Map the glyph_ids to classes: index -> glyph_id -> class_id*/
    const uint8_t * right_class_mapping;  /*Map the glyph_ids to classes: index -> glyph_id -> class_id*/
    uint8_t left_class_cnt;
    uint8_t right_class_cnt;
} lv_font_fmt_txt_kern_classes_t;


/** Bitmap formats*/
typedef enum {
    LV_FONT_FMT_TXT_PLAIN      = 0,
    LV_FONT_FMT_TXT_COMPRESSED = 1,
    LV_FONT_FMT_TXT_COMPRESSED_NO_PREFILTER = 1,
} lv_font_fmt_txt_bitmap_format_t;


/*Describe store additional data for fonts */
typedef struct {
    /*The bitmaps of all glyphs*/
    const uint8_t * glyph_bitmap;

    /*Describe the glyphs*/
    const lv_font_fmt_txt_glyph_dsc_t * glyph_dsc;

    /* Map the glyphs to Unicode characters.
     * Array of `lv_font_cmap_fmt_txt_t` variables*/
    const lv_font_fmt_txt_cmap_t * cmaps;

    /* Store kerning values.
     * Can be  `lv_font_fmt_txt_kern_pair_t *  or `lv_font_kern_classes_fmt_txt_t *`
     * depending on `kern_classes`
     */
    const void * kern_dsc;

    /*Scale kern values in 12.4 format*/
    uint16_t kern_scale;

    /*Number of cmap tables*/
    uint16_t cmap_num       : 10;

    /*Bit per pixel: 1, 2, 3, 4, 8*/
    uint16_t bpp            : 4;

    /*Type of `kern_dsc`*/
    uint16_t kern_classes   : 1;

    /*
     * storage format of the bitmap
     * from `lv_font_fmt_txt_bitmap_format_t`
     */
    uint16_t bitmap_format  : 2;

    /*Cache the last letter and is glyph id*/
    uint32_t last_letter;
    uint32_t last_glyph_id;

} lv_font_fmt_txt_dsc_t;


/**
 * Used as `get_glyph_dsc` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font_p pointer to font
 * @param dsc_out store the result descriptor here
 * @param letter an UNICODE letter code
 * @return true: descriptor is successfully loaded into `dsc_out`.
 *         false: the letter was not found, no data is loaded to `dsc_out`
 */
bool get_glyph_dsc(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter);

/**
 * Used as `get_glyph_bitmap` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font pointer to font
 * @param unicode_letter an unicode letter which bitmap should be get
 * @return pointer to the bitmap or NULL if not found
 */
const uint8_t * get_glyph_bitmap(const lv_font_t * font, uint32_t unicode_letter);


//-----------------------------------------------
// public functions
//-----------------------------------------------
void set_font(lv_font_t *f);
void set_cursor(int x, int y);
void draw_char(char c);  // draw 1 or less UTF8 character, adv. cursor
void get_bb(const char *txt, int *w, int *h);  // get bounding box
void set_bb(int x0, int x1, int y0, int y1);
void reset_bb(void);

#endif
