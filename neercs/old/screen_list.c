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
#include <pwd.h>

#include <caca.h>

#include "neercs.h"

struct screen_list *create_screen_list(void)
{

    struct screen_list *screen_list = NULL;
    struct passwd *user_info;
    char *user_dir = NULL;

    /* Create screen list */
    screen_list = (struct screen_list *)malloc(sizeof(struct screen_list));
    if (!screen_list)
    {
        fprintf(stderr, "Can't allocate memory at %s:%d\n", __FUNCTION__,
                __LINE__);
        return NULL;
    }
    screen_list->screen =
    (struct screen **)malloc(sizeof(sizeof(struct screen *)));
    if (!screen_list->screen)
    {
        fprintf(stderr, "Can't allocate memory at %s:%d\n", __FUNCTION__,
                __LINE__);
        free(screen_list);
        return NULL;
    }

    screen_list->count = 0;
    screen_list->modals.mini = 1;
    screen_list->modals.help = 0;
    screen_list->modals.status = 1;
    screen_list->modals.window_list = 0;
    screen_list->modals.python_command = 0;
    screen_list->eyecandy = 1;
    screen_list->border_size = 1;
    screen_list->title = NULL;
    screen_list->wm_type = WM_VSPLIT;
    screen_list->in_bell = 0;
    screen_list->changed = 0;
    screen_list->requested_delay = 0;
    screen_list->delay = 1000 / 60;     /* Don't refresh more than 60 times
                                         per second */
    screen_list->pty = screen_list->prevpty = 0;
    screen_list->dont_update_coords = 0;
    screen_list->screensaver.timeout = (60*5) * 1000000;
    screen_list->screensaver.data = NULL;
    screen_list->screensaver.in_screensaver = 0;
    screen_list->lock.autolock_timeout = -1;
    screen_list->lock.locked = 0;
    screen_list->lock.lock_offset = 0;
    screen_list->lock.lock_on_detach = 0;
    screen_list->comm.attached = 0;
    screen_list->comm.socket[SOCK_SERVER] = 0;
    screen_list->comm.socket[SOCK_CLIENT] = 0;
    screen_list->comm.socket_dir = NULL;
    screen_list->comm.socket_path[SOCK_SERVER] = NULL;
    screen_list->comm.socket_path[SOCK_CLIENT] = NULL;
    screen_list->comm.session_name = NULL;
    screen_list->sys.default_shell = NULL;
    screen_list->sys.user_path = NULL;
    screen_list->sys.to_grab = NULL;
    screen_list->sys.to_start = NULL;
    screen_list->sys.nb_to_grab = 0;
    screen_list->sys.attach = 0;
    screen_list->sys.forceattach = 0;
    screen_list->need_refresh = 0;

    screen_list->force_refresh = 0;
    screen_list->cube.in_switch = 0;
    screen_list->cube.duration = 1000000;

    screen_list->interpreter_props.box = NULL;

    screen_list->recurrent_list = NULL;
    screen_list->cv = NULL;
    screen_list->dp = NULL;

    memset(screen_list->lock.lockmsg, 0, 1024);
    memset(screen_list->lock.lockpass, 0, 1024);

    /* Build local config file path */
    user_dir = getenv("HOME");
    if (!user_dir)
    {
        user_info = getpwuid(getuid());
        if (user_info)
        {
            user_dir = user_info->pw_dir;
        }
    }
    if (user_dir)
    {
        screen_list->sys.user_path =
        malloc(strlen(user_dir) + strlen("/.neercsrc") + 1);
        sprintf(screen_list->sys.user_path, "%s/%s", user_dir, ".neercsrc");
    }


    screen_list->recurrent_list =
    (struct recurrent_list *)malloc(sizeof(struct recurrent_list));
    screen_list->recurrent_list->recurrent =
    (struct recurrent **)malloc(sizeof(struct recurrent *));
    if (!screen_list->recurrent_list->recurrent)
    {
        fprintf(stderr, "Can't allocate memory at %s:%d\n", __FUNCTION__,
                __LINE__);
        free(screen_list->screen);
        free(screen_list);
        return NULL;
    }
    screen_list->recurrent_list->count = 0;

    return screen_list;
}

void free_screen_list(struct screen_list *screen_list)
{
    int i;
    struct option_t *option;

    if (screen_list->dp)
        caca_free_display(screen_list->dp);

    if (screen_list->cv)
        caca_free_canvas(screen_list->cv);

    for (i = 0; i < screen_list->count; i++)
    {
        destroy_screen(screen_list->screen[i]);
    }

    if (screen_list->comm.socket_path[SOCK_SERVER])
    {
        /* FIXME test that we are the server */
        if (!screen_list->dp)
            unlink(screen_list->comm.socket_path[SOCK_SERVER]);
        free(screen_list->comm.socket_path[SOCK_SERVER]);
    }

    if (screen_list->comm.socket_path[SOCK_CLIENT])
    {
        /* FIXME test that we are the client */
        if (screen_list->dp)

            unlink(screen_list->comm.socket_path[SOCK_CLIENT]);
        free(screen_list->comm.socket_path[SOCK_CLIENT]);
    }

    if (screen_list->comm.socket[SOCK_CLIENT])
        close(screen_list->comm.socket[SOCK_CLIENT]);

    if (screen_list->comm.socket[SOCK_SERVER])
        close(screen_list->comm.socket[SOCK_SERVER]);

    if (screen_list->screen)
        free(screen_list->screen);

    option = screen_list->config;

    while (option)
    {
        struct option_t *kromeugnon = option;
        option = option->next;
        if (kromeugnon->key)
            free(kromeugnon->key);
        if (kromeugnon->value)
            free(kromeugnon->value);
        free(kromeugnon);
    }

    for (i = 0; i < screen_list->recurrent_list->count; i++)
    {
        remove_recurrent(screen_list->recurrent_list, i);
        i = 0;
    }

    if (screen_list->recurrent_list->recurrent)
        free(screen_list->recurrent_list->recurrent);
    if (screen_list->recurrent_list)
        free(screen_list->recurrent_list);

    if (screen_list->comm.session_name)
        free(screen_list->comm.session_name);

    if (screen_list->title)
        free(screen_list->title);

    free(screen_list);
}

#endif

