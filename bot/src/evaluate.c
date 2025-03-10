#include "evaluate.h"
#include "types.h"

static const int piece_value[6] = { 100, 300, 300, 500, 900, 1000000 };

int evaluate(const struct position *pos) {
	int score[2] = { 0, 0 };
	int square;

	for (square = 0; square < 64; square++) {
		int piece = pos->board[square];

		if (piece != NO_PIECE) {
			// pawn higher score when it is closer to the end.
			if (piece == PAWN) {
				int row = 1;
				row += square / 8;
				score[COLOR(piece)] += piece_value[TYPE(piece)] + (row * 20);
			}
			else
				score[COLOR(piece)] += piece_value[TYPE(piece)];
		}
	}
	return score[pos->side_to_move] - score[1 - pos->side_to_move];
}
