#pragma once
#include "observer.hpp"
class Subject
{
public:
	std::vector<Observer*> observers;
	virtual void registerObserver(Observer* observer) = 0;
	virtual void removeObserver(Observer* observer) = 0;
	virtual void notifyObservers(Event event) = 0;
	virtual void observersSize() = 0;
};