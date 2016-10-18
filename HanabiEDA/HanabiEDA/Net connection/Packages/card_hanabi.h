#ifndef CARD_HANABI_H_
#define CARD_HANABI_H_
typedef enum hanabi_color : unsigned char  {RED = 'R', YELLOW ='Y', GREEN = 'G', BLUE = 'B', WHITE = 'W', NO_COLOR = 0xFF, ANY_COLOR = 0} hanabi_color;
typedef enum hanabi_number : unsigned char {ONE = '1', TWO, THREE, FOUR, FIVE, SIX, NO_NUMBER = 0xFF, ANY_NUMBER = 0} hanabi_number;
typedef enum hanabi_card_id : unsigned char {FIRST = 1, SECOND, THIRD, FOURTH, FIFTH, SIXTH} hanabi_card_id;
#define HAND_CARDS 6
#pragma pack(push,1)
typedef struct card_hanabi
{
	hanabi_number number;
	hanabi_color color;
}card_hanabi;
#pragma pack(pop)
#endif	//CARD_HANABI_H_