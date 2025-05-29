#include "ObjLoader.h"
#include <fstream>
#include <string>
#include <sstream>


#include <string>
#include <vector>
#include <format>


using namespace std;


int ObjModel::LoadModel(const char* filename)
{

	vector<ObjVertex> V;
	vector<ObjTexCord> VT;
	vector<ObjNormal> VN;
	vector<ObjFace> F;

	string str;

	ifstream fin(filename);
	if (!fin.is_open())
	{
		throw std::format("File {} can`t be opened!", filename).c_str();
	}
	string line;
	while (std::getline(fin, line))
	{
		istringstream isstr(line);
		string mode;
		isstr >> mode;
		if (mode == "v")
		{
			ObjVertex v;
			isstr >> v.x >> v.y >> v.z;
			V.push_back(v);
		}
		else if (mode == "vt")
		{
			ObjTexCord t;
			isstr >> t.u >> t.v;
			VT.push_back(t);;
		}
		else if (mode == "vn")
		{
			ObjNormal n;
			isstr >> n.x >> n.y >> n.z;
			VN.push_back(n);
		}
		else if (mode == "f")
		{
			string face;
			ObjFace f;

			for (isstr >> face; isstr; isstr >> face)
			{
				istringstream isstr(face);
				string digit;
				int n[3];
				int i = 0;
				while (std::getline(isstr, digit, '/'))
				{
					if (digit.empty())
						n[i] = 0;
					else
						n[i] = std::stoi(digit);
					i++;
					if (i == 3) break;
				}
				f.vertex.push_back(V[n[0] - 1]);
				if (n[1] > 0)
					f.texCoord.push_back(VT[n[1] - 1]);
				if (n[2] > 0)
					f.normal.push_back(VN[n[2] - 1]);
			}

			Faces.push_back(f);
		}


	}

	glDeleteLists(listId, 1);
	listId = glGenLists(1);

	glNewList(listId, GL_COMPILE);
	RenderModel(GL_POLYGON);
	glEndList();

	return 1;

}
