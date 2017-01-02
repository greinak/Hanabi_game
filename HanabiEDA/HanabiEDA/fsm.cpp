#include <stdlib.h>
#ifdef JUSTFORTESTING
typedef void(*action)(void);

//#################################
//## HANABI FINITE STATE MACHINE ##
//#################################

typedef enum ev	//Any event may arrive at any state!!
{
//<--States expected while in initialization-->
	NAME,			//Received name request
	NAMEIS,			//Received name
	START_INFO,		//Received start info
	SW_WHO_I,		//Software decides local starts
	SW_WHO_YOU,		//Software decides remote starts
	I_START,		//Remote says it starts
	YOU_START,		//Remote says local starts
//<--States expected while in end of game -->
	LOCAL_PA,		//Local player decides to play again
	LOCAL_GO,		//Local player does not want to play again
	REMOTE_PA,		//Remote player decides to play again
	REMOTE_GO,		//Remote player does not want to play again
//<--States expected while in game-->
	LOCAL_GIVE_CLUE,		//Local player gives clue
	LOCAL_PLAY,				//Local player plays a card
	LOCAL_DISCARD,			//Local player discards a card
	SW_DRAW_NEXT,			//Draw next card (software tells us this card is not the last one)
	SW_DRAW_LAST,			//Draw next card, (software tells us this card is the last one)
	REMOTE_WE_WON,			//Remote player informs we won
	REMOTE_WE_LOST,			//Remote player informs we lost
	REMOTE_MATCH_IS_OVER,	//Remote player informs match is over
	REMOTE_GIVE_CLUE,		//Local player gives clue
	REMOTE_PLAY,			//Local player plays a card
	REMOTE_DISCARD,			//Local player discards a card
	REMOTE_PLAY_WON,		//Local player plays a card, and we won
	REMOTE_PLAY_LOST,		//Local player plays a card, and we lost
	DRAW_NEXT,				//Received draw next card
	DRAW_LAST,				//Received draw next card, this is the last one
	DRAW_FAKE,				//Received empty draw
//<--Other events-->
	BAD,			//Inform communication error to remote machine
	ERROR_EV,		//Remote machine informed a communication error		
	QUIT,			//Remote left the game
//<--Control events-->
	ACK,			//ACK
//<--FSM behaviour related events-->
	GND,					//Identifier of end of common event state blocks list. Not a real event.
};

typedef struct state_block
{
	ev	block_ev;
	action action;
	const struct state_block* next_state;
};

//Actions
//Notation: state__event


//$$$$$$$$$$$$$$$$$$$$$$$
//$ Finit state machine $
//$$$$$$$$$$$$$$$$$$$$$$$


//NOTE: Separated in two parts:
//First: States and actions while NOT IN GAME
//Second: States and actions while IN GAME
//Each part is separated in groups, in each group we have actions first ($) and then states(%)
//This organization of states and events DOES NOT AFFECT the behaviour of the FSM, since this is just an abstract organization. 

//First, let's define a do nothing function
void do_nothing(void) { return; }
//And fake state. Will never enter this state
const state_block no_state;

//<-- While NOT in game!!! -->       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: HANDSHAKE, branch: Server $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void s_wait_nameis__nameis(void)
{
	//Send ack
}
void s_wait_name__name(void)
{
	//Send nameis
}
void s_wait_nameis_ack__ack(void)
{
	//Send start info
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: HANDSHAKE, branch: Server %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block s_wait_nameis[] =
{
	{NAMEIS,s_wait_nameis__nameis,s_wait_name},
	{GND,do_nothing,&no_state}
};

const state_block s_wait_name[] =
{
	{NAME,s_wait_name__name,s_wait_nameis_ack},
	{GND,do_nothing,&no_state }
};

const state_block s_wait_nameis_ack[] =
{
	{ACK,s_wait_nameis_ack__ack,wait_start_info_ack},  // --> GO TO INITIALIZATION
	{GND,do_nothing,&no_state}
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: HANDSHAKE, branch: Client $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void c_wait_name__name(void)
{
	//Send nameis
}
void c_wait_nameis_ack__ack(void)
{
	//Send name
}
void c_wait_nameis__nameis(void)
{
	//Send ack
}
void c_wait_start_info__start_info(void)
{
	//Send ack
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: HANDSHAKE, branch: Client %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block c_wait_name[] =
{
	{NAME,c_wait_name__name,c_wait_nameis_ack},
	{GND,do_nothing,&no_state }
};
const state_block c_wait_nameis_ack[] =
{
	{ACK,c_wait_nameis_ack__ack,c_wait_nameis},
	{GND,do_nothing,&no_state }
};
const state_block c_wait_nameis[] =
{
	{NAMEIS,c_wait_nameis__nameis,c_wait_start_info},
	{GND,do_nothing,&no_state }
};
const state_block c_wait_start_info[] =
{
	{START_INFO,c_wait_start_info__start_info,wait_who},	// --> GO TO INITIALIZATION
	{GND,do_nothing,&no_state}
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: INITIALIZATION, branch: MACHINE_A $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void wait_start_info_ack__ack(void)
{
	//Send 
}
void wait_software_who__sw_who_i(void)
{
	//Send 
}
void wait_software_who__sw_who_you(void)
{
	//Send 
}
void wait_i_start_ack__ack(void)
{
	//Send 
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: INITIALIZATION, branch: MACHINE_A %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block wait_start_info_ack[] =
{
	{ACK,wait_start_info_ack__ack,wait_software_who},
	{GND,do_nothing,&no_state }
};
const state_block wait_software_who[] =
{
	{SW_WHO_I,wait_software_who__sw_who_i,wait_i_start_ack},
	{SW_WHO_YOU,wait_software_who__sw_who_you,remote_player_turn},	//  --> GO TO GAME
	{GND,do_nothing,&no_state }
};
const state_block wait_i_start_ack[] =
{
	{ACK,wait_i_start_ack__ack,local_player_turn },	// --> GO TO GAME
	{GND,do_nothing,&no_state}
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: INITIALIZATION, branch: MACHINE_B $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void wait_who__i_start(void)
{
	//Send 
}
void wait_who__you_start(void)
{
	//Send 
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: INITIALIZATION, branch: MACHINE_B %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block wait_who[] =
{
	{I_START,wait_who__i_start,remote_player_turn},
	{YOU_START,wait_who__you_start,local_player_turn},	// --> GO TO GAME
	{GND,do_nothing,&no_state }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: END OF GAME, branch: A:INFORMER (The one who informed game result) $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void a_wait_remote_play_again_answer__remote_pa(void)
{
}
void a_wait_remote_play_again_answer__remote_go(void)
{
}
void a_wait_local_play_again_answer__local_pa(void)
{
}
void a_wait_local_play_again_answer__local_go(void)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: END OF GAME, branch: A:INFORMER (The one who informed game result) %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block a_wait_remote_play_again_answer[] =
{
	{REMOTE_PA,a_wait_remote_play_again_answer__remote_pa,a_wait_local_play_again_answer},
	{REMOTE_GO,a_wait_remote_play_again_answer__remote_go,NULL },	//break
	{GND,do_nothing,&no_state }
};
const state_block a_wait_local_play_again_answer[] =
{
	{LOCAL_PA,a_wait_local_play_again_answer__local_pa,wait_start_info_ack},		//  -->> GO TO INITIALIZATION
	{LOCAL_GO,a_wait_local_play_again_answer__local_go,NULL },		//break
	{GND,do_nothing,&no_state }
};


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: END OF GAME, branch: B:INFORMED (The one who received information about game result) $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void b_wait_local_play_again_answer__local_pa(void)
{
}
void b_wait_local_play_again_answer__local_go(void)
{
}
void b_wait_remote_play_again_answer__start_info(void)
{
}
void b_wait_remote_play_again_answer__remote_go(void)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: END OF GAME, branch: B:INFORMED (The one who received information about game result) %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block b_wait_local_play_again_answer[] =
{
	{LOCAL_PA,b_wait_local_play_again_answer__local_pa,b_wait_remote_play_again_answer },
	{LOCAL_GO,b_wait_local_play_again_answer__local_go,NULL}, //break
	{GND,do_nothing,&no_state }
};
const state_block b_wait_remote_play_again_answer[] =
{
	{START_INFO,b_wait_remote_play_again_answer__start_info,wait_who},		//  --> GO TO INITIALIZATION
	{REMOTE_GO,b_wait_remote_play_again_answer__remote_go,NULL}, //break
	{GND,do_nothing,&no_state }
};

//<-- While in game -->       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: IN GAME, branch: LOCAL_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void local_player_turn__local_give_clue()
{
}
void local_player_turn__local_play()
{
}
void local_player_turn__local_discard()
{
}
void wait_remote_player_response__ack()
{
}
void wait_remote_player_response__remote_we_won()
{
}
void wait_remote_player_response__remote_we_lost()
{
}
void wait_sw_draw__sw_draw_next()
{
}
void wait_sw_draw__sw_draw_last()
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: IN GAME, branch: LOCAL_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block local_player_turn[] =
{
	{LOCAL_GIVE_CLUE,local_player_turn__local_give_clue,remote_player_turn},
	{LOCAL_PLAY,local_player_turn__local_play,wait_remote_player_response},
	{LOCAL_DISCARD,local_player_turn__local_discard,wait_remote_player_response},
	{GND,do_nothing,&no_state },
};
const state_block wait_remote_player_response[] =
{
	{ACK,wait_remote_player_response__ack,wait_sw_draw},
	{REMOTE_WE_WON,wait_remote_player_response__remote_we_won,b_wait_local_play_again_answer},		//  --> GO TO END OF GAME
	{REMOTE_WE_LOST,wait_remote_player_response__remote_we_lost,b_wait_local_play_again_answer},		//  --> GO TO END OF GAME
	{ GND,do_nothing,&no_state}
};
const state_block wait_sw_draw[] =
{
	{SW_DRAW_NEXT,wait_sw_draw__sw_draw_next,remote_player_turn},
	{SW_DRAW_LAST,wait_sw_draw__sw_draw_last,sc_1_remote_player_turn},	//	--> GO TO GAME FINISHING SCENARIO 1
	{GND,do_nothing,&no_state }
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: IN GAME, branch: REMOTE_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void remote_player_turn__remote_give_clue()
{
}
void remote_player_turn__remote_play()
{
}
void remote_player_turn__remote_discard()
{
}
void remote_player_turn__remote_play_won()
{
}
void remote_player_turn__remote_play_lost()
{
}
void wait_draw__draw_next()
{
}
void wait_draw__draw_last()
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: IN GAME, branch: REMOTE_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block remote_player_turn[] =
{
	{REMOTE_GIVE_CLUE,remote_player_turn__remote_give_clue,local_player_turn},
	{REMOTE_PLAY,remote_player_turn__remote_play,wait_draw},
	{REMOTE_DISCARD,remote_player_turn__remote_discard,wait_draw},
	{REMOTE_PLAY_WON,remote_player_turn__remote_play_won,a_wait_remote_play_again_answer},		//  --> GO TO END OF GAME
	{REMOTE_PLAY_LOST,remote_player_turn__remote_play_lost,a_wait_remote_play_again_answer},		//  --> GO TO END OF GAME
	{GND,do_nothing,&no_state }
};
const state_block wait_draw[] =
{
	{DRAW_NEXT,wait_draw__draw_next,local_player_turn},
	{DRAW_LAST,wait_draw__draw_last,sc_2_local_player_turn},	//	--> GO TO GAME FINISHING SCENARIO 2
	{GND,do_nothing,&no_state}
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: FINISHING SCENARIO 1 (LAST 2 TURNS, REMOTE FIRST), branch: REMOTE_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void sc_1_remote_player_turn__remote_give_clue(void)
{
}
void sc_1_remote_player_turn__remote_play(void)
{
}
void sc_1_remote_player_turn__remote_discard(void)
{
}
void sc_1_remote_player_turn__remote_play_won(void)
{
}
void sc_1_remote_player_turn__remote_play_lost(void)
{
}
void sc_1_wait_draw__draw_fake(void)
{
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: FINISHING SCENARIO 1 (LAST 2 TURNS, REMOTE FIRST), branch: REMOTE_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block sc_1_remote_player_turn[] =
{
	{REMOTE_GIVE_CLUE,sc_1_remote_player_turn__remote_give_clue,sc_1_local_player_turn},
	{REMOTE_PLAY,sc_1_remote_player_turn__remote_play,sc_1_wait_draw },
	{REMOTE_DISCARD,sc_1_remote_player_turn__remote_discard,sc_1_wait_draw },
	{REMOTE_PLAY_WON,sc_1_remote_player_turn__remote_play_won,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{REMOTE_PLAY_LOST,sc_1_remote_player_turn__remote_play_lost,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
};
const state_block sc_1_wait_draw[] =
{
	{DRAW_FAKE,sc_1_wait_draw__draw_fake,sc_1_local_player_turn},
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: FINISHING SCENARIO 1, (LAST 2 TURNS, REMOTE FIRST) branch: LOCAL_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void sc_1_local_player_turn__local_give_clue()
{
}
void sc_1_local_player_turn__local_play()
{
}
void sc_1_local_player_turn__local_discard()
{
}
void sc_1_wait_remote_player_response__remote_we_won()
{
}
void sc_1_wait_remote_player_response__remote_we_lost()
{
}
void sc_1_wait_remote_player_response__remote_match_is_over()
{
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: FINISHING SCENARIO 1 (LAST 2 TURNS, REMOTE FISRT), branch: LOCAL_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block sc_1_local_player_turn[] =
{
	{LOCAL_GIVE_CLUE,sc_1_local_player_turn__local_give_clue,sc_1_wait_remote_player_response},
	{LOCAL_PLAY,sc_1_local_player_turn__local_play,sc_1_wait_remote_player_response},
	{LOCAL_DISCARD,sc_1_local_player_turn__local_discard,sc_1_wait_remote_player_response},
};
const state_block sc_1_wait_remote_player_response[] =
{
	{REMOTE_WE_WON,sc_1_wait_remote_player_response__remote_we_won,b_wait_local_play_again_answer },				//  --> GO TO END OF GAME
	{REMOTE_WE_LOST,sc_1_wait_remote_player_response__remote_we_lost,b_wait_local_play_again_answer },				//  --> GO TO END OF GAME
	{REMOTE_MATCH_IS_OVER,sc_1_wait_remote_player_response__remote_match_is_over,b_wait_local_play_again_answer },	//  --> GO TO END OF GAME
};

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: LOCAL_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void sc_2_local_player_turn__local_give_clue()
{
}
void sc_2_local_player_turn__local_play()
{
}
void sc_2_local_player_turn__local_discard()
{
}
void sc_2_wait_remote_player_response__ack()
{
}
void sc_2_wait_remote_player_response__remote_we_won()
{
}
void sc_2_wait_remote_player_response__remote_we_lost()
{
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: LOCAL_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block sc_2_local_player_turn[] =
{
	{LOCAL_GIVE_CLUE,sc_2_local_player_turn__local_give_clue,sc_2_remote_player_turn},
	{LOCAL_PLAY,sc_2_local_player_turn__local_play,sc_2_wait_remote_player_response},
	{LOCAL_DISCARD,sc_2_local_player_turn__local_discard,sc_2_wait_remote_player_response},
};
const state_block sc_2_wait_remote_player_response[] =
{
	{ACK,sc_2_wait_remote_player_response__ack,sc_2_remote_player_turn},
	{REMOTE_WE_WON,sc_2_wait_remote_player_response__remote_we_won,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
	{REMOTE_WE_LOST,sc_2_wait_remote_player_response__remote_we_lost,b_wait_local_play_again_answer },		//  --> GO TO END OF GAME
};
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//$ Action group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: REMOTE_PLAYER $
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void sc_2_remote_player_turn__remote_give_clue(void)
{
}
void sc_2_remote_player_turn__remote_discard(void)
{
}
void sc_2_remote_player_turn__remote_play(void)
{
}
void sc_2_remote_player_turn__remote_play(void)
{
}
void sc_2_remote_player_turn__remote_play_won(void)
{
}
void sc_2_remote_player_turn__remote_play_lost(void)
{
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//% State group: FINISHING SCENARIO 2 (LAST 2 TURNS, LOCAL FIRST), branch: REMOTE_PLAYER %
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const state_block sc_2_remote_player_turn[] =
{
	{REMOTE_GIVE_CLUE,sc_2_remote_player_turn__remote_give_clue,a_wait_remote_play_again_answer },		//	--> GO TO END OF GAME
	{REMOTE_DISCARD,sc_2_remote_player_turn__remote_discard,a_wait_remote_play_again_answer },			//	--> GO TO END OF GAME	
	{REMOTE_PLAY,sc_2_remote_player_turn__remote_play,a_wait_remote_play_again_answer},					//	--> GO TO END OF GAME
	{REMOTE_PLAY_WON,sc_2_remote_player_turn__remote_play_won,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
	{REMOTE_PLAY_LOST,sc_2_remote_player_turn__remote_play_lost,a_wait_remote_play_again_answer },		//  --> GO TO END OF GAME
};
#endif