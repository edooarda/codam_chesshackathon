#include "evaluate.h"
#include "generate.h"
#include "move.h"
#include "types.h"
#include "stdbool.h"

static const int piece_value[6] = { 100, 300, 300, 500, 900, 10000 };

/* Losing a piece is bad! Winning is good*/
void	attack_score(int *score_total, const struct position *pos, int square) {
	struct move		move;
	size_t			i;
	int				piece;
	
	for (i = 0; i < 64; i++)
	{
		piece = pos->board[i];
		if (piece != NO_PIECE && COLOR(pos->board[square]) != pos->side_to_move)
		{
			move.from_square = i;
			move.to_square = square;
			move.promotion_type = TYPE(piece);
			if (is_legal(pos, move))
			{
				piece = pos->board[square];
				(*score_total) -= piece_value[TYPE(piece)];
				piece = pos->board[square];
				if (piece != NO_PIECE)
					(*score_total) -= piece_value[TYPE(piece)];
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
bool queens_still_in_play(const struct position *pos) {
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
		return (false);
	return (true);
}

int evaluate(const struct position *pos) {
	int score[2] = { 0, 0 };
	int square;
	int piece;

	if (!queens_still_in_play(pos)){
		//Change king score board.
	}
	for (square = 0; square < 64; square++) {
		piece = pos->board[square];
		
		if (piece != NO_PIECE)
		{
			score[COLOR(piece)] += piece_value[TYPE(piece)];
			if (TYPE(piece) == PAWN)
				score[COLOR(piece)] += pawn_score(RANK(square), piece);

			if (TYPE(piece) == KING)
				score[COLOR(piece)] -= king_middle_field_score(square);
			
		}
	}
	return score[pos->side_to_move] - score[1 - pos->side_to_move];
}
