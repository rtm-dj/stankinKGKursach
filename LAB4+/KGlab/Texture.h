#ifndef TEXTURE_H
#define TEXTURE_H

#include<string>

class Texture
{
	unsigned int texId=0;

public:
	Texture() {};
	~Texture();

	void LoadTexture(const std::string& texture_file_name);
	void Bind();

};

#endif
