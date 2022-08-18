#include "mlepch.h"

#include "EventBus.h"

namespace engine {
	EventBus* EventBus::instance_ = nullptr;

	EventBus::EventBus()
	{
		head_ = nullptr;
	}

	EventBus::~EventBus()
	{
		Close();
	}

	EventBus& EventBus::GetInstance()
	{
		if (instance_ == nullptr)
			instance_ = new EventBus();
		return *instance_;
	}

	void EventBus::Dispatch(Event& event)
	{
		System* subsriber = head_;
		while (subsriber != nullptr)
		{
			subsriber->OnEvent(event);
		}
	}

	void EventBus::AddSubscriber(System* node)
	{
		node->next_ = head_;
		head_ = node;
	}

	void EventBus::RemoveSubscriber(System* node)
	{

	}

	void EventBus::Close()
	{
		System* subsriber = head_;
		while (subsriber != nullptr)
		{
			head_ = subsriber->next_;
			subsriber->next_ = nullptr;
		}
	}
}