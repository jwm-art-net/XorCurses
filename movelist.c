#include "movelist.h"

#include "map.h"

#include <stdlib.h>

struct xmv_list *xmvlist = 0;

struct xmv_list *
xmvlist_create()
{
    if (xmvlist)
        xmvlist_destroy();
    if (!(xmvlist = malloc(sizeof(struct xmv_list))))
        return 0;
    xmvlist->first = xmvlist->last = xmvlist->current = 0;
#ifdef DEBUG
    fprintf(stderr, "created xmvlist=%lx\n", (unsigned long) xmvlist);
#endif
    return xmvlist;
}

void
xmvlist_destroy()
{
    xmvlist->current = xmvlist->first;
    while (xmvlist->current) {
        struct xmv_link *tmp = xmvlist->current;

        xmvlist->current = tmp->next;
#ifdef DEBUG
        fprintf(stderr, "destroyed xmv_link %lx\n", (unsigned long) tmp);
#endif
        free(tmp);
    }
#ifdef DEBUG
    fprintf(stderr, "destroyed xmvlist %lx\n", (unsigned long) xmvlist);
#endif
    free(xmvlist);
    xmvlist = 0;
}

struct xmv_link *
xmvlist_first()
{
    return xmvlist->current = xmvlist->first;
}

struct xmv_link *
xmvlist_last()
{
    return xmvlist->current = xmvlist->first;
}

struct xmv_link *
xmvlist_prev()
{
    if (xmvlist->current)
        if (xmvlist->current->prev)
            return (xmvlist->current = xmvlist->current->prev);
    return xmvlist->current = 0;
}

struct xmv_link *
xmvlist_next()
{
    if (xmvlist->current)
        if (xmvlist->current->next)
            return (xmvlist->current = xmvlist->current->next);
    return xmvlist->current = 0;
}

struct xmv_link *
xmvlist_cycle_prev()
{
    if (xmvlist->current) {
        if (xmvlist->current == xmvlist->first)
            return (xmvlist->current = xmvlist->last);
        else
            return (xmvlist->current = xmvlist->current->prev);
    }
    return 0;
}

struct xmv_link *
xmvlist_cycle_next()
{
    if (xmvlist->current) {
        if (xmvlist->current == xmvlist->last)
            return (xmvlist->current = xmvlist->first);
        else
            return (xmvlist->current = xmvlist->current->next);
    }
    return 0;
}

struct xmv_link *
xmvlist_current()
{
    return xmvlist->current;
}

struct xmv_link *
xmvlist_append_xor_move(struct xor_move *xmv)
{
    struct xmv_link *lnk = 0;

    if (!(lnk = malloc(sizeof(struct xmv_link))))
        return 0;
    lnk->xmv = xmv;
    lnk->next = 0;
    if (!xmvlist->first) {
        xmvlist->first = xmvlist->last = xmvlist->current = lnk;
        lnk->prev = 0;
#ifdef DEBUG
        fprintf(stderr, "created xmv_link %lx for xor_move %lx\n",
               (unsigned long) lnk, (unsigned long) xmv);
#endif
        return lnk;
    }
    xmvlist->last->next = lnk;
    lnk->prev = xmvlist->last;
    xmvlist->last = lnk;
#ifdef DEBUG
    fprintf(stderr, "created xmv_link %lx for xor_move %lx\n",
           (unsigned long) lnk, (unsigned long) xmv);
#endif
    return lnk;
}

struct xor_move *
xmvlist_unlink_xor_move(struct xmv_link *lnk)
{
#ifdef DEBUG
    fprintf(stderr, "xor_move* xmvlist_unlink_xor_move(xmv_link* lnk=%lx)\n",
           (unsigned long) lnk);
    if (lnk == xmvlist->current)
        fprintf(stderr, "\tunlinking xmvlist->current\n");
#endif
    struct xor_move *xmv = lnk->xmv;

    if (lnk == xmvlist->first) {
        if (xmvlist->first == xmvlist->last) {
            xmvlist->first = xmvlist->last = xmvlist->current = 0;
#ifdef DEBUG
            fprintf(stderr, "destroyed xmv_link (first/last) %lx for xor_move %lx\n",
                   (unsigned long) lnk, (unsigned long) xmv);
#endif
            free(lnk);
            return xmv;
        }
        if (xmvlist->current == lnk)
            xmvlist->current = lnk->next;
        xmvlist->first = lnk->next;
        xmvlist->first->prev = 0;
#ifdef DEBUG
        fprintf(stderr, "destroyed xmv_link (first) %lx for xor_move %lx\n",
               (unsigned long) lnk, (unsigned long) xmv);
#endif
        free(lnk);
        return xmv;
    }
    if (lnk == xmvlist->last) {
        if (xmvlist->current == lnk)
            xmvlist->current = xmvlist->first;
        xmvlist->last = lnk->prev;
        xmvlist->last->next = 0;
#ifdef DEBUG
        fprintf(stderr, "destroyed xmv_link (last) %lx for xor_move %lx\n",
               (unsigned long) lnk, (unsigned long) xmv);
#endif
        free(lnk);
        return xmv;
    }
    struct xmv_link *tmp = lnk->next;

    if (xmvlist->current == lnk)
        xmvlist->current = tmp;
    tmp->prev = lnk->prev;
    tmp = lnk->prev;
    tmp->next = lnk->next;
#ifdef DEBUG
    fprintf(stderr, "destroyed xmv_link %lx for xor_move %lx\n",
           (unsigned long) lnk, (unsigned long) xmv);
#endif
    free(lnk);
    return xmv;
}

struct xmv_link *xmvlist_contains_coord
    (xy_t x, xy_t y, struct xmv_link *stop_at, struct xor_move **res_prev)
{
    struct xmv_link *tmp = xmvlist->current;

    struct xmv_link *result = 0;

    xmvlist_first();
    while ((result = xmvlist->current) && xmvlist->current != stop_at) {
        struct xor_move *rpxmv = 0;

        struct xor_move *xmv = xmvlist->current->xmv;

        switch (xmv->dir) {
        case MV_LEFT:
            if (xmv->from_y == y && xmv->from_x <= x) {
                while (xmv) {
                    if (xmv->from_x == x) {
                        xmvlist->current = tmp;
                        if (res_prev)
                            *res_prev = rpxmv;
                        return result;
                    }
                    rpxmv = xmv;
                    xmv = xmv->chain;
                }
            }
            break;
        case MV_DOWN:
            if (xmv->from_x == x && xmv->from_y <= y) {
                while (xmv) {
                    if (xmv->from_y == y) {
                        xmvlist->current = tmp;
                        if (res_prev)
                            *res_prev = rpxmv;
                        return result;
                    }
                    rpxmv = xmv;
                    xmv = xmv->chain;
                }
            }
            break;
        default:
            break;
        }
        xmvlist_next();
    }
    if (res_prev)
        res_prev = 0;
    xmvlist->current = tmp;
    return 0;
}
