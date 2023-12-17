#ifndef __OBJ_PARSER_H__
#define __OBJ_PARSER_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include<filesystem>
namespace fs = std::filesystem;

template<typename T, uint16_t S>
struct MVector
{
	T _data[S];

	MVector<T, S>()
	{
		for (int i = 0; i < S; i++)
			_data[i] = 0;
	}

	MVector<T, S>(const T* data)
	{
		for (int i = 0; i < S; i++)
			_data[i] = data[i];
	}

	MVector<T, S>(std::initializer_list<T> data)
	{
		int i = 0;
		for (const auto& e : data)
		{
			if (i >= S)
				break;
			_data[i] = e;
			++i;
		}
		for (; i < S; i++)
			_data[i] = 0;
	}

	MVector<T, S> operator-() const
	{
		T data[S];
		for (int i = 0; i < S; i++)
			data[i] = _data[i] * -1;
		return MVector<T, S>(data);
	}

	MVector<T, S> operator+() const
	{
		T data[S];
		for (int i = 0; i < S; i++)
			data[i] = _data[i];
		return MVector<T, S>(data);
	}

	MVector<T, S> operator+(const MVector<T, S>& other) const
	{
		T data[S];
		for (int i = 0; i < S; i++)
			data[i] = _data[i] + other._data[i];
		return MVector<T, S>(data);
	}

	MVector<T, S> operator-(const MVector<T, S>& other) const
	{
		T data[S];
		for (int i = 0; i < S; i++)
			data[i] = _data[i] - other._data[i];
		return MVector<T, S>(data);
	}

	MVector<T, S> operator*(const T& other) const
	{
		T data[S];
		for (int i = 0; i < S; i++)
			data[i] = _data[i] * other;
		return MVector<T, S>(data);
	}

	MVector<T, S> operator/(const T& other) const
	{
		T data[S];
		for (int i = 0; i < S; i++)
			data[i] = _data[i] / other;
		return MVector<T, S>(data);
	}

	T operator!() const
	{
		T ret;
		for (int i = 0; i < S; i++)
			ret = _data[i] * _data[i];
		return ret;
	}

	T& operator[](int i)
	{
		return _data[i % S];
	}

	const T& operator[](int i) const
	{
		return _data[i % S];
	}

	operator std::string() const
	{
		std::string ret = "(";
		for (int i = 0; i < S; i++)
		{
			ret += std::to_string(_data[i]);
			if (i != S - 1)
				ret += ", ";
		}
		ret += ")";
		return ret;
	}
};


#define MVector3f MVector<float, 3>
#define MVector2f MVector<float, 2>
#define MVector4i MVector<int, 4>

MVector3f operator/(const MVector3f& a, const MVector3f& b)
{
	return MVector3f({
		a[1] * b[2] - a[2] * b[1],
		a[2] * b[0] - a[0] * b[2],
		a[0] * b[1] - a[1] * b[0]
		});
}

struct obj_parser
{
	std::vector<MVector3f> v;
	std::vector<MVector3f> vt;
	std::vector<MVector3f> vn;
	std::vector<MVector4i> idx;

	struct Vertex
	{
		MVector3f v;
		MVector3f vt;
		MVector3f vn;
	};


	enum EMode : uint8_t
	{
		EMode_ERROR = 0,
		EMode_V = 0b1,
		EMode_VT = 0b10,
		EMode_VN = 0b100
	};

	struct Material
	{
		Material()
		{
			name;
			Ns = 1000.0f;
			Ni = 0.0f;
			d = 1.0f;
			Tr = 0.0f;
			illum = 0;
		}

		// Material Name
		std::string name;
		// Ambient Color
		MVector3f Ka;
		// Diffuse Color
		MVector3f Kd;
		// Specular Color
		MVector3f Ks;
		//
		MVector3f Tf;
		// Specular Exponent
		float Ns;
		// Optical Density
		float Ni;
		// 
		float Tr;
		// Dissolve
		float d;
		// Illumination
		int illum;
		// Ambient Texture Map
		std::string map_Ka = "";
		// Diffuse Texture Map
		std::string map_Kd = "";
		// Specular Texture Map
		std::string map_Ks = "";
		// Specular Hightlight Map
		std::string map_Ns = "";
		// Alpha Texture Map
		std::string map_d = "";
		// Bump Map
		std::string map_bump = "";

		std::string to_string()
		{
			char buff[40960];
			for (int i = 0; i < 40960; i++)
				buff[i] = '\0';

			snprintf(buff, 40960, R"(newmtl %s
    Ns %f
    Ni %f
    d %f
    Tr %f
    Tf %f %f %f 
    illum %i
    Ka %f %f %f
    Kd %f %f %f
    Ks %f %f %f
    Ke %f %f %f
    map_Ka %s
    map_Kd %s
    map_Ks %s
    map_Ns %s
    map_d %s
    map_bump %s
)",
name.c_str(),
Ns,
Ni,
d,
Tr,
Tf[0], Tf[1], Tf[2],
illum,
Ka[0], Ka[1], Ka[2],
Kd[0], Kd[1], Kd[2],
Ks[0], Ks[1], Ks[2],
0.0, 0.0, 0.0,
map_Ka.c_str(),
map_Kd.c_str(),
map_Ks.c_str(),
map_Ns.c_str(),
map_d.c_str(),
map_bump.c_str());

			return std::string(buff);
		}
	};

	struct Mesh
	{
		std::vector<Vertex> v;
		std::vector<uint32_t> f;

		std::string name;

		Material MeshMaterial;

		std::string MeshMaterialName;


		uint8_t mode = EMode_ERROR;
	};

	inline bool isFirstToken(const std::string& str, const std::string& tocken, bool debug_print = false)
	{
		if (tocken.empty())
			return true;
		if (str.empty())
			return false;

		size_t token_start = str.find_first_not_of(" \t\n\r");
		if (token_start == std::string::npos)
			return false;
		size_t token_end = str.find_first_of(" \t", token_start);
		auto nstr = str.substr(token_start, token_end - token_start);
		return tocken == nstr;

		return true;
	}

	inline std::string split_from_line(std::string& str)
	{
		auto nline = str.find('\n');
		if (nline == std::string::npos)
			return "";
		std::string line = str.substr(0, nline);
		str = str.substr(nline + 1);
		return line + " ";
	}

	inline std::string tail(const std::string& in)
	{
		size_t token_start = in.find_first_not_of(" \t");
		size_t space_start = in.find_first_of(" \t", token_start);
		size_t tail_start = in.find_first_not_of(" \t", space_start);
		size_t tail_end = in.find_last_not_of(" \t");
		if (tail_start != std::string::npos && tail_end != std::string::npos)
		{
			return in.substr(tail_start, tail_end - tail_start + 1);
		}
		else if (tail_start != std::string::npos)
		{
			return in.substr(tail_start);
		}
		return "";
	}

	inline void split(const std::string& in,
		std::vector<std::string>& out,
		std::string token)
	{
		out.clear();

		std::string temp;

		for (int i = 0; i < int(in.size()); i++)
		{
			std::string test = in.substr(i, token.size());

			if (test == token)
			{
				if (!temp.empty())
				{
					out.push_back(temp);
					temp.clear();
					i += (int)token.size() - 1;
				}
				else
				{
					out.push_back("");
				}
			}
			else if (i + token.size() >= in.size())
			{
				temp += in.substr(i, token.size());
				out.push_back(temp);
				break;
			}
			else
			{
				temp += in[i];
			}
		}
	}

	inline MVector3f toMVector3f(const std::string& str)
	{
		MVector3f ret({ 0, 0, 0 });
		auto data = tail(str);

		if (data.empty())
			return ret;

		size_t start = 0;
		size_t end = 0;

		try
		{
			start = data.find_first_of(" \t");
			ret[0] = std::stof(data.substr(0, start));
			start = data.find_first_not_of(" \t", start);
			data = data.substr(start);
			if (data.empty())
				return ret;

			start = data.find_first_of(" \t");
			ret[1] = std::stof(data.substr(0, start));
			start = data.find_first_not_of(" \t", start);
			data = data.substr(start);

			if (data.empty())
				return ret;

			start = data.find_first_of(" \t");
			ret[2] = std::stof(data.substr(0, start));
		}
		catch (const std::exception&)
		{
			//_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Method/function failed.", (str + " : " + (std::string)ret + " : " + e.what()).c_str());
		}
		return ret;
	}

	inline MVector4i toIndex(std::string& str)
	{
		MVector4i ret({ -1, -1, -1, -1 });
		size_t token_start = str.find_first_not_of(" \t");
		if (token_start == std::string::npos)
			return ret;
		size_t token_end = str.find_first_of(" \t", token_start);

		std::string data = str.substr(token_start, token_end);
		size_t split = 0;

		for (int i = 0; i < 3; i++)
		{
			split = data.find('/');
			ret[i] = std::stoi(data.substr(0, split)) - 1;
			if (split == std::string::npos)
				return ret;

			data = data.substr(split + 1);
		}

		return ret;
	}

	inline std::vector<MVector4i> toIndexes(const std::string& str)
	{
		std::vector<MVector4i> ret;
		auto data = tail(str);

		MVector4i first;
		MVector4i second;
		MVector4i current;

		int i = 0;

		while (!data.empty())
		{
			size_t data_start = data.find_first_not_of(" \t");
			size_t space_start = data.find_first_of(" \t", data_start);
			size_t new_data_start = data.find_first_not_of(" \t", space_start);
			current = toIndex(data.substr(data_start, space_start));
			current[3] = i;
			if (i == 0)
				first = current;
			else if (i == 1)
				second = current;
			else
			{
				ret.push_back(first);
				ret.push_back(second);
				ret.push_back(current);
				second = current;
			}
			if (new_data_start >= data.size())
				break;
			data = data.substr(new_data_start);
			i++;
		}

		ret[0][3] = -1;

		return ret;
	}

	std::vector<Material> LoadMaterials(std::string path)
	{
		printf("%s", path.c_str());
		std::vector<Material> LoadedMaterials;
		try
		{
			// If the file is not a material file return false
			if (path.substr(path.size() - 4, path.size()) != ".mtl")
				return LoadedMaterials;

			std::ifstream file(path);

			// If the file is not found return false
			if (!file.is_open())
				return LoadedMaterials;

			Material tempMaterial;

			bool listening = false;

			// Go through each line looking for material variables
			std::string curline;
			while (std::getline(file, curline))
			{
				// new material and material name
				if (isFirstToken(curline, "newmtl"))
				{
					if (!listening)
					{
						listening = true;

						if (curline.size() > 7)
						{
							tempMaterial.name = tail(curline);
						}
						else
						{
							tempMaterial.name = "none";
						}
					}
					else
					{
						// Generate the material

						// Push Back loaded Material
						LoadedMaterials.push_back(tempMaterial);

						// Clear Loaded Material
						tempMaterial = Material();

						if (curline.size() > 7)
						{
							tempMaterial.name = tail(curline);
						}
						else
						{
							tempMaterial.name = "none";
						}
					}
				}
				// Ambient Color
				if (isFirstToken(curline, "Ka"))
				{
					std::vector<std::string> temp;
					split(tail(curline), temp, " ");

					if (temp.size() != 3)
						continue;

					tempMaterial.Ka[0] = std::stof(temp[0]);
					tempMaterial.Ka[1] = std::stof(temp[1]);
					tempMaterial.Ka[2] = std::stof(temp[2]);
				}
				// Diffuse Color
				if (isFirstToken(curline, "Kd"))
				{
					std::vector<std::string> temp;
					split(tail(curline), temp, " ");

					if (temp.size() != 3)
						continue;

					tempMaterial.Kd[0] = std::stof(temp[0]);
					tempMaterial.Kd[1] = std::stof(temp[1]);
					tempMaterial.Kd[2] = std::stof(temp[2]);
				}
				// Specular Color
				if (isFirstToken(curline, "Ks"))
				{
					std::vector<std::string> temp;
					split(tail(curline), temp, " ");

					if (temp.size() != 3)
						continue;

					tempMaterial.Ks[0] = std::stof(temp[0]);
					tempMaterial.Ks[1] = std::stof(temp[1]);
					tempMaterial.Ks[2] = std::stof(temp[2]);
				}
				//
				if (isFirstToken(curline, "Tf"))
				{
					std::vector<std::string> temp;
					split(tail(curline), temp, " ");

					if (temp.size() != 3)
						continue;

					tempMaterial.Tf[0] = std::stof(temp[0]);
					tempMaterial.Tf[1] = std::stof(temp[1]);
					tempMaterial.Tf[2] = std::stof(temp[2]);
				}
				// Specular Exponent
				if (isFirstToken(curline, "Ns"))
				{
					tempMaterial.Ns = std::stof(tail(curline));
				}
				//
				if (isFirstToken(curline, "Tr"))
				{
					tempMaterial.Tr = std::stof(tail(curline));
				}
				// Optical Density
				if (isFirstToken(curline, "Ni"))
				{
					tempMaterial.Ni = std::stof(tail(curline));
				}
				// Dissolve
				if (isFirstToken(curline, "d"))
				{
					tempMaterial.d = std::stof(tail(curline));
				}
				// Illumination
				if (isFirstToken(curline, "illum"))
				{
					tempMaterial.illum = std::stoi(tail(curline));
				}
				// Ambient Texture Map
				if (isFirstToken(curline, "map_Ka"))
				{
					tempMaterial.map_Ka = tail(curline);
				}
				// Diffuse Texture Map
				if (isFirstToken(curline, "map_Kd"))
				{
					tempMaterial.map_Kd = tail(curline);
				}
				// Specular Texture Map
				if (isFirstToken(curline, "map_Ks"))
				{
					tempMaterial.map_Ks = tail(curline);
				}
				// Specular Hightlight Map
				if (isFirstToken(curline, "map_Ns"))
				{
					tempMaterial.map_Ns = tail(curline);
				}
				// Alpha Texture Map
				if (isFirstToken(curline, "map_d"))
				{
					tempMaterial.map_d = tail(curline);
				}
				// Bump Map
				if (isFirstToken(curline, "map_Bump") || isFirstToken(curline, "map_bump") || isFirstToken(curline, "bump"))
				{
					tempMaterial.map_bump = tail(curline);
				}
			}


			LoadedMaterials.push_back(tempMaterial);


		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}

		return LoadedMaterials;
	}

	// Loaded Mesh Objects
	std::vector<Mesh> LoadedMeshes;
	std::vector<Vertex> LoadedVertexes;
	std::vector<int> LoadedIndexes;

	uint8_t mode;

	inline MVector3f get_normal(const MVector3f& a, const MVector3f& b, const MVector3f& c)
	{
		MVector3f v1 = a - b;
		MVector3f v2 = a - c;

		MVector3f ret = v1 / v2;

		float len = sqrtf(!ret);

		if (len > 0)
			return ret / len;
		else
			return MVector3f({ 0, 0, 0 });
	}

	void gen_normals(Mesh mesh)
	{
		for (int i = 0; i < mesh.f.size(); i += 3)
		{
			auto& f1 = mesh.f[i];
			auto& f2 = mesh.f[i + 1];
			auto& f3 = mesh.f[i + 2];

			auto& v1 = mesh.v[f1];
			auto& v2 = mesh.v[f2];
			auto& v3 = mesh.v[f3];

			auto n = get_normal(v1.v, v2.v, v3.v);

			v1.vn = n;
			v2.vn = n;
			v3.vn = n;
		}

		mesh.mode |= EMode_VN;
	}

	void Load(std::string data, std::string path_to_file)
	{
		std::string line = " ";
		std::string meshname = "unnamed";
		bool listening = false;
		std::string CurMeshMatName = "";
		std::string OldMeshMatName = "";

		std::vector<Material> LoadedMaterials;

		int unnamed_ids = 0;
		int mat_ids = 0;

		auto extract_mech = [&](std::string name, std::string mat)
			{
				Mesh mesh;

				mesh.mode =
					(EMode_V) |
					(vt.empty() ? EMode_ERROR : EMode_VT) |
					(vn.empty() ? EMode_ERROR : EMode_VN);

				mode = mode | mesh.mode;

				mesh.MeshMaterialName = mat;

				mesh.name = name;

				printf("extract mech: %s\n", mesh.name.c_str());

				mesh.f.resize(0);
				int clid = 0;
				int gid = 0;
				for (int i = 0; i < idx.size(); i++)
				{
					MVector4i& curr = idx[i];
					int lid = curr[3];
					if (lid == -1)
					{
						clid = -1;
						lid = 0;
						gid = mesh.v.size();
					}
					if (clid < lid)
					{
						clid = lid;
						int id = mesh.v.size();
						mesh.v.push_back(Vertex());
						mesh.v[id].v = v[curr[0]];
						if (!vt.empty())
							mesh.v[id].vt = vt[curr[1]];
						if (!vn.empty())
							mesh.v[id].vn = vn[curr[2]];
					}
					mesh.f.push_back(gid + lid);
				}
				clid = 0;
				gid = 0;
				for (int i = 0; i < idx.size(); i++)
				{
					MVector4i& curr = idx[i];
					int lid = curr[3];
					if (lid == -1)
					{
						clid = -1;
						lid = 0;
						gid = LoadedVertexes.size();
					}
					if (clid < lid)
					{
						clid = lid;
						int id = LoadedVertexes.size();
						LoadedVertexes.push_back(Vertex());
						LoadedVertexes[id].v = v[curr[0]];
						if (!vt.empty())
							LoadedVertexes[id].vt = vt[curr[1]];
						if (!vn.empty())
							LoadedVertexes[id].vn = vn[curr[2]];
					}
					LoadedIndexes.push_back(gid + lid);
				}

				idx.clear();

				LoadedMeshes.push_back(mesh);
			};

		while (!line.empty())
		{
			line = split_from_line(data);

			if (isFirstToken(line, "v"))
			{
				auto t = toMVector3f(line);
				v.push_back(t);

			}
			else if (isFirstToken(line, "vn"))
			{
				vn.push_back(toMVector3f(line));
			}
			else if (isFirstToken(line, "vt"))
			{
				vt.push_back(toMVector3f(line));
			}
			else if (isFirstToken(line, "f"))
			{
				auto indexes = toIndexes(line);

				for (int i = 0; i < indexes.size(); i++)
				{
					idx.push_back(indexes[i]);
				}
			}
			else if (isFirstToken(line, "usemtl"))
			{
				auto OldMeshMatName = CurMeshMatName;
				CurMeshMatName = tail(line);
				printf("usemtl: %s\n", CurMeshMatName.c_str());

				if (!idx.empty() && !v.empty())
				{
					if (meshname == "unnamed")
					{
						meshname = meshname + "_" + std::to_string(unnamed_ids);
						unnamed_ids++;
					}
					auto name = meshname + (mat_ids == 0 ? "" : "_" + std::to_string(mat_ids));
					mat_ids++;
					extract_mech(name, OldMeshMatName);
				}
			}
			else if (isFirstToken(line, "mtllib"))
			{
				auto file_name = tail(line);

				std::string path = fs::current_path().string();

				std::string pathtomat = path_to_file.substr(0, path_to_file.find_last_of("/\\") + 1) + file_name;

				auto loaded_mat = LoadMaterials(pathtomat);

				for (int i = 0; i < loaded_mat.size(); i++)
					LoadedMaterials.push_back(loaded_mat[i]);
			}
			if (isFirstToken(line, "o") || isFirstToken(line, "g") || line[0] == 'g' || line.empty())
			{
				if (!idx.empty() && !v.empty())
				{
					if (meshname == "unnamed")
					{
						meshname = meshname + "_" + std::to_string(unnamed_ids);
						unnamed_ids++;
					}
					auto name = meshname + (mat_ids == 0 ? "" : "_" + std::to_string(mat_ids));
					mat_ids++;
					extract_mech(name, CurMeshMatName);
				}

				mat_ids = 0;
				meshname = tail(line);

				printf("o: %s\n", meshname.c_str());

			}
		}

		for (int i = 0; i < LoadedMeshes.size(); i++)
		{
			auto& m = LoadedMeshes[i];
			auto matname = m.MeshMaterialName;
			bool mat_loaded = false;
			for (int i = 0; i < LoadedMaterials.size(); i++)
				if (LoadedMaterials[i].name == matname)
				{
					m.MeshMaterial = LoadedMaterials[i];
					mat_loaded = true;
					break;
				}

			if (!mat_loaded)
				m.MeshMaterialName = "";
		}

	}

};



#endif // __OBJ_PARSER_H__