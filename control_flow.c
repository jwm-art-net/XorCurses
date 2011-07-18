#include "control_flow.h"
#include "play_xor.h"
#include "replay.h"


void control_flow(int level)
{
    int flow = (level & FLOW_LOAD_REPLAY) ? FLOW_LOAD_REPLAY : FLOW_START;

    while(1) {
        if (flow == FLOW_LOAD_REPLAY) {
            if ((level = replay_load()) == 999)
                return;
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
