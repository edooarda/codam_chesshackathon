#include "evaluate.h"
#include "generate.h"
#include "move.h"
#include "types.h"

static const int piece_value[6] = { 100, 300, 300, 500, 900, 10000 };

/* not working
int	attact_score(const struct position *pos, int square) {
	struct move		moves[200];
	struct position	copy = *pos;
	size_t			count;
	size_t			i;
	int				score = 0;
	int				attact_square;
	
	count = generate_legal_moves(&copy, moves); 
	for (i = 0; i < count; i++)
	{
		if (moves[i].to_square == square)
		{
			attact_square = pos->board[square];
			if (attact_square != NO_PIECE)
                score -= piece_value[TYPE(attact_square)];
			else
			{
				attact_square = pos->board[moves[i].from_square];
				score += piece_value[TYPE(attact_square)];
			}
		}
    }
    return (score);
} 
*/

int evaluate(const struct position *pos) {
	int score[2] = { 0, 0 };
	int square;
	int piece;
	int	rank = 1;
	int file = 1;

	for (square = 0; square < 64; square++) {
		piece = pos->board[square];
		score[COLOR(piece)] += piece_value[TYPE(piece)];

		/* also not working */
		if (piece != NO_PIECE) {
			/* How more steps a pawn have made how higher the value*/
			if (piece == PAWN) {
				rank = RANK(square);
				score[COLOR(piece)] += rank;
			}
			/* How less pieces, how more the */

			/* Losing a piece is bad! */
			/*score[COLOR(piece)] += attact_score(pos, square);*/
			
			/* Protect the king, not wanting him to go to the middle of the game */
			if (piece == KING) {
				rank = RANK(square);
				file = FILE(square);
				if (rank > 2 && rank < 6 && file > 2 && rank < 6)
					score[COLOR(piece)] -= 100;
			}	
		}
	}
	return score[pos->side_to_move] - score[1 - pos->side_to_move];
}
