#ifndef _CONTROL_FLOW_H
#define _CONTROL_FLOW_H


/*  there's two routes to accessing the replay level

    1) selecting a level in the level menu, playing it,
        and then either quitting, completing, or dying.

    2) loading a saved replay from the level menu,
        replaying the replay and then resuming game play.

    this control_flow is to simplify other areas of the
    game code.
*/

enum FLOW
{
    FLOW_LOAD_REPLAY =      0x0010,

    FLOW_START =            0x0020,
    FLOW_CONTINUE =         0x0040,

    FLOW_CAN_PLAY =         0x0080,

    FLOW_DO_REPLAY =        0x0100,
    FLOW_DO_GAME =          0x0200,

    FLOW_INTERUPT_BREAK =   0x0400,
    FLOW_INTERUPT_MENU =    0x0800,

    FLOW_DEATH =            0x1000,
    FLOW_COMPLETE =         0x2000,

    FLOW_DO_QUIT =          0x8000,


};

void control_flow(int level);

#if DEBUG
void control_flow_print(int);
#endif

#endif
