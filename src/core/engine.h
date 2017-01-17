#ifndef ENGINE_H
#define ENGINE_H

#include <stdlib.h>
#include <stddef.h>

#include "resources.h"
#include "../networks/netadapter.h"
#include "../networks/network_command.h"
#include "../structures/cmd_queue.h"
#include "../settings.h"
#include "../statistics.h"
#include "str_scanner.h"

#define ENGINE_CLI_BUFFER_SIZE 24

#define ENGINE_HANDLE_FUNC_HEADER (struct engine* p, net_client *p_cli, str_scanner* sc)

typedef struct engine
{
    settings *settings;
    resources *resources;
    netadapter netadapter;
    statistics stats;

    struct timespec sleep;
    unsigned long total_ticks;

    int keep_running;
    
    cmd_queue cmd_in_queue;
    int (*command_proccess_func[NETWORK_COMMAND_TYPES_COUNT])ENGINE_HANDLE_FUNC_HEADER;
    
    network_command _cmd_out;

    netadapter *p_netadapter;
    network_command *p_cmd_out;



} engine;

int engine_init(engine *p_engine, settings *p_settings, resources *p_resources);

void _engine_init_solo_commands(int (**command_handle_func)ENGINE_HANDLE_FUNC_HEADER);
void _engine_init_game_prep_commands(int (**command_handle_func)ENGINE_HANDLE_FUNC_HEADER);
void _engine_init_game_play_commands(int (**command_handle_func)ENGINE_HANDLE_FUNC_HEADER);

void _engine_handle_command(void *handler, const network_command cmd);

void *engine_run(void *args);

void *engine_cli_run(void *args);

int engine_count_clients(engine *p, unsigned char status);

int engine_client_rid_by_client(engine *p, net_client *p_cli);

game_room *engine_room_by_client(engine *p, net_client *p_cli);

void engine_send_command(engine *p, net_client *p_cli, network_command *cmd);
void engine_bc_command(engine *p, game_room *p_gr, network_command *cmd);

#endif
