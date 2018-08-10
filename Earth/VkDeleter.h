#pragma once

#include <vulkan/vulkan.h>
#include <functional>

template <class T>
class VkDeleter
{
public:
	VkDeleter() : VkDeleter([](T, VkAllocationCallbacks*) {}) {}

	VkDeleter(std::function<void(T, VkAllocationCallbacks*)> deleter)
	{
		this->deleter = [=](T obj)
		{
			deleter(obj, nullptr);
		};
	}

	VkDeleter(const VkDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deleter)
	{
		this->deleter = [&instance, deleter](T obj)
		{
			deleter(instance, obj, nullptr);
		};
	}

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

	T* replace()
	{
		cleanup();
		return &object;
	}

	operator T() const
	{
		return object;
	}

	const T* operator &() const
	{
		return &object;
	}

	void operator =(T obj)
	{
		cleanup();
		object = obj;
	}

	template<class V>
	bool operator ==(V obj)
	{
		return object == (V)obj;
	}

private:
	T object{ VK_NULL_HANDLE };

	std::function<void(T)> deleter;

	void cleanup()
	{
		if (object != VK_NULL_HANDLE)
		{
			deleter(object);
		}
		object = VK_NULL_HANDLE;
	}
};

