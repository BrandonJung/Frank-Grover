#include "ecs.hpp"
#include "common.hpp"
#include "subject.hpp"
#include "observer.hpp"

class DecisionTree : public Observer
{

public:
	void step(float elapsed_ms, vec2 window_size_in_game_units);

private:
	enum EnemyState
	{
		IDLE,
		PATROL,
		RETRIEVE_BALL,
		WANTS_BALL, // may be able to combine wants ball and wants bone to
					// become chase player, if logic to swap ball / bone be
					// handled somewhere else
		WANTS_BONE
	};

	vec2 upperBounds = vec2(3600, 2400);
	vec2 lowerBounds = vec2(2000, 1000);
	vec2 motionDirection = vec2(0, 0);
	EnemyState state = IDLE;
	int timer = 0;

	void updateState(void); // placeholder
	void act(void);         // placeholder
	void steal(void);       // placeholder

	int getPositiveOrNegativeFactor(void);
	void updateVelocityFromDirection(void);
	bool insidePatrolBoundaries(vec2 position);
	bool insideXPatrolBoundaries(vec2 position);
	bool insideYPatrolBoundaries(vec2 position);
	void changeState(EnemyState state);

	// Observer Pattern
	virtual void update(Event event);
};