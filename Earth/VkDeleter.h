#pragma once

#include <vulkan/vulkan.h>
#include <functional>

template <class T>
class VkDeleter
{
public:
	// default constructor
	VkDeleter() : VkDeleter([](T, VkAllocationCallbacks*) {}) {}

	// constructor with object destructor function
	VkDeleter(std::function<void(T, VkAllocationCallbacks*)> deleter)
	{
		this->deleter = [=](T obj)
		{
			deleter(obj, nullptr);
		};
	}

	// constructor for instance-dependent objects
	VkDeleter(const VkDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deleter)
	{
		this->deleter = [&instance, deleter](T obj)
		{
			deleter(instance, obj, nullptr);
		};
	}

	// constructor for device-dependent objects
	VkDeleter(const VkDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deleter)
	{
		this->deleter = [&device, deleter](T obj)
		{
			deleter(device, obj, nullptr);
		};
	}

	~VkDeleter()
	{
		cleanup();
	}

	// replace: cleanup last object, return pointer for recording
	T* replace()
	{
		cleanup();
		return &object;
	}

	// cast to object type
	operator T() const
	{
		return object;
	}

	// cast to object address
	const T* operator &() const
	{
		return &object;
	}

	// cleanup before assigment
	void operator =(T obj)
	{
		cleanup();
		object = obj;
	}

	// compare with object
	template<class V>
	bool operator ==(V obj)
	{
		return object == (V)obj;
	}

private:
	T object{ VK_NULL_HANDLE };

	std::function<void(T)> deleter;  // object destructor function

	void cleanup()
	{
		if (object != VK_NULL_HANDLE)
		{
			deleter(object);
		}
		object = VK_NULL_HANDLE;
	}
};

