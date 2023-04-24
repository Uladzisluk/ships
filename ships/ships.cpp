#define _CRT_SECURE_NO_WARNINGS //to avoid errors of using scanf

#include <iostream>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

#define MAX_NUMBER_OF_SHIPS 10

//this struct is used to store all data of board
typedef struct board_type {
	int board_size_x;
	int board_size_y;
	int amount_of_reefs;
	int* y_reef_coordinate;
	int* x_reef_coordinate;
	int** board;
}board_type;

//this struct is used to store all data of ship
typedef struct ship {
	int ship_size;
	int number_of_moves;
	int amount_of_dead_parts;
	int number_of_shots;
	char ship_direction;
	int x_head_coordinate;
	int y_head_coordinate;
	int x_back_coordinate;
	int y_back_coordinate;
	int x_gun_coordinate;
	int y_gun_coordinate;
	int* x_coordinates;
	int* y_coordinates;
	int* x_dead_coordinates;
	int* y_dead_coordinates;
}ship;

//this struct is used to store all data of players
typedef struct player {
	int move_next;
	int x1_bound_coordinate, y1_bound_coordinate, x2_bound_coordinate, y2_bound_coordinate;
	int ships_amount[4];
	ship** ships;
}player;

//function is used to clear all memory of game, after playing
void clear_memory(player** players, board_type* board, int* mode) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < players[i]->ships_amount[j]; k++) {
				free(players[i]->ships[j][k].x_coordinates);
				free(players[i]->ships[j][k].y_coordinates);
				free(players[i]->ships[j][k].x_dead_coordinates);
				free(players[i]->ships[j][k].y_dead_coordinates);
			}
			free(players[i]->ships[j]);
		}
		free(players[i]->ships);
	}

	free(board->x_reef_coordinate);
	free(board->y_reef_coordinate);

	for (int i = 0; i < board->board_size_y; i++) {
		free(board->board[i]);
	}
	free(board->board);

	free(mode);

	free(players);
}

//function return '1' if on this coordinate is unbroken part of ship, and '0' if on this coordinate is not unbroken part of ship
int is_here_part_of_ship(player** players, int y, int x) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < players[i]->ships_amount[j]; k++) {
				ship wsk = players[i]->ships[j][k];
				for (int g = 0; g < wsk.ship_size; g++) {
					if (wsk.x_coordinates[g] == x && wsk.y_coordinates[g] == y) return 1;
				}
			}
		}
	}
	return 0;
}

//function return '1' if on this coordinate is dead part of ship, and '0' if on this coordinate is not dead part of ship
int is_here_dead_part_of_ship(player** players, int y, int x) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < players[i]->ships_amount[j]; k++) {
				ship wsk = players[i]->ships[j][k];
				for (int g = 0; g < wsk.ship_size; g++) {
					if (wsk.x_dead_coordinates[g] == x && wsk.y_dead_coordinates[g] == y) return 1;
				}
			}
		}
	}
	return 0;
}

//function return '1' if on this coordinate is reef, and '0' if on this coordinate is not reef
int is_here_reef(board_type* board, int y, int x) {
	for (int i = 0; i < board->amount_of_reefs; i++) {
		if (board->x_reef_coordinate[i] == x && board->y_reef_coordinate[i] == y) return 1;
	}
	return 0;
}

//function return '1' if on this coordinate is part of ship, '2' if on this coordinate is dead part of ship, '3' if on
//this coordinate is reef
int what_is_here(player** players, board_type* board, int y, int x) {
	int result = 0;
	if (is_here_part_of_ship(players, y, x)) result = 1;
	if (is_here_dead_part_of_ship(players, y ,x)) result = 2;
	if (is_here_reef(board, y, x)) result = 3;
	return result;
}

//function is used to refresh ships location on board, after each moving of ship
void refresh_board(player** players, board_type* board) {
	for (int i = 0; i < board->board_size_y; i++) {
		for (int j = 0; j < board->board_size_x; j++) {
			if (what_is_here(players, board, i, j) == 0) board->board[i][j] = 0;
			else if (what_is_here(players, board, i, j) == 1) board->board[i][j] = 1;
			else if (what_is_here(players, board, i, j) == 2) board->board[i][j] = 2;
			else if (what_is_here(players, board, i, j) == 3) board->board[i][j] = 3;
		}
	}
}

//function is used to initialize board on the start of game
void init_board(board_type* board) {
	board->amount_of_reefs = 0;
	board->x_reef_coordinate = NULL;
	board->y_reef_coordinate = NULL;
	board->board = (int**)malloc(sizeof(int*) * board->board_size_y);
	for (int i = 0; i < board->board_size_y; i++) {
		board->board[i] = (int*)malloc(sizeof(int) * board->board_size_x);
		for (int j = 0; j < board->board_size_x; j++) {
			board->board[i][j] = 0;
		}
	}
}


//function return amount of all unbroken parts of all player ships
int parts_remain(player** players, int which_player) {
	int sum = 0;

	for (int i = 0; i < 4; i++) {
		ship* wsk = players[which_player]->ships[i];
		for (int j = 0; j < players[which_player]->ships_amount[i]; j++) {
			sum += wsk[j].ship_size;
		}
	}

	for (int i = 0; i < 4; i++) {
		ship* wsk = players[which_player]->ships[i];
		for (int j = 0; j < players[which_player]->ships_amount[i]; j++) {
			sum -= wsk[j].amount_of_dead_parts;
		}
	}

	return sum;
}

//function is used to erase ship form board(it is used in function, which check place of ship after moving, and check too close
//location to the other ship)
void clear_ship_from_board(player** players, board_type* board, int which_player, int type, int which_ship) {
	ship wsk = players[which_player]->ships[type][which_ship];
	for (int i = 0; i < wsk.ship_size; i++) {
		board->board[wsk.y_coordinates[i]][wsk.x_coordinates[i]] = 0;
	}
}


//function is used to check win of someone player after each shot, and print about it, if someone wins
void check_win(player** players, board_type* board, int* mode) {
	int A_Parts = parts_remain(players, 0);
	int B_Parts = parts_remain(players, 1);
	if (A_Parts == 0) {
		printf("B won");
		clear_memory(players, board, mode);
		exit(0);
	}
	else if (B_Parts == 0) {
		printf("A won");
		clear_memory(players, board, mode);
		exit(0);
	}
}

//command is used to print board, ships, reefs
void print(player** players, board_type* board) {
	int mode = 0;
	scanf("%d", &mode);

	if (mode == 0) {
		for (int i = 0; i < board->board_size_y; i++) {
			for (int j = 0; j < board->board_size_x; j++) {
				if (board->board[i][j] == 0) printf(" ");
				else if (board->board[i][j] == 1) printf("+");
				else if (board->board[i][j] == 2) printf("x");
				else if (board->board[i][j] == 3) printf("#");
			}
			printf("\n");
		}
		int A_Parts = parts_remain(players, 0);
		int B_Parts = parts_remain(players, 1);
		printf("PARTS REMAINING:: A : %d B : %d\n", A_Parts, B_Parts);
	}
	if (mode == 1) {
		printf("  ");
		for (int i = 0; i < board->board_size_x; i++) printf("%d", i % 10);
		printf("\n  ");
		for (int i = 0; i < board->board_size_x; i++) printf("%d", i % 10);
		
	}
}


//command is used to set amount of player's ships of each type
void set_fleet(player** players, board_type* board, int* mode) {
	char player;
	int amount[4];

	getchar();
	scanf("%c", &player);

	for (int i = 0; i < 4; i++) {
		scanf("%d", &amount[i]);
	}

	for (int i = 0; i < 4; i++) {
		if (amount[i] > 10) {
			printf("INVALID OPERATION \"SET_FLEET %c %d %d %d %d\": THE MAX NUMBER OF SHIPS OF ONE TYPE IS 10", player, amount[0], amount[1], amount[2], amount[3]);
			clear_memory(players, board, mode);
			exit(0);
		}
	}

	if (player == 'A') {
		for (int i = 0; i < 4; i++) {
			players[0]->ships_amount[i] = amount[i];
		}
	}
	else if (player == 'B') {
		for (int i = 0; i < 4; i++) {
			players[1]->ships_amount[i] = amount[i];
		}
	}
}

//function is used to change player's turn after round
void change_player(player** players) {
	if (players[0]->move_next == 1) {
		players[0]->move_next = 0;
		players[1]->move_next = 1;
	}
	else if (players[0]->move_next == 0) {
		players[0]->move_next = 1;
		players[1]->move_next = 0;
	}
}

//command is used to establish, which player's turn is next
void next_player(player** players) {
	char which_player;
	getchar();
	scanf("%c", &which_player);
	if (which_player == 'B') {
		players[0]->move_next = 0;
		players[1]->move_next = 1;
	}
	else if(which_player == 'A'){
		players[0]->move_next = 1;
		players[1]->move_next = 0;
	}
}

//command set size of board
void set_board_size(board_type* board) {
	int y, x;
	scanf("%d %d", &y, &x);
	board->board_size_x = x;
	board->board_size_y = y;
	init_board(board);
}

//function return size of ship: '5' (CARRIER), '4' (BATTLESHIP), '3' (CRUISER), '2' (DESTROYER)
int get_ship_size(char* class_of_ship) {
	if (strcmp(class_of_ship, "CAR") == 0) return 5;
	else if (strcmp(class_of_ship, "BAT") == 0) return 4;
	else if (strcmp(class_of_ship, "CRU") == 0) return 3;
	else if (strcmp(class_of_ship, "DES") == 0) return 2;
	
	return 0;
}

//function return type of ship: '0' (CARRIER), '1' (BATTLESHIP), '2' (CRUISER), '3' (DESTROYER)
int get_ship_type(char* class_of_ship) {
	if (strcmp(class_of_ship, "CAR") == 0) return 0;
	else if (strcmp(class_of_ship, "BAT") == 0) return 1;
	else if (strcmp(class_of_ship, "CRU") == 0) return 2;
	else if (strcmp(class_of_ship, "DES") == 0) return 3;

	return 0;
}

//function is used to check, if moving ship will not move from board
void check_on_leaving_the_board(player** players, board_type* board, int which_player, int which_ship, int type, int size, char* class_of_ship, char direction, int* mode) {
	ship wsk = players[which_player]->ships[type][which_ship];
	for (int i = 0; i < size; i++) {
		if (wsk.y_coordinates[i] < 0 || wsk.y_coordinates[i] >= board->board_size_y || wsk.x_coordinates[i] < 0 || wsk.x_coordinates[i] >= board->board_size_x)
		{
			printf("INVALID OPERATION \"MOVE %d %3s %1c\": SHIP WENT FROM BOARD", which_ship, class_of_ship, direction);
			clear_memory(players, board, mode);
			exit(0);
		}
	}
}


//function is used to check, if placing ship will not be placed on board too close to the other ships,
//or if ship will be moved to close to the other ship
void check_on_close_ships(player** players, board_type* board, int y_coordinate, int x_coordinate, char direction, int which_ship, char* class_of_ship, int which_player, int* struct_of_ship, char move_direction, int* mode) {
	int size = get_ship_size(class_of_ship);
	int y = y_coordinate, x = x_coordinate;
	int result = 0;
	
	for (int i = 0; i < size; i++) {

		if (y == 0 && x == 0) {
			if ((board->board[y][x + 1] == 1) || (board->board[y + 1][x] == 1)) result = 1;
		}
		else if (y == 0 && x == (board->board_size_x - 1)) {
			if ((board->board[y + 1][x] == 1) || (board->board[y][x - 1] == 1)) result = 1;
		}
		else if (y == (board->board_size_y - 1) && x == 0) {
			if ((board->board[y - 1][x] == 1) || (board->board[y][x + 1] == 1)) result = 1;
		}
		else if (y == (board->board_size_y - 1) && x == (board->board_size_x - 1)) {
			if ((board->board[y - 1][x] == 1) || (board->board[y][x - 1] == 1)) result = 1;
		}
		else if (y == 0) {
			if ((board->board[y][x + 1] == 1) || (board->board[y + 1][x] == 1) || (board->board[y][x - 1] == 1)) result = 1;
		}
		else if (y == (board->board_size_y - 1)) {
			if ((board->board[y - 1][x] == 1) || (board->board[y][x + 1] == 1) || (board->board[y][x - 1] == 1)) result = 1;
		}
		else if (x == 0) {
			if ((board->board[y - 1][x] == 1) || (board->board[y][x + 1] == 1) || (board->board[y + 1][x] == 1)) result = 1;
		}
		else if (x == (board->board_size_x - 1)) {
			if ((board->board[y - 1][x] == 1) || (board->board[y + 1][x] == 1) || (board->board[y][x - 1] == 1)) result = 1;
		}
		else {
			if ((board->board[y - 1][x] == 1) || (board->board[y][x + 1] == 1) || (board->board[y + 1][x] == 1) || (board->board[y][x - 1] == 1)) result = 1;
		}

		if (direction == 'S') y--;
		else if (direction == 'N') y++;
		else if (direction == 'E') x--;
		else if (direction == 'W') x++;
	}

	if (result == 1) {
		if(struct_of_ship == NULL && move_direction == NULL)
		printf("INVALID OPERATION \"PLACE_SHIP %d %d %c %d %s\": PLACING SHIP TOO CLOSE TO OTHER SHIP", y_coordinate, x_coordinate, direction, which_ship, class_of_ship);
		else if(struct_of_ship != NULL){
			char player = 'A';
			if (which_player == 0) player = 'A';
			else player = 'B';
			printf("INVALID OPERATION \"SHIP %c %d %d %c %d %s ", player, y_coordinate, x_coordinate, direction, which_ship, class_of_ship);
			for (int i = 0; i < size; i++) {
				printf("%d", struct_of_ship[i]);
			}
			printf("\": PLACING SHIP TOO CLOSE TO OTHER SHIP");
		}
		else if (move_direction != NULL) {
			printf("INVALID OPERATION \"MOVE %d %3s %1c\": PLACING SHIP TOO CLOSE TO OTHER SHIP", which_ship, class_of_ship, move_direction);
		}
		clear_memory(players, board, mode);
		exit(0);
	}
}

//function is used to check, if placing ship will be placed on board and doesn't go beyond the size of board,
// if placing ship will be placed in rectangle, which is initialize by command 'INIT_POSITION',
//if placing ship will not be placed on reef
void check_placing_for_players(player** players, board_type* board, int y_coordinate, int x_coordinate, char direction, int which_ship, char* class_of_ship, int which_player, int* struct_of_ship, int* mode) {
	int length = 0, result = 0, y = y_coordinate, x = x_coordinate;

	if (strcmp(class_of_ship, "CAR") == 0) length = 5;
	else if (strcmp(class_of_ship, "BAT") == 0) length = 4;
	else if (strcmp(class_of_ship, "CRU") == 0) length = 3;
	else if (strcmp(class_of_ship, "DES") == 0) length = 2;

	switch (direction) {
	case 'S': {
		if ((((y_coordinate - length + 1) < players[which_player]->y1_bound_coordinate) && (which_player == 0)) || (((y_coordinate - length + 1) < players[which_player]->y1_bound_coordinate) && (which_player == 1))) result = 1;
		break;
	}case 'N': {
		if ((((y_coordinate + length - 1) > players[which_player]->y2_bound_coordinate) && (which_player == 0)) || (((y_coordinate + length - 1) > players[which_player]->y2_bound_coordinate) && (which_player == 1))) result = 1;
		break;
	}case 'E': {
		if ((x_coordinate - length + 1) < players[which_player]->x1_bound_coordinate) result = 1;
		break;
	}case 'W': {
		if ((x_coordinate + length - 1) > players[which_player]->x2_bound_coordinate) result = 1;
		break;
	}default: result = 3;
	}

	if (((y_coordinate > players[which_player]->y2_bound_coordinate) && which_player == 0) || ((y_coordinate > players[which_player]->y2_bound_coordinate) && which_player == 1) || (x_coordinate > players[which_player]->x2_bound_coordinate)) result = 1;
	else {
		for (int i = 0; i < length; i++) {
			if (board->board[y][x] == 3) result = 2;

			if (direction == 'S') y--;
			else if (direction == 'N') y++;
			else if (direction == 'E') x--;
			else if (direction == 'W') x++;
		}
	}
	
	if (result == 1) {
		if(struct_of_ship == NULL)
		printf("INVALID OPERATION \"PLACE_SHIP %d %d %c %d %s\": NOT IN STARTING POSITION", y_coordinate, x_coordinate, direction, which_ship, class_of_ship);
		else {
			char player = 'A';
			if (which_player == 0) player = 'A';
			else player = 'B';
			printf("INVALID OPERATION \"SHIP %c %d %d %c %d %s ", player, y_coordinate, x_coordinate, direction, which_ship, class_of_ship);
			for (int i = 0; i < length; i++) {
				printf("%d", struct_of_ship[i]);
			}
			printf("\": NOT IN STARTING POSITION");
		}
		clear_memory(players, board, mode);
		exit(0);
	}else if (result == 2) {
		if(struct_of_ship == NULL)
		printf("INVALID OPERATION \"PLACE_SHIP %d %d %c %d %s\": PLACING SHIP ON REEF", y_coordinate, x_coordinate, direction, which_player, class_of_ship);
		else {
			char player = 'A';
			if (which_player == 0) player = 'A';
			else player = 'B';
			printf("INVALID OPERATION \"SHIP %c %d %d %c %d %s ", player, y_coordinate, x_coordinate, direction, which_ship, class_of_ship);
			for (int i = 0; i < length; i++) {
				printf("%d", struct_of_ship[i]);
			}
			printf("\": PLACING SHIP ON REEF");
		}
		clear_memory(players, board, mode);
		exit(0);
	}else if (result == 3) {
		if (struct_of_ship == NULL)
		printf("INVALID OPERATION \"PLACE_SHIP %d %d %c %d %s\": THERE IS ONLY 4 DIRECTIONS: \"S\", \"W\", \"N\", \"E\"", y_coordinate, x_coordinate, direction, which_ship, class_of_ship);
		else {
			char player = 'A';
			if (which_player == 0) player = 'A';
			else player = 'B';
			printf("INVALID OPERATION \"SHIP %c %d %d %c %d %s ", player, y_coordinate, x_coordinate, direction, which_ship, class_of_ship);
			for (int i = 0; i < length; i++) {
				printf("%d", struct_of_ship[i]);
			}
			printf("\": THERE IS ONLY 4 DIRECTIONS: \"S\", \"W\", \"N\", \"E\"");
		}
		clear_memory(players, board, mode);
		exit(0);
	}
}

//function is used to check, is the placed ship already present on board
void check_repeat_of_ship(player** players, board_type* board, int type, int which_player, int which_ship, int y, int x, char direction, char* class_of_ship, int* struct_of_ship, int* mode) {
	ship* wsk = players[which_player]->ships[type];
	if (wsk[which_ship].x_coordinates != NULL) {
		if(struct_of_ship == NULL)
		printf("INVALID OPERATION \"PLACE_SHIP %d %d %c %d %s\": SHIP ALREADY PRESENT", y, x, direction, which_ship, class_of_ship);
		else {
			char player = 'A';
			if (which_player == 0) player = 'A';
			else player = 'B';
			printf("INVALID OPERATION \"SHIP %c %d %d %c %d %s ", player, y, x, direction, which_ship, class_of_ship);
			for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
				printf("%d", struct_of_ship[i]);
			}
			printf("\": SHIP ALREADY PRESENT");
		}
		clear_memory(players, board, mode);
		exit(0);
	}
}

//function is used to check, are all ships of the class already placed on the board
void check_on_all_sets(player** players, board_type* board, int type, int which_player, int which_ship, int y, int x, char direction, char* class_of_ship, int* struct_of_ship, int* mode) {
	ship* wsk = players[which_player]->ships[type];
	for (int i = 0; i < players[which_player]->ships_amount[type]; i++) {
		if (wsk[i].x_coordinates == NULL) return;
	}
	if (struct_of_ship == NULL)
	printf("INVALID OPERATION \"PLACE_SHIP %d %d %c %d %s\": ALL SHIPS OF THE CLASS ALREADY SET", y, x, direction, which_ship, class_of_ship);
	else {
		char player = 'A';
		if (which_player == 0) player = 'A';
		else player = 'B';
		printf("INVALID OPERATION \"SHIP %c %d %d %c %d %s ", player, y, x, direction, which_ship, class_of_ship);
		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			printf("%d", struct_of_ship[i]);
		}
		printf("\": ALL SHIPS OF THE CLASS ALREADY SET");
	}
	clear_memory(players, board, mode);
	exit(0);
}

//command is called when player text 'SHIP', is used to place ship for player on board
void place_ship_at_start(player** players, board_type* board, int* mode) {
	char player, direction;
	int y, x, which_ship, which_player, size, type;
	int* struct_of_ship;
	char* class_of_ship = (char*)malloc(sizeof(char) * 3);
	ship wsk;

	scanf(" %1c %d %d %1c %d %3s", &player, &y, &x, &direction, &which_ship, class_of_ship);

	size = get_ship_size(class_of_ship);
	type = get_ship_type(class_of_ship);

	if (player == 'A') which_player = 0;
	else which_player = 1;

	wsk.ship_direction = direction;
	wsk.ship_size = size;
	wsk.amount_of_dead_parts = 0;
	wsk.x_head_coordinate = x;
	wsk.y_head_coordinate = y;
	wsk.number_of_shots = wsk.ship_size;
	if (wsk.ship_size != 5) wsk.number_of_moves = 3;
	else wsk.number_of_moves = 2;

	wsk.x_coordinates = (int*)malloc(sizeof(int) * size);
	wsk.y_coordinates = (int*)malloc(sizeof(int) * size);
	wsk.x_dead_coordinates = (int*)malloc(sizeof(int) * size);
	wsk.y_dead_coordinates = (int*)malloc(sizeof(int) * size);

	struct_of_ship = (int*)malloc(sizeof(int) * size);
	for (int i = 0; i < size; i++) {
		scanf("%1d", &struct_of_ship[i]);
	}

	check_placing_for_players(players, board, y, x, direction, which_ship, class_of_ship, which_player, struct_of_ship, mode);
	check_repeat_of_ship(players, board, type, which_player, which_ship, y, x, direction, class_of_ship, struct_of_ship, mode);
	check_on_all_sets(players, board, type, which_player, which_ship, y, x, direction, class_of_ship, struct_of_ship, mode);
	check_on_close_ships(players, board, y, x, direction, which_ship, class_of_ship, which_player, struct_of_ship, NULL, mode);

	for (int i = 0; i < size; i++) {

		if (struct_of_ship[i] == 0) {
			wsk.x_dead_coordinates[i] = x;
			wsk.y_dead_coordinates[i] = y;
			wsk.amount_of_dead_parts++;
			board->board[y][x] = 2;
		}else {
			wsk.x_dead_coordinates[i] = board->board_size_x;
			wsk.y_dead_coordinates[i] = board->board_size_y;
			board->board[y][x] = 1;
		}

		wsk.x_coordinates[i] = x;
		wsk.y_coordinates[i] = y;

		if (i == 1) {
			wsk.x_gun_coordinate = x;
			wsk.y_gun_coordinate = y;
		}

		if (i == size - 1) {
			wsk.x_back_coordinate = x;
			wsk.y_back_coordinate = y;
		}

		if (direction == 'S') y--;
		else if (direction == 'N') y++;
		else if (direction == 'E') x--;
		else if (direction == 'W') x++;
	}

	players[which_player]->ships[type][which_ship] = wsk;
	free(struct_of_ship);
	return;
}

//command is used to place reef on board
void place_reef(board_type* board) {
	int y, x;
	scanf("%d %d", &y, &x);
	board->board[y][x] = 3;
	if (board->amount_of_reefs == 0) {
		board->x_reef_coordinate = (int*)malloc(sizeof(int) * (board->amount_of_reefs + 1));
		board->y_reef_coordinate = (int*)malloc(sizeof(int) * (board->amount_of_reefs + 1));
	}
	else {
		board->x_reef_coordinate = (int*)realloc(board->x_reef_coordinate, sizeof(int) * (board->amount_of_reefs + 1));
		board->y_reef_coordinate = (int*)realloc(board->y_reef_coordinate, sizeof(int) * (board->amount_of_reefs + 1));
	}
	

	board->x_reef_coordinate[board->amount_of_reefs] = x;
	board->y_reef_coordinate[board->amount_of_reefs] = y;

	board->amount_of_reefs++;
}

//command is called when player text 'INIT_POSITION' to initializate the area on board of rectangle shape for player, where
//player can place his ships
void init_position(player** players, board_type* board, int* mode) {
	char player;
	int y1, x1, y2, x2, which_player;
	scanf(" %c %d %d %d %d", &player, &y1, &x1, &y2, &x2);
	
	if (player == 'A') which_player = 0;
	else if (player == 'B') which_player = 1;
	else {
		printf("INVALID OPERATION \"INIT_POSITION %1c %d %d %d %d\": PLAYER %1c DOES NOT EXIST", player, y1, x1, y2, x2, player);
		clear_memory(players, board, mode);
		exit(0);
	}

	players[which_player]->x1_bound_coordinate = x1;
	players[which_player]->y1_bound_coordinate = y1;
	players[which_player]->x2_bound_coordinate = x2;
	players[which_player]->y2_bound_coordinate = y2;
}

//reading the input text from the user input (in particular, to get command). The rest is like in read_the_command() function
void description_commands(player** players, board_type* board, int* mode) {
	char* command = (char*)malloc(sizeof(char)*16);
	char endofline ;
	int active = 1;
	endofline = 'a';

	while (active) {

		endofline = 'a';

		scanf("%15s", command);

		if (strcmp(command, "PRINT\0") == 0) print(players, board);
		else if (strcmp(command, "SET_FLEET\0") == 0) set_fleet(players, board, mode);
		else if (strcmp(command, "NEXT_PLAYER\0") == 0) next_player(players);
		else if (strcmp(command, "BOARD_SIZE\0") == 0) set_board_size(board);
		else if (strcmp(command, "SHIP\0") == 0) place_ship_at_start(players, board, mode);
		else if (strcmp(command, "REEF\0") == 0) place_reef(board);
		else if (strcmp(command, "INIT_POSITION\0") == 0) init_position(players, board, mode);
		else if (strcmp(command, "EXTENDED_SHIPS\0") == 0) *mode = 1;
		else if (strcmp(command, "[state]\0")==0) active = 0;

		do {
			int int_endofline = getchar();
			if (int_endofline == EOF) {
				free(command);
				clear_memory(players, board, mode);
				exit(0);
			}
			endofline = (char)int_endofline;
		} while ((endofline != '\n'));
	}

	free(command);

	return;
}

//command is used to place ship on the board
void place_ship(player** players, board_type* board, int which_player, int* mode) {
	int y, x, which_ship, size=0, type=0;
	char* class_of_ship = (char*)malloc(sizeof(char)*3);
	char direction;
	ship wsk;

	scanf("%d %d %1c %d %3s", &y, &x, &direction, &which_ship, class_of_ship);

	size = get_ship_size(class_of_ship);
	type = get_ship_type(class_of_ship);

    check_placing_for_players(players, board, y, x, direction, which_ship, class_of_ship, which_player, NULL, mode);
	check_repeat_of_ship(players, board, type, which_player, which_ship, y, x, direction, class_of_ship, NULL, mode);
	check_on_all_sets(players, board, type, which_player, which_ship, y, x, direction, class_of_ship, NULL, mode);
	check_on_close_ships(players, board, y, x, direction, which_ship, class_of_ship, which_player, NULL, NULL, mode);

	wsk.ship_size = size;
	wsk.ship_direction = direction;
	wsk.amount_of_dead_parts = 0;
	wsk.x_head_coordinate = x;
	wsk.y_head_coordinate = y;
	wsk.number_of_shots = wsk.ship_size;
	if (wsk.ship_size != 5) wsk.number_of_moves = 3;
	else wsk.number_of_moves = 2;

	wsk.x_coordinates = (int*)malloc(sizeof(int)*size);
	wsk.y_coordinates = (int*)malloc(sizeof(int) * size);
	wsk.x_dead_coordinates = (int*)malloc(sizeof(int) * size);
	wsk.y_dead_coordinates = (int*)malloc(sizeof(int) * size);

	for (int i = 0; i < size; i++) {

		wsk.x_dead_coordinates[i] = board->board_size_x;
		wsk.y_dead_coordinates[i] = board->board_size_y;

		wsk.x_coordinates[i] = x;
		wsk.y_coordinates[i] = y;

		board->board[y][x] = 1;

		if (i == 1) {
			wsk.x_gun_coordinate = x;
			wsk.y_gun_coordinate = y;
		}

		if (i == size - 1) {
			wsk.x_back_coordinate = x;
			wsk.y_back_coordinate = y;
		}

		if (direction == 'S') y--;
		else if (direction == 'N') y++;
		else if (direction == 'E') x--;
		else if (direction == 'W') x++;
	}

	players[which_player]->ships[type][which_ship] = wsk;
	//free(class_of_ship);
	return;
}

//function return indicator on object of ship(struct ship) type
ship* which_ship_is_here(player** players, int y, int x) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < players[i]->ships_amount[j]; k++) {
				for (int h = 0; h < players[i]->ships[j][k].ship_size; h++) {
					if ((players[i]->ships[j][k].y_coordinates[h] == y && players[i]->ships[j][k].x_coordinates[h] == x) || (players[i]->ships[j][k].y_gun_coordinate == y && players[i]->ships[j][k].x_gun_coordinate == x)) {
						return &(players[i]->ships[j][k]);
					}
				}
			}
		}
	}
	return NULL;
}

//function return '1' if all ships are placed on board, and return '0' if not all ships are placed on board
int check_on_all_ships_placed(player** players) {
	ship wsk;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < players[i]->ships_amount[j]; k++) {
				wsk = players[i]->ships[j][k];
				if (wsk.x_coordinates == NULL) return 0;
			}
		}
	}
	return 1;
}

//function return '1' if distance between ship's gun and place where ship shoot is more than size of this ship, and return '0' if this 
//distance is less than size of this ship
int check_far_of_shooting(player** players, int which_player, int which_ship, int type, int y, int x) {
	float distance;
	int a = abs(players[which_player]->ships[type][which_ship].y_gun_coordinate - y) * abs(players[which_player]->ships[type][which_ship].y_gun_coordinate - y);
	int b = abs(players[which_player]->ships[type][which_ship].x_gun_coordinate - x) * abs(players[which_player]->ships[type][which_ship].x_gun_coordinate - x);
	distance = sqrt(a + b);

	if (distance > players[which_player]->ships[type][which_ship].ship_size) {
		return 1;
	}
	return 0;
}


//command used for shooting by ships
void shoot(player** players, board_type* board, int which_player, int* mode) {
	if (*mode == 0) {
		int y, x;
		scanf("%d %d", &y, &x);

		if (y >= board->board_size_y || x >= board->board_size_x) {
			printf("INVALID OPERATION \"SHOOT %d %d\": FIELD DOES NOT EXIST", y, x);
			clear_memory(players, board, mode);
			exit(0);
		}

		if (!check_on_all_ships_placed(players)) {
			printf("INVALID OPERATION \"SHOOT %d %d\": NOT ALL SHIPS PLACED", y, x);
			clear_memory(players, board, mode);
			exit(0);
		}

		if (board->board[y][x] == 1) {
			board->board[y][x] = 2;
			ship* wsk = which_ship_is_here(players, y, x);
			if (wsk == NULL) return;
			wsk->amount_of_dead_parts++;
			for (int i = 0; i < wsk->ship_size; i++) {
				if ((wsk->y_coordinates[i] == y) && (wsk->x_coordinates[i] == x)) {
					wsk->y_dead_coordinates[i] = y;
					wsk->x_dead_coordinates[i] = x;
				}
			}
		}
		check_win(players, board, mode);
	}
	else if (*mode == 1) {
		int which_ship, y, x, type, size;
		char* class_of_ship = (char*)malloc(sizeof(char) * 3);
		
		scanf("%d %3s %d %d", &which_ship, class_of_ship, &y, &x);

		type = get_ship_type(class_of_ship);
		size = get_ship_size(class_of_ship);

		if (y >= board->board_size_y || x >= board->board_size_x) {
			printf("INVALID OPERATION \"SHOOT %d %d\": FIELD DOES NOT EXIST", y, x);
			clear_memory(players, board, mode);
			free(class_of_ship);
			exit(0);
		}

		if (!check_on_all_ships_placed(players)) {
			printf("INVALID OPERATION \"SHOOT %d %d\": NOT ALL SHIPS PLACED", y, x);
			clear_memory(players, board, mode);
			free(class_of_ship);
			exit(0);
		}

		if (players[which_player]->ships[type][which_ship].x_gun_coordinate == board->board_size_x && players[which_player]->ships[type][which_ship].y_gun_coordinate == board->board_size_y) {
			printf("INVALID OPERATION \"SHOOT %d %3s %d %d\": SHIP CANNOT SHOOT", which_ship, class_of_ship, y, x);
			clear_memory(players, board, mode);
			free(class_of_ship);
			exit(0);
		}

		if (players[which_player]->ships[type][which_ship].number_of_shots == 0) {
			printf("INVALID OPERATION \"SHOOT %d %3s %d %d\": TOO MANY SHOOTS", which_ship, class_of_ship, y, x);
			clear_memory(players, board, mode);
			free(class_of_ship);
			exit(0);
		}

		if (check_far_of_shooting(players, which_player, which_ship, type, y, x)) {
			printf("INVALID OPERATION \"SHOOT %d %3s %d %d\": SHOOTING TOO FAR", which_ship, class_of_ship, y, x);
			clear_memory(players, board, mode);
			free(class_of_ship);
			exit(0);
		}


		ship* wsk = which_ship_is_here(players, y, x);
		if (wsk == NULL) {
			players[which_player]->ships[type][which_ship].number_of_shots--;
			free(class_of_ship);
			return;
		}
		if (board->board[y][x] == 1) {
			board->board[y][x] = 2;
			wsk->amount_of_dead_parts++;
		}
		if (wsk->x_gun_coordinate == x && wsk->y_gun_coordinate == y) {
			wsk->x_gun_coordinate = board->board_size_x;
			wsk->y_gun_coordinate = board->board_size_y;
		}
		for (int i = 0; i < wsk->ship_size; i++) {
			if ((wsk->y_coordinates[i] == y) && (wsk->x_coordinates[i] == x)) {
				wsk->y_dead_coordinates[i] = y;
				wsk->x_dead_coordinates[i] = x;
			}
		}
		
		players[which_player]->ships[type][which_ship].number_of_shots--;
		check_win(players, board, mode);
	}
}


//function used to move the ship to the front
void front_move(player** players, board_type* board, char ship_direction, int which_player, int type, int which_ship) {
	if (ship_direction == 'N') {
		players[which_player]->ships[type][which_ship].y_head_coordinate--;
		players[which_player]->ships[type][which_ship].y_back_coordinate--;

		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].y_coordinates[i]--;
			if ((players[which_player]->ships[type][which_ship].y_dead_coordinates[i]) != (board->board_size_y))
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i]--;
		}
	}
	if (ship_direction == 'S') {
		players[which_player]->ships[type][which_ship].y_head_coordinate++;
		players[which_player]->ships[type][which_ship].y_back_coordinate++;

		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].y_coordinates[i]++;
			if ((players[which_player]->ships[type][which_ship].y_dead_coordinates[i]) != (board->board_size_y))
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i]++;
		}
	}
	if (ship_direction == 'W') {
		players[which_player]->ships[type][which_ship].x_head_coordinate--;
		players[which_player]->ships[type][which_ship].x_back_coordinate--;

		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].x_coordinates[i]--;
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x))
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i]--;
		}
	}
	if (ship_direction == 'E') {
		players[which_player]->ships[type][which_ship].x_head_coordinate++;
		players[which_player]->ships[type][which_ship].x_back_coordinate++;

		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].x_coordinates[i]++;
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x))
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i]++;
		}
	}
}

//function used to move the ship to the left
void left_move(player** players, board_type* board, char ship_direction, int which_player, int type, int which_ship) {
	if (ship_direction == 'N') {
		players[which_player]->ships[type][which_ship].x_head_coordinate -= (players[which_player]->ships[type][which_ship].ship_size-1);
		players[which_player]->ships[type][which_ship].y_head_coordinate--;
		players[which_player]->ships[type][which_ship].y_back_coordinate -= players[which_player]->ships[type][which_ship].ship_size;
		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].x_coordinates[i] -= (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
			players[which_player]->ships[type][which_ship].y_coordinates[i] -= (i + 1);
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x)) {
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i] -= (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i] -= (i + 1);
			}
		}
		players[which_player]->ships[type][which_ship].ship_direction = 'W';
	}
	else if (ship_direction == 'S') {
		players[which_player]->ships[type][which_ship].x_head_coordinate += (players[which_player]->ships[type][which_ship].ship_size - 1);
		players[which_player]->ships[type][which_ship].y_head_coordinate++;
		players[which_player]->ships[type][which_ship].y_back_coordinate += players[which_player]->ships[type][which_ship].ship_size;
		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].x_coordinates[i] += (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
			players[which_player]->ships[type][which_ship].y_coordinates[i] += (i + 1);
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x)) {
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i] += (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i] += (i + 1);
			}
		}
		players[which_player]->ships[type][which_ship].ship_direction = 'E';
	}
	else if (ship_direction == 'W') {
		players[which_player]->ships[type][which_ship].y_head_coordinate += (players[which_player]->ships[type][which_ship].ship_size - 1);
		players[which_player]->ships[type][which_ship].x_head_coordinate--;
		players[which_player]->ships[type][which_ship].x_back_coordinate -= players[which_player]->ships[type][which_ship].ship_size;
		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].y_coordinates[i] += (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
			players[which_player]->ships[type][which_ship].x_coordinates[i] -= (i + 1);
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x)) {
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i] += (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i] -= (i + 1);
			}
		}
		players[which_player]->ships[type][which_ship].ship_direction = 'S';
	}
	else if (ship_direction == 'E') {
		players[which_player]->ships[type][which_ship].y_head_coordinate -= (players[which_player]->ships[type][which_ship].ship_size - 1);
		players[which_player]->ships[type][which_ship].x_head_coordinate++;
		players[which_player]->ships[type][which_ship].x_back_coordinate += players[which_player]->ships[type][which_ship].ship_size;
		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].y_coordinates[i] -= (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
			players[which_player]->ships[type][which_ship].x_coordinates[i] += (i + 1);
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x)) {
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i] -= (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i] += (i + 1);
			}
		}
		players[which_player]->ships[type][which_ship].ship_direction = 'N';
	}
}

//function used to move ship to the right
void right_move(player** players, board_type* board, char ship_direction, int which_player, int type, int which_ship) {
	if (ship_direction == 'N') {
		players[which_player]->ships[type][which_ship].x_head_coordinate += (players[which_player]->ships[type][which_ship].ship_size - 1);
		players[which_player]->ships[type][which_ship].y_head_coordinate--;
		players[which_player]->ships[type][which_ship].y_back_coordinate -= players[which_player]->ships[type][which_ship].ship_size;
		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].x_coordinates[i] += (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
			players[which_player]->ships[type][which_ship].y_coordinates[i] -= (i + 1);
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x)) {
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i] += (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i] -= (i + 1);
			}
		}
		players[which_player]->ships[type][which_ship].ship_direction = 'E';
	}
	else if (ship_direction == 'S') {
		players[which_player]->ships[type][which_ship].x_head_coordinate -= (players[which_player]->ships[type][which_ship].ship_size - 1);
		players[which_player]->ships[type][which_ship].y_head_coordinate++;
		players[which_player]->ships[type][which_ship].y_back_coordinate += players[which_player]->ships[type][which_ship].ship_size;
		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].x_coordinates[i] -= (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
			players[which_player]->ships[type][which_ship].y_coordinates[i] += (i + 1);
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x)) {
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i] -= (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i] += (i + 1);
			}
		}
		players[which_player]->ships[type][which_ship].ship_direction = 'W';
	}
	else if (ship_direction == 'W') {
		players[which_player]->ships[type][which_ship].y_head_coordinate -= (players[which_player]->ships[type][which_ship].ship_size - 1);
		players[which_player]->ships[type][which_ship].x_head_coordinate--;
		players[which_player]->ships[type][which_ship].x_back_coordinate -= players[which_player]->ships[type][which_ship].ship_size;
		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].y_coordinates[i] -= (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
			players[which_player]->ships[type][which_ship].x_coordinates[i] -= (i + 1);
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x)) {
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i] -= (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i] -= (i + 1);
			}
		}
		players[which_player]->ships[type][which_ship].ship_direction = 'N';
	}
	else if (ship_direction == 'E') {
		players[which_player]->ships[type][which_ship].y_head_coordinate += (players[which_player]->ships[type][which_ship].ship_size - 1);
		players[which_player]->ships[type][which_ship].x_head_coordinate++;
		players[which_player]->ships[type][which_ship].x_back_coordinate += players[which_player]->ships[type][which_ship].ship_size;
		for (int i = 0; i < players[which_player]->ships[type][which_ship].ship_size; i++) {
			players[which_player]->ships[type][which_ship].y_coordinates[i] += (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
			players[which_player]->ships[type][which_ship].x_coordinates[i] += (i + 1);
			if ((players[which_player]->ships[type][which_ship].x_dead_coordinates[i]) != (board->board_size_x)) {
				players[which_player]->ships[type][which_ship].y_dead_coordinates[i] += (players[which_player]->ships[type][which_ship].ship_size - (i + 1));
				players[which_player]->ships[type][which_ship].x_dead_coordinates[i] += (i + 1);
			}
		}
		players[which_player]->ships[type][which_ship].ship_direction = 'S';
	}
}

//command used to move the ship
void move(player** players, board_type* board, int which_player, int* mode) {
	int which_ship;
	char* class_of_ship = (char*)malloc(sizeof(char) * 3);
	char direction, ship_direction;
	int type;

	scanf("%d %3s %1c", &which_ship, class_of_ship, &direction);

	type = get_ship_type(class_of_ship);
	ship_direction = players[which_player]->ships[type][which_ship].ship_direction;

	if (players[which_player]->ships[type][which_ship].number_of_moves > 0) {
		switch (direction) {
		case 'F': {
			clear_ship_from_board(players, board, which_player, type, which_ship);
			front_move(players, board, ship_direction, which_player, type, which_ship);
			check_on_leaving_the_board(players, board, which_player, which_ship, type, players[which_player]->ships[type][which_ship].ship_size, class_of_ship, direction, mode);
			check_on_close_ships(players, board, players[which_player]->ships[type][which_ship].y_head_coordinate, players[which_player]->ships[type][which_ship].x_head_coordinate, players[which_player]->ships[type][which_ship].ship_direction, which_ship, class_of_ship, which_player, NULL, direction, mode);
			refresh_board(players, board);
			break;
		}
		case 'L': {
			clear_ship_from_board(players, board, which_player, type, which_ship);
			left_move(players, board, ship_direction, which_player, type, which_ship);
			check_on_leaving_the_board(players, board, which_player, which_ship, type, players[which_player]->ships[type][which_ship].ship_size, class_of_ship, direction, mode);
			check_on_close_ships(players, board, players[which_player]->ships[type][which_ship].y_head_coordinate, players[which_player]->ships[type][which_ship].x_head_coordinate, players[which_player]->ships[type][which_ship].ship_direction, which_ship, class_of_ship, which_player, NULL, direction, mode);
			refresh_board(players, board);
			break;
		}
		case 'R': {
			clear_ship_from_board(players, board, which_player, type, which_ship);
			right_move(players, board, ship_direction, which_player, type, which_ship);
			check_on_leaving_the_board(players, board, which_player, which_ship, type, players[which_player]->ships[type][which_ship].ship_size, class_of_ship, direction, mode);
			check_on_close_ships(players, board, players[which_player]->ships[type][which_ship].y_head_coordinate, players[which_player]->ships[type][which_ship].x_head_coordinate, players[which_player]->ships[type][which_ship].ship_direction, which_ship, class_of_ship, which_player, NULL, direction, mode);
			refresh_board(players, board);
			break;
		}
		default: break;
		}
		players[which_player]->ships[type][which_ship].number_of_moves--;
	}
	else {
		printf("INVALID OPERATION \"MOVE %d %3s %1c\": SHIP MOVED ALREADY", which_ship, class_of_ship, direction);
		clear_memory(players, board, mode);
		exit(0);
	}
	
	
	//free(class_of_ship);
}

//used after round to return to the ship standart number of moves
void return_the_ships_moves(player** players) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < players[i]->ships_amount[j]; k++) {
				if (players[i]->ships[j][k].ship_size != 5) players[i]->ships[j][k].number_of_moves = 3;
				else players[i]->ships[j][k].number_of_moves = 2;
			}
		}
	}
}

//used after round to return to the ship standart number of shots
void return_the_ships_shots(player** players) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < players[i]->ships_amount[j]; k++) {
				players[i]->ships[j][k].number_of_shots = players[i]->ships[j][k].ship_size;
			}
		}
	}
}


//reading the input text from the user input (in particular, to get one player command)
void players_commands(player** players, board_type* board, int which_player, int* mode) {
	char* command = (char*)malloc(sizeof(char) * 12);
	char endofline;
	int invalid_command;
	int active = 1, option = 0;
	invalid_command = 1;
	endofline = 'a';

	//until we read the command, we will continue to read
	while (active) {
		endofline = 'a';
		invalid_command = 1;

		if (scanf("%11s", command) == EOF) {
			clear_memory(players, board, mode);
			free(command);
			exit(0);
		}

		if (strcmp(command, "PLACE_SHIP\0") == 0) {
			if (option == 2) {
				printf("INVALID OPERATION \"PLACE_SHIP\": YOU HAVE ALREADY TAKEN A SHOT");
				clear_memory(players, board, mode);
				free(command);
				exit(0);
			}
			place_ship(players, board, which_player, mode);
			option = 1;
			invalid_command = 0;
		}
		else if (strcmp(command, "SHOOT\0") == 0) {
			if (option == 1) {
				printf("INVALID OPERATION \"SHOOT\": OPERATION \"PLACE_SHIP\" EXPECTED");
				free(command);
				clear_memory(players, board, mode);
				exit(0);
			}
			shoot(players, board, which_player, mode);
			option = 2;
			invalid_command = 0;
		}
		else if (strcmp(command, "MOVE\0") == 0) {
			if (option == 1) {
				printf("INVALID OPERATION \"MOVE\": OPERATION \"PLACE_SHIP\" EXPECTED");
				free(command);
				clear_memory(players, board, mode);
				exit(0);
			}
			move(players, board, which_player, mode);
			invalid_command = 0;
		}
		else if ((strcmp(command, "[playerA]\0") == 0) || (strcmp(command, "[playerB]\0") == 0)) {
			return_the_ships_moves(players);
			return_the_ships_shots(players);
			active = 0;
		}

		if (invalid_command == 1) {
			do {
				int int_endofline = getchar();
				if (int_endofline == EOF)
				{
					free(command);
					clear_memory(players, board, mode);
					exit(0);
				}
				endofline = (char)int_endofline;
			} while (endofline != '\n');
		}
	}

	free(command);
	change_player(players);
	return;
}


//reading the input text from the user input (in particular, to get one group of commands)
int read_the_command(player** players, board_type* board, int* mode) {
	char* command_groupe = (char*)malloc(sizeof(char) * 11);; //here is a name of one group
	int invalid_command;
	char endofline;
	invalid_command = 1;
	endofline = 'a';

	//until we read the name of the group, we will continue to read
	while (true) {
		endofline='a'; // is used to store the value of the end of line to check the end of line

		invalid_command = 1;
		//max number of chars in string is 11, last char is '\0'
		//we use "%10s" in order to avoid buffer overflow
		if (scanf("%10s", command_groupe) == EOF) {
			clear_memory(players, board, mode);
			exit(0);
		}

		if (strcmp(command_groupe, "[state]\0") == 0) {
			description_commands(players, board, mode);
			invalid_command = 0;
		}
		else if (strcmp(command_groupe, "[playerA]\0") == 0) {
			if (players[0]->move_next) players_commands(players, board, 0, mode);
			else {
				printf("INVALID OPERATION \"[playerA] \": THE OTHER PLAYER EXPECTED");
				clear_memory(players, board, mode);
				exit(0);
			}
			invalid_command = 0;
		}
		else if (strcmp(command_groupe, "[playerB]\0") == 0) {
			if (players[1]->move_next) players_commands(players, board, 1, mode);
			else {
				printf("INVALID OPERATION \"[playerB] \": THE OTHER PLAYER EXPECTED");
				clear_memory(players, board, mode);
				exit(0);
			}
			invalid_command = 0;
		}
		

		if (invalid_command == 1) {
			do {
				int int_endofline = getchar();
				if (int_endofline == EOF) {
					free(command_groupe);
					clear_memory(players, board, mode);
					exit(0);
				}
				endofline = (char)int_endofline;
			} while (endofline != '\n');
		}
	}

	free(command_groupe);

	return 1;
}

//we set the variable meanings, which indicate the bearing of the upper
// left and lower right corner of the rectangle in which you can place ships
//(we use this function at the beginning to set the standard rectangle size for each player to the correct board size)
void init_position_on_start(player** players, board_type* board, int which_player) {
	if (which_player == 0) {
		players[0]->x1_bound_coordinate = 0;
		players[0]->y1_bound_coordinate = 0;
		players[0]->x2_bound_coordinate = (board->board_size_x) - 1;
		players[0]->y2_bound_coordinate = (board->board_size_y)/2 - 1;
	}
	else {
		players[1]->x1_bound_coordinate = 0;
		players[1]->y1_bound_coordinate = (board->board_size_y) / 2 + 1;
		players[1]->x2_bound_coordinate = (board->board_size_x) - 1;
		players[1]->y2_bound_coordinate = (board->board_size_y) - 1;
	}
}

//initialization of player
void initplayer(player* entity) {
	entity->ships = (ship**)malloc(sizeof(ship*) * 4);

	int default_amount = 1;

	for (int i = 0; i < 4; i++) {

		entity->ships_amount[i] = default_amount++;
		entity->ships[i] = (ship*)malloc(sizeof(ship) * (MAX_NUMBER_OF_SHIPS+1));
		for (int j = 0; j < (MAX_NUMBER_OF_SHIPS+1); j++) {
			ship* wsk = entity->ships[i];
			wsk[j].ship_size = 0;
			wsk[j].amount_of_dead_parts = 0;
			wsk[j].number_of_moves = 0;
			wsk[j].number_of_shots = 0;
			wsk[j].x_coordinates = NULL;
			wsk[j].y_coordinates = NULL;
			wsk[j].x_dead_coordinates = NULL;
			wsk[j].y_dead_coordinates = NULL;
		}
	}
}

int main() {
	player** players;
	int* mode = (int*)malloc(sizeof(int)*1);
	*mode = 0;
	
	board_type board;
	board.board_size_x = 10;
	board.board_size_y = 21;
	
	init_board(&board);

	players = (player**)malloc(sizeof(player*) * 2);

	players[0] = (player*)malloc(sizeof(player));
	players[1] = (player*)malloc(sizeof(player));

	player A;
	player B;

	players[0] = &A;
	players[1] = &B;
	
	initplayer(&A);
	A.move_next = 1;
	init_position_on_start(players, &board, 0);
	initplayer(&B);
	B.move_next = 0;
	init_position_on_start(players, &board, 1);

	read_the_command(players, &board, mode);
	return 0;
}
