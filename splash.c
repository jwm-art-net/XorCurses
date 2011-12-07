#include "splash.h"
#include "types.h"
#include "screen.h"
#include "player.h"
#include "options.h"
#include "debug.h"

#include <stdlib.h>
#include <time.h>


void splash(void)
{
    char *xortitle[] = {
        "--  --  ---  ---\\",
        " \\  /  /   \\ |   |",
        "  \\/   |   | |--/",
        "  /\\   |   | |  \\",
        " /  \\  \\   / |   \\",
        "--  --  ---  - Curses",
        "X"
    };
    char *xorlns[] = {
        "           ---           ---",
        "         ------         ------",
        "      ----------       ----------",
        "",
        " -------------------------------------",
        "  XorCurses-" VERSION "      by jwm-art.net",
        "X"
    };
    char **ptr1;

    char *ptr2;

    xy_t x;

    xy_t y;

    xy_t offx = ((screen_data->garea_w - 8) / 2) * ICON_W;
    xy_t offy = ((screen_data->garea_h - 8) / 2) * ICON_H + 1;

    wclear(game_win);

    ptr1 = xortitle;
    ptr2 = ptr1[0];
    y = offy + 0;
    while (*ptr1[0] != 'X') {
        ptr2 = *ptr1;
        x = offx + 10;
        while (*ptr2) {
            if (*ptr2 != ' ')
                wattrset(game_win, COLOR_PAIR(COL_SPLASH_TITLE));
            else
                wattrset(game_win, COLOR_PAIR(0));
            mvwaddch(game_win, y, x, *ptr2);
            x++;
            ptr2++;
        }
        ptr1++;
        y++;
    }

    splash_mask(offx + 12, offy + 7, 0);

    ptr1 = xorlns;
    ptr2 = ptr1[0];
    y = offy + 16;
    wattrset(game_win, COLOR_PAIR(0));
    while (*ptr1[0] != 'X') {
        ptr2 = *ptr1;
        x = offx + 0;
        while (*ptr2) {
            if (*ptr2 != ' ')
                mvwaddch(game_win, y, x, *ptr2);
            x++;
            ptr2++;
        }
        ptr1++;
        y++;
    }
    wrefresh(game_win);
}


void splash_mask(xy_t offx, xy_t offy, int randcol)
{
    char *xormask[] = {
        " ___*******___",
        "/   -------   \\",
        "|             |",
        "\\  ---\\ /---  /",
        " | \\--- ---/ |",
        "/             \\",
        "|             |",
        "\\  \\\\     //  /",
        " \\  \\\\   //  /",
        "  \\  \\\\ //  /",
        "   \\  \\_/  /",
        "    \\     /",
        "     \\___/",
        "X"
    };

    char **ptr1 = xormask;
    char *ptr2 = ptr1[0];

    xy_t y = offy;
    wattrset(game_win, COLOR_PAIR(0));

    int fg = (randcol) ? rand() % ICON_XXX : COL_SPLASH_MASK;
    int bg = (randcol) ? rand() % ICON_XXX : COL_SPLASH_MASK_SOLID;

    while (*ptr1[0] != 'X')
    {
        ptr2 = *ptr1;
        xy_t x = offx;
        bool msk = FALSE;
        bool out = TRUE;

        while (*ptr2)
        {
            if (*ptr2 != ' ') {
                wattrset(game_win, COLOR_PAIR(fg));
                msk = TRUE;
                out = FALSE;
            }
            else if (msk == TRUE)
                wattrset(game_win, COLOR_PAIR(bg));

            if (*ptr2 != '*' && !out)
                mvwaddch(game_win, y, x, *ptr2);

            x++;
            ptr2++;
        }
        ptr1++;
        y++;
    }
}


void splatter_masks(void)
{
    int i;
    struct timespec rpause;
    struct timespec repause;
    rpause.tv_sec = 0;
    rpause.tv_nsec = 0;

    if (player.replay)
    {
        if (!options->replay_hyper)
            rpause.tv_nsec =
                options_replay_speed(options->replay_speed) / 5;
    }
    else
        rpause.tv_nsec = 12500000L;

    int maxx = getmaxx(game_win);
    int maxy = getmaxy(game_win);

    srand(time(0));

    for (i = 0; i < 150; ++i)
    {
        splash_mask(-5 + rand() % maxx, -5 + rand() % maxy, 1);
        scr_wmsg(game_win, " Well Done! Level Complete! ", 0, 0);
        wrefresh(game_win);
        nanosleep(&rpause, &repause);
    }

}


void splash_level_entry(lvl_t level)
{
    struct timespec rpause;
    struct timespec repause;
    const int count = 15; /* iterations */
    const int cpoff = 9 + (level / 3) % 5; /* ICON_* colour pair offset */
    const int ccount = (level < 12) ? 6 : 5; /* #colour bands */
    const int maxx = getmaxx(game_win);
    const int maxy = getmaxy(game_win);
    const int c = ' ';
    int i;

    rpause.tv_sec = 0;
    rpause.tv_nsec = 0;

    if (player.replay)
    {
        if (!options->replay_hyper)
            rpause.tv_nsec =
                options_replay_speed(options->replay_speed);
    }
    else
        rpause.tv_nsec = 50000000L;

    for (i = 0; i < count; ++i)
    {
        int tmp = maxx / 2;
        int r1x = tmp - i % 2;
        int r2x = tmp + i % 2;
        int r1y = maxy / 2;
        int r2y = r1y;
        int cp;

        cp = (ccount * 100 - i) % ccount;

        wattrset(game_win, COLOR_PAIR(cpoff + cp));

        for (tmp = 0; tmp <= r2x - r1x; ++tmp)
            mvwaddch(game_win, r1y, r1x + tmp, c);

        do
        {
            int x, y;

            cp = (cp + 1) % ccount;
            wattrset(game_win, COLOR_PAIR(cpoff + cp));
            r1x -= 2;
            r2x += 2;
            r1y--;
            r2y++;

            for (x = r1x; x <= r2x; ++x)
            {
                mvwaddch(game_win, r1y, x, c);
                mvwaddch(game_win, r2y, x, c);
            }

            for (y = r1y; y <= r2y; ++y)
            {
                mvwaddch(game_win, y, r1x, c);
                mvwaddch(game_win, y, r1x+1, c);
                mvwaddch(game_win, y, r2x, c);
                mvwaddch(game_win, y, r2x-1, c);
            }

        } while(r1x > 0 || r1y > 0);

        wrefresh(game_win);
        nanosleep(&rpause, &repause);
    }
}


void splash_wipe_out(void)
{
    struct timespec rpause;
    struct timespec repause;

    xy_t r1x = 0;
    xy_t r2x = screen_data->garea_w * ICON_W - 1;
    xy_t r1y = 0;
    xy_t r2y = screen_data->garea_h * ICON_H - 1;

    rpause.tv_sec = 0;
    rpause.tv_nsec = 0;

    if (player.replay)
    {
        if (!options->replay_hyper)
            rpause.tv_nsec =
                options_replay_speed(options->replay_speed) / 100;
    }
    else
        rpause.tv_nsec = 250000L;

    int cp = ICON_SPACE;

    do
    {
        xy_t x, y;

        wattrset(game_win, COLOR_PAIR(cp));

        for (x = r1x; x < r2x; ++x)
        {
            mvwaddch(game_win, r1y, x, '-');
            wrefresh(game_win);
            nanosleep(&rpause, &repause);
        }

        for (y = r1y; y < r2y; ++y)
        {
            mvwaddch(game_win, y, r2x, '|');
            wrefresh(game_win);
            nanosleep(&rpause, &repause);
        }

        for (x = r2x; x > r1x; --x)
        {
            mvwaddch(game_win, r2y, x, '-');
            wrefresh(game_win);
            nanosleep(&rpause, &repause);
        }

        for (y = r2y; y > r1y; --y)
        {
            mvwaddch(game_win, y, r1x, '|');
            wrefresh(game_win);
            nanosleep(&rpause, &repause);
        }

        r1x++;
        r1y++;
        r2x--;
        r2y--;

        cp = (cp + 1) % ICON_XXX;

    } while(r1x <= r2x && r1y <= r2y);

    wrefresh(game_win);
}




