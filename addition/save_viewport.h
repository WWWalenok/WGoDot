#ifndef __SAVE_VIEWPORT_H__
#define __SAVE_VIEWPORT_H__

#include "core/string/ustring.h"
#include "core/math/color.h"

template<typename T = Node>
T* find_node(Node* start_node, String name)
{
	if (!start_node)
	{
		return nullptr;
	}

	String node_path = start_node->get_name();

	if (name == node_path)
	{
		T* t = Object::cast_to<T>(start_node);
		if (t)
			return t;
	}

	for (int i = 0; i < start_node->get_child_count(); i++)
	{
		T* ret = find_node<T>(start_node->get_child(i), name);
		if (ret)
			return ret;
	}
	return nullptr;
};

template<typename T>
T* find_node_by_type(Node* start_node)
{
	if (!start_node)
	{
		return nullptr;
	}

	String node_path = start_node->get_path();
	auto tc = Object::cast_to<T>(start_node);
	if (tc)
	{
		return tc;
	}

	for (int i = 0; i < start_node->get_child_count(); i++)
	{
		auto ret = find_node_by_type<T>(start_node->get_child(i));
		if (ret)
			return ret;
	}
	return nullptr;
};


void save_viewport(String to = "", int width = 0, int height = 0, Color background = Color(1, 1, 1, 1));

#endif // __SAVE_VIEWPORT_H__