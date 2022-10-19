#include "mlepch.h"

#include "EventDispatcher.h"
#include "Runtime/Core/Base/Log.h"

namespace engine {
	void EventDispatcher::Dispatch(Event& event)
	{
		for (auto it = observer_funcitons_.begin(); it != observer_funcitons_.end(); ++it)
		{
			(*it).second(event);
		}
	}
	void EventDispatcher::AddObserver(std::pair<std::string, std::function<void(const Event&)>> name_function_pair)
	{
		observer_funcitons_.insert(name_function_pair);
	}

	void EventDispatcher::RemoveObserver(std::string name)
	{
		if (observer_funcitons_.erase(name) == 0)
			MLE_CORE_INFO("RemoveObserver:Target Observer is not fund in the dispatcher's observer");
	}
}