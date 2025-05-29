#include "Render.h"
#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "GUItextRectangle.h"
#include "MyShaders.h"
#include "Texture.h"


#include "ObjLoader.h"


#include "debout.h"



//внутренняя логика "движка"
#include "MyOGL.h"
extern OpenGL gl;
#include "Light.h"
Light light;
#include "Camera.h"
Camera camera;


bool texturing = true;
bool lightning = true;
bool alpha = false;


//переключение режимов освещения, текстурирования, альфаналожения
void switchModes(OpenGL *sender, KeyEventArg arg)
{
	//конвертируем код клавиши в букву
	auto key = LOWORD(MapVirtualKeyA(arg.key, MAPVK_VK_TO_CHAR));

	switch (key)
	{
	case 'L':
		lightning = !lightning;
		break;
	case 'T':
		texturing = !texturing;
		break;
	case 'A':
		alpha = !alpha;
		break;
	}
}

//умножение матриц c[M1][N1] = a[M1][N1] * b[M2][N2]
template<typename T, int M1, int N1, int M2, int N2>
void MatrixMultiply(const T* a, const T* b, T* c)
{
	for (int i = 0; i < M1; ++i)
	{
		for (int j = 0; j < N2; ++j)
		{
			c[i * N2 + j] = T(0);
			for (int k = 0; k < N1; ++k)
			{
				c[i * N2 + j] += a[i * N1 + k] * b[k * N2 + j];
			}
		}
	}
}

//Текстовый прямоугольничек в верхнем правом углу.
//OGL не предоставляет возможности для хранения текста
//внутри этого класса создается картинка с текстом (через виндовый GDI),
//в виде текстуры накладывается на прямоугольник и рисуется на экране.
//Это самый простой способ что то написать на экране
//но ооооочень не оптимальный
GuiTextRectangle text;

//айдишник для текстуры
GLuint texId;
//выполняется один раз перед первым рендером

ObjModel f;


Shader cassini_sh;
Shader phong_sh;
Shader vb_sh;
Shader simple_texture_sh;

Texture stankin_tex, vb_tex, monkey_tex;



void initRender()
{

	cassini_sh.VshaderFileName = "shaders/v.vert";
	cassini_sh.FshaderFileName = "shaders/cassini.frag";
	cassini_sh.LoadShaderFromFile();
	cassini_sh.Compile();

	phong_sh.VshaderFileName = "shaders/v.vert";
	phong_sh.FshaderFileName = "shaders/light.frag";
	phong_sh.LoadShaderFromFile();
	phong_sh.Compile();

	vb_sh.VshaderFileName = "shaders/v.vert";
	vb_sh.FshaderFileName = "shaders/vb.frag";
	vb_sh.LoadShaderFromFile();
	vb_sh.Compile();

	simple_texture_sh.VshaderFileName = "shaders/v.vert";
	simple_texture_sh.FshaderFileName = "shaders/textureShader.frag";
	simple_texture_sh.LoadShaderFromFile();
	simple_texture_sh.Compile();

	stankin_tex.LoadTexture("textures/stankin.png");
	vb_tex.LoadTexture("textures/vb.png");
	monkey_tex.LoadTexture("textures/monkey.png");


	f.LoadModel("models//monkey.obj_m");
	//==============НАСТРОЙКА ТЕКСТУР================
	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	

	//================НАСТРОЙКА КАМЕРЫ======================
	camera.caclulateCameraPos();

	//привязываем камеру к событиям "движка"
	gl.WheelEvent.reaction(&camera, &Camera::Zoom);
	gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);
	gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave);
	gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag);
	gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag);
	//==============НАСТРОЙКА СВЕТА===========================
	//привязываем свет к событиям "движка"
	gl.MouseMovieEvent.reaction(&light, &Light::MoveLight);
	gl.KeyDownEvent.reaction(&light, &Light::StartDrug);
	gl.KeyUpEvent.reaction(&light, &Light::StopDrug);
	//========================================================
	//====================Прочее==============================
	gl.KeyDownEvent.reaction(switchModes);
	text.setSize(512, 180);
	//========================================================
	   

	camera.setPosition(2, 1.5, 1.5);
	
}
float view_matrix[16];
double full_time = 0;
int location = 0;
void Render(double delta_time)
{    
	

	full_time += delta_time;
	
	//натройка камеры и света
	//в этих функциях находятся OGLные функции
	//которые устанавливают параметры источника света
	//и моделвью матрицу, связанные с камерой.

	if (gl.isKeyPressed('F')) //если нажата F - свет из камеры
	{
		light.SetPosition(camera.x(), camera.y(), camera.z());
	}
	camera.SetUpCamera();
	//забираем моделвью матрицу сразу после установки камера
	//так как в ней отсуствуют трансформации glRotate...
	//она, фактически, является видовой.
	glGetFloatv(GL_MODELVIEW_MATRIX,view_matrix);

	

	light.SetUpLight();

	//рисуем оси
	gl.DrawAxes();

	

	glBindTexture(GL_TEXTURE_2D, 0);

	//включаем нормализацию нормалей
	//чтобв glScaled не влияли на них.

	glEnable(GL_NORMALIZE);  
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	//включаем режимы, в зависимости от нажания клавиш. см void switchModes(OpenGL *sender, KeyEventArg arg)
	if (lightning)
		glEnable(GL_LIGHTING);
	if (texturing)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0); //сбрасываем текущую текстуру
	}
		
	if (alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
		
	//=============НАСТРОЙКА МАТЕРИАЛА==============


	//настройка материала, все что рисуется ниже будет иметь этот метериал.
	//массивы с настройками материала
	float  amb[] = { 0.2, 0.2, 0.1, 1. };
	float dif[] = { 0.4, 0.65, 0.5, 1. };
	float spec[] = { 0.9, 0.8, 0.3, 1. };
	float sh = 0.2f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); 
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH); //закраска по Гуро      
			   //(GL_SMOOTH - плоская закраска)

	//============ РИСОВАТЬ ТУТ ==============

	

	//квадратик станкина

	

	//рисуем квадратик с овалом Кассини!
	
	cassini_sh.UseShader();

	location = glGetUniformLocationARB(cassini_sh.program, "Time");
	glUniform1fARB(location, full_time);
	location = glGetUniformLocationARB(cassini_sh.program, "size");
	glUniform2fARB(location, 100, 100);
	
	glPushMatrix();

	glTranslated(0, -1.2, 0);

	
	glBegin(GL_QUADS);
	glNormal3d(0, 0, 1);
	glTexCoord2d(1, 1);
	glVertex3d(0.5, 0.5, 0);
	glTexCoord2d(1, 0);
	glVertex3d(0.5, -0.5, 0);
	glTexCoord2d(0, 0);
	glVertex3d(-0.5, -0.5, 0);
	glTexCoord2d(0, 1);
	glVertex3d(-0.5, 0.5, 0);
	glEnd();


	glPopMatrix();







	//Квадратик с освещением
	phong_sh.UseShader();

	float light_pos[4] = { light.x(),light.y(), light.z(), 1 };
	float light_pos_v[4];
	
	//переносим координаты света в видовые координаты
	MatrixMultiply<float, 1, 4, 4, 4>(light_pos, view_matrix, light_pos_v);

	
	location = glGetUniformLocationARB(phong_sh.program, "Ia");
	glUniform3fARB(location, 1, 1, 1);
	location = glGetUniformLocationARB(phong_sh.program, "Id");
	glUniform3fARB(location, 1, 1, 1);
	location = glGetUniformLocationARB(phong_sh.program, "Is");
	glUniform3fARB(location, 1, 1, 1);

	location = glGetUniformLocationARB(phong_sh.program, "ma");
	glUniform3fARB(location, 0.1, 0.1, 0.1);
	location = glGetUniformLocationARB(phong_sh.program, "md");
	glUniform3fARB(location, 0.6, 0.6, 0.6);
	location = glGetUniformLocationARB(phong_sh.program, "ms");
	glUniform4fARB(location, 0, 1, 0, 300);
		
	
	location = glGetUniformLocationARB(phong_sh.program, "light_pos_v");
	glUniform3fvARB(location,1,light_pos_v);
	
	glPushMatrix();

	glTranslated(0, 0, 0);
	

	glBegin(GL_QUADS);
	glNormal3d(0, 0, 1);
	glTexCoord2d(1, 1);
	glVertex3d(0.5, 0.5, 0);
	glTexCoord2d(1, 0);
	glVertex3d(0.5, -0.5, 0);
	glTexCoord2d(0, 0);
	glVertex3d(-0.5, -0.5, 0);
	glTexCoord2d(0, 1);
	glVertex3d(-0.5, 0.5, 0);
	glEnd();


	glPopMatrix();



	//Квадратик без освещения

	Shader::DontUseShaders();

	glBindTexture(GL_TEXTURE_2D, 0);

	glPushMatrix();

	glTranslated(1.2, 0, 0);


	glBegin(GL_QUADS);
	glNormal3d(0, 0, 1);
	glTexCoord2d(1, 1);
	glVertex3d(0.5, 0.5, 0);
	glTexCoord2d(1, 0);
	glVertex3d(0.5, -0.5, 0);
	glTexCoord2d(0, 0);
	glVertex3d(-0.5, -0.5, 0);
	glTexCoord2d(0, 1);
	glVertex3d(-0.5, 0.5, 0);
	glEnd();


	glPopMatrix();


	//квадратик с ВБ


	vb_sh.UseShader();

	glActiveTexture(GL_TEXTURE0);
	stankin_tex.Bind();
	glActiveTexture(GL_TEXTURE1);
	vb_tex.Bind();

	location = glGetUniformLocationARB(vb_sh.program, "time");
	glUniform1fARB(location, full_time);
	location = glGetUniformLocationARB(vb_sh.program, "tex_stankin");
	glUniform1iARB(location, 0);
	location = glGetUniformLocationARB(vb_sh.program, "tex_vb");
	glUniform1iARB(location, 1);

	glPushMatrix();

	glTranslated(0, 1.2, 0);
		glBegin(GL_QUADS);
		glNormal3d(0, 0, 1);
		glTexCoord2d(1, 1);
		glVertex3d(0.5, 0.5, 0);
		glTexCoord2d(1, 0);
		glVertex3d(0.5, -0.5, 0);
		glTexCoord2d(0, 0);
		glVertex3d(-0.5, -0.5, 0);
		glTexCoord2d(0, 1);
		glVertex3d(-0.5, 0.5, 0);
	glEnd();


	glPopMatrix();


	//обезьянка без шейдеров
	glPushMatrix();
	Shader::DontUseShaders();
	glActiveTexture(GL_TEXTURE0);
	monkey_tex.Bind();
	glShadeModel(GL_SMOOTH);
	glTranslated(-1, 0, 0.5);
	glScaled(0.1, 0.1, 0.1);
	glRotated(180, 0, 0, 1);
	f.Draw();
	glPopMatrix();

	//обезьянка с шейдерами


	simple_texture_sh.UseShader();
	location = glGetUniformLocationARB(simple_texture_sh.program, "tex");
	glUniform1iARB(location, 0);
	glActiveTexture(GL_TEXTURE0);
	monkey_tex.Bind();


	glPushMatrix();
	glTranslated(-1, 1, 0.5);
	glScaled(0.1, 0.1, 0.1);
	glRotated(180, 0, 0, 1);
	f.Draw();
	glPopMatrix();

	//===============================================
	
	
	//сбрасываем все трансформации
	glLoadIdentity();
	camera.SetUpCamera();	
	Shader::DontUseShaders();
	//рисуем источник света
	light.DrawLightGizmo();

	//================Сообщение в верхнем левом углу=======================
	glActiveTexture(GL_TEXTURE0);
	//переключаемся на матрицу проекции
	glMatrixMode(GL_PROJECTION);
	//сохраняем текущую матрицу проекции с перспективным преобразованием
	glPushMatrix();
	//загружаем единичную матрицу в матрицу проекции
	glLoadIdentity();

	//устанавливаем матрицу паралельной проекции
	glOrtho(0, gl.getWidth() - 1, 0, gl.getHeight() - 1, 0, 1);

	//переключаемся на моделвью матрицу
	glMatrixMode(GL_MODELVIEW);
	//сохраняем матрицу
	glPushMatrix();
    //сбразываем все трансформации и настройки камеры загрузкой единичной матрицы
	glLoadIdentity();

	//отрисованное тут будет визуалзироватся в 2д системе координат
	//нижний левый угол окна - точка (0,0)
	//верхний правый угол (ширина_окна - 1, высота_окна - 1)

	
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(3);
	ss << "T - " << (texturing ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"текстур" << std::endl;
	ss << "L - " << (lightning ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"освещение" << std::endl;
	ss << "A - " << (alpha ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"альфа-наложение" << std::endl;
	ss << L"F - Свет из камеры" << std::endl;
	ss << L"G - двигать свет по горизонтали" << std::endl;
	ss << L"G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << L"Коорд. света: (" << std::setw(7) <<  light.x() << "," << std::setw(7) << light.y() << "," << std::setw(7) << light.z() << ")" << std::endl;
	ss << L"Коорд. камеры: (" << std::setw(7) << camera.x() << "," << std::setw(7) << camera.y() << "," << std::setw(7) << camera.z() << ")" << std::endl;
	ss << L"Параметры камеры: R=" << std::setw(7) << camera.distance() << ",fi1=" << std::setw(7) << camera.fi1() << ",fi2=" << std::setw(7) << camera.fi2() << std::endl;
	ss << L"delta_time: " << std::setprecision(5)<< delta_time << std::endl;
	ss << L"full_time: " << std::setprecision(2) << full_time << std::endl;

	
	text.setPosition(10, gl.getHeight() - 10 - 180);
	text.setText(ss.str().c_str());
	
	text.Draw();

	//восстанавливаем матрицу проекции на перспективу, которую сохраняли ранее.
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	
}   



