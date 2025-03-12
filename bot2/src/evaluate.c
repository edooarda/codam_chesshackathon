#include "evaluate.h"
#include "generate.h"
#include "move.h"
#include "types.h"

static const int piece_value[6] = { 100, 300, 300, 500, 900, 10000 };

/* Losing a piece is bad! Winning is good*/
void	attack_score(int *score_total, const struct position *pos, int square) {
	struct move		moves[200];
	size_t			count = generate_legal_moves(pos, moves);
	size_t			i;
	int				attact_square;
	int				attacking_square;
	
	for (i = 0; i < count; i++)
	{
		if (moves[i].to_square == square)
		{
			attact_square = pos->board[square];
			if (attact_square != NO_PIECE)
				(*score_total) -= piece_value[TYPE(attact_square)] * 2;
			
			attacking_square = pos->board[moves[i].from_square];
			if (attacking_square != NO_PIECE)
				(*score_total) += piece_value[TYPE(attacking_square)];
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
		return (100);
	return (0);
}

int evaluate(const struct position *pos) {
	int score[2] = { 0, 0 };
	int square;
	int piece;

	for (square = 0; square < 64; square++) {
		piece = pos->board[square];
		
		if (piece != NO_PIECE)
		{
			score[COLOR(piece)] += piece_value[TYPE(piece)];

			/* PAWN SCORE */
			if (TYPE(piece) == PAWN)
				score[COLOR(piece)] += pawn_score(RANK(square), piece);

			/* ATTACK SCORE */
			attack_score(&score[COLOR(piece)], pos, square);
			
			/* KING SCORE */
			if (TYPE(piece) == KING)
				score[COLOR(piece)] -= king_middle_field_score(square);
			
		}
	}
	return score[pos->side_to_move] - score[1 - pos->side_to_move];
}
