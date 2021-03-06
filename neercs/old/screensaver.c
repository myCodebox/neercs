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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

#include <caca.h>

#include "neercs.h"


void screensaver_init(struct screen_list *screen_list)
{
    screensaver_flying_toasters_init(screen_list);
}

void screensaver_kill(struct screen_list *screen_list)
{
    screensaver_flying_toasters_kill(screen_list);
}

void draw_screensaver(struct screen_list *screen_list)
{
    screensaver_flying_toasters(screen_list);
}




/* Flying Toasters */

#define COUNT 15
#define PRECISION 100

char toaster_text[3][99] = { {
 "      __._ \n"
 "   .-'== _',\n"
 "  <|_= .-'  |\n"
 "   | --|   \\'.-_ \n"
 "   |   | \\  \" _.\n"
 "    `-_|.-\\_.-\n"},
{
 "      __._ \n"
 "   .-'== _',\n"
 "  \\|_= .-'  |\n"
 "   | --|  __'-.\n"
 "   |   | ___.-\n"
 "    `-_|.-\n"},
{
 "   _- __._\n"
 "  /.-'== _',_.-.\n"
 "  \\|_= .-'/  _.'\n"
 "   | --| / .-\n"
 "   |   |  _.|\n"
 "    `-_|.-\n"}
};

char toaster_mask[3][99] = { {

 "      __._ \n"
 "   .-'== _',\n"
 "  <|_=X.-'XX|\n"
 "   |X--|XXX\\'.-_ \n"
 "   |XXX|X\\XX\"X_.\n"
 "    `-_|.-\\_.-\n"},
    {

 "      __._ \n"
 "   .-'== _',\n"
 "  \\|_= .-'XX|\n"
 "   |X--|XX__'-.\n"
 "   |XXX|X___.-\n"
 "    `-_|.-\n"},
{
 "   _- __._\n"
 "  /.-'== _',_.-.\n"
 "  \\|_= .-'/XX_.'\n"
 "   |X--|X/X.-\n"
 "   |XXX|XX_.|\n"
 "    `-_|.-\n"}
};

struct flying_toaster
{
    int x[COUNT], y[COUNT], s[COUNT];
    caca_canvas_t **toaster;
    caca_canvas_t **mask;
};

void screensaver_flying_toasters_init(struct screen_list *screen_list)
{
    struct flying_toaster *flying_toaster;
    int w = caca_get_canvas_width(screen_list->cv);
    int h = caca_get_canvas_height(screen_list->cv);
    int i;

    flying_toaster = malloc(sizeof(struct flying_toaster));
    flying_toaster->toaster =
        (caca_canvas_t **) malloc(sizeof(caca_canvas_t *) * 3);
    flying_toaster->mask =
        (caca_canvas_t **) malloc(sizeof(caca_canvas_t *) * 3);

    for (i = 0; i < 3; i++)
    {
        flying_toaster->toaster[i] = caca_create_canvas(0, 0);
        flying_toaster->mask[i] = caca_create_canvas(0, 0);
        caca_import_canvas_from_memory(flying_toaster->toaster[i],
                                       toaster_text[i],
                                       strlen(toaster_text[i]), "utf8");
        caca_import_canvas_from_memory(flying_toaster->mask[i],
                                       toaster_mask[i],
                                       strlen(toaster_mask[i]), "utf8");
    }

    for (i = 0; i < COUNT; i++)
    {
        flying_toaster->x[i] = (rand() % w) * PRECISION;
        flying_toaster->y[i] = (rand() % h) * PRECISION;
        flying_toaster->s[i] = (rand() % 3) * PRECISION;
    }


    screen_list->screensaver.data = flying_toaster;
}

void screensaver_flying_toasters_kill(struct screen_list *screen_list)
{
    struct flying_toaster *flying_toaster = screen_list->screensaver.data;

    caca_free_canvas(flying_toaster->toaster[0]);
    caca_free_canvas(flying_toaster->toaster[1]);
    caca_free_canvas(flying_toaster->toaster[2]);
    free(flying_toaster->toaster);
    free(flying_toaster);
    screen_list->screensaver.data = NULL;
}

void screensaver_flying_toasters(struct screen_list *screen_list)
{
    struct flying_toaster *d = screen_list->screensaver.data;
    int i, w, h, x, y, s;
    if (!d)
        return;

    w = caca_get_canvas_width(screen_list->cv);
    h = caca_get_canvas_height(screen_list->cv);

    caca_set_color_ansi(screen_list->cv, CACA_WHITE, CACA_BLACK);
    caca_clear_canvas(screen_list->cv);

    for (i = 0; i < COUNT; i++)
    {
        x = d->x[i] / PRECISION;
        y = d->y[i] / PRECISION;
        s = d->s[i] / PRECISION;

        caca_blit(screen_list->cv, x, y, d->toaster[s], d->mask[s]);

        d->x[i] -= 40;
        d->y[i] += 10;

        if (d->x[i] / PRECISION + caca_get_canvas_width(d->toaster[s]) <= 0)
            d->x[i] = ((rand() % w) / 3 + w) * PRECISION;
        if (d->y[i] / PRECISION >= h)
            d->y[i] = ((rand() % h) / 2 - h) * PRECISION;

        d->s[i] = ((d->s[i] + 24) % (3 * PRECISION));
    }
}

#endif
