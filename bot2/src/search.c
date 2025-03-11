#include "search.h"
#include "evaluate.h"
#include "generate.h"

#include <limits.h>

/* has to contain more moves for finding a pattren? */
static struct move last_move;
static int index = 0;

static int is_last_move(struct move move) {
	if (index == 0)
		return (0);
	if (move.from_square == last_move.from_square && 
		move.to_square == last_move.to_square &&
		move.promotion_type == last_move.promotion_type)
		return (1);
	if (move.from_square == last_move.to_square && 
		move.to_square == last_move.from_square &&
		move.promotion_type == last_move.promotion_type)
		return (1);
	return (0);
}

void	add_move(struct move move) {
	last_move.from_square = move.to_square;
	last_move.to_square = move.from_square;
	last_move.promotion_type = move.promotion_type;
	index++;
}

struct search_result minimax2(time_t start_time, const struct position *pos, int depth, int alpha, int beta) {
	struct search_result	result;
	struct move				moves[MAX_MOVES];
	int 					score;
	size_t					count;
	size_t					index;
	time_t					now;

	now = time(NULL);
	if (depth == 0   || now - start_time > 30) {
		result.score = evaluate(pos);
		return (result);
	}

	count = generate_legal_moves(pos, moves);
	if (count == 0) {
		result.score = evaluate(pos);
		return (result);
	}

	result.score = INT_MIN;
	for (index = 0; index < count; index++) {
		struct position copy = *pos;

		if (is_last_move(moves[index]))
			continue ;
		do_move(&copy, moves[index]);

		score = -minimax2(start_time, &copy, depth - 1, -alpha, -beta).score;
		if (score > result.score)
		{
			result.score = score;
			result.move = moves[index];
		}
		if (score > alpha)
			alpha = score;
		if (alpha >= beta)
			break ;
	}
	return (result);
}

struct search_result minimax(const struct position *pos, int depth) {
	struct search_result result;

	result.score = -1000000;

	if (depth == 0) {
		/* we have reached our search depth, so evaluate the position.       */
		result.score = evaluate(pos);
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
			score = -minimax(&copy, depth - 1).score;

			/* update the best move if we found a better one.                */
			if (score > result.score) {
				result.move = moves[index];
				result.score = score;
			}
		}
	}

	return result;
}

/*
struct move search(const struct search_info *info) {
	return minimax(info->pos, 4).move;

} */

struct move search(const struct search_info *info) {
	const time_t start_time = time(NULL);
	const struct move move = minimax2(start_time, info->pos, 6, INT_MIN, INT_MAX).move;
	add_move(move);
	return (move);
}
