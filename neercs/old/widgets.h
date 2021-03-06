﻿/*
 *  neercs — console-based window manager
 *
 *  Copyright © 2009—2010 Jean-Yves Lamoureux <jylam@lnxscene.org>
 *
 *  This program is free software. It comes without any warranty, to
 *  the extent permitted by applicable law. You can redistribute it
 *  and/or modify it under the terms of the Do What the Fuck You Want
 *  to Public License, Version 2, as published by the WTFPL Task Force.
 *  See http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <caca.h>


enum input_box_code
{
    INPUT_BOX_ESC,
    INPUT_BOX_RET,
    INPUT_BOX_NOTHING,
};

struct input_box
{
    caca_canvas_t *cv;
    int x, y;
    int w, h;
    int size;
    char *command;
    char *output_err;
    char *output_res;
};

struct input_box *widget_ibox_init(caca_canvas_t * cv, int w, int h);
int widget_ibox_draw(struct input_box *box);
int widget_ibox_handle_key(struct input_box *box, unsigned int c);
char* widget_ibox_get_text(struct input_box *box);
void widget_ibox_destroy(struct input_box *box);
void widget_ibox_set_error(struct input_box *box, char *err);
void widget_ibox_set_msg(struct input_box *box, char *msg);
