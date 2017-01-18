#include <stdlib.h>
#include <string.h>

#include "game_room.h"
#include "colors.h"
#include "player.h"
#include "../generic.h"

char game_room_status_letter(unsigned char game_state) {
    switch (game_state) {
        case GAME_ROOM_STATE_DONE:
            return 'D';
        case GAME_ROOM_STATE_IDLE:
            return 'I';
        case GAME_ROOM_STATE_LOBBY:
            return 'L';
        case GAME_ROOM_STATE_RUNNING:
            return 'W';
        case GAME_ROOM_STATE_SUMMARIZATION:
            return 'S';

    }
}

int game_room_init(game_room *p, int size, net_client *p_cli) {
    game_room_clean_up(p);

    p->state = GAME_ROOM_STATE_LOBBY;
    p->size = size > GAME_ROOM_MAX_PLAYERS ? GAME_ROOM_MAX_PLAYERS : size;
    colors_init(&p->player_colors);

    int clientRID = game_room_put_client(p, p_cli);
    p->leaderClient = clientRID;

    return clientRID;
}

void game_room_clean_up(game_room *p) {
    int i;

    for (i = 0; i < GAME_ROOM_MAX_PLAYERS; i++) {
        p->clients[i] = NULL;
        player_init(p->players + i, ITEM_EMPTY);
    }

    memset(p->tanks, 0, sizeof (tank) * GAME_ROOM_MAX_PLAYERS);
    memset(p->projectiles, 0, sizeof (projectile) * GAME_ROOM_MAX_PROJECTILES);
    p->size = 0;
    p->state = GAME_ROOM_STATE_IDLE;

}

int game_room_count_players(game_room *p) {
    int i, n = 0;

    for (i = 0; i < p->size; i++) {
        if ((p->players + i)->client_rid != ITEM_EMPTY) {
            n++;
        }
    }

    return n;
}

int game_room_get_open_player_slots(game_room *p) {
    return p->size - game_room_count_players(p);
}

int game_room_count_clients(game_room *p){
    int i, n = 0;

    for (i = 0; i < p->size; i++) {
        if (p->clients[i] != NULL) {
            n++;
        }
    }

    return n;
}

int game_room_get_open_client_slots(game_room *p) {
    return p->size - game_room_count_clients(p);
}

int game_room_find_client(game_room *p, net_client *p_cli) {
    int i;
    for (i = 0; i < p->size; i++) {
        if (p->clients[i] == p_cli) {
            return i;
        }
    }

    return -1;
}

int game_room_put_client(game_room *p_game_room, net_client *p_cli) {
    int i, n = -1;

    for (i = 0; i < p_game_room->size; i++) {
        if (p_game_room->clients[i] == p_cli) {
            return i;
        }
        if (p_game_room->clients[i] == NULL && n == -1) {
            n = i;
        }
    }
    if (n != -1) {
        p_game_room->clients[n] = p_cli;
    }
    return n;
}