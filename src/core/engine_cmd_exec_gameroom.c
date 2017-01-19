#include <stdlib.h>
#include <stdio.h>

#include "engine.h"

#include "../logger.h"

int _exe_gr_msg_plain ENGINE_HANDLE_FUNC_HEADER
{   
    int clientRID;
    
    if(p_cgr == NULL){
        return ENGINE_CMDEXE_WRONG_CONTEXT;
    }
    clientRID = game_room_find_client(p_cgr, p_cli);
    
    network_command_prepare(p->p_cmd_out, NCT_MSG_PLAIN);
    network_command_append_byte(p->p_cmd_out, clientRID);
    network_command_append_str(p->p_cmd_out, sc->str);
    
    netadapter_broadcast_command_p(p->p_netadapter, p_cgr->clients, GAME_ROOM_MAX_PLAYERS, p->p_cmd_out);
}

int _exe_gr_msg_rcon ENGINE_HANDLE_FUNC_HEADER {
    return ENGINE_CMDEXE_ILLEGAL_OP;
}

int _exe_gr_sync_phase ENGINE_HANDLE_FUNC_HEADER {
    if(p_cgr == NULL){
        return ENGINE_CMDEXE_WRONG_CONTEXT;
    }
    
    network_command_prepare(p->p_cmd_out, NCT_ROOM_SYNC_PHASE);
    network_command_append_byte(p->p_cmd_out, p_cgr->state);
    engine_send_command(p, p_cli, p->p_cmd_out);
    
    return 0;
}

int _exe_gr_ready_state ENGINE_HANDLE_FUNC_HEADER {
    int new_ready_state;
    int clientRID;
    if(sc->length < 2){
        return ENGINE_CMDEXE_DATA_TOO_SHORT;
    }
    if(p_cgr == NULL){
        return ENGINE_CMDEXE_WRONG_CONTEXT;
    }
    
    clientRID = game_room_find_client(p_cgr, p_cli);
    if(clientRID == ITEM_EMPTY){
        return ENGINE_CMDEXE_WRONG_CONTEXT;
    }
    
    new_ready_state = strsc_byte(sc);
    if(new_ready_state == p_cgr->ready_state[clientRID]){
        return 0;
    }
    
    network_command_prepare(p->p_cmd_out, NCT_ROOM_READY_STATE);
    network_command_append_byte(p->p_cmd_out, new_ready_state);
    network_command_append_byte(p->p_cmd_out, clientRID);
    engine_bc_command(p, p_cgr, p->p_cmd_out);
    
    if(game_room_is_everyone_ready(p_cgr)){
        engine_game_room_begin(p, p_cgr);
    }
    
    return 0;
}



void _engine_init_gameroom_commands(int (**actions)ENGINE_HANDLE_FUNC_HEADER){
    actions[NCT_MSG_PLAIN] = &_exe_gr_msg_plain;
//    actions[NCT_MSG_RCON] = &_exe_gr_msg_rcon;
    actions[NCT_ROOM_SYNC_PHASE] = &_exe_gr_sync_phase;
    actions[NCT_ROOM_READY_STATE] = &_exe_gr_ready_state;
    
    
    
    
    glog(LOG_WARNING, "NOT IMPLEMENTED YET");
}
