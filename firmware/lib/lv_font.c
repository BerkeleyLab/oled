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

// This is the essence of the LVGL bitmap-font engine.
// Code is mostly verbatim copied from LVGL and stripped down:
//   * no compression (not effective for small sizes anyway)
//   * only 4 bpp

#include <stdio.h>
#include "lv_font.h"
#include "frame_buffer.h"

/** Searches base[0] to base[n - 1] for an item that matches *key.
 *
 * @note The function cmp must return negative if its first
 *  argument (the search key) is less that its second (a table entry),
 *  zero if equal, and positive if greater.
 *
 *  @note Items in the array must be in ascending order.
 *
 * @param key    Pointer to item being searched for
 * @param base   Pointer to first element to search
 * @param n      Number of elements
 * @param size   Size of each element
 * @param cmp    Pointer to comparison function (see #lv_font_codeCompare as a comparison function
 * example)
 *
 * @return a pointer to a matching item, or NULL if none exists.
 */
static void * _lv_utils_bsearch(const void * key, const void * base, uint32_t n, uint32_t size,
                         int32_t (*cmp)(const void * pRef, const void * pElement))
{
    const char * middle;
    int32_t c;

    for(middle = base; n != 0;) {
        middle += (n / 2) * size;
        if((c = (*cmp)(key, middle)) > 0) {
            n    = (n / 2) - ((n & 1) == 0);
            base = (middle += size);
        }
        else if(c < 0) {
            n /= 2;
            middle = base;
        }
        else {
            return (char *)middle;
        }
    }
    return NULL;
}

/** Code Comparator.
 *
 *  Compares the value of both input arguments.
 *
 *  @param[in]  pRef        Pointer to the reference.
 *  @param[in]  pElement    Pointer to the element to compare.
 *
 *  @return Result of comparison.
 *  @retval < 0   Reference is greater than element.
 *  @retval = 0   Reference is equal to element.
 *  @retval > 0   Reference is less than element.
 *
 */
static int32_t unicode_list_compare(const void * ref, const void * element)
{
    return ((int32_t)(*(uint16_t *)ref)) - ((int32_t)(*(uint16_t *)element));
}

static uint32_t get_glyph_dsc_id(const lv_font_t * font, uint32_t letter)
{
    if(letter == '\0') return 0;

    lv_font_fmt_txt_dsc_t * fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;

    /*Check the cache first*/
    if(letter == fdsc->last_letter) return fdsc->last_glyph_id;

    uint16_t i;
    for(i = 0; i < fdsc->cmap_num; i++) {

        /*Relative code point*/
        uint32_t rcp = letter - fdsc->cmaps[i].range_start;
        if(rcp > fdsc->cmaps[i].range_length) continue;
        uint32_t glyph_id = 0;
        if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY) {
            glyph_id = fdsc->cmaps[i].glyph_id_start + rcp;
        }
        else if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL) {
            const uint8_t * gid_ofs_8 = fdsc->cmaps[i].glyph_id_ofs_list;
            glyph_id = fdsc->cmaps[i].glyph_id_start + gid_ofs_8[rcp];
        }
        else if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_SPARSE_TINY) {
            uint16_t key = rcp;
            uint8_t * p = _lv_utils_bsearch(&key, fdsc->cmaps[i].unicode_list, fdsc->cmaps[i].list_length,
                                            sizeof(fdsc->cmaps[i].unicode_list[0]), unicode_list_compare);

            if(p) {
                uintptr_t ofs = (uintptr_t)(p - (uint8_t *) fdsc->cmaps[i].unicode_list);
                ofs = ofs >> 1;     /*The list stores `uint16_t` so the get the index divide by 2*/
                glyph_id = fdsc->cmaps[i].glyph_id_start + ofs;
            }
        }
        else if(fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_SPARSE_FULL) {
            uint16_t key = rcp;
            uint8_t * p = _lv_utils_bsearch(&key, fdsc->cmaps[i].unicode_list, fdsc->cmaps[i].list_length,
                                            sizeof(fdsc->cmaps[i].unicode_list[0]), unicode_list_compare);

            if(p) {
                uintptr_t ofs = (uintptr_t)(p - (uint8_t *) fdsc->cmaps[i].unicode_list);
                ofs = ofs >> 1;     /*The list stores `uint16_t` so the get the index divide by 2*/
                const uint8_t * gid_ofs_16 = fdsc->cmaps[i].glyph_id_ofs_list;
                glyph_id = fdsc->cmaps[i].glyph_id_start + gid_ofs_16[ofs];
            }
        }

        /*Update the cache*/
        fdsc->last_letter = letter;
        fdsc->last_glyph_id = glyph_id;
        return glyph_id;
    }

    fdsc->last_letter = letter;
    fdsc->last_glyph_id = 0;
    return 0;
}

/**
 * Used as `get_glyph_bitmap` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font pointer to font
 * @param unicode_letter an unicode letter which bitmap should be get
 * @return pointer to the bitmap or NULL if not found
 */
const uint8_t * get_glyph_bitmap(const lv_font_t * font, uint32_t unicode_letter)
{
    if(unicode_letter == '\t') unicode_letter = ' ';

    lv_font_fmt_txt_dsc_t * fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;
    uint32_t gid = get_glyph_dsc_id(font, unicode_letter);
    if(!gid) return NULL;

    const lv_font_fmt_txt_glyph_dsc_t * gdsc = &fdsc->glyph_dsc[gid];

    if(fdsc->bitmap_format == LV_FONT_FMT_TXT_PLAIN) {
        if(gdsc) return &fdsc->glyph_bitmap[gdsc->bitmap_index];
    }
    /*Don't Handle compressed bitmap*/
    else {
        return NULL;
    }

    /*If not returned earlier then the letter is not found in this font*/
    return NULL;
}

/**
 * Used as `get_glyph_dsc` callback in LittelvGL's native font format if the font is uncompressed.
 * @param font_p pointer to font
 * @param dsc_out store the result descriptor here
 * @param letter an UNICODE letter code
 * @return true: descriptor is successfully loaded into `dsc_out`.
 *         false: the letter was not found, no data is loaded to `dsc_out`
 */
bool get_glyph_dsc(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter)
{
    bool is_tab = false;
    if(unicode_letter == '\t') {
        unicode_letter = ' ';
        is_tab = true;
    }
    lv_font_fmt_txt_dsc_t * fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;
    uint32_t gid = get_glyph_dsc_id(font, unicode_letter);
    if(!gid) return false;

    /*Put together a glyph dsc*/
    const lv_font_fmt_txt_glyph_dsc_t * gdsc = &fdsc->glyph_dsc[gid];

    // int32_t kv = ((int32_t)((int32_t)kvalue * fdsc->kern_scale) >> 4);

    uint32_t adv_w = gdsc->adv_w;
    if(is_tab) adv_w *= 2;

    // adv_w += kv;
    adv_w  = (adv_w + (1 << 3)) >> 4;

    dsc_out->adv_w = adv_w;
    dsc_out->box_h = gdsc->box_h;
    dsc_out->box_w = gdsc->box_w;
    dsc_out->ofs_x = gdsc->ofs_x;
    dsc_out->ofs_y = gdsc->ofs_y;
    dsc_out->bpp   = (uint8_t)fdsc->bpp;

    if(is_tab) dsc_out->box_w = dsc_out->box_w * 2;

    return true;
}

/**
 * Decode an UTF-8 character from a string.
 * @param txt pointer to '\0' terminated string
 * @param i start byte index in 'txt' where to start.
 *          After call it will point to the next UTF-8 char in 'txt'.
 *          NULL to use txt[0] as index
 * @return the decoded Unicode character or 0 on invalid UTF-8 code
 */
static uint32_t lv_txt_utf8_next(const char * txt, uint32_t * i)
{
    /* Unicode to UTF-8
     * 00000000 00000000 00000000 0xxxxxxx -> 0xxxxxxx
     * 00000000 00000000 00000yyy yyxxxxxx -> 110yyyyy 10xxxxxx
     * 00000000 00000000 zzzzyyyy yyxxxxxx -> 1110zzzz 10yyyyyy 10xxxxxx
     * 00000000 000wwwzz zzzzyyyy yyxxxxxx -> 11110www 10zzzzzz 10yyyyyy 10xxxxxx
     * */

    uint32_t result = 0;

    /*Dummy 'i' pointer is required*/
    uint32_t i_tmp = 0;
    if(i == NULL) i = &i_tmp;

    /*Normal ASCII*/
    if((txt[*i] & 0x80) == 0) {
        result = txt[*i];
        (*i)++;
    }
    /*Real UTF-8 decode*/
    else {
        /*2 bytes UTF-8 code*/
        if((txt[*i] & 0xE0) == 0xC0) {
            result = (uint32_t)(txt[*i] & 0x1F) << 6;
            (*i)++;
            if((txt[*i] & 0xC0) != 0x80) return 0; /*Invalid UTF-8 code*/
            result += (txt[*i] & 0x3F);
            (*i)++;
        }
        /*3 bytes UTF-8 code*/
        else if((txt[*i] & 0xF0) == 0xE0) {
            result = (uint32_t)(txt[*i] & 0x0F) << 12;
            (*i)++;

            if((txt[*i] & 0xC0) != 0x80) return 0; /*Invalid UTF-8 code*/
            result += (uint32_t)(txt[*i] & 0x3F) << 6;
            (*i)++;

            if((txt[*i] & 0xC0) != 0x80) return 0; /*Invalid UTF-8 code*/
            result += (txt[*i] & 0x3F);
            (*i)++;
        }
        /*4 bytes UTF-8 code*/
        else if((txt[*i] & 0xF8) == 0xF0) {
            result = (uint32_t)(txt[*i] & 0x07) << 18;
            (*i)++;

            if((txt[*i] & 0xC0) != 0x80) return 0; /*Invalid UTF-8 code*/
            result += (uint32_t)(txt[*i] & 0x3F) << 12;
            (*i)++;

            if((txt[*i] & 0xC0) != 0x80) return 0; /*Invalid UTF-8 code*/
            result += (uint32_t)(txt[*i] & 0x3F) << 6;
            (*i)++;

            if((txt[*i] & 0xC0) != 0x80) return 0; /*Invalid UTF-8 code*/
            result += txt[*i] & 0x3F;
            (*i)++;
        }
        else {
            (*i)++; /*Not UTF-8 char. Go the next.*/
        }
    }
    return result;
}

const lv_font_t *cur_font;

// 4bpp fonts only
static void draw_glyph4(const uint8_t *bmp, unsigned x, unsigned y, unsigned w, unsigned h)
{
    int nBitsLoaded = 0;
    unsigned bits;

    y -= h;

    for (unsigned row=0; row<h; row++) {
        for (unsigned col=0; col<w; col++) {
            if (nBitsLoaded < 4) {
                bits = *bmp++;
                nBitsLoaded += 8;
            }
            addPixel(col + x, row + y, bits >> 4);
            bits <<= 4;
            nBitsLoaded -= 4;
        }
    }
}

// get (approximate) bounding box (width and height) of the rendered text
void get_bb(const char *txt, int *w, int *h)
{
    int x=0;
    uint32_t c_ind = 0;
    unsigned dc = lv_txt_utf8_next(txt, &c_ind);
    lv_font_glyph_dsc_t g;
    while (dc) {
        get_glyph_dsc(cur_font, &g, dc);
        x += g.adv_w;
        dc = lv_txt_utf8_next(txt, &c_ind);
    }
    if (w)
        *w = x;
    if (h)
        *h = cur_font->line_height;
}


void draw_str(const char *txt, int x, int y)
{
    int x_start = x;
    uint32_t c_ind = 0;
    unsigned dc = lv_txt_utf8_next(txt, &c_ind);
    y += cur_font->line_height - cur_font->base_line;
    lv_font_glyph_dsc_t g;
    while (dc) {
        if (dc == '\n' || dc == '\r')
            x = x_start;
        if (dc == '\n') {
            y += cur_font->line_height;
            dc = lv_txt_utf8_next(txt, &c_ind);
            continue;
        }
        get_glyph_dsc(cur_font, &g, dc);
        // Don't draw empty characters
        if (g.box_h > 0 && g.box_w > 0) {
            const uint8_t *bmp = get_glyph_bitmap(cur_font, dc);
            draw_glyph4(bmp, x + g.ofs_x, y - g.ofs_y, g.box_w, g.box_h);
        }
        x += g.adv_w;
        dc = lv_txt_utf8_next(txt, &c_ind);
    }
}

void set_font(lv_font_t *f)
{
    if (f)
        cur_font = f;
}

// void print_font(void)
// {
//     lv_font_glyph_dsc_t g;

//     // dmonstrate looking up glyphs
//     for (unsigned c=0x20; c<=0x80; c++) {
//         printf("%1c (%02x): ", (char)c, (unsigned)c);
//         if (get_glyph_dsc(cur_font, &g, c, 0)) {
//             printf("adv_w: %2d ", g.adv_w); // The glyph needs this space. Draw the next glyph after this width. 8 bit integer, 4 bit fractional */
//             printf("box_w: %2d ", g.box_w);  // Width of the glyph's bounding box
//             printf("box_h: %2d ", g.box_h);  // Height of the glyph's bounding box*/
//             printf("ofs_x: %2d ", g.ofs_x);   // x offset of the bounding box*/
//             printf("ofs_y: %2d ", g.ofs_y);  // y offset of the bounding box*/
//             printf("bpp: %2d\n", g.bpp);   // Bit-per-pixel: 1, 2, 4, 8*/
//         } else {
//             printf("letter not found\n");
//         }
//     }
// }
