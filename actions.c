#include "actions.h"
#include "map.h"
#include "debug.h"

#include <stdlib.h>

struct xor_action actions[ICON_XXX] = {
    {MVI_NONE, CT_PASS, MV_NONE, MV_NONE},      /* space     */
    {MVI_NONE, CT_BLOCK, MV_NONE, MV_NONE},     /* wall      */
    {MVI_NONE, CT_FILTER, MV_NONE, MV_HORIZ},   /* h-field   */
    {MVI_NONE, CT_FILTER, MV_NONE, MV_VERT},    /* v-field   */
    {MVI_NONE, CT_PICKUP, MV_NONE, MV_NONE},    /* mask      */
    {MVI_GRAVITY, CT_PUSH | CT_PDEATH, MV_DOWN, MV_HORIZ},   /* fish    */
    {MVI_GRAVITY, CT_PUSH | CT_PDEATH, MV_LEFT, MV_VERT},    /* chicken */
    {MVI_GRAVITY, CT_PUSH | CT_EXPLODE | CT_PDEATH,
                            MV_DOWN, MV_HORIZ}, /* h-bomb    */
    {MVI_GRAVITY, CT_PUSH | CT_EXPLODE | CT_PDEATH,
                             MV_LEFT, MV_VERT}, /* v-bomb    */
    {MVI_NONE, CT_HARDPUSH, MV_ANY, MV_ANY},    /* doll      */
    {MVI_NONE, CT_PICKUP, MV_NONE, MV_NONE},    /* sad mask  */
    {MVI_NONE, CT_PICKUP, MV_NONE, MV_NONE},    /* map       */
    {MVI_NONE, CT_EXIT, MV_NONE, MV_NONE},      /* exit      */
    {MVI_NONE, CT_TELEPORT, MV_NONE, MV_NONE},  /* teleport  */
    {MVI_PLAYER, CT_BLOCK, MV_ANY, MV_NONE},    /* magus     */
    {MVI_PLAYER, CT_BLOCK, MV_ANY, MV_NONE}     /* questor   */
};

struct xor_move *
create_xor_move(xy_t x, xy_t y, su_t move)
{
    struct xor_move *xmv;

    if (!(xmv = malloc(sizeof(struct xor_move))))
        return 0;
    xmv->from_obj = map->buf[y][x];
    xmv->from_x = x;
    xmv->from_y = y;
    xmv->dir = move;
    xmv->to_x = xmv->to_y = 0;
    xmv->chain = 0;
    xmv->moves_count = 0;

    debug("x:%d y:%d move:%d return:%lx\n", x, y, move, (unsigned long)xmv);
    debug("xmv->from_obj=%s\n", icons[xmv->from_obj].name);

    return xmv;
}

/*
    the following two functions are a bit cyclical in their
    operation so think twice if you spot something seeming
    at first glance nonsensical. (ie james morris, you/me,
    we know we have a habbit of scanning our code and
    thinking wtf!?!? and then editing without realising...
    and we hate these comments which now seem obvious and
    so untidy...
*/

struct xor_move *
create_gravity_chain_xydir(xy_t x, xy_t y, su_t dir)
{
    struct xor_move *head = 0;

    struct xor_move *xmv = 0;

    struct xor_move *tmp = 0;

    debug("x:%d y:%d dir:%d\n", x, y, dir);

    do {
        int icon = map->buf[y][x];

        if (actions[icon].mvini != MVI_GRAVITY) {

            debug("..return <xor_move* head=%lx>\n", (unsigned long) head);
            return head;
        }
        if (actions[icon].mvi_dir != dir) {
            debug("..return <xor_move* head=%lx>\n", (unsigned long) head);
            return head;
        }
        if (!(xmv = malloc(sizeof(struct xor_move)))) {
            debug("..!malloc! return <xor_move* head=%lx>\n",
                   (unsigned long) head);
            return head;
        }
        if (!head)
            head = xmv;
        if (tmp)
            tmp->chain = xmv;
        xmv->from_obj = icon;
        xmv->from_x = x;
        xmv->from_y = y;
        xmv->dir = dir;
        xmv->to_x = xmv->to_y = 0;
        xmv->chain = 0;
        xmv->moves_count = 0;

        debug("\tfrom_obj=%s from_x=%d from_y=%d\n", icons[icon].name,x,y);

        tmp = xmv;
        if (dir == MV_DOWN)
            y--;
        else
            x++;
    } while (TRUE);
    return head;
}

struct xor_move *
create_gravity_chain(struct xor_move *xmv)
{
    struct xor_move *head = 0;

    struct xor_move *tmp = 0;

    struct xor_move *oxmv = xmv;

    su_t icon;

    xy_t x, y;

    su_t dir;

    x = xmv->from_x;
    y = xmv->from_y;
    dir = xmv->dir;

    debug("create_gravity_chain(xor_move* xmv=%lx)\n", (unsigned long) xmv);

    do {
        icon = map->buf[y][x];
        if (actions[icon].mvini != MVI_GRAVITY) {
            if (xmv != oxmv)
                free(xmv);
            return head;
        }
        if (actions[icon].mvi_dir != dir) {
            if (xmv != oxmv)
                free(xmv);
            return head;
        }
        if (!head)
            head = xmv;
        if (tmp)
            tmp->chain = xmv;
        xmv->from_obj = icon;   /* these are redundant */
        xmv->from_x = x;        /* on the first pass   */
        xmv->from_y = y;        /* oh well...          */
        xmv->dir = dir;
        xmv->to_x = xmv->to_y = 0;
        xmv->chain = 0;
        xmv->moves_count = 0;
        tmp = xmv;
        if (dir == MV_DOWN)
            y--;
        else
            x++;
        if (!(xmv = malloc(sizeof(struct xor_move))))
            return head;
    } while (TRUE);
    return head;
}

void
destroy_gravity_chain(struct xor_move *xmv)
{
    struct xor_move *tmp;

    while (xmv) {
        tmp = xmv;
        xmv = xmv->chain;
        free(tmp);
    }
}
