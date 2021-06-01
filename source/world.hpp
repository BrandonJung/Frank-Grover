#pragma once

// internal
#include "common.hpp"
#include "player.hpp"
#include "ui.hpp"
#include "observer.hpp"
#include "conversation.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

const float LIGHT_UP_TIME = 1000;
// Container for all our entities and game logic. Individual rendering / update
// is deferred to the relative update() methods
class WorldSystem : public Observer
{
public:
	// Creates a window
	WorldSystem(ivec2 window_size_px, vec2 window_size_in_game_units);

	// Releases all associated resources
	~WorldSystem();

	// restart level
	void restart(bool loadSave);

	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Check for collisions
	void handle_collisions();

	// GUI
	void loadUI(glm::ivec2 screenSize);

	// Should the game be over ?
	bool is_over() const;

	// OpenGL window handle
	GLFWwindow* window;
	vec2 mapSize;

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 mouse_pos);
	void on_mouse_button(int button, int action, int mods);

	// Input state
	bool input_up{false};
	bool input_down{false};
	bool input_left{false};
	bool input_right{false};
	bool input_mouse_click{false};
	glm::vec2 mouse_screen_position{-100, -100};
	glm::vec2 mouse_world_position{-100, -100};

	bool isGrabbing{false};
	entt::entity grabObject;

	float uiPanelOpacity = 0.9f;
	glm::vec4 uiPanelColor = {1, 1, 0, uiPanelOpacity};

	// audio clips
	Mix_Chunk* walk_sound;
	Mix_Chunk* bone_sound;
	Mix_Chunk* seagull_sound1;
	Mix_Chunk* seagull_sound2;
	Mix_Chunk* seagull_sound3;
	Mix_Chunk* seagull_sound4;
	Mix_Music* background_music;
	const int VOLUME_INCREMENT = 2;
	void incrementVolume(int amount);
	int music_volume = MIX_MAX_VOLUME / 20;
	int fx_volume = MIX_MAX_VOLUME / 10;

	// Loads the audio
	void init_audio();
	void playRandomSeagullSound(void);

	// Loads the script
	void load_scripts(void);
	// Loads the game map
	void load_level(std::string path);
	// Write and Load saves
	void load_save_state(std::string path);
	std::string generateSaveData(entt::entity entity, vec2 position);
	void write_save();

	// Number of bones collected
	unsigned int points;
	const unsigned int num_bones_from_quests = 2;
	unsigned int total_bone_count{num_bones_from_quests};

	// Game state
	float current_speed;
	entt::entity main_player;
	entt::entity bonetree;
	entt::entity flipper_in_tree;

	bool near_interaction = false;
    bool near_conversation = false;

	const int num_litter = 8;
	vec2 litter[8];
	vec2 socks_pos;
	bool rendered_litter = false;
	bool rendered_socks = false;
	bool rendered_unicyclist_reward = false;
	bool rendered_litter_reward = false;
	bool rendered_ball = false;

	float socksMovementEffectTimer = 5000.f;
	bool socksMovementEffect = false;
	bool socksEffect = false;

	float ballRespawnTimer = 15000.f;
	bool ballRespawn = false;
	// Respawn ball at bowl after Frank picks up ball
	void respawnBall();

	int lastPickedUp;

	int playerHasBall;
	int playerHasSocks;
    
    // toggle for night (for debugging purposes)
    bool lightEffectsOn = true;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// UI Resources
	UI::Font gameFont;
	Texture uiBoneTexture{}; // to be replaced with bones later
	entt::entity uiBonePanel;
	UI::Clickable button;
	UI::Clickable tooltip;
	UI::Clickable playButton;
	bool tooltipOnOffState{false};
	bool buttonOnOffState{false};
	bool controls = false;

	entt::entity uiLoadingScreenPanel;
	Texture uiLoadingScreenTexture;

	Texture uiBallTexture{};
	entt::entity uiBallPanel;

	Texture uiToolTipTexture{};
	entt::entity uiToolTipPanel;

	Texture uiToolTipWordsTexture{};
	entt::entity uiToolTipWordsPanel;

	entt::entity congratsPanel;
	entt::entity congratsPanelTooltip;
	bool currentlyDisplayingCongrats = false;
	bool haveDisplayedHalfway = false;
	bool congratsPanelsInitialized = false;

	void setColorTimer(entt::entity panel, bool gained);

	// helper function to start a conversation
	void converse(Conversation& conversation);

	//throw ball
	void throwBall(vec2 target);

	// Frank drops bone
	void dropBone();

	// check if player is near the dumpster for litter quest
	bool isPlayerNearDumpster();

	// Observer Pattern
	virtual void update(Event event);

	// Dirt particles
	void spawnDirtParticlePuff(uint32_t count, vec2 position);

	// Print end of game messages to the screen
	void displayText(char* text);
};

struct ColorTimer
{
	float timer = LIGHT_UP_TIME; // in milliseconds
};
