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

int	is_possible_move(struct move move, const struct position *pos) {
	int color;
	int	i;
	int	reach;
	const int	knight_moves[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
	const int	bishop_directions[4] = { -9, -7, 7, 9 };
	const int	rook_directions[4] = { -8, 8, -1, 1 };
	const int	king_moves[] = {-9, -8, -7, -1, 1, 7, 8, 9};
	
	if (move.promotion_type == PAWN) {
		if (COLOR(pos->board[move.from_square]) == WHITE)
			color = 1;
		else
			color = -1;
		if (move.from_square + (7 * color) == move.to_square ||
			move.from_square + (9 * color) == move.to_square)
			return (1);
	}
	if (move.promotion_type == KNIGHT) {
		for (i = 0; i < 8; i++)
		{
			if (move.from_square + knight_moves[i] == move.to_square)
				return (1);
		}
		return (0);
	}
	if (move.promotion_type == BISHOP || move.promotion_type == QUEEN) {
		for (i = 0; i < 4; i++)
		{
			reach = move.from_square;
			while (1)
			{
				reach += bishop_directions[i];
				if (reach < 0 || reach >= 64 || (FILE(reach) == 0 && FILE(move.from_square) == 7) || (FILE(reach) == 7 && FILE(move.from_square) == 0))
					break;
				if (reach == move.to_square)
					return (1);
			}
		}
		if (move.promotion_type == BISHOP)
			return (0);
	}
	if (move.promotion_type == ROOK || move.promotion_type == QUEEN) {
		for (i = 0; i < 4; i++)
		{
			reach = move.from_square;
			while (1)
			{
				reach += rook_directions[i];
				if (reach < 0 || reach >= 64)
					break;
				if (reach == move.to_square)
					return (1);
			}
		}
		return (0);
	}
	if (move.promotion_type == KING) {
		for (i = 0; i < 8; i++)
		{
			if (move.from_square + king_moves[i] == move.to_square)
				return (1);
		}
		return (0);
	}
	return (0);
}

/* Losing a piece is bad! Winning is good*/
void	attack_score(int *score_total, const struct position *pos, int square) {
	struct move		move;
	size_t			i;
	int				piece;
	const int		value[6] = { 10, 30, 30, 50, 90, 1000 };
	
	for (i = 0; i < 64; i++)
	{
		piece = pos->board[i];
		if (piece != NO_PIECE && COLOR(pos->board[square]) != pos->side_to_move)
		{
			move.from_square = i;
			move.to_square = square;
			move.promotion_type = TYPE(piece);
			
			if (is_possible_move(move, pos))
			{
				piece = pos->board[square];
				(*score_total) -= value[TYPE(piece)];
			}
		}
	}
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
