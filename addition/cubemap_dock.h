#ifndef ADD_CUBEMAP_DOCK_H
#define ADD_CUBEMAP_DOCK_H

#include "core/io/config_file.h"
#include "core/io/resource_importer.h"
#include "editor/editor_file_system.h"
#include "editor/editor_inspector.h"
#include "scene/gui/box_container.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/option_button.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/tree.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/split_container.h"
#include "scene/gui/file_dialog.h"

class TreeItem;

class CubeMapDock : public Control {
	GDCLASS(CubeMapDock, Control);

	ScrollContainer *scroll_container = nullptr;
	TabContainer *tab_container = nullptr;
	HSplitContainer* split = nullptr;
	VBoxContainer *select = nullptr;
	Button *top_btn = nullptr;
	Button *bottom_btn = nullptr;
	Button *left_btn = nullptr;
	Button *right_btn = nullptr;
	Button *front_btn = nullptr;
	Button *back_btn = nullptr;
	Button *make_btn = nullptr;
	Tree *tree = nullptr;
	FileDialog *file_dialog = nullptr;
	PopupMenu *popup_menu = nullptr;


private:
	static CubeMapDock *singleton;

	enum ESelectSize : int
	{
		SSIDE_TOP = 0,
		SSIDE_BOTTOM,
		SSIDE_LEFT,
		SSIDE_RIGHT,
		SSIDE_FRONT,
		SSIDE_BACK
	};

	ESelectSize select_side;

	String 
		top_path,
		bottom_path,
		left_path,
		right_path,
		front_path,
		back_path;

public:
	static CubeMapDock *get_singleton() { return singleton; }
	void UpdateTree();
protected:
	void _on_top_btn_pressed();
	void _on_bottom_btn_pressed();
	void _on_left_btn_pressed();
	void _on_right_btn_pressed();
	void _on_front_btn_pressed();
	void _on_back_btn_pressed();
	void _on_make_btn_pressed();
	void _on_file_dialog_file_selected(String path);
	void _on_popup_menu_index_pressed(int id);
	void _tree_gui_input(const Ref<InputEvent> &p_event);

public:
	CubeMapDock();
	~CubeMapDock();
};

#endif // ADD_CUBEMAP_DOCK_H
