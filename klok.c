/*
 * vim:ts=4:sw=4:expandtab
 *
 * © 2016 Boris Faure
 *
 * See LICENSE for licensing information
 *
 */


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <xcb/xcb.h>
#include <ev.h>
#include <cairo.h>
#include <cairo/cairo-xcb.h>

#include "i3lock.h"
#include "unlock_indicator.h"
#include "klok.h"

extern bool debug_mode;
extern struct ev_loop *main_loop;

static double font_size;

static struct color {
    double r;
    double g;
    double b;
    double a;
} on, off, shadow;


// 11x10 (WxH)
struct letter {
    const char *letter;
    bool is_on;
};

#define NB_WIDTH 11
#define NB_HEIGHT 10
static struct letter letters[NB_HEIGHT][NB_WIDTH] = {};


static void
klok_init(cairo_t *cairo)
{
    cairo_select_font_face(cairo,
                           "monospace",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);

    on.r = 0.796;
    on.g = 0.294;
    on.b = 0.086;
    on.a = 0.9;
    off.r = 0;
    off.g = 0;
    off.b = 0;
    off.a = 0.9;
    shadow.r = 0;
    shadow.g = 0;
    shadow.b = 0;
    shadow.a = 0.7;

    letters[0][0].letter = "I";
    letters[0][1].letter = "T";
    letters[0][2].letter = "L"; // RANDOM
    letters[0][3].letter = "I";
    letters[0][4].letter = "S";
    letters[0][5].letter = "A"; // RANDOM
    letters[0][6].letter = "S"; // RANDOM
    letters[0][7].letter = "T"; // RANDOM
    letters[0][8].letter = "I"; // RANDOM
    letters[0][9].letter = "M"; // RANDOM
    letters[0][10].letter = "E"; // RANDOM

    letters[1][0].letter = "A";
    letters[1][1].letter = "C"; // RANDOM
    letters[1][2].letter = "Q";
    letters[1][3].letter = "U";
    letters[1][4].letter = "A";
    letters[1][5].letter = "R";
    letters[1][6].letter = "T";
    letters[1][7].letter = "E";
    letters[1][8].letter = "R";
    letters[1][9].letter = "D"; // RANDOM
    letters[1][10].letter = "C"; // RANDOM

    letters[2][0].letter = "T";
    letters[2][1].letter = "W";
    letters[2][2].letter = "E";
    letters[2][3].letter = "N";
    letters[2][4].letter = "T";
    letters[2][5].letter = "Y";
    letters[2][6].letter = "F";
    letters[2][7].letter = "I";
    letters[2][8].letter = "V";
    letters[2][9].letter = "E";
    letters[2][10].letter = "X"; // RANDOM

    letters[3][0].letter = "H";
    letters[3][1].letter = "A";
    letters[3][2].letter = "L";
    letters[3][3].letter = "F";
    letters[3][4].letter = "B"; // RANDOM
    letters[3][5].letter = "T";
    letters[3][6].letter = "E";
    letters[3][7].letter = "N";
    letters[3][8].letter = "F"; // RANDOM
    letters[3][9].letter = "T";
    letters[3][10].letter = "O";

    letters[4][0].letter = "P";
    letters[4][1].letter = "A";
    letters[4][2].letter = "S";
    letters[4][3].letter = "T";
    letters[4][4].letter = "E"; // RANDOM
    letters[4][5].letter = "R"; // RANDOM
    letters[4][6].letter = "U"; // RANDOM
    letters[4][7].letter = "N";
    letters[4][8].letter = "I";
    letters[4][9].letter = "N";
    letters[4][10].letter = "E";

    letters[5][0].letter = "O";
    letters[5][1].letter = "N";
    letters[5][2].letter = "E";
    letters[5][3].letter = "S";
    letters[5][4].letter = "I";
    letters[5][5].letter = "X";
    letters[5][6].letter = "T";
    letters[5][7].letter = "H";
    letters[5][8].letter = "R";
    letters[5][9].letter = "E";
    letters[5][10].letter = "E";

    letters[6][0].letter = "F";
    letters[6][1].letter = "O";
    letters[6][2].letter = "U";
    letters[6][3].letter = "R";
    letters[6][4].letter = "F";
    letters[6][5].letter = "I";
    letters[6][6].letter = "V";
    letters[6][7].letter = "E";
    letters[6][8].letter = "T";
    letters[6][9].letter = "W";
    letters[6][10].letter = "O";

    letters[7][0].letter = "E";
    letters[7][1].letter = "I";
    letters[7][2].letter = "G";
    letters[7][3].letter = "H";
    letters[7][4].letter = "T";
    letters[7][5].letter = "E";
    letters[7][6].letter = "L";
    letters[7][7].letter = "E";
    letters[7][8].letter = "V";
    letters[7][9].letter = "E";
    letters[7][10].letter = "N";

    letters[8][0].letter = "S";
    letters[8][1].letter = "E";
    letters[8][2].letter = "V";
    letters[8][3].letter = "E";
    letters[8][4].letter = "N";
    letters[8][5].letter = "T";
    letters[8][6].letter = "W";
    letters[8][7].letter = "E";
    letters[8][8].letter = "L";
    letters[8][9].letter = "V";
    letters[8][10].letter = "E";

    letters[9][0].letter = "T";
    letters[9][1].letter = "E";
    letters[9][2].letter = "N";
    letters[9][3].letter = "S"; // RANDOM
    letters[9][4].letter = "E"; // RANDOM
    letters[9][5].letter = "O";
    letters[9][6].letter = "C";
    letters[9][7].letter = "L";
    letters[9][8].letter = "O";
    letters[9][9].letter = "C";
    letters[9][10].letter = "K";
}

static void
switch_letters_on(void)
{
    struct tm tm;
    time_t t = time(NULL);
    int x, y;

    localtime_r(&t, &tm);

    /* Clear previous run */
    for (y = 0; y < NB_HEIGHT; y++) {
        for (x = 0; x < NB_WIDTH; x++) {
            letters[y][x].is_on = false;
        }
    }
    /* IT IS */
    letters[0][0].is_on = true;
    letters[0][1].is_on = true;
    letters[0][3].is_on = true;
    letters[0][4].is_on = true;

    /* A QUARTER */
    if ((15 <= tm.tm_min && tm.tm_min < 20)
        ||  (45 <= tm.tm_min && tm.tm_min < 50))
    {
        letters[1][0].is_on = true;
        letters[1][2].is_on = true;
        letters[1][3].is_on = true;
        letters[1][4].is_on = true;
        letters[1][5].is_on = true;
        letters[1][6].is_on = true;
        letters[1][7].is_on = true;
        letters[1][8].is_on = true;
    }
    /* TWENTY */
    if ((20 <= tm.tm_min && tm.tm_min < 30)
        ||  (35 <= tm.tm_min && tm.tm_min < 45))
    {
        letters[2][0].is_on = true;
        letters[2][1].is_on = true;
        letters[2][2].is_on = true;
        letters[2][3].is_on = true;
        letters[2][4].is_on = true;
        letters[2][5].is_on = true;
    }

    /* FIVE */
    if ((5 <= tm.tm_min && tm.tm_min < 10)
        ||  (25 <= tm.tm_min && tm.tm_min < 30)
        ||  (35 <= tm.tm_min && tm.tm_min < 40)
        ||  (55 <= tm.tm_min && tm.tm_min < 60))
    {
        letters[2][6].is_on = true;
        letters[2][7].is_on = true;
        letters[2][8].is_on = true;
        letters[2][9].is_on = true;
    }

    /* HALF */
    if (30 <= tm.tm_min && tm.tm_min < 35) {
        letters[3][0].is_on = true;
        letters[3][1].is_on = true;
        letters[3][2].is_on = true;
        letters[3][3].is_on = true;
    }

    /* TEN */
    if ((10 <= tm.tm_min && tm.tm_min < 15)
        ||  (50 <= tm.tm_min && tm.tm_min < 55))
    {
        letters[3][5].is_on = true;
        letters[3][6].is_on = true;
        letters[3][7].is_on = true;
    }

    /* TO */
    if (35 <= tm.tm_min && tm.tm_min < 60) {
        letters[3][9].is_on = true;
        letters[3][10].is_on = true;
    }

    /* PAST */
    if (5 <= tm.tm_min && tm.tm_min < 35) {
        letters[4][0].is_on = true;
        letters[4][1].is_on = true;
        letters[4][2].is_on = true;
        letters[4][3].is_on = true;
    }

    /* NINE */
    if (((tm.tm_hour == 8 || tm.tm_hour == 20) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 9 || tm.tm_hour == 21) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[4][7].is_on = true;
        letters[4][8].is_on = true;
        letters[4][9].is_on = true;
        letters[4][10].is_on = true;
    }

    /* ONE */
    if (((tm.tm_hour == 0 || tm.tm_hour == 12) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 1 || tm.tm_hour == 13) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[5][0].is_on = true;
        letters[5][1].is_on = true;
        letters[5][2].is_on = true;
    }

    /* SIX */
    if (((tm.tm_hour == 5 || tm.tm_hour == 17) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 6 || tm.tm_hour == 18) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[5][3].is_on = true;
        letters[5][4].is_on = true;
        letters[5][5].is_on = true;
    }

    /* THREE */
    if (((tm.tm_hour == 2 || tm.tm_hour == 14) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 3 || tm.tm_hour == 15) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[5][6].is_on = true;
        letters[5][7].is_on = true;
        letters[5][8].is_on = true;
        letters[5][9].is_on = true;
        letters[5][10].is_on = true;
    }

    /* FOUR */
    if (((tm.tm_hour == 3 || tm.tm_hour == 15) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 4 || tm.tm_hour == 16) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[6][0].is_on = true;
        letters[6][1].is_on = true;
        letters[6][2].is_on = true;
        letters[6][3].is_on = true;
    }

    /* FIVE */
    if (((tm.tm_hour == 4 || tm.tm_hour == 16) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 5 || tm.tm_hour == 17) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[6][4].is_on = true;
        letters[6][5].is_on = true;
        letters[6][6].is_on = true;
        letters[6][7].is_on = true;
    }

    /* TWO */
    if (((tm.tm_hour == 1 || tm.tm_hour == 13) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 2 || tm.tm_hour == 14) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[6][8].is_on = true;
        letters[6][9].is_on = true;
        letters[6][10].is_on = true;
    }

    /* EIGHT */
    if (((tm.tm_hour == 7 || tm.tm_hour == 19) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 8 || tm.tm_hour == 20) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[7][0].is_on = true;
        letters[7][1].is_on = true;
        letters[7][2].is_on = true;
        letters[7][3].is_on = true;
        letters[7][4].is_on = true;
    }

    /* ELEVEN */
    if (((tm.tm_hour == 10 || tm.tm_hour == 22) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 11 || tm.tm_hour == 23) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[7][5].is_on = true;
        letters[7][6].is_on = true;
        letters[7][7].is_on = true;
        letters[7][8].is_on = true;
        letters[7][9].is_on = true;
        letters[7][10].is_on = true;
    }

    /* SEVEN */
    if (((tm.tm_hour == 6 || tm.tm_hour == 18) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 7 || tm.tm_hour == 19) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[8][0].is_on = true;
        letters[8][1].is_on = true;
        letters[8][2].is_on = true;
        letters[8][3].is_on = true;
        letters[8][4].is_on = true;
    }

    /* TWELVE */
    if (((tm.tm_hour == 11 || tm.tm_hour == 23) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 0 || tm.tm_hour == 12) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[8][5].is_on = true;
        letters[8][6].is_on = true;
        letters[8][7].is_on = true;
        letters[8][8].is_on = true;
        letters[8][9].is_on = true;
        letters[8][10].is_on = true;
    }

    /* TEN */
    if (((tm.tm_hour == 9 || tm.tm_hour == 21) &&
         (tm.tm_min >= 35 ))
        || ((tm.tm_hour == 10 || tm.tm_hour == 22) &&
            (0 <= tm.tm_min && tm.tm_min < 35 )))
    {
        letters[9][0].is_on = true;
        letters[9][1].is_on = true;
        letters[9][2].is_on = true;
    }

    /* OCLOCK */
    if (0 <= tm.tm_min && tm.tm_min < 5) {
        letters[9][5].is_on = true;
        letters[9][6].is_on = true;
        letters[9][7].is_on = true;
        letters[9][8].is_on = true;
        letters[9][9].is_on = true;
        letters[9][10].is_on = true;
    }
}

static void
find_best_text_size(cairo_t *cr,
                    uint32_t max_letter_width,
                    uint32_t max_letter_height,
                    cairo_text_extents_t *exts)
{
    double size = 10.0;
    cairo_scaled_font_t *sf;

    while (size < 200) {
        cairo_text_extents_t extents;
        cairo_set_font_size(cr, size);
        sf = cairo_get_scaled_font(cr);
        cairo_scaled_font_text_extents(sf, "W", &extents);
        DEBUG("size:%f width:%f height:%f\n",
              size, extents.width, extents.height);
        if (extents.width > max_letter_width ||
            extents.height > max_letter_height) {
            size -= 1.0;
            break;
        }

        size += 1.0;
    }
    DEBUG("final size: %f\n", size);
    cairo_set_font_size(cr, size);
    sf = cairo_get_scaled_font(cr);
    cairo_scaled_font_text_extents(sf, "W", exts);
    font_size = size;
}

static void
draw_letter(cairo_t *cr,
            struct letter *letter,
            uint32_t off_x,
            uint32_t off_y)
{
    cairo_move_to(cr, off_x, off_y);
    cairo_text_path(cr, letter->letter);
    if (letter->is_on) {
        cairo_set_source_rgba(cr, on.r, on.g, on.b, on.a);
    } else {
        cairo_set_source_rgba(cr, off.r, off.g, off.g, off.a);
    }
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, shadow.r, shadow.g, shadow.b, shadow.a);
    cairo_set_line_width(cr, font_size / 25.0);
    cairo_stroke(cr);
}

static void
draw_letters(cairo_t *cr,
             uint32_t resolution_w,
             uint32_t resolution_h)
{
    uint32_t sq = resolution_w;
    uint32_t max_letter_width, max_letter_height;
    cairo_text_extents_t extents;
    uint32_t orig_x_offset, orig_y_offset;
    uint32_t dx, dy;
    int x, y;

    sq = resolution_h;
    if (resolution_w < sq)
        sq = resolution_w;

    sq *= 0.9;

    orig_y_offset = (resolution_h - sq) / 2;
    orig_x_offset = (resolution_w - sq) / 2;
    DEBUG("sq=%d orig_y_offset=%d orig_x_offset=%d\n",
          sq, orig_y_offset, orig_x_offset);


    max_letter_width = 8 * sq / (10 * 11);
    max_letter_height= 8 * sq / (10 * 10);

    DEBUG("max_letter_width=%d max_letter_height=%d\n",
          max_letter_width, max_letter_height);

    find_best_text_size(cr, max_letter_width, max_letter_height,
                        &extents);
    dx = (sq - NB_WIDTH * extents.width) / (NB_WIDTH - 1);
    dy = (sq - NB_HEIGHT * extents.height) / (NB_HEIGHT - 1);
    DEBUG("dx=%d dy=%d\n", dx, dy);
    for (y = 0; y < NB_HEIGHT; y++) {
        for (x = 0; x < NB_WIDTH; x++) {
            uint32_t off_x, off_y;

            off_x = dx * x + x * extents.width + orig_x_offset;
            off_y = dy * y + y * extents.height + orig_y_offset + extents.height;
            draw_letter(cr, &letters[y][x], off_x, off_y);
        }
    }
}

void
draw_klok(cairo_t *cairo,
          uint32_t resolution_w,
          uint32_t resolution_h)
{
    DEBUG("resolution_w:%d resolution_h:%d\n",
          resolution_w, resolution_h);

    klok_init(cairo);
    switch_letters_on();
    draw_letters(cairo, resolution_w, resolution_h);
}

static void
time_change(struct ev_loop *loop, ev_timer *w, int revents)
{
    redraw_screen();
}

void
klok_add_timer(void)
{
    ev_timer *timer;
    struct tm tm;
    time_t t = time(NULL);

    localtime_r(&t, &tm);
    timer = calloc(sizeof(struct ev_timer), 1);

    ev_timer_init(timer, time_change,
                  5 * 60 - ((tm.tm_min % 5) * 60 + tm.tm_sec),
                  5 * 60);
    ev_timer_start(main_loop, timer);

}
