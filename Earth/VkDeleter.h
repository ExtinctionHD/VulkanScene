#pragma once

#include <vulkan/vulkan.h>
#include <functional>

template <class T>
class VkDeleter
{
public:
	// конструктор по умолчанию
	VkDeleter() : VkDeleter([](T, VkAllocationCallbacks*) {}) {}

	// конструктор приимающий функцию деструктор
	VkDeleter(std::function<void(T, VkAllocationCallbacks*)> deleter)
	{
		this->deleter = [=](T obj)
		{
			deleter(obj, nullptr);
		};
	}

	// конструктор дл€ объектов зависимых от экземпл€ра
	VkDeleter(const VkDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deleter)
	{
		this->deleter = [&instance, deleter](T obj)
		{
			deleter(instance, obj, nullptr);
		};
	}

	// конструктор дл€ объектов зависимых от логического устройства
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

	// замена объекта: очистка предыдущего и сохранение нового
	T* replace()
	{
		cleanup();
		return &object;
	}

	// приведение к объекту
	operator T() const
	{
		return object;
	}

	// вз€тие адресса
	const T* operator &() const
	{
		return &object;
	}

	// очистка при копировании
	void operator =(T obj)
	{
		cleanup();
		object = obj;
	}

	// сравнение
	template<class V>
	bool operator ==(V obj)
	{
		return object == (V)obj;
	}

private:
	T object{ VK_NULL_HANDLE };

	std::function<void(T)> deleter;  // функци€ деструктор

	void cleanup()  // очистка объекта
	{
		if (object != VK_NULL_HANDLE)
		{
			deleter(object);
		}
		object = VK_NULL_HANDLE;
	}
};

