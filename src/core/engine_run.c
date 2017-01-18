#include <string.h>

#include "engine.h"

#include "../localisation.h"
#include "../logger.h"

int _engine_process_command(engine *p, net_client *p_cli, network_command cmd) {
    int(* handle_action) ENGINE_HANDLE_FUNC_HEADER;
    str_scanner scanner;

    memset(p->p_cmd_out, 0, sizeof (network_command));

    if (cmd.type < 0 || cmd.type > NETWORK_COMMAND_TYPES_COUNT) {
        network_command_prepare(&p_cli->connection->_out_buffer, NCT_LEAD_DISCONNECT);
        snprintf(p_cli->connection->_out_buffer.data, NETWORK_COMMAND_DATA_LENGTH,
                g_loc.server_protection_illegal_cmd_type, cmd.type);
        engine_send_command(p, p_cli, &p_cli->connection->_out_buffer);
        return 1;
    }

    handle_action = p->command_proccess_func[cmd.type];
    if (handle_action == NULL) {
        glog(LOG_WARNING, "Engine: No handle action for %d", cmd.type);

        network_command_prepare(&p_cli->connection->_out_buffer, NCT_LEAD_DISCONNECT);
        snprintf(p_cli->connection->_out_buffer.data, NETWORK_COMMAND_DATA_LENGTH,
                g_loc.server_protection_unimplemented_cmd_type, cmd.type);
        engine_send_command(p, p_cli, &p_cli->connection->_out_buffer);

        return 2;
    }

    str_scanner_set(&scanner, cmd.data, cmd.length);

    if (handle_action(p, p_cli, &scanner)) {
        netadapter_handle_invallid_command(p->p_netadapter, p_cli, cmd);
        p->p_netadapter->stats->commands_received_invalid++;
        if (p_cli->connection->invalid_counter > p->p_netadapter->ALLOWED_INVALLID_MSG_COUNT) {
            return 3;
        }
    }

    return 0;
}

void _engine_process_queue(engine *p) {
    network_command cmd;

    while (!cmd_queue_is_empty(&p->cmd_in_queue)) {
        cmd = cmd_queue_get(&p->cmd_in_queue);
        net_client *p_cli = netadapter_get_client_by_aid(&p->netadapter, cmd.client_aid);

        int ret_val = _engine_process_command(p, p_cli, cmd);
        if (ret_val) {
            glog(LOG_FINE, "Engine: Closing connection on socket %02d, reason = %d", p_cli->connection->socket, ret_val);
            netadapter_close_connection_by_client(p->p_netadapter, p_cli);
        }
    }
}

void _engine_check_idle_client(engine *p, net_client *p_client, time_t now) {
    tcp_connection *p_con = p_client->connection;
    int idle_time = now - p_con->last_active;

    switch (p_client->status) {
        default:
        case NET_CLIENT_STATUS_CONNECTED:
            if (idle_time > p->netadapter.ALLOWED_IDLE_TIME) {
                network_command_strprep(&p_con->_out_buffer, NCT_LEAD_MARCO, g_loc.netcli_dcreason_unresponsive);
                netadapter_send_command(p->p_netadapter, p_con, &p_con->_out_buffer);
                p_client->status = NET_CLIENT_STATUS_UNRESPONSIVE;
            }
            break;
        case NET_CLIENT_STATUS_UNRESPONSIVE:
            if (idle_time > p->netadapter.ALLOWED_UNRESPONSIVE_TIME) {
                netadapter_close_connection_by_client(p->p_netadapter, p_client);
            }
            break;
    }
}

void _engine_check_active_clients(engine *p) {
    int i;
    net_client *p_cli;
    for (i = 0; i < p->resources->clients_length; i++) {
        p_cli = p->resources->clients + i;

        if (p_cli->connection != NULL) { // connection is open

        } else { // connection is not open
            if (p_cli->status != NET_CLIENT_STATUS_DISCONNECTED &&
                    p_cli->status != NET_CLIENT_STATUS_EMPTY) {
                glog(LOG_FINE, "Engine: Client %d found to be disconnected", i);
                p_cli->status = NET_CLIENT_STATUS_DISCONNECTED;
            }
        }

    }
}

void *engine_run(void *args) {
    engine *p = (engine *) args;

    p->netadapter.command_handler = p;
    p->netadapter.command_handle_func = &_engine_handle_command;

    p->stats.run_start = clock();
    glog(LOG_INFO, "Engine: Starting");
    while (p->keep_running) {
        _engine_check_active_clients(p);
        _engine_process_queue(p);
        

        p->total_ticks++;
        if (p->total_ticks > p->settings->MAX_TICKRATE * 30) {
            
        }
        nanosleep(&p->sleep, NULL);
    }

    p->stats.run_end = clock();
    glog(LOG_INFO, "Engine: Finished");
    netadapter_shutdown(&p->netadapter);
    return NULL;
}