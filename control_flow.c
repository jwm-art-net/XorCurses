#include "control_flow.h"
#include "play_xor.h"
#include "replay.h"


#include "debug.h"


void control_flow(int level)
{
    int flow = (level & FLOW_LOAD_REPLAY) ? FLOW_LOAD_REPLAY : FLOW_START;

    while(1) {
        if (flow == FLOW_LOAD_REPLAY) {
            if ((level = replay_load()) == 999)
                return;
            debug("We're here!\n");
            flow = replay_xor(level | FLOW_START);
        }
        else {
            flow = play_xor(level | flow);
            if (flow == FLOW_DO_QUIT)
                return;
        }

        flow = replay_menu(flow);
        if (flow == FLOW_DO_QUIT)
            return;
    }

}

#if DEBUG
void control_flow_print(int flow)
{
    EBIT_MSG(flow, FLOW_LOAD_REPLAY)
    EBIT_MSG(flow, FLOW_START)
    EBIT_MSG(flow, FLOW_DO_REPLAY)
    EBIT_MSG(flow, FLOW_DO_GAME)
    EBIT_MSG(flow, FLOW_INTERUPT_BREAK)
    EBIT_MSG(flow, FLOW_INTERUPT_MENU)
    EBIT_MSG(flow, FLOW_DEATH)
    EBIT_MSG(flow, FLOW_COMPLETE)
    EBIT_MSG(flow, FLOW_DO_QUIT)
}
#endif
