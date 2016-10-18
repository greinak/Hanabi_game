#include "Package_start_info.h"

#pragma pack(push,1)
//Package
typedef struct package_data
{
	unsigned char header;
	card_hanabi	client_hand[HAND_CARDS];
	card_hanabi	server_hand[HAND_CARDS];
}package_data;
#pragma pack(pop)

Package_start_info::Package_start_info()
{
	package_data* p_data = (package_data*)raw_data;
	card_hanabi card;
	card.color = NO_COLOR;
	card.number = NO_NUMBER;
	this->raw_data_size = sizeof(package_data);
	p_data->header = START_INFO_ID;
	for (unsigned int i = 0; i < HAND_CARDS; i++)
	{
		p_data->client_hand[i] = card; 
		p_data->server_hand[i] = card;
	}
}

bool Package_start_info::set_info(const card_hanabi client_hand[HAND_CARDS], const card_hanabi server_hand[HAND_CARDS])
{
	package_data* p_data = (package_data*)raw_data;
	if (check_cards(client_hand, server_hand))
	{
		for (unsigned int i = 0; i < HAND_CARDS; i++)
		{
			p_data->client_hand[i] = client_hand[i];
			p_data->server_hand[i] = server_hand[i];
		}
		return true;
	}
	return false;
}

bool Package_start_info::get_info(card_hanabi client_hand[HAND_CARDS], card_hanabi server_hand[HAND_CARDS])
{
	package_data* p_data = (package_data*)raw_data;
	if (check_cards(p_data->client_hand, p_data->server_hand))
	{
		for (unsigned int i = 0; i < HAND_CARDS; i++)
		{
			client_hand[i] = p_data->client_hand[i];
			server_hand[i] = p_data->server_hand[i];
		}
		return true;
	}
	return false;
}

bool Package_start_info::is_raw_data_valid(const char * data, size_t data_length)
{
	const package_data* p_data = (const package_data*)data;
	if (data_length == sizeof(package_data) && p_data->header == START_INFO_ID)
		return check_cards(p_data->client_hand, p_data->server_hand);
	return false;
}

bool Package_start_info::check_cards(const card_hanabi client_hand[HAND_CARDS], const card_hanabi server_hand[HAND_CARDS])
{
	card_hanabi card_A, card_B;
	bool cards_ok = true;
	for (unsigned int i = 0; i < HAND_CARDS * 2; i++)
	{
		if (i < HAND_CARDS)
			card_A = client_hand[i];
		else
			card_A = server_hand[i-HAND_CARDS];
		if ((ONE > card_A.number || card_A.number > SIX) ||
			(
				card_A.color != RED && card_A.color != YELLOW &&
				card_A.color != GREEN && card_A.color != BLUE &&
				card_A.color != WHITE
				)
			)
		{
			cards_ok = false;
			break;
		}
		for (unsigned int j = i + 1; j < HAND_CARDS * 2; j++)
		{
			if (j < HAND_CARDS)
				card_B = client_hand[i];
			else
				card_B = server_hand[i- HAND_CARDS];
			if (card_A.color == card_B.color && card_A.number == card_B.number)
			{
				cards_ok = false;
				break;
			}
		}
		if (!cards_ok)
			break;
	}
	return cards_ok;
}
