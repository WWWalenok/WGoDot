#ifndef __PLAYER_HANDLER_H__
#define __PLAYER_HANDLER_H__

#include "scene/main/node.h"
#include <string>

class AddPlayerHandler : public Node
{
	GDCLASS(AddPlayerHandler, Node);
private:

	String data;

protected:
	void _notification(int p_what);
	static void _bind_methods();
	void set_editor_description(const String &p_editor_description);
	String get_editor_description() const;
	
public:

	std::string path_to_export = "";
	
	static void set_is_editor(bool v);

	AddPlayerHandler();
	~AddPlayerHandler();
};

#endif // __PLAYER_HANDLER_H__