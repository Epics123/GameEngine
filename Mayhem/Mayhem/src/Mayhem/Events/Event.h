#pragma once

#include "Mayhem/Core.h"
#include <functional>
#include <string>

namespace Mayhem
{
	enum class EventType
	{
		NONE = 0,
		WINDOW_CLOSE, WINDOW_RESIZE, WINDOW_FOCUS, WINDOW_LOST_FOCUS, WINDOW_MOVED,
		KEY_PRESSED, KEY_RELEASED,
		MOUSE_BUTTON_PRESSED, MOUSE_BUTTON_RELEASED, MOUSE_MOVE, MOUSE_SCROLL
	};

	enum EventCategory
	{
		None = 0,
		APPLICATION   = 1 << 0,
		INPUT         = 1 << 1,
		KEYBOARD      = 1 << 2,
		MOUSE	      = 1 << 3,
		MOUSE_BUTTON  = 1 << 4
	};

#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }

	class MAYHEM_API Event
	{
	public:
		virtual EventType getEventType() const = 0;
		virtual const char* getName() const = 0;
		virtual int getCategoryFlags() const = 0;
		virtual std::string toString() const { return getName(); }

		inline bool isInCategory(EventCategory category)
		{
			return getCategoryFlags() & category;
		}

	protected:
		bool mHandled = false;
	};
}
