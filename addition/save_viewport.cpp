
#include "editor/editor_file_system.h"
#include "editor/editor_inspector.h"
#include "editor/editor_node.h"
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

#include "editor/scene_tree_dock.h"
#include "scene/main/node.h"
#include "core/io/dir_access.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/2d/camera_2d.h"
#include "scene/gui/color_rect.h"
#include "scene/animation/animation_player.h"
#include "editor/editor_node.h"
#include "scene/resources/surface_tool.h"
#include "scene/main/viewport.h"
#include "save_viewport.h"
// ----Beging of view port saving

void save_viewport(String to, int width, int height, Color background)
{
	auto root = EditorNode::get_singleton()->get_tree()->get_edited_scene_root();
	if (root)
	{
		auto root_as_CanvasItem = Object::cast_to<CanvasItem>(root);
		auto root_as_3d = Object::cast_to<Node3D>(root);
		auto root_as_wp = Object::cast_to<SubViewport>(root);
		if (root_as_CanvasItem)
		{
			print_line("root_as_CanvasItem");
			auto size = root->get_viewport()->get_camera_rect_size();

			auto camera = memnew(Camera2D);
			auto vwp = memnew(SubViewport);
			SceneTreeDock::get_singleton()->add_root_node(vwp);
			if(width > 0)
				size[0] = width;
			
			if(height > 0)
				size[1] = height;
			
			vwp->set_size(size);

			vwp->set_transparent_background(true);

			vwp->add_child(camera);
			camera->set_owner(vwp);
			camera->set_anchor_mode(Camera2D::ANCHOR_MODE_FIXED_TOP_LEFT);
			camera->set_enabled(1);
			camera->make_current();

			vwp->set_update_mode(SubViewport::UPDATE_ALWAYS);

			auto color_rect = memnew(ColorRect);

			color_rect->set_anchors_preset(Control::PRESET_FULL_RECT);
			color_rect->set_size(size);
			color_rect->set_color(background);

			vwp->add_child(color_rect);
			color_rect->set_owner(vwp);
			color_rect->force_update_transform();
			color_rect->draw_rect(Rect2({0, 0}, size), background);

			vwp->add_child(root);
			root->set_owner(vwp);

			vwp->update_canvas_items();
			vwp->get_tree()->process(0.001);
			RenderingServer::get_singleton()->sync();
			RenderingServer::get_singleton()->draw(true, 0.001);
			Ref<Image> img = vwp->get_texture()->get_image()->duplicate();


			img->save_png(to);

			vwp->remove_child(root);
			vwp->remove_child(camera);
			vwp->remove_child(color_rect);

			//img.unref();

			SceneTreeDock::get_singleton()->add_root_node(root);
		}
		else if(root_as_3d)
		{
			print_line("root_as_3d");
			SubViewport* vwp = nullptr;
			Camera3D* camera = find_node_by_type<Camera3D>(root_as_3d);

			SubViewport* tvwp = nullptr;
			if(!vwp)
			{
				vwp = tvwp = memnew(SubViewport);

				vwp->set_name("TempSubViewport");
				root->get_parent()->remove_child(root);
				vwp->add_child(root);
				

				root->set_owner(vwp);
				vwp->set_update_mode(SubViewport::UPDATE_ALWAYS);

				SceneTreeDock::get_singleton()->add_root_node(vwp);
			}

			Camera3D* tcamera = nullptr;
			if(!camera)
			{
				camera = tcamera = memnew(Camera3D);
				root->add_child(camera);
				camera->set_owner(root);
				camera->set_name("Camera");
				camera->set_transform(Transform3D(Basis({0, 1, 0}, Math_PI * 0.5)));
				camera->set_position({ 1, 0, 0 });
			}

			camera->set_current(true);
			vwp->update_canvas_items();
			vwp->get_tree()->process(0.001);
			RenderingServer::get_singleton()->sync();
			RenderingServer::get_singleton()->draw(true, 0.001);
			Ref<Image> img = vwp->get_texture()->get_image()->duplicate();
			img->save_png(to);
			print_line(to);

			if(tvwp)
			{
				vwp->remove_child(root);
				root->remove_child(camera);
				SceneTreeDock::get_singleton()->add_root_node(root);
				//memdelete(tvwp);
			}

			// if(tcamera)
			// 	memdelete(tcamera);
		}
		else if(root_as_wp)
		{
			print_line("root_as_wp");
			SubViewport* vwp = root_as_wp;
			Camera3D* camera = find_node_by_type<Camera3D>(root_as_3d);

			if(camera)
			{
				vwp->set_update_mode(SubViewport::UPDATE_ALWAYS);
				camera->set_current(true);
				vwp->update_canvas_items();
				vwp->get_tree()->process(0.001);
				RenderingServer::get_singleton()->sync();
				RenderingServer::get_singleton()->draw(true, 0.001);
				RenderingServer::get_singleton()->draw(true, 0.001);
				Ref<Image> img = vwp->get_texture()->get_image()->duplicate();
				img->save_png(to);
				img.unref();
			}
		}
	}
}

// ----End of view port saving

