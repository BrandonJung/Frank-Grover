#include "conversation.hpp"
#include "ui.hpp"
#include "player.hpp"

// TODO emplace an interaction component with the entity
Conversation::Conversation(entt::entity entity, Quest quest,
						   std::vector<Line> lines[NUM_CONVERSATION_STATES], UI::Font font)
{
	this->tooltip = _ecs.create();
	this->gameFont = font;
	this->entity = entity;
	this->quest = quest;
	this->conversing = false;
	this->firstEncounter = true;
	this->oneTimeOnly = false;
	this->state = INITIAL;
	this->currentLine = 0;
	for (int i = 0; i < NUM_CONVERSATION_STATES; i++)
	{
		this->lines[i] = lines[i];
	}
    if (!_ecs.has<Interaction>(entity))
        _ecs.emplace<Interaction>(entity);
	std::cout << "Conversation created with "
			  << std::to_string(static_cast<uint32_t>(entity)) << ", " << quest
			  << ", " << lines << std::endl;
}

Conversation::~Conversation(void)
{
	for (int i = 0; i < NUM_CONVERSATION_STATES; i++)
	{
		lines[i].clear();
	}
	if (_ecs.valid(tooltip))
	{
		if (_ecs.has<UILabel>(tooltip))
		{
			_ecs.remove<UILabel>(tooltip);
		}
		_ecs.destroy(tooltip);
	}
}
void Conversation::setConversationInProgress(bool value) { conversing = value; }

bool Conversation::conversationInProgress(void) { return conversing; }

void Conversation::setState(ConversationState value) { state = value; }

ConversationState Conversation::getState(void) { 
	// change the state to quest incomplete for the conditional rendering in
	// world.cpp
	if (state == INITIAL && firstEncounter == false)
	{
		return QUEST_INCOMPLETE;
	}
	return state;
}


Quest Conversation::getQuest(void) { return quest; }

void Conversation::setOneTimeOnly(bool value) { oneTimeOnly = value; }

bool Conversation::getFirstEncounter(void) { return firstEncounter; }

void Conversation::setFirstEncounter(bool value) { firstEncounter = value; }

bool Conversation::getCurrentLine(void) { return currentLine; }

void Conversation::setCurrentLine(int value) { currentLine = value; }

bool Conversation::converse(bool quests[NUM_QUESTS], vec2 window_size)
{
	if (!conversing)
	{
		currentLine = 0;
		conversing = true;
		if (firstEncounter && !oneTimeOnly)
		{
			std::cout << "First encounter\n";
			firstEncounter = false;
			state = INITIAL;
		}
		else
		{
			if (quests[quest])
			{
				if (state == QUEST_INCOMPLETE || state == INITIAL)
				{
					std::cout << "Quest complete\n";
					state = QUEST_COMPLETE;
				}
				else
				{
					std::cout << "End\n";
					state = END;
				}
				
			}
			else
			{
				std::cout << "Quest incomplete\n";
				state = QUEST_INCOMPLETE;
			}
		}
	}
	return nextLine(window_size);
}

bool Conversation::nextLine(vec2 window_size) {
	// remove old speech bubble
	if (_ecs.has<UIPanel>(entity))
	{
		_ecs.remove<UIPanel>(entity);
	}
	if (_ecs.has<UILabel>(entity))
	{
		_ecs.remove<UILabel>(entity);
	}
	if (_ecs.valid(this->tooltip))
	{
		if (_ecs.has<UILabel>(this->tooltip))
		{
			_ecs.remove<UILabel>(this->tooltip);
		}
	}
	else
	{
		this->tooltip = _ecs.create();
	}
	
	if (conversing)
	{
		while (lines[state].size() == 0 && state != END && (!oneTimeOnly || state < QUEST_INCOMPLETE))
		{
			// states are allowed to be empty, skip to the next non-empty state
			// unless this conversation is displayed one time only, then make sure not to display the text until
			// the quest is completed
			int nextState = static_cast<int>(state);
			nextState++;
			state = ConversationState(nextState);
		}
		if (currentLine >= lines[state].size())
		{
			if (state == QUEST_COMPLETE)
			{
				state = END;
			}
			currentLine = 0;
			conversing = false;
			return conversing;
		}
		// check which entity is conversing, the player or the other entity
		Line line = lines[state][currentLine];
		Motion motion;
		if (line.isMyLine)
		{
			motion = _ecs.get<Motion>(entity);
		}
		else
		{
			entt::entity player = _ecs.view<Player>()[0];
			motion = _ecs.get<Motion>(player);
		}
		

		float fontSize = 20;
		vec2 maxSpeechBubbleSize = {fontSize * 20, fontSize * 5};
		float factor;
		if (motion.scale.x < 0)
		{
			// if scale is negative, entity is facing right so speech bubble should be to the right of the entity (positive x-direction)
			factor = 1.0;
		}
		else
		{
			// otherwise speech bubble should be to the left (negative x-direction)
			factor = -1.0;
		}
		vec2 worldPos = vec2(motion.position.x + factor * motion.scale.x / 2,
							 motion.position.y - motion.scale.y / 2);
		glm::ivec3 pos = UI::worldPosToUIPos(worldPos, window_size);
		float x_pos = pos.x ;
		float y_pos = pos.y;
		float opacity = 1.f;
		//UI::setFont(gameFont, fontSize); 
		vec2 size = UI::getMaxLineWidthAndNumLines(line.text, maxSpeechBubbleSize, gameFont, fontSize);
		/*std::cout << "text to be printed is: " << line.text << "\n max line width is " << size.x << ", num lines is "
				  << size.y << std::endl;*/
		// todo get rid of if statement
		vec2 panelSize = vec2(size.x + fontSize * 3, size.y + fontSize * 2);
		_ecs.emplace<UIPanel>(entity, UIPanel{{x_pos, y_pos, 1},
							panelSize,
											  {1, 1, 1, opacity},
											  40});
		_ecs.emplace<UILabel>(entity, UILabel{
											  gameFont,          // font
											  fontSize,                // font size
											  line.text,         // text
											  {x_pos + fontSize, y_pos, 2}, // position
											  vec2(size.x + fontSize * 2, size.y + fontSize * 2), // size
											  {0, 0, 0, opacity} // color
										  });
		char* tooltipText = "Press E to continue...";
		float tooltipFontSize = fontSize / 2;
		vec2 tooltipSize = UI::getMaxLineWidthAndNumLines(
			tooltipText, maxSpeechBubbleSize, gameFont, tooltipFontSize);
		if (tooltipSize.x > panelSize.x)
		{
			tooltipText = "Press E...";
			tooltipSize = UI::getMaxLineWidthAndNumLines(
				tooltipText, maxSpeechBubbleSize, gameFont, tooltipFontSize);
		}
		UILabel label = UILabel
		{
			gameFont,         // font
				tooltipFontSize, // font size
				tooltipText,  // text
				{x_pos + panelSize.x - tooltipSize.x - tooltipFontSize,
				 y_pos + tooltipSize.y, 2}, // position
				tooltipSize,                              // size
			{
				0.6, 0.6, 0.6, opacity
			} // color
		};
		_ecs.emplace<UILabel>(this->tooltip, label);
		currentLine++;
	}
	return conversing;
}
