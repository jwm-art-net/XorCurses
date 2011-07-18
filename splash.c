#include "splash.h"
#include "types.h"
#include "screen.h"
#include "version.h"

void
splash()
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

    bool msk = FALSE;

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
    ptr1 = xormask;
    ptr2 = ptr1[0];
    y = offy + 7;
    wattrset(game_win, COLOR_PAIR(0));
    while (*ptr1[0] != 'X') {
        ptr2 = *ptr1;
        x = offx + 12;
        msk = FALSE;
        while (*ptr2) {
            if (*ptr2 != ' ') {
                wattrset(game_win, COLOR_PAIR(COL_SPLASH_MASK));
                msk = TRUE;
            }
            else if (msk == TRUE)
                wattrset(game_win, COLOR_PAIR(COL_SPLASH_MASK_SOLID));
            if (*ptr2 != '*')
                mvwaddch(game_win, y, x, *ptr2);
            x++;
            ptr2++;
        }
        ptr1++;
        y++;
    }
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

