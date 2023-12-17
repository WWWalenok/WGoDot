#include "skeleton_object.h"
#include "core/io/json.h"
#include <sstream>
#include <algorithm>

SkeletonObject::Bone empty_bone;

// ----Begin of bone json class

SkeletonObject::Bone& SkeletonObject::operator[](int i)
{
	int s = bones.size();
	i = (i < 0) ? (i % s + s) : (i % s);
	return bones[i];
}

SkeletonObject::Bone& SkeletonObject::operator[](std::string name)
{
	for(auto& el : bones)
		if(el.name == name)
			return el;

	return empty_bone;
}

std::string SkeletonObject::to_json() const
{
	static auto format = 
R"(  {
    "name": "%s",
    "parent": %i,
    "transform_mat": 
    [
      [%f, %f, %f, %f],
      [%f, %f, %f, %f],
      [%f, %f, %f, %f],
      [%f, %f, %f, %f]
    ]

  })";

	std::stringstream fout;
	fout << "[\n";
	int s_s = bones.size();
	for (int j = 0; j < s_s; j++)
	{
		const auto& b = bones[j].mat;

		auto size = snprintf(0, 0, format,
			bones[j].name.data(),
			bones[j].parent,
			b[0][0], b[0][1], b[0][2], b[0][3],
			b[1][0], b[1][1], b[1][2], b[1][3],
			b[2][0], b[2][1], b[2][2], b[2][3],
			b[3][0], b[3][1], b[3][2], b[3][3]
		);

		std::string buff(size + 1, ' ');

		snprintf(buff.data(), buff.size(), format,
			bones[j].name.data(),
			bones[j].parent,
			b[0][0], b[0][1], b[0][2], b[0][3],
			b[1][0], b[1][1], b[1][2], b[1][3],
			b[2][0], b[2][1], b[2][2], b[2][3],
			b[3][0], b[3][1], b[3][2], b[3][3]
		);

		fout << buff.c_str();
		if (j < s_s - 1)
			fout << ",\n";
		else
			fout << "\n";
	}

	fout << "]\n";

	return fout.str();
}

std::string SkeletonObject::to_header() const
{
	static auto format = 
R"(  {
    {
      {%f, %f, %f, %f},
      {%f, %f, %f, %f},
      {%f, %f, %f, %f},
      {%f, %f, %f, %f}
    },
	"%s",
    %i

  })";

	std::stringstream fout;
		fout <<
		R"(#ifndef __G_SKELETON_H__
#define __G_SKELETON_H__

#include "rigging_dock.h"

const std::vector<SkeletonObject::Bone> g_skeleton = {
)";
	int s_s = bones.size();
	for (int j = 0; j < s_s; j++)
	{
		const auto& b = bones[j].mat;

		auto size = snprintf(0, 0, format,
			b[0][0], b[0][1], b[0][2], b[0][3],
			b[1][0], b[1][1], b[1][2], b[1][3],
			b[2][0], b[2][1], b[2][2], b[2][3],
			b[3][0], b[3][1], b[3][2], b[3][3],
			bones[j].name.data(),
			bones[j].parent

		);

		std::string buff(size + 1, ' ');

		snprintf(buff.data(), buff.size(), format,
			b[0][0], b[0][1], b[0][2], b[0][3],
			b[1][0], b[1][1], b[1][2], b[1][3],
			b[2][0], b[2][1], b[2][2], b[2][3],
			b[3][0], b[3][1], b[3][2], b[3][3],
			bones[j].name.data(),
			bones[j].parent
		);

		fout << buff.c_str();
		if (j < s_s - 1)
			fout << ",\n";
		else
			fout << "\n";
	}

	fout <<
		R"(};

#endif // __G_SKELETON_H__)";

	return fout.str();
}

bool SkeletonObject::from_json(std::string json_str)
{
	bones.clear();
	print_line(__FUNCTION__);

	auto __clear = [this](){
		bones.clear();
		return false;
	};

	JSON json;
	json.parse(json_str.c_str());
	ERR_FAIL_COND_V(!json.get_data().is_array(), __clear());

	const Array array = json.get_data();

	for (int i = 0; i < array.size(); i++)
	{
		ERR_FAIL_COND_V(array[i].get_type() != Variant::Type::DICTIONARY, __clear());
		Dictionary dict = array[i];
		ERR_FAIL_COND_V(!dict.has("name"), false);
		ERR_FAIL_COND_V(!dict.has("parent"), false);
		ERR_FAIL_COND_V(!dict.has("transform_mat"), false);

		ERR_FAIL_COND_V(dict["name"].get_type() != Variant::Type::STRING, __clear());
		ERR_FAIL_COND_V(
			dict["parent"].get_type() != Variant::Type::FLOAT &&
			dict["parent"].get_type() != Variant::Type::INT,
			false);
		ERR_FAIL_COND_V(!dict["transform_mat"].is_array(), __clear());

		Bone bone;

		bone.name = (const char*)(String(dict["name"]).ascii());
		if (dict["parent"].get_type() == Variant::Type::FLOAT)
			bone.parent = (float)dict["parent"];
		else
			if (dict["parent"].get_type() == Variant::Type::FLOAT)
				bone.parent = dict["parent"];
		Array transform_mat = dict["transform_mat"];
		for (int a = 0; a < 4; a++)
		{
			ERR_FAIL_COND_V(!transform_mat[a].is_array(), __clear());
			Array transform_mat_line = transform_mat[a];

			for (int b = 0; b < 4; b++)
				bone.mat[a][b] = transform_mat_line[b];
		}

		bones.push_back(bone);
	}
	return true;
}

void SkeletonObject::from_skeleton3d(Skeleton3D* skelet)
{
	bones.clear();
	auto s_s = skelet->get_bone_count();
	;
	for (int j = 0; j < s_s; j++)
	{
		Transform3D trans = skelet->get_bone_pose(j);
		Basis& b = trans.basis;
		Vector3& or = trans.origin;
		float mat[4][4]
		{
			{b[0][0], b[0][1], b[0][2], 0.0},
			{b[1][0], b[1][1], b[1][2], 0.0},
			{b[2][0], b[2][1], b[2][2], 0.0},
			{or  [0], or  [1], or  [2], 1.0}
		};
		add_bone((const char*)(skelet->get_bone_name(j).ascii()), skelet->get_bone_parent(j), mat);
	}
}

void SkeletonObject::from_skeleton2d(Skeleton2D* skelet)
{
	bones.clear();
	auto s_s = skelet->get_bone_count();
	for (int j = 0; j < s_s; j++)
	{
		Transform2D t = skelet->get_bone(j)->get_rest();
		float mat[4][4]
		{
			{t[0][0], t[0][1], 0.0, 0.0},
			{t[1][0], t[1][1], 0.0, 0.0},
			{0.0, 0.0, 1.0, 0.0},
			{t[2][0], t[2][1], 0.0, 1.0}
		};
		// add_bone(
		// 	(const char*)(skelet->get_bone(j)->get_name().operator String().ascii()), 
		// 	skelet->get_bone(j)->, mat);
	}
}

std::vector<std::pair<int, float>> SkeletonObject::get_best_points(const Vector3& v, Skeleton3D* skelet) const
{
	std::vector<std::pair<int, float>> dists;
	std::vector<std::pair<int, float>> out;
	if(bones.size() == 0)
		return out;
	for(int i = 0; i < bones.size(); i++)
	{
		const auto& b = skelet->get_bone_global_rest(i).origin;
		float r = (b - v).length_squared();
		
		if(r > bones[i].end_er * bones[i].end_er || i == 0)
			dists.push_back({i, 0});
		else
			dists.push_back({i, expf(-r)});

	}
	
	std::sort(dists.begin(), dists.end(), [](std::pair<int, float> a, std::pair<int, float> b){
		return a.second > b.second;
	});
	
	int mid = dists[0].first;

	out.push_back(dists[0]);
	bool forse = expf(-bones[mid].max_er * bones[mid].max_er) > dists[0].second;
	for(int i = 1; i < dists.size(); i++)
	{
		int id = dists[i].first;
		if(bones[id].parent == mid || bones[mid].parent == id || forse)
			out.push_back(dists[i]);

		if(out.size() == 4)
			break;
	}
	while(out.size() < 4)
	{
		out.push_back({0, 0});
	}

	return out;
}

void SkeletonObject::add_bone(std::string name, std::string s_parent, float mat[4][4])
{
	for(int i = 0; i < size(); ++i)
		if(bones[i].name == name)
			return;
	
	int parent = -1;
	for(int i = 0; i < size(); ++i)
	{
		if(bones[i].name == s_parent)
		{
			parent = i;
			break;
		}
	}

	Bone bone;
	memcpy(bone.mat, mat, sizeof(float) * 16);
	bone.name = name;
	bone.parent = parent;

	bones.push_back(bone);
}

void SkeletonObject::add_bone(std::string name, int parent, float mat[4][4])
{
	for(int i = 0; i < size(); ++i)
		if(bones[i].name == name)
			return;

	if(parent >= size())
		parent = -1;

	Bone bone;
	memcpy(bone.mat, mat, sizeof(float) * 16);
	bone.name = name;
	bone.parent = parent;

	bones.push_back(bone);
}

void SkeletonObject::add_bone(SkeletonObject::Bone bone)
{
	for(int i = 0; i < size(); ++i)
		if(bones[i].name == bone.name)
			return;

	if(bone.parent >= size())
		bone.parent = -1;

	bones.push_back(bone);
}

void SkeletonObject::applay_to_skeleton3d(Skeleton3D* skelet) const
{
	skelet->clear_bones();
	for (int i = 0; i < size(); i++)
	{
		int id = skelet->get_bone_count();
		skelet->add_bone(bones[i].name.c_str());
		if (bones[i].parent >= 0)
			skelet->set_bone_parent(id, bones[i].parent);
		auto& mat = bones[i].mat;
		Transform3D trans = Transform3D(
			mat[0][0], mat[0][1], mat[0][2],
			mat[1][0], mat[1][1], mat[1][2],
			mat[2][0], mat[2][1], mat[2][2],
			mat[3][0], mat[3][1], mat[3][2]
		);

		skelet->set_bone_rest(id, trans);
		skelet->reset_bone_pose(id);
	}
}

void SkeletonObject::applay_to_skeleton2d(Skeleton2D* skelet) const
{
	
}

// ----End of bone json class