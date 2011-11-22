#include "screen.h"
#include "game_display.h"
#include "info.h"
#include "options.h"
#include "map.h"
#include "debug.h"

#include <curses.h>
#include <stdlib.h>
#include <string.h>

#define IOBUF 80

struct xor_screen *screen_data = 0;

WINDOW *game_win = 0;

WINDOW *info_win = 0;

static float info_w_ratio = MIN_INFO_W / (float) MIN_W;

su_t
screen_create()
{
    initscr();
    if (has_colors() == FALSE) {
        endwin();
        err_msg("Your terminal does not support color\n");
        return 0;
    }
    noecho();
    nonl();
    cbreak();
    nodelay(game_win, FALSE);
    start_color();
    curs_set(0);
    if (screen_data)
        screen_destroy();
    screen_data = malloc(sizeof(struct xor_screen));
    if (!screen_data)
        return 0;
    memset(screen_data, 0, sizeof(struct xor_screen));
    /*                              fg              bg          */
    init_pair(COL_G_TXT,            COLOR_BLACK,    COLOR_YELLOW);
    init_pair(COL_G_TXT_HI,         COLOR_WHITE,    COLOR_RED);
    init_pair(COL_G_TXT_DISABLED,   COLOR_WHITE,    COLOR_YELLOW);
    init_pair(COL_G_TXT_STATUS,     COLOR_WHITE,    COLOR_BLUE);
    init_pair(COL_I_TXT,            COLOR_WHITE,    COLOR_BLACK);
    init_pair(COL_I_TXT_HI,         COLOR_BLACK,    COLOR_YELLOW);
    init_pair(COL_I_TXT_DISABLED,   COLOR_RED,      COLOR_BLACK);
    init_pair(COL_I_TXT_STATUS,     COLOR_WHITE,    COLOR_BLUE);
    init_pair(COL_I_MAP_WALL,       COLOR_BLACK,    COLOR_YELLOW);
    init_pair(COL_TOO_SMALL,        COLOR_BLACK,    COLOR_YELLOW);
    init_pair(COL_SPLASH_TITLE,     COLOR_WHITE,    COLOR_RED);
    init_pair(COL_SPLASH_MASK,      COLOR_BLUE,     COLOR_BLACK);
    init_pair(COL_SPLASH_MASK_SOLID,COLOR_BLUE,     COLOR_BLUE);
    return 1;
}

void
screen_destroy()
{
    if (screen_data) {
        free(screen_data);
        screen_data = 0;
    }
    if (game_win) {
        delwin(game_win);
        game_win = 0;
    }
    if (info_win) {
        delwin(info_win);
        info_win = 0;
    }
    endwin();

    debug("Ncurses says bye bye!\n");
}

su_t
screen_size()
{
    int maxx = getmaxx(stdscr);
    int maxy = getmaxy(stdscr);
    if (maxx < MIN_W || maxy < 24)
        return 0;
    /*  The info panel width is calculated from a ratio and then
       restricted to just large enough to accommodate a map display. */
    screen_data->info_w = info_w_ratio * maxx;
    if (screen_data->info_w >= MAP_W + 2)
        screen_data->info_w = MAP_W + 2;

    /*  game area width */
    screen_data->garea_w = (options->oldschool_play
                            ? DEF_GAREA_W
                            : (maxx - screen_data->info_w) / ICON_W);
    /* game area top-left x coordinate */
    screen_data->ga_tlx =
        (maxx - screen_data->info_w - screen_data->garea_w * ICON_W) / 2;
    /* centering... */
    if (screen_data->garea_w > MAX_GAREA_W) {
        screen_data->ga_tlx +=
            ((screen_data->garea_w - MAX_GAREA_W) * ICON_W) / 2;
        screen_data->garea_w = MAX_GAREA_W;
    }
    /* game area height... */
    screen_data->garea_h = (options->oldschool_play
                            ? DEF_GAREA_H : maxy / ICON_H);
    /* game area top-left y coordinate */
    screen_data->ga_tly = (maxy - screen_data->garea_h * ICON_H) / 2;
    /* centering... */
    if (screen_data->garea_h > MAX_GAREA_H) {
        screen_data->ga_tly +=
            ((screen_data->garea_h - MAX_GAREA_H) * ICON_H) / 2;
        screen_data->garea_h = MAX_GAREA_H;
    }
    /* info window height */

    screen_data->info_h =
        (maxy > MAX_INFO_H_OLDSCHOOL ? MAX_INFO_H_OLDSCHOOL : maxy);

/*  this commented out because it only made sense when the largest
    possible game area was large enough to accommodate a full detail
    map display (heightwise).
    if (options->oldschool_play)
        screen_data->info_h = (maxy > MAX_INFO_H_OLDSCHOOL
                                        ? MAX_INFO_H_OLDSCHOOL : maxy);
    else
        screen_data->info_h = (maxy > MAX_INFO_H ? MAX_INFO_H : maxy);
*/
    /* info window too small for unscaled map display ? */
    if (screen_data->info_h - 6 <= MAP_H
     || screen_data->info_w - 2 < MAP_W)
    {
        screen_data->scale_map = TRUE;  /* scale it down that is */
        screen_data->info_h = screen_data->garea_h * ICON_H;
        screen_data->i_tly = screen_data->ga_tly;
        screen_data->map_tlx = 1 + (screen_data->info_w - 2 - HMAP_W) / 2;
        screen_data->map_tly = 5 + (screen_data->info_h - 6 - HMAP_H) / 2;
    }
    else {
        screen_data->scale_map = FALSE;
        screen_data->i_tly = (maxy - screen_data->info_h) / 2;
        screen_data->map_tlx = 1 + (screen_data->info_w - 2 - MAP_W) / 2;
        screen_data->map_tly = 6 + (screen_data->info_h - 6 - MAP_H) / 2;
    }

    screen_data->i_tlx = screen_data->ga_tlx
                        + screen_data->garea_w * ICON_W;
    if (game_win) {
        if (wresize(game_win,
                    screen_data->garea_h * ICON_H,
                    screen_data->garea_w * ICON_W)
            == ERR)
        {
            err_msg("problem resizing game win!");
            exit(EXIT_FAILURE);
        }
        if (mvwin(game_win, screen_data->ga_tly, screen_data->ga_tlx)
            == ERR)
        {
            err_msg("problem moving game win!");
            exit(EXIT_FAILURE);
        }
    }
    else {
        game_win = newwin(screen_data->garea_h * ICON_H,
                          screen_data->garea_w * ICON_W,
                          screen_data->ga_tly, screen_data->ga_tlx);
        if (!game_win)
            return 0;
        scrollok(game_win, FALSE);
        nodelay(game_win, FALSE);
        keypad(game_win, TRUE);
    }

    debug(" ga_tlx: %3d  ga_tly: %3d\n",
              screen_data->ga_tlx, screen_data->ga_tly);
    debug("garea_w: %3d garea_h: %3d\n",
              screen_data->garea_w, screen_data->garea_h);

    if (info_win) {
        if (wresize(info_win, screen_data->info_h, screen_data->info_w)
            == ERR)
        {
            err_msg("problem resizing info win!");
            exit(EXIT_FAILURE);
        }
        if (mvwin(info_win, screen_data->i_tly, screen_data->i_tlx)
            == ERR)
        {
            err_msg("problem moving info win!");
            exit(EXIT_FAILURE);
        }
    }
    else {
        info_win = newwin(screen_data->info_h,
                          screen_data->info_w,
                          screen_data->i_tly, screen_data->i_tlx);
        if (!info_win)
            return 0;
        scrollok(info_win, FALSE);
        keypad(info_win, TRUE);
    }

    debug("  i_tlx: %3d   i_tly: %3d\n",
              screen_data->i_tlx, screen_data->i_tly);
    debug(" info_w: %3d  info_h: %3d\n",
              screen_data->info_w, screen_data->info_h);
    debug("stdmaxx: %3d stdmaxy: %3d\n",
              maxx, maxy);

    return 1;
}

su_t
screen_resize()
{
    if (!screen_size()) {
        char warn[] = "** TOO SMALL **";
        char wc[2] = " ";
        su_t wl = strlen(warn);
        do {
            int maxx = getmaxx(stdscr);
            int maxy = getmaxy(stdscr);
            attrset(COLOR_PAIR(COL_TOO_SMALL));
            ctr_t c = 0;
            for (int y = 0; y < maxy; y++) {
                for (int x = 0; x < maxx; x++) {
                    wc[0] = (c < wl ? warn[c] : '/');
                    mvwprintw(stdscr, y, x, wc);
                    c++;
                    if (c == 37)
                        c = 0;
                }
            }
            wgetch(stdscr);
        } while (!screen_size());
    }
    wclear(stdscr);
    wrefresh(stdscr);
    wclear(game_win);
    if (screen_data->game_win_repaint_cb)
        screen_data->game_win_repaint_cb();
    wclear(info_win);
    if (screen_data->info_win_repaint_cb)
        screen_data->info_win_repaint_cb();
    return 1;
}

int
scr_wmsg_pause(
    WINDOW * win,
    char *msg,
    int len,
    struct scrxy *msgxy,
    bool pause)
{
    if (!win || !msg)
        return 0;
    if (!len)
        len = strlen(msg);
    wattrset(win, COLOR_PAIR((win == game_win ? COL_G_TXT : COL_I_TXT)));
    bool autofree = FALSE;
    if (!msgxy) {
        msgxy = malloc(sizeof(struct scrxy));
        autofree = TRUE;
    }
    msgxy->x = (getmaxx(win) - (len + 2)) / 2;
    msgxy->y = (getmaxy(win) - 3) / 2;
    char spc[IOBUF + 1];
    for (int i = 0; i <= IOBUF; ++i)
        spc[i] = ' ';
    if (len > IOBUF)
        len = IOBUF;
    spc[len] = '\0';
    for (int yy = msgxy->y - 1; yy <= msgxy->y + 1; yy++)
        mvwprintw(win, yy, msgxy->x, spc);
    mvwprintw(win, msgxy->y, msgxy->x, msg);
    if (autofree)
        free(msgxy);
    if (pause) {
        flushinp();
        wrefresh(win);
        halfdelay(10);
        wgetch(win);
        nocbreak(); /* nocbreak leaves halfdelay mode */
        cbreak();   /* but we want cbreak mode */
    }
    return len;
}

char *
scr_wmsg_read(WINDOW * win, char *msg, int readlen)
{
    if (!win || !msg)
        return 0;
    int len = strlen(msg);
    int maxx = getmaxx(win);
    int mxl = (IOBUF > maxx ? maxx : IOBUF);
    if (len >= mxl)
        return 0;
    if (!readlen)
        readlen = mxl - len;
    if (readlen < 0)
        return 0;
    int tlen = len + readlen;
    if (tlen > mxl)
        return 0;
    wattrset(win, COLOR_PAIR((win == game_win ? COL_G_TXT : COL_I_TXT)));
    struct scrxy msgxy;
    msgxy.x = (maxx - (tlen + 2)) / 2;
    msgxy.y = (getmaxy(win) - 3) / 2;
    char spc[mxl];
    for (int i = 0; i <= mxl; ++i)
        spc[i] = ' ';
    spc[tlen] = '\0';
    for (int yy = msgxy.y - 1; yy <= msgxy.y + 1; yy++)
        mvwprintw(win, yy, msgxy.x, spc);
    mvwprintw(win, msgxy.y, msgxy.x, msg);
    char buf[mxl];
    wmove(game_win, msgxy.y, msgxy.x + len);
    echo();
    wgetnstr(game_win, buf, readlen);
    noecho();
    char *ret = malloc(strlen(buf) + 1);
    strcpy(ret, buf);
    return ret;
}

char
scr_menu(WINDOW *win,
    char** menu, int count, int* shortcuts,
    int select, int *restore)
{
    int w = 0;
    for (int i = 0; i < count; ++i){
        int l = strlen(menu[i]);
        if (shortcuts[i] & MENU_SHORTCUT_NUMERIC)
            l += 4;     /* for adding '12. '    */
        else if (shortcuts[i] & 0xff)
            l += 5;     /* for adding ' k - '   */
        if (l > w)
            w = l;
    }
    w += 2;
    int y = (getmaxy(win) - count - 1) / 2;
    char spcs[w + 1];
    for (int i = 0; i < w; ++i)
        spcs[i] = ' ';
    spcs[w] = '\0';
    char* opts[count];
    /*  copy menu items text into opts, padding with spaces and
        showing shortcut keys if any.
    */
    for (int i = 0; i < count; ++i){
        int l = strlen(menu[i]);
        opts[i] = malloc((w + 1) * sizeof(char));
        strcpy(opts[i], spcs);
        if (shortcuts[i] & MENU_HIDDEN)
            continue;
        else if (menu[i][0] == '\0')
            shortcuts[i] |= MENU_BLANK;
        else if (shortcuts[i] & MENU_SHORTCUT_NUMERIC) {
            snprintf(opts[i], l + 7,
                " %2d. %s", shortcuts[i] & 0xff, menu[i]);
            opts[i][l + 5] = ' ';
        }
        else if ((shortcuts[i] & 0xff) > 32) {
            snprintf(opts[i], l + 7,
                "  %c - %s", shortcuts[i] & 0xff, menu[i]);
            opts[i][l + 6] = ' ';
        }
        else
            strncpy(opts[i] + 1, menu[i], l);
    }
    /* make sure select is a valid menu item */
    while (shortcuts[select] & MENU_NONSELECT_MASK){
        if (++select == count)
            select = 0;
    }
    int x = (getmaxx(win) - w) / 2;
    nodelay(win, FALSE);

    int colp, colp_hi, colp_dis, colp_status;
    if (win == game_win){
        colp = COL_G_TXT;
        colp_hi = COL_G_TXT_HI;
        colp_dis = COL_G_TXT_DISABLED;
        colp_status = COL_G_TXT_STATUS;
    }
    else {
        colp = COL_I_TXT;
        colp_hi = COL_I_TXT_HI;
        colp_dis = COL_I_TXT_DISABLED;
        colp_status = COL_I_TXT_STATUS;
    }

    wattrset(win, COLOR_PAIR(colp));
    do {
        int oy = 0;
        if (y > 1) {
            wattrset(win, COLOR_PAIR(colp));
            mvwprintw(win, y - 1, x, spcs);
        }
        for (int i = 0; i < count; i++){
            if (!(shortcuts[i] & MENU_HIDDEN)) {
                if (i == select)
                    wattrset(win, COLOR_PAIR(colp_hi));
                else if (shortcuts[i] & MENU_DISABLED)
                    wattrset(win, COLOR_PAIR(colp_dis));
                else if (shortcuts[i] & MENU_STATUS)
                    wattrset(win, COLOR_PAIR(colp_status));
                else
                    wattrset(win, COLOR_PAIR(colp));
                mvwprintw(win, y + oy, x, opts[i]);
                ++oy;
                if (shortcuts[i] & MENU_LABEL) {
                    wattrset(win, COLOR_PAIR(colp));
                    mvwprintw(win, y + oy, x, spcs);
                    ++oy;
                }
            }
        }
        if (y > 1) {
            wattrset(win, COLOR_PAIR(colp));
            mvwprintw(win, y + oy, x, spcs);
        }
        int key = wgetch(win);
        if (key != ERR) {
            switch (key) {
            case KEY_RESIZE:
                screen_resize();
                y = (getmaxy(win) - count) / 2;
                x = (getmaxx(win) - w) / 2;
                break;
            case '\'':
            case KEY_UP:
                if (select > 0)
                    for (int i = select - 1; i >= 0; --i)
                        if (!(shortcuts[i] & MENU_NONSELECT_MASK)) {
                            select = i;
                            break;
                        }
                break;
            case '/':
            case KEY_DOWN:
                if (select < count)
                    for (int i = select + 1;  i < count; ++i)
                        if (!(shortcuts[i] & MENU_NONSELECT_MASK)) {
                            select = i;
                            break;
                        }
                break;
            case '\r':
                if (restore)
                    *restore = select;
                for (int n = 0; n < count; ++n)
                    free(opts[n]);
                return select;
            default:
                for (int i = 0; i < count; ++i){
                    int k = shortcuts[i];
                    if (k & MENU_NONSHORTCUT_MASK)
                        continue;
                    if (k & MENU_SHORTCUT_NUMERIC)
                        k = ((k & 0x00ff) < 10) ? '0' + k : 0;
                    if (k & MENU_SHORTCUT_ACTIVATES)
                        if (key == (k & 0x00ff)) {
                            if (restore)
                                *restore = select;
                            for (int n = 0; n < count; ++n)
                                free(opts[n]);
                            return i;
                        }
                    if (key == (k & 0x00ff)) {
                        select = i;
                        if (shortcuts[i] & MENU_HIDDEN)
                            select--;
                        break;
                    }
                }
            }
        }
    } while (1);
}

