#include "cubemap_dock.h"

#include "core/config/project_settings.h"
#include "editor/editor_node.h"
#include "editor/editor_resource_preview.h"
#include "editor/editor_scale.h"
#include "scene/gui/tree.h"
#include "editor/editor_settings.h"

CubeMapDock *CubeMapDock::singleton = nullptr;

#include <iostream>



void CubeMapDock::_on_top_btn_pressed()
{
	Point2 pos = top_btn->get_position();
	
	select_side = SSIDE_TOP;
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, {800, 800});
	file_dialog->popup(rect);
}

void CubeMapDock::_on_bottom_btn_pressed()
{
	Point2 pos = bottom_btn->get_position();

	select_side = SSIDE_BOTTOM;
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, {800, 800});
	file_dialog->popup(rect);
}

void CubeMapDock::_on_left_btn_pressed()
{
	Point2 pos = left_btn->get_position();

	select_side = SSIDE_LEFT;
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, {800, 800});
	file_dialog->popup(rect);
}

void CubeMapDock::_on_right_btn_pressed()
{
	Point2 pos = right_btn->get_position();

	select_side = SSIDE_RIGHT;
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, {800, 800});
	file_dialog->popup(rect);
}

void CubeMapDock::_on_front_btn_pressed()
{
	Point2 pos = front_btn->get_position();
	
	select_side = SSIDE_FRONT;
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, {800, 800});
	file_dialog->popup(rect);
}

void CubeMapDock::_on_back_btn_pressed()
{
	Point2 pos = back_btn->get_position();

	select_side = SSIDE_BACK;
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, {800, 800});
	file_dialog->popup(rect);
}

void CubeMapDock::_on_make_btn_pressed()
{

}

void CubeMapDock::_on_file_dialog_file_selected(String path)
{
	switch (select_side)
	{
	case SSIDE_TOP:
		top_path = path;
		break;
	case SSIDE_BOTTOM:
		bottom_path = path;
		break;
	case SSIDE_LEFT:
		left_path = path;
		break;
	case SSIDE_RIGHT:
		right_path = path;
		break;
	case SSIDE_FRONT:
		front_path = path;
		break;
	case SSIDE_BACK:
		back_path = path;
		break;
	
	default:
		break;
	}

	top_btn->set_text("Top: " + top_path);
	bottom_btn->set_text("Bottom: " + bottom_path);
	left_btn->set_text("Left: " + left_path);
	right_btn->set_text("Right: " + right_path);
	front_btn->set_text("Front: " + front_path);
	back_btn->set_text("Back: " + back_path);
}

void _rec_UpdateTree(Tree* tree, TreeItem* root, const String& path)
{
	
	auto dir_access = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	dir_access->change_dir(path);
	dir_access->list_dir_begin();
	
	List<String> files;
	List<String> dirs;
	String item = dir_access->get_next();

	while (!item.is_empty())
	{
		if(dir_access->current_is_dir())
		{
			if (item == "." || item == ".." || item.begins_with(".")) 
			{
				item = dir_access->get_next();
				continue;
			}
		}
		else
		{
			if (!(item.ends_with(".png") || item.ends_with(".jpg") || item.ends_with(".bmp"))) 
			{
				item = dir_access->get_next();
				continue;
			}
		}

		bool is_hidden = dir_access->current_is_hidden();

		if (!is_hidden) 
		{
			if (!dir_access->current_is_dir()) 
			{
				files.push_back(item);
			} 
			else 
			{
				dirs.push_back(item);
			}
		}
		item = dir_access->get_next();
	}

	dirs.sort_custom<NaturalNoCaseComparator>();
	files.sort_custom<NaturalNoCaseComparator>();

	while (!dirs.is_empty()) 
	{
		String &dir_name = dirs.front()->get();
		TreeItem *ti = tree->create_item(root);
		ti->set_text(0, dir_name);

		Dictionary d;
		d["name"] = dir_name;
		d["path"] = path + dir_name + "/";
		d["dir"] = true;

		_rec_UpdateTree(tree, ti, path + dir_name + "/");

		ti->set_metadata(0, d);

		dirs.pop_front();
	}

	String base_dir = dir_access->get_current_dir();

	while (!files.is_empty()) 
	{
		bool match = true;

		if (match) 
		{
			TreeItem *ti = tree->create_item(root);
			ti->set_text(0, files.front()->get());
			ti->add_button(0, nullptr);

			Dictionary d;
			d["name"] = files.front()->get();
			d["path"] = path + files.front()->get();
			d["dir"] = false;
			ti->set_metadata(0, d);
		}

		files.pop_front();
	}
}

void CubeMapDock::_on_popup_menu_index_pressed(int id)
{
	Dictionary d = popup_menu->get_meta("file");
	String path = d["path"];
	switch (id)
	{
	case 0:
		top_path = path;
		break;
	case 1:
		bottom_path = path;
		break;
	case 2:
		left_path = path;
		break;
	case 3:
		right_path = path;
		break;
	case 4:
		front_path = path;
		break;
	case 5:
		back_path = path;
		break;
	default:
		break;
	}
	top_btn->set_text("Top: " + top_path);
	bottom_btn->set_text("Bottom: " + bottom_path);
	left_btn->set_text("Left: " + left_path);
	right_btn->set_text("Right: " + right_path);
	front_btn->set_text("Front: " + front_path);
	back_btn->set_text("Back: " + back_path);
}

void CubeMapDock::_tree_gui_input(const Ref<InputEvent> &p_event) 
{
	Ref<InputEventMouseButton> mb_event = p_event;
	if(mb_event.is_valid())
	{
		if(mb_event->is_pressed())
		{
			if(mb_event->get_button_index() == MouseButton::RIGHT)
			{
				auto position = mb_event->get_position();
				auto item = tree->get_item_at_position(position);
				if(item != nullptr)
				{
					Dictionary d = item->get_metadata(0);
					if(d["dir"])
						return;
					Point2 wpos = get_position() + tree->get_position() + get_window()->get_position() + Point2(10, 10);
					Rect2i rect = Rect2i(position + wpos, {0, 0});
					popup_menu->popup(rect);
					popup_menu->set_meta("file", d);
				}
				else
				{
					UpdateTree();
				}
			}
		}
	}

}


// See void FileDialog::update_file_list()
void CubeMapDock::UpdateTree()
{
	tree->clear();
	tree->get_vscroll_bar()->set_value(0);
	TreeItem *root = tree->create_item();

	_rec_UpdateTree(tree, root, "res://");
}


CubeMapDock::CubeMapDock() {
	singleton = this;
	set_name("CubeMap");

	set_size({500,500});
	
	scroll_container = memnew(ScrollContainer);
	tab_container = memnew(TabContainer);
	split = memnew(HSplitContainer);
	select = memnew(VBoxContainer);
	top_btn = memnew(Button);
	bottom_btn = memnew(Button);
	left_btn = memnew(Button);
	right_btn = memnew(Button);
	front_btn = memnew(Button);
	back_btn = memnew(Button);
	make_btn = memnew(Button);
	tree = memnew(Tree);
	file_dialog = memnew(FileDialog);
	popup_menu = memnew(PopupMenu);


//================== this ==================
	this->set_name("CubeMap");
	this->set_anchors_preset(Control::PRESET_FULL_RECT);
	this->set_anchor(SIDE_RIGHT, 1.0);
	this->set_anchor(SIDE_BOTTOM, 1.0);
	this->set_offset(SIDE_RIGHT, -787.0);
	this->set_offset(SIDE_BOTTOM, -2.0);
	this->set_h_grow_direction(Control::GROW_DIRECTION_BOTH);
	this->set_v_grow_direction(Control::GROW_DIRECTION_BOTH);
//================== scroll_container ==================
	scroll_container->set_name("ScrollContainer");
	this->add_child(scroll_container);
	scroll_container->set_layout_direction(Control::LAYOUT_DIRECTION_LOCALE);
	scroll_container->set_anchors_preset(Control::PRESET_FULL_RECT);
	scroll_container->set_anchor(SIDE_RIGHT, 1.0);
	scroll_container->set_anchor(SIDE_BOTTOM, 1.0);
	scroll_container->set_h_grow_direction(Control::GROW_DIRECTION_BOTH);
	scroll_container->set_v_grow_direction(Control::GROW_DIRECTION_BOTH);
	scroll_container->set_scale(Vector2(1, 0.983573));
//================== tab_container ==================
	tab_container->set_name("TabContainer");
	//scroll_container->add_child(tab_container);
	tab_container->set_h_size_flags(3);
	tab_container->set_v_size_flags(3);
//================== split ==================
	split->set_name("Split");
	scroll_container->add_child(split);
	split->set_h_size_flags(3);
	split->set_v_size_flags(3);
	split->set_split_offset(50);
	split->set_dragger_visibility(SplitContainer::DraggerVisibility::DRAGGER_VISIBLE);
//================== select ==================
	select->set_name("Select");
	split->add_child(select);
//================== top_btn ==================
	top_btn->set_name("Top_btn");
	select->add_child(top_btn);
	top_btn->set_text("Top:");
	top_btn->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
	top_btn->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_CHAR);
//================== bottom_btn ==================
	bottom_btn->set_name("Bottom_btn");
	select->add_child(bottom_btn);
	bottom_btn->set_text("Bottom:");
	bottom_btn->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
	bottom_btn->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_CHAR);
//================== left_btn ==================
	left_btn->set_name("Left_btn");
	select->add_child(left_btn);
	left_btn->set_text("Left:");
	left_btn->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
	left_btn->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_CHAR);
//================== right_btn ==================
	right_btn->set_name("Right_btn");
	select->add_child(right_btn);
	right_btn->set_text("Right:");
	right_btn->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
	right_btn->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_CHAR);
//================== front_btn ==================
	front_btn->set_name("Front_btn");
	select->add_child(front_btn);
	front_btn->set_text("Front:");
	front_btn->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
	front_btn->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_CHAR);
//================== back_btn ==================
	back_btn->set_name("Back_btn");
	select->add_child(back_btn);
	back_btn->set_text("Back:");
	back_btn->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
	back_btn->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_CHAR);
//================== make_btn ==================
	make_btn->set_name("Make_btn");
	select->add_child(make_btn);
	make_btn->set_text("Make");
	make_btn->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_CHAR);
//================== tree ==================
	tree->set_name("Tree");
	split->add_child(tree);
	tree->set_columns(2);
	tree->set_select_mode(Tree::SELECT_ROW);
//================== file_dialog ==================
	file_dialog->set_name("FileDialog");
	this->add_child(file_dialog);
	file_dialog->set_title("Open a File");
	file_dialog->set_size(Vector2i(327, 300));
	file_dialog->set_ok_button_text("Open");
	file_dialog->set_file_mode(FileDialog::FILE_MODE_OPEN_FILE);
	file_dialog->set_access(FileDialog::Access::ACCESS_FILESYSTEM);
	file_dialog->set_filters({"*.png"});
//================== popup_menu ==================
	popup_menu->set_name("PopupMenu");
	this->add_child(popup_menu);
	popup_menu->set_size(Vector2i(136, 170));
	popup_menu->set_item_count(6);
	popup_menu->set_item_text(0, "Set as top");
	popup_menu->set_item_id(0, 0);
	popup_menu->set_item_text(1, "Set as bottom");
	popup_menu->set_item_id(1, 1);
	popup_menu->set_item_text(2, "Set as left");
	popup_menu->set_item_id(2, 2);
	popup_menu->set_item_text(3, "Set as right");
	popup_menu->set_item_id(3, 3);
	popup_menu->set_item_text(4, "Set as front");
	popup_menu->set_item_id(4, 4);
	popup_menu->set_item_text(5, "Set as back");
	popup_menu->set_item_id(5, 5);


	top_btn->connect("pressed", callable_mp(this, &CubeMapDock::_on_top_btn_pressed));
	bottom_btn->connect("pressed", callable_mp(this, &CubeMapDock::_on_bottom_btn_pressed));
	left_btn->connect("pressed", callable_mp(this, &CubeMapDock::_on_left_btn_pressed));
	right_btn->connect("pressed", callable_mp(this, &CubeMapDock::_on_right_btn_pressed));
	front_btn->connect("pressed", callable_mp(this, &CubeMapDock::_on_front_btn_pressed));
	back_btn->connect("pressed", callable_mp(this, &CubeMapDock::_on_back_btn_pressed));
	make_btn->connect("pressed", callable_mp(this, &CubeMapDock::_on_make_btn_pressed));
	file_dialog->connect("file_selected", callable_mp(this, &CubeMapDock::_on_file_dialog_file_selected));
	popup_menu->connect("index_pressed", callable_mp(this, &CubeMapDock::_on_popup_menu_index_pressed));
	tree->connect("gui_input", callable_mp(this, &CubeMapDock::_tree_gui_input));
	UpdateTree();
}


CubeMapDock::~CubeMapDock() {
	singleton = nullptr;
}
