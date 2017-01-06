#ifndef CARD_H_
#define CARD_H_

//Hanabi card

#define HANABI_TOTAL_COLORS 5
#define HANABI_TOTAL_NUMBERS 5
#define HANABI_HAND_SIZE	6

typedef enum { COLOR_BLUE, COLOR_GREEN, COLOR_RED, COLOR_WHITE, COLOR_YELLOW, NO_COLOR} card_color_t;
static const char* color_string[] = { "blue","green","red","white","yellow","no color" };	//For loading.
typedef enum {NUMBER_ONE, NUMBER_TWO, NUMBER_THREE, NUMBER_FOUR, NUMBER_FIVE, NO_NUMBER} card_number_t;
enum { TOTAL_ONES = 3, TOTAL_TWOS = 2, TOTAL_THREES = 2, TOTAL_FOURS = 2, TOTAL_FIVES = 1 };
#define HANABI_TOTAL_CARDS (TOTAL_ONES+TOTAL_TWOS+TOTAL_THREES+TOTAL_FOURS+TOTAL_FIVES)*HANABI_TOTAL_COLORS



class card
{
public:
	//Initialize card. 
	card(card_color_t color = NO_COLOR, card_number_t number = NO_NUMBER);
	//Get card data.
	card_color_t get_color() const;
	card_number_t get_number() const;
	bool operator==(const card& other) const;
	bool operator!=(const card& other) const;
private:
	card_color_t color;
	card_number_t number;
};

#endif