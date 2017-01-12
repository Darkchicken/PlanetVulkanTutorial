#pragma once

/*
code used is from https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code
author is Alexander Overvoorde
*/
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>

#include <functional>

// Wrapper to keep this as part of the engine namespace
namespace PVEngine
{
	// template declaration to make a generic class
	template <typename T>

	class VDeleter
	{
	public:
		// Default Constructor
		// contains an empty delete function for making lists of deleter objects
		VDeleter() : VDeleter([](T, VkAllocationCallbacks*) {}) {}

		// Overloaded Constructor
		// Creates a VDeleter with just the cleanup function as an argument
		VDeleter(std::function<void(T, VkAllocationCallbacks*)> deletef)
		{
			this->deleter = [=](T obj) { deletef(obj, nullptr); };
		}

		// Overloaded Constructor
		// Creates a VDeleter with a VKInstance reference and a cleanup function as arguments
		VDeleter(const VDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef)
		{
			this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
		}

		// Overloaded Constructor
		// Creates a VDeleter with a VKDevice reference and a cleanup function as arguments
		VDeleter(const VDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef)
		{
			this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
		}

		// Destructor
		// called when the object goes out of scope, triggers a call to cleanup
		~VDeleter()
		{
			cleanup();
		}

		// Overloaded & operator
		// Returns a reference to the object
		const T* operator &() const
		{
			return &object;
		}

		// Replace Function
		// Calls the cleanup function to remove old object, then returns a reference
		T* replace()
		{
			cleanup();
			return &object;
		}

		// Overloaded Cast Operator
		// Returns a copy of the object
		operator T() const
		{
			return object;
		}

		// Overloaded Assignment Operator
		// Using the operator now calls cleanup, then replaces object with assignment
		void operator=(T rhs)
		{
			if (rhs != object)
			{
				cleanup();
				object = rhs;
			}
		}

		// Overloaded Comparison Operator
		// Returns true if the stored object is equal to (V rhs)
		template<typename V>
		bool operator==(V rhs)
		{
			return object == T(rhs);
		}

	private:
		// Generic object to store Vulkan object, initialized to null
		T object{ VK_NULL_HANDLE };
		// Function to use to delete object, used by cleanup()
		std::function<void(T)> deleter;

		// Cleanup Function
		// If the object contained is not null, call the deleter function
		// Then assign the object a null value
		void cleanup()
		{
			if (object != VK_NULL_HANDLE)
			{
				deleter(object);
			}
			object = VK_NULL_HANDLE;
		}
	};
}
