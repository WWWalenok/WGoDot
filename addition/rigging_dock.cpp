

// get inworld position example
/*
Camera3D *const camera =  get_viewport()->get_camera_3d();
auto from = camera->project_ray_origin(get_viewport()->get_mouse_position());
auto to = from + camera->project_ray_normal(get_viewport()->get_mouse_position()) * 1000;
PhysicsDirectSpaceState3D::RayParameters ray_params;
ray_params.from = from;
ray_params.to = to;
PhysicsDirectSpaceState3D::RayResult r;
bool intersected = get_tree()->get_root()->get_world_3d()->get_direct_space_state()->intersect_ray(ray_params, r);
*/

#include "rigging_dock.h"
#include <signal.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

RiggingSystemDock* RiggingSystemDock::singleton = nullptr;

#include "g_skeleton.h"
#include "rigging_dock_mech.ply.h"

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

#include "obj_parser.h"

#include "delaunator.hpp"

#include "modules/gltf/gltf_document.h"

#include "save_viewport.h"


struct surface_t
{
	PackedVector3Array vertices = PackedVector3Array();
	PackedVector3Array normals = PackedVector3Array();
	PackedVector3Array texcoord = PackedVector3Array();
	PackedInt32Array indices = PackedInt32Array();
	PackedFloat32Array weights = PackedFloat32Array();
	PackedInt32Array bones = PackedInt32Array();
	Ref<StandardMaterial3D> material = nullptr;
};

struct bone_t
{
	float r;
	int id;
};
// You can find examples of working with animation in "tests/scene/test_animation.h"

#if !defined(TEST_JSON)
const char TEST_JSON[] = R"JSON(
[
	{
		"name": "root",
		"parent": -1,
		"transform_mat": [
			[1.0, 0.0, 0.0, 0.0],
			[0.0, 1.0, 0.0, 0.0],
			[0.0, 0.0, 1.0, 0.0],
			[0.0, 0.0, 0.0, 1.0]
		]
	},
	{
		"name": "bone0",
		"parent": 0,
		"transform_mat": [
			[1.0, 0.0, 0.0, 0.0],
			[0.0, 1.0, 0.0, 0.0],
			[0.0, 0.0, 1.0, 0.0],
			[0.0, 0.0, 0.0, 1.0]
		]
	},
	{
		"name": "bone01",
		"parent": 1,
		"transform_mat": [
			[1.0, 0.0, 0.0, 0.0],
			[0.0, 1.0, 0.0, 0.0],
			[0.0, 0.0, 1.0, 0.0],
			[0.0, 0.0, 0.0, 1.0]
		]
	},
	{
		"name": "bone1",
		"parent": 0,
		"transform_mat": [
			[1.0, 0.0, 0.0, 0.0],
			[0.0, 1.0, 0.0, 0.0],
			[0.0, 0.0, 1.0, 0.0],
			[0.0, 0.0, 0.0, 1.0]
		]
	},
	{
		"name": "bone11",
		"parent": 3,
		"transform_mat": [
			[1.0, 0.0, 0.0, 0.0],
			[0.0, 1.0, 0.0, 0.0],
			[0.0, 0.0, 1.0, 0.0],
			[0.0, 0.0, 0.0, 1.0]
		]
	}
])JSON";
#endif

void print_trans(const Transform3D& pr)
{
	printf(
		"        {\n"
		"            {%-6f, %-6f, %-6f, %-6f},\n"
		"            {%-6f, %-6f, %-6f, %-6f},\n"
		"            {%-6f, %-6f, %-6f, %-6f},\n"
		"            {%-6f, %-6f, %-6f, %-6f}\n"
		"        },\n",
		pr.basis[0][0], pr.basis[0][1], pr.basis[0][2], 0.0,
		pr.basis[1][0], pr.basis[1][1], pr.basis[1][2], 0.0,
		pr.basis[2][0], pr.basis[2][1], pr.basis[2][2], 0.0,
		pr.origin[0], pr.origin[1], pr.origin[2], 1.0
	);
}

namespace pds
{

struct Point2f
{
	float x = 0, y = 0;
};

inline float rand(float s)
{
	static const float r_max = RAND_MAX + 1;
	return (::rand() * s) / r_max;
}

Point2f generateRandomPointAround(const Point2f& point, float mindist)
{
	//non-uniform, favours points closer to the inner ring, leads to denser packings
	//random radius between mindist and 2 * mindist
	float radius = mindist * (rand(1) + 1);
	//random angle
	float angle = 6.28318530717958647 * rand(1);
	//the new point is generated around the point (x, y)
	float newX = point.x + radius * cos(angle);
	float newY = point.y + radius * sin(angle);
	return { newX, newY };
};

template<unsigned short int gird_size = 1000>
std::vector<float> pds(float (*get_min_r)(float, float), float r_max, int width, int height, int new_points_count)
{
	typedef Point2f Point;
	//Create the grid
	auto cellSize = r_max / sqrt(2);
	int x_size = width / cellSize;
	int y_size = height / cellSize;

	std::vector<std::vector<std::vector<Point>>> grid(std::vector<std::vector<Point>>(std::vector<Point>(), y_size + 1), x_size + 1);

	std::function<std::vector<Point>& (const Point&)> GetGrid = [&](const Point& point) -> std::vector<Point>&
		{
			auto gridX = (int)(point.x / cellSize);
			auto gridY = (int)(point.y / cellSize);
			return grid[gridX][gridY];
		};

	std::function<bool(const Point&, float)> Chek = [&](const Point& point, float mindist)
		{
			auto gridX = (int)(point.x / cellSize);
			auto gridY = (int)(point.y / cellSize);
			auto gridX1 = gridX - 2;
			auto gridX2 = gridX + 2;
			if (gridX1 < 0) 			gridX1 = 0;
			if (gridX2 > x_size) 	gridX2 = x_size;
			auto gridY1 = gridY - 2;
			auto gridY2 = gridY + 2;
			if (gridY1 < 0) 			gridY1 = 0;
			if (gridY2 > y_size) 	gridY2 = x_size;

			//get the neighbourhood if the point in the grid
			for (gridX = gridX1; gridX <= gridX2; ++gridX) for (gridY = gridY1; gridY <= gridY2; ++gridY)
			{
				auto points = grid[gridX][gridY];
				for (const auto& el : points)
				{
					float dx = el.x - point.x;
					float dy = el.y - point.y;
					if (dx * dx + dy * dy < mindist)
						return false;
				}
			}
			return true;
		};

	std::list<Point> processList;
	std::vector<float> samplePoints;

	Point firstPoint = { rand(width), rand(height) };

	//update containers
	processList.push_back(firstPoint);
	samplePoints.push_back(firstPoint.x);
	samplePoints.push_back(firstPoint.y);
	GetGrid(firstPoint).push_back(firstPoint);

	//generate other points from points in queue.
	while (!processList.empty())
	{
		auto point = processList.pop_front();
		float min_dist = get_min_r(point.x, point.y);
		for (i = 0; i < new_points_count; i++)
		{
			auto newPoint = generateRandomPointAround(point, min_dist);
			//check that the point is in the image region
			//and no points exists in the point's neighbourhood
			if (Chek(newPoint, min_dist))
			{
				//update containers
				processList.push_back(newPoint);
				samplePoints.push_back(newPoint.x);
				samplePoints.push_back(newPoint.y);
				GetGrid(newPoint).push_back(newPoint);
			}
		}
	}
	return samplePoints;
}


};


// ----Beginning works with json bones

surface_t make_surf(const SkeletonObject& s, int subdiv, float ox, float oy, float w, float h, Skeleton3D* skelet)
{
	int numVerticesPerRow = subdiv + 1;

	surface_t ret;
	for (int i = 0; i < subdiv + 1; i++)
	{
		for (int j = 0; j < subdiv + 1; j++)
		{
			ret.vertices.push_back({ j / (float)subdiv * h + oy, i / (float)subdiv * w + ox, 0 });
			ret.texcoord.push_back({ j / (float)subdiv, 1 - i / (float)subdiv, 0 });
			ret.normals.push_back({ 0, 0, 1 });
		}
	}
	for (int i = 0; i < subdiv; i++)
	{
		for (int j = 0; j < subdiv; j++)
		{
			int topLeft = i * numVerticesPerRow + j;
			int topRight = topLeft + 1;
			int bottomLeft = (i + 1) * numVerticesPerRow + j;
			int bottomRight = bottomLeft + 1;

			ret.indices.push_back(bottomLeft);
			ret.indices.push_back(topRight);
			ret.indices.push_back(topLeft);

			ret.indices.push_back(bottomLeft);
			ret.indices.push_back(bottomRight);
			ret.indices.push_back(topRight);
		}
	}
	if (skelet != nullptr)
	{
		auto vs = ret.vertices.size();
		for (int i = 0; i < vs; ++i)
		{
			auto best = s.get_best_points(ret.vertices[i], skelet);

			for (int j = 0; j < 4; j++)
			{
				if (best[j].second > 0.1)
					best[j].second = powf(best[j].second, 5) * 10;
				else
					best[j].second = 0;
			}

			float sm = 1e-30;


			for (int j = 0; j < 4; j++)
				sm += best[j].second;

			for (int j = 0; j < 4; j++)
			{
				ret.weights.push_back(best[j].second / sm);
				ret.bones.push_back(best[j].first);
			}
		}
	}
	return ret;
}

void rig(ArrayMesh*& mesh, Skeleton3D*& skelet)
{
	int ss_s = mesh->get_surface_count();
	std::vector<surface_t> ss;
	for (int __s = 0; __s < ss_s; __s++)
	{
		surface_t s;
		auto arrs = mesh->surface_get_arrays(__s);
		s.vertices = arrs[Mesh::ARRAY_VERTEX];
		s.weights = arrs[Mesh::ARRAY_WEIGHTS];
		s.bones = arrs[Mesh::ARRAY_BONES];

		s.normals = arrs[Mesh::ARRAY_NORMAL];
		s.texcoord = arrs[Mesh::ARRAY_TEX_UV];
		s.indices = arrs[Mesh::ARRAY_INDEX];

		printf("%i %i %i %i %i %i \n",
			s.vertices.size(),
			s.weights.size(),
			s.bones.size(),
			s.normals.size(),
			s.texcoord.size(),
			s.indices.size()
		);

		s.material = mesh->surface_get_material(__s);

		ss.push_back(s);
	}

	mesh->clear_surfaces();

	mesh = memnew(ArrayMesh);

	int s_s = skelet->get_bone_count();

	SkeletonObject l_skeleton;

	l_skeleton.from_skeleton3d(skelet);

	l_skeleton.applay_to_skeleton3d(skelet);

	skelet->reset_bone_poses();
	skelet->register_skin(skelet->create_skin_from_rest_transforms());

	for (int __s = 0; __s < ss_s; __s++)
	{
		auto& vertices = ss[__s].vertices;
		auto& normals = ss[__s].normals;
		auto& texcoord = ss[__s].texcoord;
		auto& indices = ss[__s].indices;
		auto& weights = ss[__s].weights;
		auto& bones = ss[__s].bones;

		bones.clear();
		weights.clear();

		for (int i = 0; i < vertices.size(); i++)
		{
			auto bs = l_skeleton.get_best_points(vertices[i], skelet);

			float s = bs[0].second + bs[1].second + bs[2].second + bs[3].second;
			// printf("s: %f, 1: %f, 2: %f, 3: %f, 4: %f\n",
			// 	s * 1.0,
			// 	bs[0].second * 1.0,
			// 	bs[1].second * 1.0,
			// 	bs[2].second * 1.0,
			// 	bs[3].second * 1.0
			// );

			for (int k = 0; k < 4; k++)
			{
				bones.push_back(bs[k].first);
				weights.push_back(bs[k].second / s);
			}
		}

		Array arrays;
		arrays.resize(Mesh::ARRAY_MAX);


		arrays[Mesh::ARRAY_VERTEX] = vertices;
		if (normals.size() > 0)
		{
			arrays[Mesh::ARRAY_NORMAL] = normals;
		}
		if (texcoord.size() > 0)
		{
			arrays[Mesh::ARRAY_TEX_UV] = texcoord;
		}
		arrays[Mesh::ARRAY_INDEX] = indices;
		arrays[Mesh::ARRAY_WEIGHTS] = weights;
		arrays[Mesh::ARRAY_BONES] = bones;
		int s_id = mesh->get_surface_count();
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);


		mesh->surface_set_material(s_id, ss[__s].material);
	}

}

void unrig(ArrayMesh*& mesh)
{
	int ss_s = mesh->get_surface_count();
	std::vector<surface_t> ss;
	for (int __s = 0; __s < ss_s; __s++)
	{
		surface_t s;
		auto arrs = mesh->surface_get_arrays(__s);
		s.vertices = arrs[Mesh::ARRAY_VERTEX];
		s.weights = arrs[Mesh::ARRAY_WEIGHTS];
		s.bones = arrs[Mesh::ARRAY_BONES];

		s.normals = arrs[Mesh::ARRAY_NORMAL];
		s.texcoord = arrs[Mesh::ARRAY_TEX_UV];
		s.indices = arrs[Mesh::ARRAY_INDEX];

		printf("%i %i %i %i %i %i \n",
			s.vertices.size(),
			s.weights.size(),
			s.bones.size(),
			s.normals.size(),
			s.texcoord.size(),
			s.indices.size()
		);

		s.material = mesh->surface_get_material(__s);

		ss.push_back(s);
	}

	mesh->clear_surfaces();

	mesh = memnew(ArrayMesh);

	for (int __s = 0; __s < ss_s; __s++)
	{
		surface_t& s = ss[__s];

		auto& vertices = s.vertices;
		auto& normals = s.normals;
		auto& texcoord = s.texcoord;
		auto& indices = s.indices;
		auto& weights = s.weights;
		auto& bones = s.bones;

		bones.clear();
		weights.clear();

		printf("%i %i %i %i %i %i \n",
			vertices.size(),
			weights.size(),
			bones.size(),
			normals.size(),
			texcoord.size(),
			indices.size()
		);



		Array arrays;
		arrays.resize(Mesh::ARRAY_MAX);


		arrays[Mesh::ARRAY_VERTEX] = vertices;
		if (normals.size() > 0)
		{
			arrays[Mesh::ARRAY_NORMAL] = normals;
		}
		if (texcoord.size() > 0)
		{
			arrays[Mesh::ARRAY_TEX_UV] = texcoord;
		}
		arrays[Mesh::ARRAY_INDEX] = indices;

		int s_id = mesh->get_surface_count();
		printf("%i\n", s_id);
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);


		mesh->surface_set_material(s_id, ss[__s].material);
	}
}

RiggingSystemDock::RiggingSystemRet RiggingSystemDock::Gen(std::string obj)
{
	print_line("step 1");
	ArrayMesh* mesh = memnew(ArrayMesh);
	MeshInstance3D* meshinst = memnew(MeshInstance3D);

	std::vector<surface_t> surfacees;
	if (obj.size() == 0)
	{
		surfacees.push_back(surface_t());
		for (int i = 0; i < rigging_dock_vertex_s; i++)
			surfacees[0].vertices.append(Vector3(rigging_dock_vertex[i * 3], rigging_dock_vertex[i * 3 + 1], rigging_dock_vertex[i * 3 + 2]));

		for (int i = 0; i < rigging_dock_face_s; i += 3)
		{
			surfacees[0].indices.append(rigging_dock_face[i + 2]);
			surfacees[0].indices.append(rigging_dock_face[i + 1]);
			surfacees[0].indices.append(rigging_dock_face[i + 0]);
		}
	}
	else
	{
		obj_parser parser;
		parser.Load(obj, obj_path);
		int id = 0;

		for (auto& e : parser.LoadedMeshes)
		{
			surface_t surface;
			auto& v = e.v;
			auto& f = e.f;
			int s = v.size();

			for (int i = 0; i < s; ++i)
			{
				surface.vertices.push_back(Vector3
				(
					v[i].v[0],
					v[i].v[1],
					v[i].v[2]
				));
				if (e.mode & obj_parser::EMode_VT)
					surface.texcoord.push_back(Vector3
					(
						v[i].vt[0],
						1 - v[i].vt[1],
						0
					));

				if (e.mode & obj_parser::EMode_VN)
					surface.normals.push_back(Vector3
					(
						v[i].vn[0],
						v[i].vn[1],
						v[i].vn[2]
					));
			}

			s = f.size();

			for (int i = 0; i < s; i += 3)
			{
				surface.indices.push_back(f[i + 2]);
				surface.indices.push_back(f[i + 1]);
				surface.indices.push_back(f[i + 0]);
			}

			if (!e.MeshMaterialName.empty())
			{
				Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);

				material->set_name(e.MeshMaterialName.c_str());

				auto& Kd = e.MeshMaterial.Kd;
				auto& Ks = e.MeshMaterial.Ks;
				auto& Ns = e.MeshMaterial.Ns;
				auto& d = e.MeshMaterial.d;
				auto& Tr = e.MeshMaterial.Tr;

				auto a = d * 0.5 + (1 - Tr) * 0.5;

				std::string pathtomat = obj_path.substr(0, obj_path.find_last_of("/\\") + 1);
				if (!e.MeshMaterial.map_Kd.empty())
				{
					material->set_texture(BaseMaterial3D::TextureParam::TEXTURE_ALBEDO, ImageTexture::create_from_image(Image::load_from_file((pathtomat + e.MeshMaterial.map_Kd).c_str())));
				}
				else
					material->set_albedo(Color(Kd[0], Kd[1], Kd[2], a));

				if (!e.MeshMaterial.map_Ns.empty())
					material->set_texture(BaseMaterial3D::TextureParam::TEXTURE_METALLIC, ImageTexture::create_from_image(Image::load_from_file((pathtomat + e.MeshMaterial.map_Ns).c_str())));
				else
					material->set_metallic((1000.0 - Ns) / 1000.0);




				surface.material = material;
			}
			surfacees.push_back(surface);

		}
	}

	std::vector<Vector3> origins;

	Skeleton3D* skelet = memnew(Skeleton3D);

	s_object.applay_to_skeleton3d(skelet);

	origins.resize(skelet->get_bone_count());

	int s_s = skelet->get_bone_count();

	for (int j = 0; j < s_s; j++)
		skelet->reset_bone_pose(j);

	for (int __s = 0; __s < surfacees.size(); __s++)
	{
		auto& vertices = surfacees[__s].vertices;
		auto& normals = surfacees[__s].normals;
		auto& texcoord = surfacees[__s].texcoord;
		auto& indices = surfacees[__s].indices;

		Array arrays;
		arrays.resize(Mesh::ARRAY_MAX);


		arrays[Mesh::ARRAY_VERTEX] = vertices;
		if (normals.size() > 0)
		{
			arrays[Mesh::ARRAY_NORMAL] = normals;
		}
		if (texcoord.size() > 0)
		{
			arrays[Mesh::ARRAY_TEX_UV] = texcoord;
		}
		arrays[Mesh::ARRAY_INDEX] = indices;
		int s_id = mesh->get_surface_count();
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

		mesh->surface_set_material(s_id, surfacees[__s].material);
	}

	rig(mesh, skelet);

	meshinst->set_mesh(mesh);
	skelet->set_name("Generated_Skeleton3D");
	meshinst->set_name("Generated_MeshInstance3D");

	return { mesh, meshinst, skelet };
}

void RiggingSystemDock::find_skeleton_and_print_path(Node* start_node, String state)
{
	if (!start_node)
	{
		return;
	}

	node_path = start_node->get_path();
	//print_line(node_path);

	if (start_node->get_name().operator String() == "Generated_Skeleton3D")
	{
		print_line(node_path);
		path_node = start_node;
		set_target_skeleton(start_node, state);
	}

	for (int i = 0; i < start_node->get_child_count(); i++)
	{
		find_skeleton_and_print_path(start_node->get_child(i), state);
	}
}

void RiggingSystemDock::set_target_skeleton(Node* node, String state)
{
	Skeleton3D* skeleton = Object::cast_to<Skeleton3D>(node);
	if (skeleton)
	{
		print_line("Skeleton pose was found!");
		target_skeleton = skeleton;
	}
}

void RiggingSystemDock::save_current_pose(String state)
{
	if (!target_skeleton)
	{
		ERR_PRINT("Target skeleton not set or not found");
		return;
	}

	Vector<Vector3> position;
	Vector<Quaternion> rotation;

	position.resize(target_skeleton->get_bone_count());
	rotation.resize(target_skeleton->get_bone_count());

	for (int i = 0; i < target_skeleton->get_bone_count(); i++)
	{
		position.write[i] = target_skeleton->get_bone_pose_position(i) / target_skeleton->get_motion_scale();
		rotation.write[i] = target_skeleton->get_bone_pose_rotation(i);
	}

	current_positions.push_back(position);
	current_rotations.push_back(rotation);
	print_line("Skeleton pose saved!");
}

void RiggingSystemDock::lerp_pose_with_animation_player()
{
	get_tree()->get_edited_scene_root()->print_tree();

	ERR_FAIL_COND(!target_skeleton);
	ERR_FAIL_COND(!anim_player);
	ERR_FAIL_COND(!anim_lib);
	ERR_FAIL_COND(!last_genned.meshinst);
	ERR_FAIL_COND(!last_genned.skelet);
	ERR_FAIL_COND(!handler);
	ERR_FAIL_COND(!vwp);

	List<Vector<Vector3>> add_positions;
	List<Vector<Quaternion>> add_rotations;

	if (line_edit_select_anim_folder->get_text().size() > 0)
	{
		auto dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
		if (dir->change_dir(line_edit_select_anim_folder->get_text()) == OK)
		{
			auto files = dir->get_files();

			for (int i = 0; i < files.size(); i++) if (files[i].ends_with(".glb") || files[i].ends_with(".json"))
			{
				if (files[i].ends_with(".glb"))
				{
					HashMap<StringName, Variant> p_options;
					String p_path = line_edit_select_anim_folder->get_text() + "/" + files[i];

					print_line(p_path);

					Ref<GLTFDocument> gltf;
					gltf.instantiate();
					Ref<GLTFState> state;
					state.instantiate();
					Error err = gltf->append_from_file(p_path, state);
					if (err != OK)
					{
						return;
					}

					bool trimming = false;
					bool remove_immutable = true;
					auto scene = gltf->generate_scene(state, 30, trimming, remove_immutable);

					auto new_skeleton = find_node_by_type<Skeleton3D>(scene);

					auto b_p = new_skeleton->get_bone_pose(0);

					auto trans = Transform3D(
						{ 1.00,  0.00, 0.00 },
						{ 0.00,  1.00, 0.00 },
						{ 0.00,  0.00, 1.00 },
						{ 0.00,  0.00, 0.00 }
					);

					new_skeleton->set_bone_rest(0, trans);

					new_skeleton->reset_bone_poses();

					Vector<Vector3> position;
					Vector<Quaternion> rotation;

					position.resize(new_skeleton->get_bone_count());
					rotation.resize(new_skeleton->get_bone_count());

					for (int i = 0; i < new_skeleton->get_bone_count(); i++)
					{
						position.write[i] = new_skeleton->get_bone_pose_position(i) / new_skeleton->get_motion_scale();
						rotation.write[i] = new_skeleton->get_bone_pose_rotation(i);
					}

					add_positions.push_back(position);
					add_rotations.push_back(rotation);

					std::function<void(Node*)> free = [&free](Node* cur)
						{
							int c = cur->get_child_count();
							std::vector<Node*> childs;
							for (int i = 0; i < c; ++i)
								childs.push_back(cur->get_child(i));

							for (int i = 0; i < c; ++i)
							{
								cur->remove_child(childs[i]);
								free(childs[i]);
							}
							memdelete(cur);
						};

					free(scene);
				}
				else
				{
					String p_path = line_edit_select_anim_folder->get_text() + "/" + files[i];
					std::ifstream fin(p_path.ascii().get_data());
					fin.seekg(0, std::ios::end);
					size_t size = fin.tellg();
					skeleton_json_str.resize(size, ' ');
					fin.seekg(0);
					fin.read(skeleton_json_str.data(), size);
					SkeletonObject s;
					s.from_json(skeleton_json_str);
					auto new_skeleton = memnew(Skeleton3D);

					s.applay_to_skeleton3d(new_skeleton);
					
					auto trans = Transform3D(
						{ 0.00,  0.00, -1.00 },
						{ -1.00,  0.00, 0.00 },
						{ 0.00,  1.00, 0.00 },
						{ 0.00,  0.00, 0.00 }
					);

					new_skeleton->set_bone_rest(0, trans);


					Vector<Vector3> position;
					Vector<Quaternion> rotation;

					position.resize(new_skeleton->get_bone_count());
					rotation.resize(new_skeleton->get_bone_count());

					for (int i = 0; i < new_skeleton->get_bone_count(); i++)
					{
						position.write[i] = new_skeleton->get_bone_pose_position(i) / new_skeleton->get_motion_scale();
						rotation.write[i] = new_skeleton->get_bone_pose_rotation(i);
					}

					add_positions.push_back(position);
					add_rotations.push_back(rotation);

					memdelete(new_skeleton);
				}
			}
		}
	}

	// Ensure the poses match the number of bones.
	for (int i = 0; i < current_positions.size(); i++)
	{
		ERR_FAIL_COND(current_positions[i].size() != target_skeleton->get_bone_count());
	}

	for (int i = 0; i < current_rotations.size(); i++)
	{
		ERR_FAIL_COND(current_rotations[i].size() != target_skeleton->get_bone_count());
	}

	for (int i = 0; i < add_positions.size(); i++)
	{
		ERR_FAIL_COND(add_positions[i].size() != target_skeleton->get_bone_count());
	}

	for (int i = 0; i < add_rotations.size(); i++)
	{
		ERR_FAIL_COND(add_rotations[i].size() != target_skeleton->get_bone_count());
	}

	// Create a new animation named "LerpPose".
	Ref<Animation> animation = memnew(Animation);
	animation->set_name("LerpPose");
	float interval_seconds = 1;
	animation->set_length(interval_seconds * (current_positions.size() + add_positions.size() - 1));

	// Loop through each bone and create tracks for them.
	String path = last_genned.meshinst->get_path_to(target_skeleton);

	print_line(path);

	for (int i = 0; i < target_skeleton->get_bone_count(); i++)
	{

		String bone_path = path + ":" + target_skeleton->get_bone_name(i);

		// Add a track for the bone's transform.
		int track_idx_r = animation->add_track(Animation::TYPE_ROTATION_3D);
		int track_idx_p = animation->add_track(Animation::TYPE_POSITION_3D);
		animation->track_set_path(track_idx_r, bone_path);
		animation->track_set_path(track_idx_p, bone_path);


		// Insert keyframes.
		for (int j = 0; j < current_rotations.size(); j++)
		{
			animation->track_insert_key(track_idx_r, interval_seconds * j, current_rotations[j][i]);
			animation->track_insert_key(track_idx_p, interval_seconds * j, current_positions[j][i]);
		}

		auto add_time = current_rotations.size() * interval_seconds;

		for (int j = 0; j < add_positions.size(); j++)
		{
			animation->track_insert_key(track_idx_r, add_time + interval_seconds * j, add_rotations[j][i]);
			animation->track_insert_key(track_idx_p, add_time + interval_seconds * j, add_positions[j][i]);
		}
	}

	for (int j = 0; j < current_rotations.size(); j++)
	{
		print_line(vformat("-------%d-------", j));
	}

	anim_player->add_animation_library("LerpLib", anim_lib);

	auto c_anim_lib = anim_player->get_animation_library("LerpLib");

    List<StringName> talist;

    talist = List<StringName>();

    c_anim_lib->get_animation_list(&talist);

    for(auto el : talist)
    {
        print_line(el);
    }

	// Add the animation to the AnimationPlayer and play it.
	if (c_anim_lib->has_animation("LerpPose"))
		c_anim_lib->remove_animation("LerpPose");
	c_anim_lib->add_animation("LerpPose", animation);

    talist = List<StringName>();

    c_anim_lib->get_animation_list(&talist);

    for(auto el : talist)
    {
        print_line(el);
    }

    talist = List<StringName>();

    anim_player->get_animation_list(&talist);

    for(auto el : talist)
    {
        print_line(el);
    }
	
	handler->path_to_export = path_to_export;
	EditorNode::get_singleton()->save_all_scenes();
	EditorNode::get_singleton()->run_play_custom(vwp->get_scene_file_path());
}

void RiggingSystemDock::ensure_animation_player()
{
	print_line("Failure #1!");

	if (!last_genned.meshinst)
	{
		last_genned.meshinst = find_node<MeshInstance3D>(get_tree()->get_edited_scene_root(), "Generated_MeshInstance3D");

		ERR_FAIL_COND(!last_genned.meshinst);
	}

	if (!last_genned.skelet)
	{
		last_genned.skelet = find_node<Skeleton3D>(get_tree()->get_edited_scene_root(), "Generated_Skeleton3D");

		ERR_FAIL_COND(!last_genned.skelet);
	}

	anim_player = find_node<AnimationPlayer>(get_tree()->get_edited_scene_root(), "Generated_AnimPlayer");
	if (!anim_player)
	{
		anim_player = memnew(AnimationPlayer);
		last_genned.meshinst->add_child(anim_player);
		anim_player->set_name("Generated_AnimPlayer");
	}
	SceneTreeDock::get_singleton()->get_tree_editor()->update_tree();
	anim_lib = anim_player->get_animation_library("LerpLib").ptr();
	if (!anim_lib)
	{
		anim_lib = memnew(AnimationLibrary);
	}

	handler = find_node<AddPlayerHandler>(get_tree()->get_edited_scene_root(), "Handler");

	vwp = find_node<SubViewport>(get_tree()->get_edited_scene_root(), "SubViewport");

	find_skeleton_and_print_path(get_tree()->get_edited_scene_root(), "");

	lerp_pose_with_animation_player();
	// anim_player->set_owner(get_node(this.set_owner()));
}

void RiggingSystemDock::save_start_position()
{
	print_line("print_all_node_paths!");

	Node* root = get_tree()->get_root();
	line_edit_select_state->set_text(std::to_string(current_rotations.size()).c_str());
	find_skeleton_and_print_path(root, "start");
	save_current_pose("start");
}

void RiggingSystemDock::save_final_position()
{
	print_line("print_all_node_paths!");

	Node* root = get_tree()->get_root();
	find_skeleton_and_print_path(root, "final");
}

void RiggingSystemDock::on_button_pressed()
{
	vwp = memnew(SubViewport);

	while (vwp->get_child_count() > 0)
	{
		vwp->remove_child(vwp->get_child(0));
	}

	if (!get_tree()->get_edited_scene_root())
		SceneTreeDock::get_singleton()->add_root_node(vwp);
	else if (get_tree()->get_edited_scene_root() != vwp)
		SceneTreeDock::get_singleton()->add_root_node(vwp);

	last_genned = Gen(obj);

	vwp->add_child(last_genned.meshinst);

	camera = memnew(Camera3D);

	auto env = memnew(Environment);

	env->set_background(Environment::BG_COLOR);
	env->set_bg_color(Color(0, 0, 0, 1));

	env->set_name("Genned_Environment");

	camera->set_environment(env);

	vwp->add_child(camera);
	camera->set_owner(vwp);
	camera->set_name("Camera");
	camera->set_position({ 0, 0, 2 });

	auto light = memnew(OmniLight3D);
	vwp->add_child(light);
	light->set_owner(vwp);
	light->set_name("Genned_OmniLight3D_1");
	light->set_position({ 2, 2, 0 });

	light = memnew(OmniLight3D);
	vwp->add_child(light);
	light->set_owner(vwp);
	light->set_name("Genned_OmniLight3D_2");
	light->set_position({ 2, -2, 0 });
	light = memnew(OmniLight3D);
	vwp->add_child(light);
	light->set_owner(vwp);
	light->set_name("Genned_OmniLight3D_3");
	light->set_position({ -2, 2, 0 });

	light = memnew(OmniLight3D);
	vwp->add_child(light);
	light->set_owner(vwp);
	light->set_name("Genned_OmniLight3D_4");
	light->set_position({ -2, -2, 0 });

	last_genned.meshinst->add_child(last_genned.skelet);
	last_genned.meshinst->set_owner(vwp);
	last_genned.skelet->set_owner(vwp);

	anim_player = memnew(AnimationPlayer);

	last_genned.meshinst->add_child(anim_player);
	anim_player->set_owner(vwp);
	anim_player->set_name("Generated_AnimPlayer");

	anim_lib = memnew(AnimationLibrary);

	handler = memnew(AddPlayerHandler);

	vwp->add_child(handler);
	handler->set_owner(vwp);
	handler->set_name("Handler");

	anim_player->add_animation_library("LerpLib", anim_lib);
	vwp->set_name("SubViewport");

	auto path = last_genned.skelet->get_path();
	print_line("skelet->get_path: " + path);
	path = last_genned.meshinst->get_path_to(last_genned.skelet);
	print_line("meshinst->get_path_to(skelet): " + path);

	last_genned.meshinst->set_skeleton_path(path);
	last_genned.meshinst->set_transform(Transform3D(Basis({0, 1, 0}, Math_PI * 0.5)));
	target_skeleton = last_genned.skelet;
}

void RiggingSystemDock::on_button_open_obj_pressed()
{
	Point2 pos = button_select_obj->get_position();
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, { 800, 800 });
	file_dialog_open_obj->popup(rect);
}

void RiggingSystemDock::on_button_open_skeleton_pressed()
{
	Point2 pos = button_select_skeleton->get_position();
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, { 800, 800 });
	file_dialog_open_skeleton->popup(rect);
}

void RiggingSystemDock::on_button_export_animation_pressed()
{
	Point2 pos = button_export_animation->get_position();
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, { 800, 800 });
	file_dialog_export_animation->popup(rect);
}

void RiggingSystemDock::on_button_select2dtexture_pressed()
{
	Point2 pos = button_export_animation->get_position();
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, { 800, 800 });
	file_dialog_select_texture->popup(rect);
}

void RiggingSystemDock::on_button_export_skeleton_pressed()
{
	ERR_FAIL_COND(!(last_genned.skelet));
	int s_s = last_genned.skelet->get_bone_count();

	std::string path = "__out.bone.json";
	auto l_path = line_edit_export_skeleton->get_text();
	if (!l_path.is_empty())
		path = l_path.ascii();

	s_object.from_skeleton3d(last_genned.skelet);

	std::ofstream fout(path);
	fout << s_object.to_json();
	fout.close();

	fout = std::ofstream("g_skeleton.h");
	fout << s_object.to_header();
	fout.close();
}

void RiggingSystemDock::on_button_generate2d_pressed()
{
	vwp = memnew(SubViewport);

	while (vwp->get_child_count() > 0)
	{
		vwp->remove_child(vwp->get_child(0));
	}

	if (!get_tree()->get_edited_scene_root())
		SceneTreeDock::get_singleton()->add_root_node(vwp);
	else if (get_tree()->get_edited_scene_root() != vwp)
		SceneTreeDock::get_singleton()->add_root_node(vwp);

	ArrayMesh* mesh = memnew(ArrayMesh);
	MeshInstance3D* meshinst = memnew(MeshInstance3D);
	Skeleton3D* skelet = memnew(Skeleton3D);

	std::vector<surface_t> surfacees;

	s_object2d.applay_to_skeleton3d(skelet);

	
	
	
	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	Size2 size(1, 1);
	material->set_name("test");
	if(line_edit_select2dtexture->get_text().size())
	{
		material->set_texture(BaseMaterial3D::TextureParam::TEXTURE_ALBEDO, ImageTexture::create_from_image(Image::load_from_file(line_edit_select2dtexture->get_text())));

		material->set_transparency(BaseMaterial3D::Transparency::TRANSPARENCY_ALPHA_SCISSOR);

		material->set_alpha_scissor_threshold(0.5);

		size = material->get_texture(BaseMaterial3D::TextureParam::TEXTURE_ALBEDO)->get_size();
	}

	print_line(size);

	float m = MAX(size.width, size.height);

	surfacees.push_back(make_surf(s_object2d, 50, size.height * -0.5 / m, size.width * -0.5 / m, size.height / m, size.width / m, skelet));
	surfacees[0].material = material;
	

	for (int __s = 0; __s < surfacees.size(); __s++)
	{
		auto& vertices = surfacees[__s].vertices;
		auto& normals = surfacees[__s].normals;
		auto& texcoord = surfacees[__s].texcoord;
		auto& indices = surfacees[__s].indices;

		Array arrays;
		arrays.resize(Mesh::ARRAY_MAX);


		arrays[Mesh::ARRAY_VERTEX] = vertices;
		if (normals.size() > 0)
		{
			arrays[Mesh::ARRAY_NORMAL] = normals;
		}
		if (texcoord.size() > 0)
		{
			arrays[Mesh::ARRAY_TEX_UV] = texcoord;
		}
		arrays[Mesh::ARRAY_INDEX] = indices;
		int s_id = mesh->get_surface_count();
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

		mesh->surface_set_material(s_id, surfacees[__s].material);
	}

	rig(mesh, skelet);

	meshinst->set_mesh(mesh);
	skelet->set_name("Generated_Skeleton3D");
	meshinst->set_name("Generated_MeshInstance3D");

	vwp->add_child(meshinst);

	camera = memnew(Camera3D);

	auto env = memnew(Environment);

	env->set_background(Environment::BG_COLOR);
	env->set_bg_color(Color(0, 0, 0, 1));

	env->set_name("Genned_Environment");

	camera->set_environment(env);

	vwp->add_child(camera);
	camera->set_owner(vwp);
	camera->set_name("Camera");
	camera->set_position({ 0, 0, 2 });

	auto light = memnew(OmniLight3D);
	vwp->add_child(light);
	light->set_owner(vwp);
	light->set_name("Genned_OmniLight3D_1");
	light->set_position({ 0, 2, 2 });

	light = memnew(OmniLight3D);
	vwp->add_child(light);
	light->set_owner(vwp);
	light->set_name("Genned_OmniLight3D_2");
	light->set_position({ 0, -2, 2 });
	light = memnew(OmniLight3D);
	vwp->add_child(light);
	light->set_owner(vwp);
	light->set_name("Genned_OmniLight3D_3");
	light->set_position({ 0, 2, -2 });

	light = memnew(OmniLight3D);
	vwp->add_child(light);
	light->set_owner(vwp);
	light->set_name("Genned_OmniLight3D_4");
	light->set_position({ 0, -2, -2 });

	meshinst->add_child(skelet);
	meshinst->set_owner(vwp);
	skelet->set_owner(vwp);

	anim_player = memnew(AnimationPlayer);

	meshinst->add_child(anim_player);
	anim_player->set_owner(vwp);
	anim_player->set_name("Generated_AnimPlayer");

	anim_lib = memnew(AnimationLibrary);

	handler = memnew(AddPlayerHandler);

	vwp->add_child(handler);
	handler->set_owner(vwp);
	handler->set_name("Handler");

	anim_player->add_animation_library("LerpLib", anim_lib);
	vwp->set_name("SubViewport");

	auto path = skelet->get_path();
	print_line("skelet->get_path: " + path);
	path = meshinst->get_path_to(skelet);
	print_line("meshinst->get_path_to(skelet): " + path);

	meshinst->set_skeleton_path(path);
	target_skeleton = skelet;

	last_genned.mesh = mesh;
	last_genned.skelet = skelet;
	last_genned.meshinst = meshinst;
}

void RiggingSystemDock::on_button_save_viewport_pressed()
{
	String path = "test_a.png";
	save_viewport(path);
}

void RiggingSystemDock::on_button_select_anim_folder_pressed()
{
	Point2 pos = button_export_animation->get_position();
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, { 800, 800 });
	file_dialog_select_anim_folder->popup(rect);
}

void RiggingSystemDock::on_button_open_skeleton2d_pressed()
{
	Point2 pos = button_select_skeleton->get_position();
	Point2 wpos = get_position() + get_window()->get_position() + Point2(20, 20);
	Rect2i rect = Rect2i(pos + wpos, { 800, 800 });
	file_dialog_open_skeleton2d->popup(rect);
}

void RiggingSystemDock::on_button_reset_state_pressed()
{
	current_rotations.clear();
	current_positions.clear();
	line_edit_select_state->set_text(std::to_string(current_rotations.size()).c_str());
}

void RiggingSystemDock::file_dialog_open_obj_file_selected(String path)
{
	line_edit_select_obj->set_text(path);
	obj_path = path.ascii().get_data();
	std::ifstream fin(obj_path);
	fin.seekg(0, std::ios::end);
	size_t size = fin.tellg();
	obj.resize(size, ' ');
	fin.seekg(0);
	fin.read(obj.data(), size);
}

void RiggingSystemDock::file_dialog_open_skeleton_file_selected(String path)
{
	line_edit_select_skeleton->set_text(path);
	if (path.ends_with(".json"))
	{
		std::ifstream fin(path.ascii().get_data());
		fin.seekg(0, std::ios::end);
		size_t size = fin.tellg();
		skeleton_json_str.resize(size, ' ');
		fin.seekg(0);
		fin.read(skeleton_json_str.data(), size);
		s_object.from_json(skeleton_json_str);
		
		Skeleton3D* t_s = memnew(Skeleton3D);

		s_object.applay_to_skeleton3d(t_s);

		auto trans = Transform3D(
			{ 0.00,  0.00, -1.00 },
			{ -1.00,  0.00, 0.00 },
			{ 0.00,  1.00, 0.00 },
			{ 0.00,  0.00, 0.00 }
		);

		t_s->set_bone_rest(0, trans);

		s_object.from_skeleton3d(t_s);

		memdelete(t_s);
	}
	else if (path.ends_with(".glb"))
	{

		HashMap<StringName, Variant> p_options;
		String p_path = "E:\\Repos\\WGdotTest\\untitled.glb";

		Ref<GLTFDocument> gltf;
		gltf.instantiate();
		Ref<GLTFState> state;
		state.instantiate();
		Error err = gltf->append_from_file(p_path, state);
		if (err != OK)
		{
			return;
		}

		bool trimming = false;
		bool remove_immutable = true;
		auto scene = gltf->generate_scene(state, 30, trimming, remove_immutable);

		auto new_skeleton = find_node_by_type<Skeleton3D>(scene);

		auto b_p = new_skeleton->get_bone_pose(0);

		auto trans = Transform3D(
			{ 1.00,  0.00, 0.00 },
			{ 0.00,  1.00, 0.00 },
			{ 0.00,  0.00, 1.00 },
			{ 0.00,  0.00, 0.00 }
		);

		new_skeleton->set_bone_rest(0, trans);

		new_skeleton->reset_bone_poses();

		s_object.from_skeleton3d(new_skeleton);

		std::function<void(Node*)> free = [&free](Node* cur)
			{
				int c = cur->get_child_count();
				std::vector<Node*> childs;
				for (int i = 0; i < c; ++i)
					childs.push_back(cur->get_child(i));

				for (int i = 0; i < c; ++i)
				{
					cur->remove_child(childs[i]);
					free(childs[i]);
				}
				memdelete(cur);
			};

		free(scene);
	}
}

void RiggingSystemDock::file_dialog_export_animation_file_selected(String path)
{
	line_edit_export_animation->set_text(path);
	path_to_export = path.ascii().get_data();
}

void RiggingSystemDock::file_dialog_select_texture_file_selected(String path)
{
	line_edit_select2dtexture->set_text(path);
}

void RiggingSystemDock::file_dialog_select_anim_folder_file_selected(String path)
{
	line_edit_select_anim_folder->set_text(path);

	auto dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (dir->change_dir(path) == OK)
	{
		auto files = dir->get_files();

		for (int i = 0; i < files.size(); i++) if (files[i].ends_with(".glb"))
		{
			print_line(files[i]);
		}
	}
}

void RiggingSystemDock::file_dialog_open_skeleton2d_file_selected(String path)
{
	line_edit_select_skeleton->set_text(path);
	if (path.ends_with(".json"))
	{
		std::ifstream fin(path.ascii().get_data());
		fin.seekg(0, std::ios::end);
		size_t size = fin.tellg();
		skeleton2d_json_str.resize(size, ' ');
		fin.seekg(0);
		fin.read(skeleton_json_str.data(), size);
		s_object2d.from_json(skeleton_json_str);

		Skeleton3D* t_s = memnew(Skeleton3D);

		s_object.applay_to_skeleton3d(t_s);

		// auto trans = Transform3D(
		// 	{ 0.00,  0.00, -1.00 },
		// 	{ -1.00,  0.00, 0.00 },
		// 	{ 0.00,  1.00, 0.00 },
		// 	{ 0.00,  0.00, 0.00 }
		// );

		auto trans = Transform3D(
			{ 1.00,  0.00, 0.00 },
			{ 0.00,  1.00, 0.00 },
			{ 0.00,  0.00, 1.00 },
			{ 0.00,  0.00, 0.00 }
		);

		t_s->set_bone_rest(0, trans);

		s_object.from_skeleton3d(t_s);

		memdelete(t_s);
	}
	else if (path.ends_with(".glb"))
	{

		HashMap<StringName, Variant> p_options;
		String p_path = "E:\\Repos\\WGdotTest\\untitled.glb";

		Ref<GLTFDocument> gltf;
		gltf.instantiate();
		Ref<GLTFState> state;
		state.instantiate();
		Error err = gltf->append_from_file(p_path, state);
		if (err != OK)
		{
			return;
		}

		bool trimming = false;
		bool remove_immutable = true;
		auto scene = gltf->generate_scene(state, 30, trimming, remove_immutable);

		auto new_skeleton = find_node_by_type<Skeleton3D>(scene);

		auto b_p = new_skeleton->get_bone_pose(0);

		auto trans = Transform3D(
			{ 1.00,  0.00, 0.00 },
			{ 0.00,  1.00, 0.00 },
			{ 0.00,  0.00, 1.00 },
			{ 0.00,  0.00, 0.00 }
		);

		new_skeleton->set_bone_rest(0, trans);

		new_skeleton->reset_bone_poses();

		s_object2d.from_skeleton3d(new_skeleton);

		std::function<void(Node*)> free = [&free](Node* cur)
			{
				int c = cur->get_child_count();
				std::vector<Node*> childs;
				for (int i = 0; i < c; ++i)
					childs.push_back(cur->get_child(i));

				for (int i = 0; i < c; ++i)
				{
					cur->remove_child(childs[i]);
					free(childs[i]);
				}
				memdelete(cur);
			};

		free(scene);
	}
}

RiggingSystemDock::RiggingSystemDock()
{
	for (int i = 0; i < g_skeleton.size(); ++i)
		s_object.add_bone(g_skeleton[i]);

	for (int i = 0; i < g_skeleton2d.size(); ++i)
		s_object2d.add_bone(g_skeleton2d[i]);

	{
		Skeleton3D* t_s = memnew(Skeleton3D);

		s_object.applay_to_skeleton3d(t_s);

		auto trans = Transform3D(
			{ 0.00,  0.00, -1.00 },
			{ -1.00,  0.00, 0.00 },
			{ 0.00,  1.00, 0.00 },
			{ 0.00,  0.00, 0.00 }
		);

		t_s->set_bone_rest(0, trans);

		s_object.from_skeleton3d(t_s);
		
		s_object2d.applay_to_skeleton3d(t_s);

		t_s->set_bone_rest(0, trans);

		s_object2d.from_skeleton3d(t_s);

		memdelete(t_s);
	}

	singleton = this;
	set_name("RiggingSystem");
	set_size({ 500,500 });


	AddPlayerHandler::set_is_editor(true);

	// Include autogenned code part
#include "layouts/rigging_dock_cpp.h"


	file_dialog_open_obj = memnew(FileDialog);
	file_dialog_open_skeleton = memnew(FileDialog);
	file_dialog_export_animation = memnew(FileDialog);
	file_dialog_select_texture = memnew(FileDialog);
	file_dialog_select_anim_folder = memnew(FileDialog);
	file_dialog_open_skeleton2d = memnew(FileDialog);

	//================== file_dialog_open_obj ==================
	file_dialog_open_obj->set_name("FileDialogOpenObj");
	this->add_child(file_dialog_open_obj);
	file_dialog_open_obj->set_title("Open a object(*.obj) File");
	file_dialog_open_obj->set_size(Vector2i(327, 300));
	file_dialog_open_obj->set_ok_button_text("Open");
	file_dialog_open_obj->set_file_mode(FileDialog::FILE_MODE_OPEN_FILE);
	file_dialog_open_obj->set_access(FileDialog::Access::ACCESS_FILESYSTEM);
	file_dialog_open_obj->set_filters({ "*.obj" });
	//================== file_dialog_open_skeleton ==================
	file_dialog_open_skeleton->set_name("FileDialogOpenSkeleton");
	this->add_child(file_dialog_open_skeleton);
	file_dialog_open_skeleton->set_title("Open a skelet(*.json, *.glb) File");
	file_dialog_open_skeleton->set_size(Vector2i(327, 300));
	file_dialog_open_skeleton->set_ok_button_text("Open");
	file_dialog_open_skeleton->set_file_mode(FileDialog::FILE_MODE_OPEN_FILE);
	file_dialog_open_skeleton->set_access(FileDialog::Access::ACCESS_FILESYSTEM);
	file_dialog_open_skeleton->set_filters({ "*.json", "*.glb" });
	//================== file_dialog_export_animation ==================
	file_dialog_export_animation->set_name("FileDialogExportAnimation");
	this->add_child(file_dialog_export_animation);
	file_dialog_export_animation->set_title("Select folder for export animation");
	file_dialog_export_animation->set_size(Vector2i(327, 300));
	file_dialog_export_animation->set_ok_button_text("Select");
	file_dialog_export_animation->set_file_mode(FileDialog::FILE_MODE_OPEN_DIR);
	file_dialog_export_animation->set_access(FileDialog::Access::ACCESS_FILESYSTEM);
	//================== file_dialog_select_texture ==================
	file_dialog_select_texture->set_name("FileDialogSelectTexture");
	this->add_child(file_dialog_select_texture);
	file_dialog_select_texture->set_title("Open a texture(*.png) File");
	file_dialog_select_texture->set_size(Vector2i(327, 300));
	file_dialog_select_texture->set_ok_button_text("Select");
	file_dialog_select_texture->set_file_mode(FileDialog::FILE_MODE_OPEN_FILE);
	file_dialog_select_texture->set_access(FileDialog::Access::ACCESS_FILESYSTEM);
	file_dialog_select_texture->set_filters({ "*.png" });
	//================== file_dialog_select_anim_folder ==================
	file_dialog_select_anim_folder->set_name("FileDialogExportAnimation");
	this->add_child(file_dialog_select_anim_folder);
	file_dialog_select_anim_folder->set_title("Select folder for import animation");
	file_dialog_select_anim_folder->set_size(Vector2i(327, 300));
	file_dialog_select_anim_folder->set_ok_button_text("Select");
	file_dialog_select_anim_folder->set_file_mode(FileDialog::FILE_MODE_OPEN_DIR);
	file_dialog_select_anim_folder->set_access(FileDialog::Access::ACCESS_FILESYSTEM);
	//================== file_dialog_open_skeleton2d ==================
	file_dialog_open_skeleton2d->set_name("FileDialogOpenSkeleton");
	this->add_child(file_dialog_open_skeleton2d);
	file_dialog_open_skeleton2d->set_title("Open a skelet(*.json, *.glb) File");
	file_dialog_open_skeleton2d->set_size(Vector2i(327, 300));
	file_dialog_open_skeleton2d->set_ok_button_text("Open");
	file_dialog_open_skeleton2d->set_file_mode(FileDialog::FILE_MODE_OPEN_FILE);
	file_dialog_open_skeleton2d->set_access(FileDialog::Access::ACCESS_FILESYSTEM);
	file_dialog_open_skeleton2d->set_filters({ "*.json", "*.glb" });
	

	button_generate->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_pressed));
	button_select_obj->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_open_obj_pressed));
	button_select_skeleton->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_open_skeleton_pressed));

	button_select_state->connect("pressed", callable_mp(this, &RiggingSystemDock::save_start_position));
	//button_select_final_state->connect("pressed", callable_mp(this, &RiggingSystemDock::save_final_position));
	button_generate_animation->connect("pressed", callable_mp(this, &RiggingSystemDock::ensure_animation_player));
	// button_rig->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_rig_pressed));
	// button_unrig->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_unrig_pressed));
	button_export_skeleton->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_export_skeleton_pressed));
	button_export_animation->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_export_animation_pressed));
	button_select2dtexture->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_select2dtexture_pressed));

	button_generate2d->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_generate2d_pressed));
	button_save_viewport->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_save_viewport_pressed));
	button_select_anim_folder->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_select_anim_folder_pressed));
	button_select_skeleton2d->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_open_skeleton2d_pressed));
	button_reset_state->connect("pressed", callable_mp(this, &RiggingSystemDock::on_button_reset_state_pressed));

	file_dialog_open_obj->connect("file_selected", callable_mp(this, &RiggingSystemDock::file_dialog_open_obj_file_selected));
	file_dialog_open_skeleton->connect("file_selected", callable_mp(this, &RiggingSystemDock::file_dialog_open_skeleton_file_selected));
	file_dialog_export_animation->connect("dir_selected", callable_mp(this, &RiggingSystemDock::file_dialog_export_animation_file_selected));
	file_dialog_select_anim_folder->connect("dir_selected", callable_mp(this, &RiggingSystemDock::file_dialog_select_anim_folder_file_selected));
	file_dialog_select_texture->connect("file_selected", callable_mp(this, &RiggingSystemDock::file_dialog_select_texture_file_selected));
	file_dialog_open_skeleton2d->connect("file_selected", callable_mp(this, &RiggingSystemDock::file_dialog_open_skeleton2d_file_selected));
}

RiggingSystemDock::~RiggingSystemDock()
{
	singleton = nullptr;
}

// ----Ending works with json bones