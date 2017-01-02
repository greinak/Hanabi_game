#include "deck.h"
#include <algorithm>

void deck::add_card(const card & card, unsigned int n)
{
	for (unsigned int i = 0; i < n; i++)
		card_deck.push_back(card);
}

unsigned int deck::count_cards(const card & c)
{
	return count(card_deck.begin(), card_deck.end(), c);
}

bool deck::remove_card(const card & c)
{
	bool found = false;
	vector<card>::iterator it = find(card_deck.begin(), card_deck.end(), c);
	if ((found = (it != card_deck.end())))
		card_deck.erase(it);
	return found;
}

void deck::shuffle()
{
	random_shuffle(card_deck.begin(), card_deck.end());
}

unsigned int deck::size()
{
	return card_deck.size();
}

card deck::peek_top()
{
	return card_deck.back();
}

card deck::pick_top()
{
	card return_card = peek_top();
	card_deck.pop_back();
	return return_card;
}
