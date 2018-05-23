#pragma once
#include "ApplicationTickable.hpp"
#include "AsyncLoadable.hpp"
#include "Shared/Jobs.hpp"
#include "GUI/Canvas.hpp"

/*
	Loading and transition screen
*/
class TransitionScreen : public IApplicationTickable
{
public:
	TransitionScreen(IAsyncLoadableApplicationTickable* next);
	~TransitionScreen();

	// Called when target screen is loaded
	//	can also give a null pointer if the screen didn't load successfully
	Delegate<IAsyncLoadableApplicationTickable*> OnLoadingComplete;

	bool Init() override;
	void Tick(float deltaTime) override;

	bool DoLoad() const;
	void OnFinished(Job job);

private:
	shared_ptr<Canvas> m_loadingOverlay;
	IAsyncLoadableApplicationTickable* m_tickableToLoad;
	Job m_loadingJob;

	enum Transition
	{
		In,
		Wait,
		Out,
		End
	};

	Transition m_transition = Transition::In;
	float m_transitionTimer;
};
