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

char color_on[9] = "cb4b16b2";
char color_off[9] = "0000007f";
char color_shadow[9] = "0000007f";

char *klok_font = "monospace";

// 11x10 (WxH)
struct letter {
    char letter[2];
    bool is_on;
};

#define NB_WIDTH 11
#define NB_HEIGHT 10
static struct letter letters[NB_HEIGHT][NB_WIDTH] = {};

static int
char_to_int(const char c)
{
    int i;
    i = c - '0';
    if (i > 16)
        i -= 'a' - 'A';
    return i;
}

static double
hex_string_to_double(const char *str)
{
    int r;
    r = char_to_int(str[0]) * 16 + char_to_int(str[1]);
    return r / 255.0;
}

static void
string_to_color(const char *color, struct color *c)
{
    size_t len = strlen(color);

    c->r = hex_string_to_double(color);
    c->g = hex_string_to_double(color+2);
    c->b = hex_string_to_double(color+4);

    if (len == 6)
        c->a = 1.0;
    else
        c->a = hex_string_to_double(color+6);
}

static char
random_letter(void)
{
    return 'A' + rand() % ('Z'-'A');
}

static void
klok_init(cairo_t *cairo)
{
    static bool letters_inited = false;

    cairo_select_font_face(cairo,
                           klok_font,
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);

    if (letters_inited)
        return;

    string_to_color(color_on, &on);
    string_to_color(color_off, &off);
    string_to_color(color_shadow, &shadow);

    srand(time(NULL));

    letters[0][0].letter[0] = 'I';
    letters[0][1].letter[0] = 'T';
    letters[0][2].letter[0] = random_letter();
    letters[0][3].letter[0] = 'I';
    letters[0][4].letter[0] = 'S';
    letters[0][5].letter[0] = random_letter();
    letters[0][6].letter[0] = random_letter();
    letters[0][7].letter[0] = random_letter();
    letters[0][8].letter[0] = random_letter();
    letters[0][9].letter[0] = random_letter();
    letters[0][10].letter[0] = random_letter();

    letters[1][0].letter[0] = 'A';
    letters[1][1].letter[0] = random_letter();
    letters[1][2].letter[0] = 'Q';
    letters[1][3].letter[0] = 'U';
    letters[1][4].letter[0] = 'A';
    letters[1][5].letter[0] = 'R';
    letters[1][6].letter[0] = 'T';
    letters[1][7].letter[0] = 'E';
    letters[1][8].letter[0] = 'R';
    letters[1][9].letter[0] = random_letter();
    letters[1][10].letter[0] = random_letter();

    letters[2][0].letter[0] = 'T';
    letters[2][1].letter[0] = 'W';
    letters[2][2].letter[0] = 'E';
    letters[2][3].letter[0] = 'N';
    letters[2][4].letter[0] = 'T';
    letters[2][5].letter[0] = 'Y';
    letters[2][6].letter[0] = 'F';
    letters[2][7].letter[0] = 'I';
    letters[2][8].letter[0] = 'V';
    letters[2][9].letter[0] = 'E';
    letters[2][10].letter[0] = random_letter();

    letters[3][0].letter[0] = 'H';
    letters[3][1].letter[0] = 'A';
    letters[3][2].letter[0] = 'L';
    letters[3][3].letter[0] = 'F';
    letters[3][4].letter[0] = random_letter();
    letters[3][5].letter[0] = 'T';
    letters[3][6].letter[0] = 'E';
    letters[3][7].letter[0] = 'N';
    letters[3][8].letter[0] = random_letter();
    letters[3][9].letter[0] = 'T';
    letters[3][10].letter[0] = 'O';

    letters[4][0].letter[0] = 'P';
    letters[4][1].letter[0] = 'A';
    letters[4][2].letter[0] = 'S';
    letters[4][3].letter[0] = 'T';
    letters[4][4].letter[0] = random_letter();
    letters[4][5].letter[0] = random_letter();
    letters[4][6].letter[0] = random_letter();
    letters[4][7].letter[0] = 'N';
    letters[4][8].letter[0] = 'I';
    letters[4][9].letter[0] = 'N';
    letters[4][10].letter[0] = 'E';

    letters[5][0].letter[0] = 'O';
    letters[5][1].letter[0] = 'N';
    letters[5][2].letter[0] = 'E';
    letters[5][3].letter[0] = 'S';
    letters[5][4].letter[0] = 'I';
    letters[5][5].letter[0] = 'X';
    letters[5][6].letter[0] = 'T';
    letters[5][7].letter[0] = 'H';
    letters[5][8].letter[0] = 'R';
    letters[5][9].letter[0] = 'E';
    letters[5][10].letter[0] = 'E';

    letters[6][0].letter[0] = 'F';
    letters[6][1].letter[0] = 'O';
    letters[6][2].letter[0] = 'U';
    letters[6][3].letter[0] = 'R';
    letters[6][4].letter[0] = 'F';
    letters[6][5].letter[0] = 'I';
    letters[6][6].letter[0] = 'V';
    letters[6][7].letter[0] = 'E';
    letters[6][8].letter[0] = 'T';
    letters[6][9].letter[0] = 'W';
    letters[6][10].letter[0] = 'O';

    letters[7][0].letter[0] = 'E';
    letters[7][1].letter[0] = 'I';
    letters[7][2].letter[0] = 'G';
    letters[7][3].letter[0] = 'H';
    letters[7][4].letter[0] = 'T';
    letters[7][5].letter[0] = 'E';
    letters[7][6].letter[0] = 'L';
    letters[7][7].letter[0] = 'E';
    letters[7][8].letter[0] = 'V';
    letters[7][9].letter[0] = 'E';
    letters[7][10].letter[0] = 'N';

    letters[8][0].letter[0] = 'S';
    letters[8][1].letter[0] = 'E';
    letters[8][2].letter[0] = 'V';
    letters[8][3].letter[0] = 'E';
    letters[8][4].letter[0] = 'N';
    letters[8][5].letter[0] = 'T';
    letters[8][6].letter[0] = 'W';
    letters[8][7].letter[0] = 'E';
    letters[8][8].letter[0] = 'L';
    letters[8][9].letter[0] = 'V';
    letters[8][10].letter[0] = 'E';

    letters[9][0].letter[0] = 'T';
    letters[9][1].letter[0] = 'E';
    letters[9][2].letter[0] = 'N';
    letters[9][3].letter[0] = random_letter();
    letters[9][4].letter[0] = random_letter();
    letters[9][5].letter[0] = 'O';
    letters[9][6].letter[0] = 'C';
    letters[9][7].letter[0] = 'L';
    letters[9][8].letter[0] = 'O';
    letters[9][9].letter[0] = 'C';
    letters[9][10].letter[0] = 'K';

    letters_inited = true;
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
        if (extents.width > max_letter_width ||
            extents.height > max_letter_height) {
            size -= 1.0;
            break;
        }

        size += 1.0;
    }
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
    max_letter_width = 8 * sq / (10 * 11);
    max_letter_height= 8 * sq / (10 * 10);

    find_best_text_size(cr, max_letter_width, max_letter_height,
                        &extents);
    dx = (sq - NB_WIDTH * extents.width) / (NB_WIDTH - 1);
    dy = (sq - NB_HEIGHT * extents.height) / (NB_HEIGHT - 1);
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
