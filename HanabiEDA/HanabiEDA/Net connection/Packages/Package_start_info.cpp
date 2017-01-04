#include "Package_start_info.h"
#include <algorithm>
#include "card_hanabi.h"

using namespace std;

#pragma pack(push,1)
//Package
typedef struct package_data
{
	unsigned char header;
	card_hanabi	receptor_hand[TOTAL_HAND_CARDS];
	card_hanabi	transmisor_hand[TOTAL_HAND_CARDS];
}package_data;
#pragma pack(pop)

Package_start_info::Package_start_info()
{
	package_data* p_data = (package_data*)raw_data;
	card_hanabi card;
	card.color = NO_CARD_COLOR;
	card.number = NO_CARD_NUMBER;
	this->raw_data_size = sizeof(package_data);
	p_data->header = START_INFO_ID;
	for (unsigned int i = 0; i < HAND_CARDS; i++)
	{
		p_data->receptor_hand[i] = card;
		p_data->transmisor_hand[i] = card;
	}
}

bool Package_start_info::set_info(const card receptor_hand[TOTAL_HAND_CARDS], const card transmisor_hand[TOTAL_HAND_CARDS])
{
	package_data* p_data = (package_data*)raw_data;
	const card((*hands[2])) = { receptor_hand , transmisor_hand };
	card_hanabi(*package_hands[2]) = { p_data->receptor_hand ,p_data->transmisor_hand };
	bool cards_ok = true;
	for(unsigned int i = 0 ; i < 2 && cards_ok; i++)
		for (unsigned int j = 0; j < HAND_CARDS && cards_ok ; j++)
		{
			card_hanabi card;
			switch (hands[i][j].get_color())
			{
			case COLOR_RED:
				card.color = RED;
				break;
			case COLOR_GREEN:
				card.color = GREEN;
				break;
			case COLOR_BLUE:
				card.color = BLUE;
				break;
			case COLOR_YELLOW:
				card.color = YELLOW;
				break;
			case COLOR_WHITE:
				card.color = WHITE;
				break;
			default:
				cards_ok = false;
			}
			switch (hands[i][j].get_number())
			{
			case NUMBER_ONE:
				card.number = ONE;
				break;
			case NUMBER_TWO:
				card.number = TWO;
				break;
			case NUMBER_THREE:
				card.number = THREE;
				break;
			case NUMBER_FOUR:
				card.number = FOUR;
				break;
			case NUMBER_FIVE:
				card.number = FIVE;
				break;
			default:
				cards_ok = false;
			}
			if(cards_ok)
				package_hands[i][j] = card;
		}
	return cards_ok;
}

bool Package_start_info::get_info(card receptor_hand[TOTAL_HAND_CARDS], card transmisor_hand[TOTAL_HAND_CARDS])
{
	package_data* p_data = (package_data*)raw_data;
	card((*hands[2])) = { receptor_hand , transmisor_hand };
	const card_hanabi(*package_hands[2]) = { p_data->receptor_hand ,p_data->transmisor_hand };
	bool cards_ok = true;
	card_color_t color;
	card_number_t number;
	for (unsigned int i = 0; i < 2 && cards_ok; i++)
		for (unsigned int j = 0; j < HAND_CARDS && cards_ok; j++)
		{
			switch (package_hands[i][j].color)
			{
			case RED:
				color = COLOR_RED;
				break;
			case GREEN:
				color = COLOR_GREEN;
				break;
			case BLUE:
				color = COLOR_BLUE;
				break;
			case YELLOW:
				color = COLOR_YELLOW;
				break;
			case WHITE:
				color = COLOR_WHITE;
				break;
			default:
				cards_ok = false;
			}
			switch (package_hands[i][j].number)
			{
			case ONE:
				number = NUMBER_ONE;
				break;
			case TWO:
				number = NUMBER_TWO;
				break;
			case THREE:
				number = NUMBER_THREE;
				break;
			case FOUR:
				number = NUMBER_FOUR;
				break;
			case FIVE:
				number = NUMBER_FIVE;
				break;
			default:
				cards_ok = false;
			}
			if(cards_ok)
				hands[i][j] = card(color, number);
		}
	return cards_ok;
}

bool Package_start_info::is_raw_data_valid(const char * data, size_t data_length)
{
	bool card_ok = false;
	const package_data* p_data = (const package_data*)data;
	if (data_length == sizeof(package_data) && p_data->header == START_INFO_ID)
	{
		const card_hanabi(*hands[2]) = { p_data->receptor_hand,p_data->transmisor_hand };
		unsigned char colors[] = { BLUE,GREEN,RED,WHITE,YELLOW };
		unsigned char numbers[] = { ONE,TWO,THREE,FOUR,FIVE };
		unsigned char color_length = sizeof(colors) / sizeof(colors[0]);
		unsigned char number_length = sizeof(numbers) / sizeof(numbers[0]);
		card_ok = true;
		for (int i = 0; i < 2 && card_ok; i++)
		{
			for (unsigned int j = 0; j < TOTAL_HAND_CARDS && card_ok; j++)
			{
				card_ok &= find(colors, colors + color_length, hands[i][j].color) != colors + color_length;
				card_ok &= find(numbers, numbers + number_length, hands[i][j].number) != numbers + number_length;
			}
		}
	}
	return card_ok;
}
