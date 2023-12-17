#ifndef ADD_RIGGING_H
#define ADD_RIGGING_H


#include "editor/editor_file_system.h"
#include "editor/editor_inspector.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/2d/skeleton_2d.h"
#include "scene/2d/mesh_instance_2d.h"
#include "scene/gui/button.h"
#include "scene/gui/flow_container.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/file_dialog.h"
#include "scene/animation/animation_player.h"
#include "core/io/json.h"
#include <string>
#include <vector>

#include "player_handler.h"
#include "skeleton_object.h"


class TreeItem;


class RiggingSystemDock : public Control {
	GDCLASS(RiggingSystemDock, Control);
public:
	
	struct RiggingSystemRet
	{
		ArrayMesh* mesh = nullptr;
		MeshInstance3D* meshinst = nullptr;
		Skeleton3D* skelet = nullptr;
	};

private:

	static RiggingSystemDock *singleton;
	
// Include autogenned header part
#include "layouts/rigging_dock_hpp.h"
	
	String node_path = "";
	Node* path_node = nullptr;

	FileDialog *file_dialog_open_obj = nullptr;
	FileDialog *file_dialog_open_skeleton = nullptr;
	FileDialog *file_dialog_export_animation = nullptr;
	FileDialog *file_dialog_select_texture = nullptr;
	FileDialog *file_dialog_select_anim_folder = nullptr;
	FileDialog *file_dialog_open_skeleton2d = nullptr;
	
	SubViewport* vwp = nullptr;
	AnimationPlayer* anim_player = nullptr;
	AnimationLibrary* anim_lib = nullptr;
	Camera3D* camera = nullptr;
	AddPlayerHandler* handler = nullptr;

	std::string obj = "";
	std::string skeleton_json_str = "";
	std::string skeleton2d_json_str = "";
	std::string obj_path = "";
	std::string path_to_export = "";

	RiggingSystemRet last_genned;

	void on_button_pressed();
	void on_button_open_obj_pressed();
	void on_button_open_skeleton_pressed();
	void file_dialog_open_obj_file_selected(String path);
	void file_dialog_open_skeleton_file_selected(String path);
	void file_dialog_export_animation_file_selected(String path);
	void file_dialog_select_texture_file_selected(String path);
	void file_dialog_select_anim_folder_file_selected(String path);
	void file_dialog_open_skeleton2d_file_selected(String path);
	void find_skeleton_and_print_path(Node* start_node, String state);
	void save_start_position();
	void save_final_position();
	void interpolate_animation();
	void lerp_pose_with_animation_player();
	void on_button_export_skeleton_pressed();
	void on_button_export_animation_pressed();
	void on_button_generate2d_pressed();
	void on_button_select2dtexture_pressed();
	void on_button_save_viewport_pressed();
	void on_button_select_anim_folder_pressed();
	void on_button_open_skeleton2d_pressed();
	void on_button_reset_state_pressed();

protected:
	List<Vector<Vector3>> current_positions;
	List<Vector<Quaternion>> current_rotations;
	Skeleton3D *target_skeleton;
	SkeletonObject s_object;
	SkeletonObject s_object2d;

public:
    void capture_start_pose();
	void capture_final_pose();
	void ensure_animation_player();
	void set_target_skeleton(Node* node, String state);
	void save_current_pose(String state);

	static RiggingSystemDock *get_singleton() { return singleton; }

	RiggingSystemRet Gen(std::string obj = "");

	RiggingSystemDock();
	~RiggingSystemDock();
};

#endif // ADD_CUBEMAP_DOCK_H
