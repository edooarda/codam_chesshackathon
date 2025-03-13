#include "evaluate.h"
#include "generate.h"
#include "move.h"
#include "types.h"
#include "stdbool.h"

static const int piece_value[6] = { 100, 300, 300, 500, 900, 10000 };

static const int pawn_table_w[64] = { 0,  0,  0,  0,  0,  0,  0,  0,
										50, 50, 50, 50, 50, 50, 50, 50,
										10, 10, 20, 30, 30, 20, 10, 10,
										5,  5, 10, 25, 25, 10,  5,  5,
										0,  0,  0, 20, 20,  0,  0,  0,
										5, -5,-10,  0,  0,-10, -5,  5,
										5, 10, 10,-20,-20, 10, 10,  5,
										0,  0,  0,  0,  0,  0,  0,  0
};

static const int king_table_w[64] = { -30,-40,-40,-50,-50,-40,-40,-30,
												-30,-40,-40,-50,-50,-40,-40,-30,
												-30,-40,-40,-50,-50,-40,-40,-30,
												-30,-40,-40,-50,-50,-40,-40,-30,
												-20,-30,-30,-40,-40,-30,-30,-20,
												-10,-20,-20,-20,-20,-20,-20,-10,
												20, 20,  0,  0,  0,  0, 20, 20,
												20, 30, 10,  0,  0, 10, 30, 20
										};
static const int king_table_endgame_w[64] = { -50,-40,-30,-20,-20,-30,-40,-50,
												-30,-20,-10,  0,  0,-10,-20,-30,
												-30,-10, 20, 30, 30, 20,-10,-30,
												-30,-10, 30, 40, 40, 30,-10,-30,
												-30,-10, 30, 40, 40, 30,-10,-30,
												-30,-10, 20, 30, 30, 20,-10,-30,
												-30,-30,  0,  0,  0,  0,-30,-30,
												-50,-30,-30,-30,-30,-30,-30,-50
												};
static const int queen_table_w[64] = { -20,-10,-10, -5, -5,-10,-10,-20,
											-10,  0,  0,  0,  0,  0,  0,-10,
											-10,  0,  5,  5,  5,  5,  0,-10,
											-5,  0,  5,  5,  5,  5,  0, -5,
											0,  0,  5,  5,  5,  5,  0, -5,
											-10,  5,  5,  5,  5,  5,  0,-10,
											-10,  0,  5,  0,  0,  0,  0,-10,
											-20,-10,-10, -5, -5,-10,-10,-20
										};
static const int rook_table_w[64] = { 0,  0,  0,  0,  0,  0,  0,  0,
										5, 10, 10, 10, 10, 10, 10,  5,
										-5,  0,  0,  0,  0,  0,  0, -5,
										-5,  0,  0,  0,  0,  0,  0, -5,
										-5,  0,  0,  0,  0,  0,  0, -5,
										-5,  0,  0,  0,  0,  0,  0, -5,
										-5,  0,  0,  0,  0,  0,  0, -5,
										0,  0,  0,  5,  5,  0,  0,  0
									};
static const int bishop_table_w[64] = { -20,-10,-10,-10,-10,-10,-10,-20,
										-10,  0,  0,  0,  0,  0,  0,-10,
										-10,  0,  5, 10, 10,  5,  0,-10,
										-10,  5,  5, 10, 10,  5,  5,-10,
										-10,  0, 10, 10, 10, 10,  0,-10,
										-10, 10, 10, 10, 10, 10, 10,-10,
										-10,  5,  0,  0,  0,  0,  5,-10,
										-20,-10,-10,-10,-10,-10,-10,-20
									};
static const int knight_table_w[64] = {-50,-40,-30,-30,-30,-30,-40,-50,
										-40,-20,  0,  0,  0,  0,-20,-40,
										-30,  0, 10, 15, 15, 10,  0,-30,
										-30,  5, 15, 20, 20, 15,  5,-30,
										-30,  0, 15, 20, 20, 15,  0,-30,
										-30,  5, 10, 15, 15, 10,  5,-30,
										-40,-20,  0,  5,  5,  0,-20,-40,
										-50,-40,-30,-30,-30,-30,-40,-50
									};

int tablescore(int piece, int color, int square, int endgame) {
	int new_square;
	if (color == WHITE)
		new_square = 63;
	else
		new_square = 63 - square;
	if (piece == PAWN)
		return (pawn_table_w[new_square]); /*flip to black*/
	else if (piece == KNIGHT)
		return (knight_table_w[new_square]);
	else if (piece == QUEEN)
		return (queen_table_w[new_square]);
	else if (piece == ROOK)
		return (rook_table_w[new_square]);
	else if (piece == KING && endgame)
		return (king_table_endgame_w[new_square]);
	else if (piece == KING)
		return (king_table_w[new_square]);
	else if (piece == BISHOP)
		return (bishop_table_w[new_square]);
	return (0);
}

/* How more steps a pawn have made how higher the value */
int	pawn_score(int rank, int piece) {
	if (COLOR(piece) == WHITE)
		return (rank);
	else
		return (7 - rank); 
}

/* Protect the king, not wanting him to go to the middle of the game */
int	king_middle_field_score(int square) {
	int rank;
	int file;

	rank = RANK(square);
	file = FILE(square);
	if (rank > 1 && rank < 6 && file > 1 && file < 6)
		return (10);
	return (0);
}

/*Check if the queens are still in play*/
int queens_still_in_play(const struct position *pos) {
	int square;
	int piece;
	int count = 2;
	for (square = 0; square < 64; square++){
		piece = pos->board[square];
		if (piece != NO_PIECE){
			if (TYPE(piece) == QUEEN)
				count--;
		}
	}
	if (count == 0)
		return (0);
	return (1);
}

int evaluate(const struct position *pos) {
	int score[2] = { 0, 0 };
	int square;
	int piece;
	int	no_queens = 0;

	if (!queens_still_in_play(pos))
		no_queens = 1;
	for (square = 0; square < 64; square++) {
		piece = pos->board[square];
		
		if (piece != NO_PIECE)
		{
			score[COLOR(piece)] += piece_value[TYPE(piece)];
			
			score[COLOR(piece)] += tablescore(TYPE(piece), COLOR(piece), square, no_queens);

			if (TYPE(piece) == PAWN)
				score[COLOR(piece)] += pawn_score(RANK(square), piece);

			if (TYPE(piece) == KING)
				score[COLOR(piece)] -= king_middle_field_score(square);

		}
	}
	return score[pos->side_to_move] - score[1 - pos->side_to_move];
}
