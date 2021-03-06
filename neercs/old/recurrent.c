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

#include "neercs.h"


int handle_recurrents(struct screen_list *screen_list)
{
    int refresh = 0, i;
    /* Recurrent functions */
    for (i = 0; i < screen_list->recurrent_list->count; i++)
    {
        if (screen_list->recurrent_list->recurrent[i]->function)
        {
            refresh |=
                screen_list->recurrent_list->recurrent[i]->
                function(screen_list,
                         screen_list->recurrent_list->recurrent[i],
                         screen_list->recurrent_list->recurrent[i]->user,
                         get_us());
        }
    }
    /* Delete recurrent functions */
    for (i = 0; i < screen_list->recurrent_list->count; i++)
    {
        if (screen_list->recurrent_list->recurrent[i]->kill_me)
        {
            remove_recurrent(screen_list->recurrent_list, i);
            refresh = 1;
            i = 0;
        }
    }
    return refresh;
}


/* Add recurrent function. It will be called at each main loop iteration,
   unless it is removed */
int add_recurrent(struct recurrent_list *list,
                  int (*func) (struct screen_list *, struct recurrent * rec,
                               void *user, long long unsigned int t),
                  void *user)
{
    if (list == NULL || func == NULL)
        return -1;

    list->recurrent = (struct recurrent **)realloc(list->recurrent,
                                                   sizeof(struct recurrent *)
                                                   * (list->count + 1));

    if (!list->recurrent)
        fprintf(stderr, "Can't allocate memory at %s:%d\n", __FUNCTION__,
                __LINE__);

    list->recurrent[list->count] = malloc(sizeof(struct recurrent));

    list->recurrent[list->count]->kill_me = 0;
    list->recurrent[list->count]->function = func;
    list->recurrent[list->count]->user = user;
    list->recurrent[list->count]->start_time = get_us();
    list->count++;

    return list->count - 1;
}


/* Remove recurrent. Do *NOT* call this from a recurrent function. */
int remove_recurrent(struct recurrent_list *list, int n)
{

    if (n >= list->count)
        return -1;

    memmove(&list->recurrent[n],
            &list->recurrent[n + 1],
            sizeof(struct recurrent *) * (list->count - (n + 1)));

    free(list->recurrent[n]);
    list->recurrent = (struct recurrent **)realloc(list->recurrent,
                                                   sizeof(sizeof
                                                          (struct recurrent *))
                                                   * (list->count));
    if (!list->recurrent)
        fprintf(stderr, "Can't allocate memory at %s:%d\n", __FUNCTION__,
                __LINE__);



    list->count--;
    return 0;
}

#endif
