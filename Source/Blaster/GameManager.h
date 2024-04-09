#pragma once

#include "CoreMinimal.h"

class BLASTER_API GameManager
{
public:
	static GameManager& GetInstance()
	{
		static GameManager instance; // Guaranteed to be destroyed and instantiated on first use.
		return instance;
	}

private:
	//static GameManager Instance;
	GameManager();  // Private constructor
	~GameManager(); // Private destructor
};