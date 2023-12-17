
#include "player_handler.h"
#include <functional>
#include <fstream>
#include <string>

#include "scene/3d/skeleton_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/animation/animation_player.h"
#include "scene/main/viewport.h"
#include "scene/3d/camera_3d.h"
#include "core/io/dir_access.h"

static bool is_editor = false;

void AddPlayerHandler::set_is_editor(bool v)
{
	is_editor = v;
}

void AddPlayerHandler::_bind_methods()
{
	ADD_GROUP("Player Handler", "player_handler_");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "player_handler_description", PROPERTY_HINT_MULTILINE_TEXT), "set_editor_description", "get_editor_description");

}

void AddPlayerHandler::set_editor_description(const String& p_editor_description)
{
	data = p_editor_description;
}

String AddPlayerHandler::get_editor_description() const
{
	return data;
}

void AddPlayerHandler::_notification(int p_what)
{
	switch (p_what)
	{
	case NOTIFICATION_READY:
	{
		if (is_editor)
		{

			print_line("Started from the editor viewport");
			print_line(p_what);
		}
		else
		{


			auto proces_func = [this]()
				{
					print_line("proces_func");
					std::string path_to_export = "";
					std::function<Node* (Node*, String)> find = [&find](Node* start_node, String name) -> Node*
						{
							if (!start_node)
							{
								return nullptr;
							}

							String node_path = start_node->get_path();

							if (node_path.find(name) != -1)
							{
								return start_node;
							}

							for (int i = 0; i < start_node->get_child_count(); i++)
							{
								Node* ret = find(start_node->get_child(i), name);
								if (ret)
									return ret;
							}
							return nullptr;
						};
					
					auto root = this->get_parent();
					while(root->get_parent())
						root = root->get_parent();

					SubViewport* vwp = Object::cast_to<SubViewport>(find(root, "SubViewport"));
					AnimationPlayer* anim_player = Object::cast_to<AnimationPlayer>(find(root, "Generated_AnimPlayer"));
					Camera3D* camera = Object::cast_to<Camera3D>(find(root, "Camera"));

					anim_player->play("LerpLib/LerpPose");
					const auto& anim = anim_player->get_animation("LerpLib/LerpPose");
					float step = 0.1;
					int len = anim->get_length() / step + 1;
					print_line(len);
					anim_player->pause();
					camera->set_current(true);
					vwp->set_update_mode(SubViewport::UPDATE_ALWAYS);
					//RS::get_singleton()->viewport_set_active(vwp->get_viewport_rid(), true);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					std::string pth = "";
					if (path_to_export.size() != 0)
						pth = path_to_export + "/";
					DirAccess::create(DirAccess::ACCESS_FILESYSTEM)->make_dir_recursive_absolute((pth + "out/").c_str());
					for (int i = 0; i < len; i++)
					{
						anim_player->seek(i * step, true);
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
						vwp->update_canvas_items();
						vwp->set_size(vwp->get_size());
						Ref<Image> img = vwp->get_texture()->get_image()->duplicate();
						img->save_png((pth + "out/" + std::to_string(i) + ".png").c_str());

					}

					this->get_tree()->quit();

				};
			std::thread ths(proces_func);
			ths.detach();


			std::time_t tp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			auto lct = std::localtime(&tp);
			char time_buff[512];
			snprintf(time_buff, 512, "Y%.2iM%.2iD%.2iTh%.2im%.2is%.2i", lct->tm_year % 100, lct->tm_mon, lct->tm_mday, lct->tm_hour, lct->tm_min, lct->tm_sec);
			print_line(String(time_buff));
			print_line("Started not from the editor viewport");
			print_line(p_what);

		}
		break;
	}
	default:
		break;
	};



}

AddPlayerHandler::AddPlayerHandler()
{

}

AddPlayerHandler::~AddPlayerHandler()
{

}


