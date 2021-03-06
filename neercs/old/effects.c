/*
 *  neercs        console-based window manager
 *  Copyright (c) 2006-2010 Sam Hocevar <sam@hocevar.net>
 *                2008-2010 Jean-Yves Lamoureux <jylam@lnxscene.org>
 *                All Rights Reserved
 *
 *  This program is free software. It comes without any warranty, to
 *  the extent permitted by applicable law. You can redistribute it
 *  and/or modify it under the terms of the Do What The Fuck You Want
 *  To Public License, Version 2, as published by Sam Hocevar. See
 *  http://www.wtfpl.net/ for more details.
 */

#if defined HAVE_CONFIG_H
#   include "config.h"
#endif

#if !defined _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <endian.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <caca.h>

#include "neercs.h"

void draw_thumbnails(struct screen_list *screen_list)
{
    char const *const *fonts;
    caca_dither_t *d;
    caca_font_t *f;
    uint8_t *buf;
    int i, y =
        caca_get_canvas_height(screen_list->cv) - 6 - screen_list->modals.status;
    int miniw, minih;

    if (screen_list->count)
    {
        fonts = caca_get_font_list();
        f = caca_load_font(fonts[0], 0);

        miniw = caca_get_canvas_width(screen_list->screen[0]->cv)
            * caca_get_font_width(f);
        minih = caca_get_canvas_height(screen_list->screen[0]->cv)
            * caca_get_font_height(f);
        buf = malloc(4 * miniw * minih);

#if defined(HAVE_ENDIAN_H)
        if (__BYTE_ORDER == __BIG_ENDIAN)
#else
        /* This is compile-time optimised with at least -O1 or -Os */
        uint32_t const tmp = 0x12345678;
        if (*(uint8_t const *)&tmp == 0x12)
#endif
            d = caca_create_dither(32, miniw, minih, 4 * miniw,
                                   0xff0000, 0xff00, 0xff, 0x0);
        else
            d = caca_create_dither(32, miniw, minih, 4 * miniw,
                                   0xff00, 0xff0000, 0xff000000, 0x0);

        for (i = 0; i < screen_list->count; i++)
        {
            if (!screen_list->screen[i]->changed && !screen_list->changed)
                continue;
            caca_render_canvas(screen_list->screen[i]->cv, f, buf,
                               miniw, minih, miniw * 4);
            caca_dither_bitmap(screen_list->cv, 20 * i, y, 19, 6, d, buf);
            caca_set_color_ansi(screen_list->cv, CACA_WHITE, CACA_BLUE);

            if (screen_list->pty == i)
                caca_draw_cp437_box(screen_list->cv, 20 * i, y, 19, 6);
            caca_printf(screen_list->cv, 20 * i, y, "(%i)", i + 1);
        }

        caca_free_dither(d);
        caca_free_font(f);

        free(buf);
    }

}

/* FIXME, make this stuff more configurable */
void draw_status(struct screen_list *screen_list)
{
    int x = 0, y = caca_get_canvas_height(screen_list->cv) - 1;


    /* caca_fill_box(screen_list->cv, x, y,
       caca_get_canvas_width(screen_list->cv), 1, '#'); */

    /* Hour */
    {
        time_t now = time((time_t *) 0);
        struct tm *t = localtime(&now);
        char hour[256];
        sprintf(hour, "%02d:%02d", t->tm_hour, t->tm_min);

        caca_set_color_ansi(screen_list->cv, CACA_LIGHTBLUE, CACA_BLUE);
        caca_printf(screen_list->cv, x, y, "[");

        caca_set_color_ansi(screen_list->cv, CACA_DEFAULT, CACA_BLUE);
        caca_printf(screen_list->cv, x + 1, y, hour);
        caca_set_color_ansi(screen_list->cv, CACA_LIGHTBLUE, CACA_BLUE);
        caca_printf(screen_list->cv, x + strlen(hour) + 1, y, "]");
        x += 7;

    }

    /* Window */
    {
        char text[256];
        sprintf(text, "%d/%d", screen_list->pty + 1, screen_list->count);
        caca_set_color_ansi(screen_list->cv, CACA_BLUE, CACA_BLUE);
        caca_put_char(screen_list->cv, x, y, '#');
        x++;
        caca_set_color_ansi(screen_list->cv, CACA_LIGHTBLUE, CACA_BLUE);
        caca_printf(screen_list->cv, x, y, "Window:");
        caca_set_color_ansi(screen_list->cv, CACA_BLUE, CACA_BLUE);
        caca_put_char(screen_list->cv, x + 7, y, '#');
        caca_set_color_ansi(screen_list->cv, CACA_DEFAULT, CACA_BLUE);
        caca_printf(screen_list->cv, x + 8, y, text);
        x += 8 + strlen(text);
    }

    /* Window Manager */
    {
        char text[256];

        switch (screen_list->wm_type)
        {
        case WM_CARD:
            strcpy(text, "card");
            break;
        case WM_HSPLIT:
            strcpy(text, "hsplit");
            break;
        case WM_VSPLIT:
            strcpy(text, "vsplit");
            break;
        case WM_FULL:
        default:
            strcpy(text, "full");
            break;

        }

        caca_set_color_ansi(screen_list->cv, CACA_BLUE, CACA_BLUE);
        caca_put_char(screen_list->cv, x, y, '#');
        x++;
        caca_set_color_ansi(screen_list->cv, CACA_LIGHTBLUE, CACA_BLUE);
        caca_printf(screen_list->cv, x, y, "WM:");
        caca_set_color_ansi(screen_list->cv, CACA_BLUE, CACA_BLUE);
        caca_put_char(screen_list->cv, x + 3, y, '#');
        caca_set_color_ansi(screen_list->cv, CACA_DEFAULT, CACA_BLUE);
        caca_printf(screen_list->cv, x + 4, y, text);
        x += 4 + strlen(text);
    }

    /* Help (must be the last one ) */
    {
        char text[256];
        sprintf(text, "Help: ctrl-a-?");
        caca_set_color_ansi(screen_list->cv, CACA_BLUE, CACA_BLUE);
        while (x <
               (int)(caca_get_canvas_width(screen_list->cv) - strlen(text)))
        {
            caca_put_char(screen_list->cv, x, y, '#');
            x++;
        }
        caca_set_color_ansi(screen_list->cv, CACA_DEFAULT, CACA_BLUE);
        caca_printf(screen_list->cv, x, y, text);
    }


}



int update_window_list(int c, struct screen_list *screen_list)
{
    debug("Got %x\n", c);

    switch (c)
    {
    case 0x111:
        if (screen_list->modals.cur_in_list > 0)
            screen_list->modals.cur_in_list--;
        break;
    case 0x112:
        if (screen_list->modals.cur_in_list < screen_list->count - 1)
            screen_list->modals.cur_in_list++;
        break;
    case 0xd:
        screen_list->modals.window_list = 0;
        screen_list->prevpty = screen_list->pty;
        screen_list->pty = screen_list->modals.cur_in_list;
        break;
    case 0x22:
        screen_list->modals.window_list = 0;
        break;
    default:
        break;
    }

    return 1;
}

void draw_list(struct screen_list *screen_list)
{
    int i;
    int w = (caca_get_canvas_width(screen_list->cv));
    int h = (caca_get_canvas_height(screen_list->cv));

    debug("Drawing list\n");
    caca_set_color_ansi(screen_list->cv, CACA_BLACK, CACA_BLACK);
    caca_fill_box(screen_list->cv, 0, 0, w, h, '#');
    caca_set_color_ansi(screen_list->cv, CACA_DEFAULT, CACA_DEFAULT);
    caca_draw_cp437_box(screen_list->cv, 0, 0, w, h);

    caca_printf(screen_list->cv, 2, 1, "Num         Name");
    for (i = 0; i < screen_list->count; i++)
    {
        char line[1024];
        if (screen_list->modals.cur_in_list == i)
            caca_set_color_ansi(screen_list->cv, CACA_BLACK, CACA_WHITE);
        else
            caca_set_color_ansi(screen_list->cv, CACA_DEFAULT, CACA_DEFAULT);
        sprintf(line, "%d           %s", i + 1, screen_list->screen[i]->title);

        caca_printf(screen_list->cv, 2, i + 3, line);
    }
}

/* Close a window by animating it collapsing */
/* Total close time */
#define DELAY 500000.0f
int close_screen_recurrent(struct screen_list *screen_list,
                           struct recurrent *rec, void *user,
                           long long unsigned int t)
{
    long long unsigned int delta = t - rec->start_time;

    screen_list->dont_update_coords = 1;
    screen_list->delay = 0;
    rec->kill_me = 0;
    if (delta >= DELAY || (!screen_list->eyecandy))
    {
        rec->kill_me = 1;
        remove_screen(screen_list, screen_list->pty, 1);
        screen_list->pty = screen_list->prevpty>screen_list->count?screen_list->count:screen_list->prevpty;
        screen_list->prevpty = 0;
        screen_list->dont_update_coords = 0;
    }
    else
    {
        float r = 1 - ((DELAY - (DELAY - delta)) / DELAY);
        caca_canvas_t *old, *new;
        struct screen *s = screen_list->screen[screen_list->pty];
        int w = s->orig_w * r;
        int h = s->orig_h * r;

        /* libcaca canvas resize function is bugged, do it by hand */
        old = s->cv;
        new = caca_create_canvas(w, h);
        caca_blit(new, 0, 0, old, NULL);
        s->cv = new;
        caca_free_canvas(old);
        set_tty_size(s->fd, w, h);

        s->w = w;
        s->h = h;

        s->x = (s->orig_x * r) + ((s->orig_w / 2) - s->w / 2);
        s->y = (s->orig_y * r) + ((s->orig_h / 2) - s->h / 2);
    }
    screen_list->changed = 1;
    return 1;
}

#endif

