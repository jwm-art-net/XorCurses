#include "icons.h"


struct xor_icon icons[ICON_XXX] = {
    {
     COLOR_BLACK,
     COLOR_WHITE,
     {
      {' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' '}
      },
     " "},
    {
     COLOR_RED,
     COLOR_WHITE,
     {
      {'|', '_', '_', '_', '_'},
      {'_', '|', '_', '_', '_'},
      {'_', '_', '_', '|', '_'}
      },
     "Wall"},
    {
     COLOR_BLACK,
     COLOR_CYAN,
     {
      {'<', '-', '>', '<', '>'},
      {'>', '<', '-', '>', '<'},
      {'<', '>', '<', '-', '>'}
      },
     "H Force Field"},
    {
     COLOR_BLACK,
     COLOR_RED,
     {
      {'^', '|', 'V', '^', 'V'},
      {'|', 'V', '^', '|', '^'},
      {'V', '^', '|', 'V', '|'}
      },
     "V Force Field"},
    {
     COLOR_BLACK,
     COLOR_BLUE,
     {
      {' ', '_', '_', '_', ' '},
      {'(', 'o', '\\', 'o', ')'},
      {' ', '\\', 'v', '/', ' '}
      },
     "Mask"},
    {
     COLOR_BLACK,
     COLOR_GREEN,
     {
      {' ', '\\', '|', '/', ' '},
      {'\\', 'u', 'U', 'u', '/'},
      {' ', 'o', '!', '/', ' '}
      },
     "Fish"},
    {
     COLOR_BLACK,
     COLOR_WHITE,
     {
      {'<', '0', ')', ' ', ' '},
      {'(', '\\', '|', '/', '/'},
      {' ', '/', ' ', '\\', ' '}
      },
     "Chicken"},
    {
     COLOR_BLACK,
     COLOR_WHITE,
     {
      {' ', '_', '_', '_', '*'},
      {'/', ' ', 'H', ' ', '\\'},
      {'\\', '_', '_', '_', '/'}
      },
     "H Bomb"},
    {
     COLOR_BLACK,
     COLOR_GREEN,
     {
      {'+', '-', '-', '-', '+'},
      {'!', ' ', 'V', ' ', '!'},
      {'+', '-', '-', '-', '+'}
      },
     "V Bomb"},
    {
     COLOR_BLACK,
     COLOR_RED,
     {
      {' ', '+', 'O', '+', ' '},
      {'/', '(', '_', ')', '\\'},
      {' ', '/', ' ', '\\', ' '}
      },
     "Doll"},
    {
     COLOR_BLACK,
     COLOR_BLUE,
     {
      {' ', '_', '_', '_', ' '},
      {'(', 'O', '|', 'O', ')'},
      {'|', '/', '~', '\\', '|'}
      },
     "Light Switch"},
    {
     COLOR_WHITE,
     COLOR_BLACK,
     {
      {'-', '|', '-', '-', '-'},
      {'-', '-', '@', '-', '|'},
      {'|', '@', '-', '|', 'x'}
      },
     "Map"},
    {
     COLOR_WHITE,
     COLOR_BLACK,
     {
      {'[', '=', '=', '=', ']'},
      {'[', '=', '=', 'X', ']'},
      {'[', '=', '=', '=', ']'}
      },
     "Exit"},
    {
     COLOR_RED,
     COLOR_BLUE,
     {
      {'B', ' ', '|', ' ', 'M'},
      {'-', '-', '+', '-', '-'},
      {'U', ' ', '|', ' ', 'S'}
      },
     "Teleport"},
    {
     COLOR_RED,
     COLOR_YELLOW,
     {
      {'+', '=', '=', '=', '+'},
      {'|', '^', 'Q', '^', '|'},
      {'\\', '=', '=', '=', '/'}
      },
     "Questor"},
    {
     COLOR_BLUE,
     COLOR_WHITE,
     {
      {'+', '=', '=', '=', '+'},
      {'|', '/', 'M', '\\', '|'},
      {'\\', '=', '=', '=', '/'}
      },
     "Magus"},
    {
     COLOR_BLACK,
     COLOR_YELLOW,
     {
      {' ', '\\', '^', '/', ' '},
      {' ', '<', 'X', '>', ' '},
      {' ', '/', 'V', '\\', ' '}
      },
     "Explosion1"},
    {
     COLOR_YELLOW,
     COLOR_RED,
     {
      {'\\', 'x', '|', 'x', '/'},
      {'<', '-', 'O', '-', '>'},
      {'/', 'x', '|', 'x', '\\'}
      },
     "Explosion2"},
    {
     COLOR_BLACK,
     COLOR_RED,
     {
      {'\'', '\'', ' ', '\'', '\''},
      {'-', ' ', ' ', ' ', '-'},
      {'.', '.', ' ', '.', '.'}
      },
     "Explosion3"}
};


struct xor_icon wall_icons[4] = {
    {
     COLOR_RED,
     COLOR_WHITE,
     {
      {'|', '_', '_', '_', '_'},
      {'_', '|', '_', '_', '_'},
      {'_', '_', '_', '|', '_'}
      },
     "Wall1"},
    {
     COLOR_RED,
     COLOR_WHITE,
     {
      {'\\', ',', '\'', '/', ','},
      {'`', '\\', '/', ',', '\\'},
      {',', '/', '\\', '/', '`'}
      },
     "Wall2"},
    {
     COLOR_RED,
     COLOR_WHITE,
     {
      {'o', '(', ')', 'O', 'o'},
      {'O', 'o', 'O', '(', ')'},
      {'(', ')', 'o', 'O', 'o'}
      },
     "Wall3"},
    {
     COLOR_RED,
     COLOR_WHITE,
     {
      {'+', '|', '-', '+', '|'},
      {'-', '+', '|', '-', '+'},
      {'|', '-', '+', '|', '-'}
      },
     "Wall4"}
};


void init_icons(void)
{
    for (su_t i = 1; i < ICON_XXX; i++)
        init_pair(i, icons[i].fg, icons[i].bg);
}


void init_wall(lvl_t level, bool show)
{
    NCURSES_COLOR_T bg[6] = {
        COLOR_RED,
        COLOR_BLUE,
        COLOR_CYAN,
        COLOR_MAGENTA,
        COLOR_GREEN,
        COLOR_BLACK,
    };
    NCURSES_COLOR_T fg[6] = {
        COLOR_WHITE,
        COLOR_CYAN,
        COLOR_BLACK,
        COLOR_RED,
        COLOR_MAGENTA,
        COLOR_BLACK
    };
    su_t wall = (level + 3) % 4;

    su_t cix = (show ? (!level ? 4 : level / 4) : 5);

    init_pair(ICON_WALL, fg[cix], bg[cix]);
    for (xy_t y = 0; y < ICON_H; y++)
        for (xy_t x = 0; x < ICON_W; x++)
            icons[ICON_WALL].chrs[y][x] =
                (show) ? wall_icons[wall].chrs[y][x] : ' ';
}


void win_icon_dump(WINDOW * win, xy_t x, xy_t y, su_t icon)
{
    wattrset(win, COLOR_PAIR(icon));
    for (xy_t yy = 0; yy < ICON_H; yy++)
        for (xy_t xx = 0; xx < ICON_W; xx++)
            mvwaddch(win, y + yy, x + xx, icons[icon].chrs[yy][xx]);
}


su_t mapchar_to_icon(char c)
{
    switch (c) {
    case ' ':
        return ICON_SPACE;
    case '#':
        return ICON_WALL;
    case '-':
        return ICON_H_FIELD;
    case '|':
        return ICON_V_FIELD;
    case '@':
        return ICON_MASK;
    case '!':
        return ICON_FISH;
    case '<':
        return ICON_CHICKEN;
    case 'o':
        return ICON_H_BOMB;
    case 'x':
        return ICON_V_BOMB;
    case 'D':
        return ICON_DOLL;
    case 'S':
        return ICON_SWITCH;
    case 'M':
        return ICON_MAP;
    case 'E':
        return ICON_EXIT;
    case '+':
        return ICON_TELEPORT;
    case '1':
        return ICON_PLAYER0;
    case '2':
        return ICON_PLAYER1;
    }
    return ICON_SPACE;
}


char icon_to_mapchar(su_t icon)
{
    switch (icon) {
    case ICON_SPACE:
        return ' ';
    case ICON_WALL:
        return '#';
    case ICON_H_FIELD:
        return '-';
    case ICON_V_FIELD:
        return '|';
    case ICON_MASK:
        return '@';
    case ICON_FISH:
        return '!';
    case ICON_CHICKEN:
        return '<';
    case ICON_H_BOMB:
        return 'o';
    case ICON_V_BOMB:
        return 'x';
    case ICON_DOLL:
        return 'D';
    case ICON_SWITCH:
        return 'S';
    case ICON_MAP:
        return 'M';
    case ICON_EXIT:
        return 'E';
    case ICON_TELEPORT:
        return '+';
    case ICON_PLAYER0:
        return '1';
    case ICON_PLAYER1:
        return '2';
    }
    return 0;
}

