#include "search.h"
#include "evaluate.h"
#include "generate.h"
#include "types.h"

#include <limits.h>
#include <time.h>

static struct move	last_moves[3];
static int			move_index = 0;

static int is_last_move(struct move move) {
	int	i;

	for (i = 0; i < 3; i++)
	{
		if (move.from_square == last_moves[i].from_square && 
			move.to_square == last_moves[i].to_square &&
			move.promotion_type == last_moves[i].promotion_type)
			return (1);
	}
	return (0);
}

void add_move(struct move move) {
	last_moves[move_index % 3] = move;
	move_index++;
}

/* adding SEE (Static Exchange Evaluation) ? */

/* MVV-LVA: Most Valuable Victim - Least Valuable Attacker */
static const struct position *sort_pos;

int compare_moves(const void *a, const void *b) {
	static const int	piece_value[6] = { 100, 300, 300, 500, 900, 10000};
	const struct move	*move1 = (const struct move *)a;
	const struct move	*move2 = (const struct move *)b;
	int					captured1 = sort_pos->board[move1->to_square];
	int					captured2 = sort_pos->board[move2->to_square];
	int					piece1 = sort_pos->board[move1->from_square];
	int					piece2 = sort_pos->board[move2->from_square];
	int					score1 = 0;
	int					score2 = 0;

	if (captured1 != NO_PIECE)
		score1 += piece_value[TYPE(captured1)] * 10 - piece_value[TYPE(piece1)];
	if (captured2 != NO_PIECE)
		score2 += piece_value[TYPE(captured2)] * 10 - piece_value[TYPE(piece2)];

	if (move1->promotion_type != NO_PIECE)
		score1 += piece_value[move1->promotion_type];
	if (move2->promotion_type != NO_PIECE)
		score2 += piece_value[move2->promotion_type];
    return (score2 - score1);
}

struct search_result minimax2(time_t start_time, const struct position *pos, int depth, int alpha, int beta) {
	struct search_result	result;
	struct move				moves[MAX_MOVES];
	int 					score;
	size_t					count;
	size_t					index;
	time_t					now;

	if (depth == 0)
	{
		result.score = evaluate(pos);
		return (result);
	}

	count = generate_legal_moves(pos, moves);
	if (count == 0)
	{
		result.score = evaluate(pos);
		return (result);
	}

	sort_pos = pos;
	qsort(moves, count, sizeof(struct move), compare_moves);

	result.score = INT_MIN;
	result.move = moves[0];

	for (index = 0; index < count; index++) {
		struct position copy = *pos;

		if (is_last_move(moves[index]))
			continue ;

		do_move(&copy, moves[index]);


		if (index % 5 == 0)
		{ 
			now = time(NULL);
			if (now - start_time >= 45)
				break;
		}

		score = -minimax2(start_time, &copy, depth - 1, -alpha, -beta).score * ((6 - depth) / 2);
		if (score > result.score)
		{
			result.score = score;
			result.move = moves[index];
		}
		if (score > alpha)
		{
			alpha = score;
			if (alpha >= beta)
				return result;
		}
	}
	return (result);
}

struct search_result minimax(const struct position *pos, int depth) {
	struct search_result result;

	result.score = -1000000;

	if (depth == 0) {
		result.score = evaluate(pos) * (6 * depth);
	} else {
		struct move moves[MAX_MOVES];
		size_t count = generate_legal_moves(pos, moves);
		size_t index;

		for (index = 0; index < count; index++) {
			struct position copy = *pos;
			int score;

			/* do a move, the current player in `copy` is then the opponent, */
			/* and so when we call minimax we get the score of the opponent. */
			do_move(&copy, moves[index]);

			/* minimax is called recursively. this call returns the score of */
			/* the opponent, so we must negate it to get our score.          */
			score = -minimax(&copy, depth - 1).score * (6 - depth);

			/* update the best move if we found a better one.                */
			if (score > result.score) {
				result.move = moves[index];
				result.score = score;
			}
		}
	}

	return result;
}

struct move search(const struct search_info *info) {
	const time_t	start_time = time(NULL);
	int				depth = 5;
	struct move		move;

	move = minimax2(start_time, info->pos, depth, INT_MIN, INT_MAX).move;
	add_move(move);
	return (move);
}
