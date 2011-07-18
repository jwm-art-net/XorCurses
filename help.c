#include "help.h"

#include "options.h"
#include "screen.h"
#include "game_display.h"

#include <string.h>
#include <stdlib.h>

enum {
    HELP_MENU_TITLE,
    HELP_MENU_CANCEL,
    HELP_MENU_GAME,
    HELP_MENU_KEYS,
    HELP_MENU_OBJECTS,
    HELP_MENU_QUIT,
    HELP_MENU_QUIT_ALT,
    HELP_MENU_COUNT
};

enum {
    HELP_RET_QUIT,
    HELP_RET_CANCEL,
    HELP_RET_MENU
};

enum {
    HELP_WIDTH = 42
};

enum {
    /*  or this to the help item to be displayed    */
    HELP_MENU_CANCELLED = 0x0100
};

static char** helpmenu = 0;
static int shortcuts[HELP_MENU_COUNT];

static int return_to_row = 0;

int help_show(int item);

void help_menu_create()
{
    helpmenu = malloc(sizeof(char*) * HELP_MENU_COUNT);
    helpmenu[HELP_MENU_TITLE] =     "Help Menu";
    shortcuts[HELP_MENU_TITLE] =    0 | MENU_LABEL;

    helpmenu[HELP_MENU_CANCEL] =    "Cancel";
    shortcuts[HELP_MENU_CANCEL] =   'c' | MENU_SHORTCUT_ACTIVATES;

    helpmenu[HELP_MENU_GAME] =      "Game Play";
    shortcuts[HELP_MENU_GAME] =     'g' | MENU_SHORTCUT_ACTIVATES;

    helpmenu[HELP_MENU_KEYS] =      "Keys";
    shortcuts[HELP_MENU_KEYS] =     'k' | MENU_SHORTCUT_ACTIVATES;

    helpmenu[HELP_MENU_OBJECTS] =   "Objects";
    shortcuts[HELP_MENU_OBJECTS] =  'o' | MENU_SHORTCUT_ACTIVATES;

    helpmenu[HELP_MENU_QUIT] =      "Quit Help";
    shortcuts[HELP_MENU_QUIT] =     'q' | MENU_SHORTCUT_ACTIVATES;

    helpmenu[HELP_MENU_QUIT_ALT] =  "";
    shortcuts[HELP_MENU_QUIT_ALT]
        = 'h' | MENU_SHORTCUT_ACTIVATES | MENU_HIDDEN;
}

void help_menu_destroy()
{
    if (helpmenu)
        free(helpmenu);
}

void help_menu_repaint()
{
    wattrset(info_win, COLOR_PAIR(0));
    box(info_win, 0 ,0);
}

void help_menu_config_for(int item)
{
    if (!item)
        shortcuts[HELP_MENU_CANCEL] |= MENU_DISABLED | MENU_HIDDEN;
    else
        shortcuts[HELP_MENU_CANCEL] &= ~ (MENU_DISABLED | MENU_HIDDEN);

    if (item == HELP_MENU_GAME)
        shortcuts[HELP_MENU_GAME] |= MENU_DISABLED | MENU_HIDDEN;
    else
        shortcuts[HELP_MENU_GAME] &= ~ (MENU_DISABLED | MENU_HIDDEN);

    if (item == HELP_MENU_KEYS)
        shortcuts[HELP_MENU_KEYS] |= MENU_DISABLED | MENU_HIDDEN;
    else
        shortcuts[HELP_MENU_KEYS] &= ~ (MENU_DISABLED | MENU_HIDDEN);

    if (item == HELP_MENU_OBJECTS)
        shortcuts[HELP_MENU_OBJECTS] |= MENU_DISABLED | MENU_HIDDEN;
    else
        shortcuts[HELP_MENU_OBJECTS] &= ~ (MENU_DISABLED | MENU_HIDDEN);
}

void
help_menu()
{
    help_menu_config_for(0);
    int select = 0;
    int back = 0;
    int reselect = HELP_RET_MENU;
    return_to_row = 0;
    do {
        if (reselect == HELP_RET_QUIT)
            return;
        else if (reselect == HELP_RET_MENU) {
            back = select & 0x00ff;
            select = scr_menu(game_win,
                              helpmenu, HELP_MENU_COUNT, shortcuts,
                              back, 0);
        }
        switch(select & 0x00ff){
        case HELP_MENU_QUIT:
        case HELP_MENU_QUIT_ALT:
            return;
        case HELP_MENU_CANCEL:
            select = back | HELP_MENU_CANCELLED;
            reselect = HELP_RET_CANCEL;
            break;
        case HELP_MENU_GAME:
        case HELP_MENU_KEYS:
        case HELP_MENU_OBJECTS:
            reselect = help_show(select);
            break;
        }
    } while (1);
}

char** read_help_text(int item)
{
    char* filename = 0;
    switch(item) {
    case HELP_MENU_GAME:
        filename = "help_game.txt";
        break;
    case HELP_MENU_KEYS:
        filename = "help_keys.txt";
        break;
    case HELP_MENU_OBJECTS:
        filename = "help_objects.txt";
        break;
    default:
        return 0;
    }
    char *hf = options_file_path(filename, options->data_dir);
    if (!hf)
        return 0;
    FILE *fp = fopen(hf, "r");
    free(hf);
    if (!fp)
        return 0;
    int row = 0;
    char buf[HELP_WIDTH];
    while (fgets(buf, HELP_WIDTH, fp)) ++row;
    char **help = malloc((row + 1) * sizeof(char*));
    if (!help)
        return 0;
    fseek(fp, 0, SEEK_SET);
    for (int i = 0; i < row; ++i) {
        fgets(buf, HELP_WIDTH, fp);
        help[i] = malloc((strlen(buf) + 1) * sizeof(char));
        strcpy(help[i], buf);
    }
    help[row] = 0;
    fclose(fp);
    return help;
}

void help_text_destroy(char** help)
{
    for (int i = 0; help[i] != 0; ++i)
        free(help[i]);
    free(help);
}

int help_show(int pitem)
{
    int item = pitem & 0x00ff;
    char** help;
    switch(item) {
    case HELP_MENU_GAME:
    case HELP_MENU_KEYS:
    case HELP_MENU_OBJECTS:
        help_menu_config_for(item);
        help = read_help_text(item);
        break;
    default:
        return HELP_RET_MENU;
    }
    if (!help)
        return HELP_RET_MENU;
    int row = 0;
    while(help[++row]);
    xy_t maxrows = row - 1;
    xy_t h = screen_data->garea_h * ICON_H;
    xy_t indx = screen_data->garea_w * ICON_W - 4;
    row = (pitem & HELP_MENU_CANCELLED) ? return_to_row : 0;
    wclear(game_win);
    wattrset(game_win, COLOR_PAIR(0));
    int key;
    void (*cur_repaint_cb) () = screen_data->game_win_repaint_cb;
    screen_data->game_win_repaint_cb = 0;
    int x = (screen_data->garea_w * ICON_W - HELP_WIDTH) / 2;
    do {
        wattrset(game_win, COLOR_PAIR(0));
        for (xy_t r = 0; r < h; r++)
            if (r + row < maxrows)
                mvwprintw(game_win, r, x, help[r + row]);
        if (item == HELP_MENU_OBJECTS)
            for (su_t i = 0; i < 12; i++)
                game_win_icon_dump(x, i * 4 - row, i + ICON_H_FIELD);
        if (row > 0) {
            wattrset(game_win, COLOR_PAIR(COL_G_TXT_STATUS));
            mvwprintw(game_win, 0, indx, "/|\\");
        }
        if (row + h < maxrows) {
            wattrset(game_win, COLOR_PAIR(COL_G_TXT_STATUS));
            mvwprintw(game_win, h - 1, indx, "\\|/");
        }
        switch ((key = wgetch(game_win))) {
        case 'h':
        case 'H':
            return_to_row = row;
            help_text_destroy(help);
            (screen_data->game_win_repaint_cb = cur_repaint_cb)();
            return HELP_RET_MENU;
        case 'q':
        case 'Q':
            help_text_destroy(help);
            (screen_data->game_win_repaint_cb = cur_repaint_cb)();
            return HELP_RET_QUIT;
        case KEY_RESIZE:
            screen_resize();
            xy_t oh = h;
            h = screen_data->garea_h * ICON_H;
            if (h > oh)
                if (row + h > maxrows)
                    row -= (row + h) - maxrows;
            break;
        case KEY_UP:
        case '\'':
            if (row > 0)
                row--;
            break;
        case KEY_DOWN:
        case '/':
            if (row + h < maxrows)
                row++;
            break;
        case KEY_PPAGE:
            row -= (h - 1);
            if (row < 0)
                row = 0;
            break;
        case KEY_NPAGE:
            row += (h - 1);
            if (row > maxrows - h)
                row = maxrows - h;
            break;
        }
    } while (1);
}

