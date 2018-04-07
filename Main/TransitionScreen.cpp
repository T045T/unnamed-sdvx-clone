#include "stdafx.h"
#include "TransitionScreen.hpp"
#include "Application.hpp"
#include "Shared/Jobs.hpp"
#include <GUI/GUI.hpp>
#include <GUI/Spinner.hpp>
#include "AsyncLoadable.hpp"
#include "Global.hpp"

TransitionScreen::TransitionScreen(IAsyncLoadableApplicationTickable* next)
	: m_tickableToLoad(next)
{}

TransitionScreen::~TransitionScreen()
{
	// In case of forced removal of this screen
	if (!m_loadingJob->IsFinished())
		m_loadingJob->Terminate();

	g_rootCanvas->Remove(std::dynamic_pointer_cast<GUIElementBase>(m_loadingOverlay));
}

bool TransitionScreen::Init()
{
	if (!m_tickableToLoad)
		return false;

	m_loadingOverlay = std::make_shared<Canvas>();

	// Fill screen with black
	auto black = std::make_shared<Panel>();
	Canvas::Slot* blackSlot = m_loadingOverlay->Add(black);
	blackSlot->anchor = Anchors::Full;
	blackSlot->SetZOrder(0);
	black->color = Color::Black;

	auto spinner = std::make_shared<Spinner>(g_commonGUIStyle);
	Canvas::Slot* spinnerSlot = m_loadingOverlay->Add(spinner);
	spinnerSlot->anchor = Anchor(1.0f, 1.0f); // Right bottom corner
	spinnerSlot->padding = Margin(-50, -50, 50, 50);
	spinnerSlot->autoSizeX = true;
	spinnerSlot->autoSizeY = true;
	spinnerSlot->alignment = Vector2(1.0f, 1.0f);
	spinnerSlot->SetZOrder(1);

	Canvas::Slot* slot = g_rootCanvas->Add(std::dynamic_pointer_cast<GUIElementBase>(m_loadingOverlay));
	slot->anchor = Anchors::Full;
	slot->SetZOrder(1000); // Loading screen on top of all things

	m_loadingJob = JobBase::CreateLambda([&]()
	{
		return DoLoad();
	});
	m_loadingJob->OnFinished.Add(this, &TransitionScreen::OnFinished);
	g_jobSheduler->Queue(m_loadingJob);

	return true;
}

void TransitionScreen::Tick(float deltaTime)
{
	m_transitionTimer += deltaTime;

	if (m_transition == In)
	{
	}
	else if (m_transition == Out)
	{
		if (m_transitionTimer > 0.0f)
		{
			m_transition = End;
			g_application->RemoveTickable(this);
		}
	}
}

bool TransitionScreen::DoLoad() const
{
	if (!m_tickableToLoad)
		return false;

	if(!m_tickableToLoad->AsyncLoad())
	{
		Logf("[Transition] Failed to load tickable", Logger::Error);
		return false;
	}

	return true;
}

void TransitionScreen::OnFinished(Job job)
{
	// Finalize?
	if (job->IsSuccessfull())
	{
		if (m_tickableToLoad && !m_tickableToLoad->AsyncFinalize())
		{
			Logf("[Transition] Failed to finalize loading of tickable", Logger::Error);
			delete m_tickableToLoad;
			m_tickableToLoad = nullptr;
		}
	}
	else
	{
		Logf("[Transition] Failed to load tickable", Logger::Error);
		delete m_tickableToLoad;
		m_tickableToLoad = nullptr;
	}

	if (m_tickableToLoad)
	{
		Logf("[Transition] Finished loading tickable", Logger::Info);
		g_application->AddTickable(m_tickableToLoad, this);
	}

	OnLoadingComplete.Call(m_tickableToLoad);
	m_transition = Out;
	m_transitionTimer = 0.0f;
}

