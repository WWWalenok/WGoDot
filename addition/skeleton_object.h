#ifndef __SCELETON_OBJECT_H__
#define __SCELETON_OBJECT_H__

#include <string>
#include <vector>
#include "scene/3d/skeleton_3d.h"
#include "scene/2d/skeleton_2d.h"
#include "initializer_list"

struct SkeletonObject
{
	struct Bone
	{
		float mat[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
		std::string name = "";
		int parent = -1;
		float max_er = 0.2;
		float end_er = 0.3;
	};

	std::vector<Bone> bones;

	Bone& operator[](int i);
	Bone& operator[](std::string name);

	std::string to_json() const;
	std::string to_header() const;
	bool from_json(std::string json);

	std::vector<std::pair<int, float>> get_best_points(const Vector3& pos, Skeleton3D* skelet) const;

	size_t size() const { return bones.size(); }

	void add_bone(std::string name, std::string parent, float mat[4][4]);

	void add_bone(std::string name, int parent, float mat[4][4]);

	void add_bone(Bone bone);

	void applay_to_skeleton3d(Skeleton3D* skelet) const;

	void applay_to_skeleton2d(Skeleton2D* skelet) const;

	void from_skeleton3d(Skeleton3D* skelet);

	void from_skeleton2d(Skeleton2D* skelet);
};





#endif // __SCELETON_OBJECT_H__