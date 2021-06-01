#pragma once
#include "ecs.hpp"
#include "common.hpp"
#include "ui.hpp"
#include <vector>

// change to class enum for type safety if possible
// Some of the quests are just tutorials
enum Quest {
	INTRO,
	BALL_TUTORIAL,
	FRANK_ATTACK,
	LITTER,
	UNICYCLIST,
    NIGHT_TUTORIAL,
	INFORMANT_SQUIRREL,
	POOR_SQUIRREL,
	NUM_QUESTS,
};

// it is allowed for a quest to not have any script associated with a certain conversation state
enum ConversationState
{
	INITIAL, // this conversation is displayed the first time the player talks to the entity
	QUEST_INCOMPLETE, // this conversation is displayed subsequent times the player talks to the entity, if the quest is not yet complete
	QUEST_COMPLETE, // this conversation is displayed the first time the player talks to the entity after completing the quest
	END, // this conversation is displayed subsequent times the player talks to the entity, if the quest is complete
	NUM_CONVERSATION_STATES, // this should always be last because it represents the total number of states
};

// If the text to be displayed is too long, consider splitting it up into two separate Lines.
struct Line
{
	bool isMyLine; // is it the current entity saying this line, or the other entity?
	std::string text; // the text to be displayed in the speech bubble
};

class Conversation
{
private:
	entt::entity entity;     // the entity that will be speaking during this conversation (other than Grover)
	Quest quest;             // the quest associated with this conversation
	bool conversing;         // are we currently displaying lines on screen?
	bool firstEncounter; // is it the first time Grover has conversed with the
						 // entity?
	bool oneTimeOnly;        // this is true for tutorial speech bubbles which should only appear once ever
	int currentLine;         // which line in lines[state] we currently need to display in a speech bubble
	std::vector<Line> lines[NUM_CONVERSATION_STATES];
	entt::entity tooltip;
	UI::Font gameFont;
	// This array has NUM_STATES entries, each entry corresponds to the script for each conversation state.
	// Some of the states may have no script, this is allowed.
	// Each entry in the array is a vector of the lines in the script.

public:
	~Conversation();
	Conversation(entt::entity entity, Quest quest,
				 std::vector<Line> lines[NUM_CONVERSATION_STATES],
				 UI::Font font);
	ConversationState state; // which state the conversation is in
	void setConversationInProgress(bool value);
	bool conversationInProgress(void);
	bool converse(bool quests[NUM_QUESTS], vec2 window_size);
	bool nextLine(vec2 window_size);
	void setOneTimeOnly(bool value);
	void setState(ConversationState value);
	ConversationState getState(void);
	bool getFirstEncounter(void);
	void setFirstEncounter(bool value);
	bool getCurrentLine(void);
	void setCurrentLine(int value);
	Quest getQuest(void);
};

// NOTE: if you want to display text once only based on a certain condition:
// Set the quests_completed variable in world.cpp to true when that condition is true
// In the script file, associate the text with the QUEST_COMPLETE state and keep the other states empty


