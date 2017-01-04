#ifndef DECK_H_
#define DECK_H_

#include <vector>
#include <cstdlib>
#include "card.h"
#include <algorithm>

using namespace std;

class deck
{
public:
	//Adds n card to top of deck.
	void add_card(const card& card, unsigned int n);
	//Count how many cards of given type are there in the deck
	unsigned int count_cards(const card& c);
	//Removes one sample of specific card. False if card is not in deck
	bool remove_card(const card& c);
	//Shuffle deck
	void shuffle();
	//Returns next card in deck, without removing it
	card peek_top();
	//Returns next card in deck, removing it
	card pick_top();
	//Returns size of deck
	unsigned int size();
	//Removes all cards from deck
	void clear();
private:
	vector<card> card_deck;	//Using vector instead of list, since vector has random access to elements.
};

#endif	//DECK_H_