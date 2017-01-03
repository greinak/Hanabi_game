#ifndef CARD_H_
#define CARD_H_

//Hanabi card

#define HANABI_TOTAL_COLORS 5
#define HANABI_TOTAL_NUMBERS 5

typedef enum {NO_COLOR = 0xFF, COLOR_YELLOW = 'Y',  COLOR_RED = 'R', COLOR_BLUE = 'B', COLOR_WHITE = 'W' , COLOR_GREEN = 'G'} card_color_t;
typedef enum {NO_NUMBER = 0xFF, NUMBER_ONE = '1', NUMBER_TWO = '2', NUMBER_THREE = '3', NUMBER_FOUR = '4', NUMBER_FIVE = '5'} card_number_t;
class card
{
public:
	//Initialize card. 
	card(card_color_t color = NO_COLOR, card_number_t number = NO_NUMBER);
	//Get card data.
	card_color_t get_color() const;
	card_number_t get_number() const;
	bool operator==(const card& other);
private:
	card_color_t color;
	card_number_t number;
};

#endif