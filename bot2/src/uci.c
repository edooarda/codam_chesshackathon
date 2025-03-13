#include "uci.h"
#include "search.h"
#include "move.h"
#include "types.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

static char *get_line(FILE *stream) {
	size_t capacity = 1024;
	size_t size = 0;
	char *string = malloc(capacity);

	while (fgets(string + size, capacity - size, stream)) {
		size += strlen(string + size);

		if (string[size - 1] == '\n') {
			return string;
		}

		capacity *= 2;
		string = realloc(string, capacity);
	}

	free(string);

	return NULL;
}

static char *get_token(char *string, char *store) {
	string += strlen(string);
	*string = *store;

	while (isspace(*string)) {
		string++;
	}

	if (*string) {
		char *token = string;

		while (*string && !isspace(*string)) {
			string++;
		}

		*store = *string;
		*string = '\0';

		return token;
	}

	return NULL;
}

static char last_move_other_side[] = "0000";

static void uci_position(struct position *pos, char *token, char *store) {
	token = get_token(token, store);

	if (token && !strcmp(token, "startpos")) {
		parse_position(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		token = get_token(token, store);
	} else if (token && !strcmp(token, "fen")) {
		char *fen = get_token(token, store);
		int index;

		token = fen;

		for (index = 0; token && index < 5; index++) {
			token = get_token(token, store);
		}

		if (token) {
			parse_position(pos, fen);
			token = get_token(token, store);
		}
	}

	if (token && !strcmp(token, "moves")) {
		while ((token = get_token(token, store))) {
			struct move move;

			if (parse_move(&move, token) == SUCCESS) {
				strcpy(last_move_other_side, token);
				do_move(pos, move);
			}
		}
	}
}

static int moves_total_game = 0;
struct opening {
	char	moves[3][6];
	int		move_count;
	int		is_white;
};

static struct opening openings[5] = {
	{ { "e2e4", "g1f3", "d2d4" }, 3, 1 },
	{ { "d2d4", "c2c4", "g1f3" }, 3, 1 },
	{ { "c7c5", "d7d6", "g8f6" }, 3, 0 },
	{ { "e7e5", "g8f6", "d7d6" }, 3, 0 },
	{ { "d7d5", "c7c6", "e7e6" }, 3, 0 } 
};

int openingsbook(const struct position *pos, int moves) {
	static char		move[6];
	int				i;

	for (i = 0; i < 3; i++) {
		if ((pos->side_to_move == WHITE && openings[i].is_white) ||
			(pos->side_to_move == BLACK && !openings[i].is_white))
		{
			if (moves < openings[i].move_count) {
				strcpy(move, openings[i].moves[moves]);
				printf("bestmove %s\n", move);
				return (1);
			}
		}
	}
	return (0);
}

/*
struct move search(const struct search_info *info) {
	return minimax(info->pos, 4).move;

}*/

static void uci_go(const struct position *pos, char *token, char *store) {
	struct search_info info;
	struct move move;
	char buffer[] = { '\0', '\0', '\0', '\0', '\0', '\0' };

	info.pos = pos;
	info.time[WHITE] = 0;
	info.time[BLACK] = 0;
	info.increment[WHITE] = 0;
	info.increment[BLACK] = 0;

	while ((token = get_token(token, store))) {
		if (!strcmp(token, "searchmoves")) {
			break;
		} else if (!strcmp(token, "ponder")) {
			continue;
		} else if (!strcmp(token, "infinite")) {
			continue;
		} else if (!strcmp(token, "wtime")) {
			token = get_token(token, store);
			info.time[WHITE] = token ? atoi(token) : 0;
		} else if (!strcmp(token, "btime")) {
			token = get_token(token, store);
			info.time[BLACK] = token ? atoi(token) : 0;
		} else if (!strcmp(token, "winc")) {
			token = get_token(token, store);
			info.increment[WHITE] = token ? atoi(token) : 0;
		} else if (!strcmp(token, "binc")) {
			token = get_token(token, store);
			info.increment[BLACK] = token ? atoi(token) : 0;
		} else {
			token = get_token(token, store);
		}

		if (!token) {
			break;
		}
	}

	if (openingsbook(pos, moves_total_game))
	{
		moves_total_game++; 
		return ;
	}
	move = search(&info);

	buffer[0] = "abcdefgh"[FILE(move.from_square)];
	buffer[1] = '1' + RANK(move.from_square);
	buffer[2] = "abcdefgh"[FILE(move.to_square)];
	buffer[3] = '1' + RANK(move.to_square);

	if (move.promotion_type != NO_TYPE) {
		buffer[4] = "pnbrqk"[move.promotion_type];
	}

	printf("bestmove %s\n", buffer);
}

void uci_run(const char *name, const char *author) {
	char *line;
	int quit = 0;
	struct position pos;

	while (!quit && (line = get_line(stdin))) {
		char *token = line;
		char store = *token;

		*token = '\0';

		while ((token = get_token(token, &store))) {
			if (!strcmp(token, "quit")) {
				quit = 1;
			} else if (!strcmp(token, "uci")) {
				printf("id name %s\n", name);
				printf("id author %s\n", author);
				printf("uciok\n");
			} else if (!strcmp(token, "isready")) {
				printf("readyok\n");
			} else if (!strcmp(token, "position")) {
				uci_position(&pos, token, &store);
			} else if (!strcmp(token, "go")) {
				uci_go(&pos, token, &store);
			} else if (!strcmp(token, "setoption")) {
				break;
			} else if (!strcmp(token, "register")) {
				break;
			} else {
				continue;
			}

			break;
		}

		free(line);
		fflush(stdout);
	}
}
