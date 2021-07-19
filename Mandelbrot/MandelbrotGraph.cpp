#include "MandelbrotGraph.h"
#include <glad/gl.h>
#include <string>
#include <fstream>
#include <streambuf>

void MandelbrotGraph::Recalculate()
{
	m_imgPos = MapCoordsToPos({ 0, 0 });
	m_imgEndPos = MapCoordsToPos((ui::Vec2f)m_size / m_resolutionScale);

	uint shader_handle = m_shader.getNativeHandle();
	glUseProgram(shader_handle);

	GLint size_loc = glGetUniformLocation(shader_handle, "size");
	GLint xRange_loc = glGetUniformLocation(shader_handle, "xRange");
	GLint yRange_loc = glGetUniformLocation(shader_handle, "yRange");

	glUniform2ui(size_loc, (uint)(m_size.x * m_resolutionScale), (uint)(m_size.y * m_resolutionScale));
	glUniform2d(xRange_loc, m_xRange.min, m_xRange.max);
	glUniform2d(yRange_loc, m_yRange.min, m_yRange.max);
	m_shader.setUniform("maxIters", m_maxIters);

	m_target.create((uint)(m_size.x * m_resolutionScale), (uint)(m_size.y * m_resolutionScale));

	sf::RectangleShape shape((ui::Vec2f)m_size * m_resolutionScale);
	m_target.draw(shape, &m_shader);
	m_target.generateMipmap();

	m_spr = sf::Sprite(m_target.getTexture());
}

MandelbrotGraph::MandelbrotGraph()
	: m_pos(0, 0)
	, m_size(100, 100)
	, m_zoom(2)
	, m_center(0, 0)
	, m_view(-m_zoom, m_zoom)
	, m_mousePressed(false)
	, m_aspect(1)
	, m_maxIters(2048)
	, m_resolutionScale(1)
{
	std::ifstream file("rsc/mandelbrot.frag");
	m_coreShader = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	SetColorFunc(ColorFunction("vec3 get_color(int i) { return vec3(1, 1, 1); }"));
}

MandelbrotGraph::MandelbrotGraph(const ColorFunction& colorFunc)
	: m_pos(0, 0)
	, m_size(100, 100)
	, m_zoom(2)
	, m_center(0, 0)
	, m_view(-m_zoom, m_zoom)
	, m_mousePressed(false)
	, m_aspect(1)
	, m_maxIters(2048)
	, m_resolutionScale(1)
{
	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));

	std::ifstream file("rsc/mandelbrot.frag");
	m_coreShader = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	SetColorFunc(colorFunc);
}

std::pair<ui::Vec2d, ui::Vec2d> MandelbrotGraph::GetRange()
{
	return { m_xRange, m_yRange };
}

void MandelbrotGraph::SetResolutionScale(float resolutionScale)
{
	m_resolutionScale = resolutionScale;
	Recalculate();
}

void MandelbrotGraph::MakeScreenShot(const std::string& fileName, float resolutionScale, int maxIterations)
{
	float prevResScale = m_resolutionScale;
	int prevMaxIters = m_maxIters;

	m_resolutionScale = resolutionScale;
	m_maxIters = maxIterations;

	Recalculate();

	m_target.getTexture().copyToImage().saveToFile(fileName);

	m_resolutionScale = prevResScale;
	m_maxIters = prevMaxIters;

	Recalculate();
}

void MandelbrotGraph::SetAntialiasingLevel(int antialiasingLevel)
{
	m_antialiasingLevel = antialiasingLevel;
	Recalculate();
}

void MandelbrotGraph::MakeScreenShot(const std::string& fileName)
{
	MakeScreenShot(fileName, m_resolutionScale, m_maxIters);
}

ui::Vec2d MandelbrotGraph::MapPosToCoords(const ui::Vec2d& pos)
{
	return ui::Vec2d
	(
		ui::map(pos.x, m_xRange.min, m_xRange.max, 0, m_size.x),
		ui::map(pos.y, m_yRange.min, m_yRange.max, m_size.y, 0)
	) + m_pos;
}

ui::Vec2d MandelbrotGraph::MapCoordsToPos(const ui::Vec2d& coords)
{
	return
	{
		ui::map(coords.x - m_pos.x, 0, m_size.x, m_xRange.min, m_xRange.max),
		ui::map(coords.y - m_pos.y, m_size.y, 0, m_yRange.min, m_yRange.max)
	};
}

void MandelbrotGraph::SetPosition(const ui::Vec2d& pos)
{
	m_pos = pos;
}

void MandelbrotGraph::SetSize(const ui::Vec2u& size)
{
	m_size = size;

	UpdateView();

	dirty = true;

	Recalculate();
}

void MandelbrotGraph::SetMaxIters(int maxIters)
{
	m_maxIters = maxIters;

	Recalculate();
}

void MandelbrotGraph::UpdateView()
{
	m_view = { -m_zoom, m_zoom };

	m_aspect = m_size.x / (double)m_size.y;

	m_xRange = ui::Vec2d(m_center.x, m_center.x) + m_view * m_aspect;
	m_yRange = ui::Vec2d(m_center.y, m_center.y) + m_view;
}

void MandelbrotGraph::Update(const sf::RenderWindow& window)
{
	if (m_mousePressed)
	{
		ui::Vec2d mousePos = MapCoordsToPos((ui::Vec2d)sf::Mouse::getPosition(window));
		ui::Vec2d delta = MapCoordsToPos(m_startPos) - mousePos;

		m_center += delta;

		m_startPos = (ui::Vec2d)sf::Mouse::getPosition(window);

		UpdateView();
	}
}

void MandelbrotGraph::CheckInput(const sf::RenderWindow& window, ui::Event& e)
{
	if (e.type == sf::Event::MouseWheelMoved && m_mousePressed)
	{
		switch (e.mouseWheel.delta)
		{
		case 1:
			m_zoom /= 1.2f;
			break;

		case -1:
			m_zoom *= 1.2f;
			break;

		default:
			break;
		}

		// Zoom where the mouse is
		ui::Vec2d iMousePos = MapCoordsToPos((ui::Vec2d)sf::Mouse::getPosition(window));

		UpdateView();

		ui::Vec2d fMousePos = MapCoordsToPos((ui::Vec2d)sf::Mouse::getPosition(window));
		
		ui::Vec2d delta = fMousePos - iMousePos;

		m_center -= delta;
	}

	if (e.type == sf::Event::MouseButtonPressed && e.key.code == sf::Mouse::Left && !e.handled)
	{
		m_mousePressed = true;
		m_startPos = (ui::Vec2d)sf::Mouse::getPosition(window);
	}

	if (e.type == sf::Event::MouseButtonReleased && e.key.code == sf::Mouse::Left)
	{
		m_mousePressed = false;
		Recalculate();
	}
}

void MandelbrotGraph::SetZoom(const double& zoom)
{
	m_zoom = zoom;

	UpdateView();

	Recalculate();
}

void MandelbrotGraph::SetCenter(const ui::Vec2d& center)
{
	m_center = center;

	UpdateView();

	Recalculate();
}

void MandelbrotGraph::Draw(sf::RenderWindow& window)
{
	ui::Vec2d p = MapPosToCoords(m_imgPos);
	ui::Vec2d f = MapPosToCoords(m_imgEndPos);

	double scale = (f.x - p.x) / m_size.x;

	m_spr.setPosition((ui::Vec2f)p);
	m_spr.setScale((float)scale, (float)scale);

	window.draw(m_spr);
}

ui::Vec2d MandelbrotGraph::GetCenter()
{
	return m_center;
}

double MandelbrotGraph::GetZoom()
{
	return m_zoom;
}

void MandelbrotGraph::SetColorFunc(const ColorFunction& colorFunc)
{
	std::stringstream ss;
	ss << "#version 460\n\n";
	for (const auto& u : colorFunc.GetUniforms())
		ss << "uniform float " << u.name << ";\n";

	ss << colorFunc.GetSource() << '\n';
	ss << m_coreShader;

	//std::cout << ss.str() << '\n';

	if (!m_shader.loadFromMemory(ss.str(), sf::Shader::Fragment))
		std::cout << "Error loading shader\n";

	for (const auto& u : colorFunc.GetUniforms())
	{
		SetUniform(u.name, u.default_val);
	}

	Recalculate();
}

void MandelbrotGraph::SetUniform(const std::string& name, float val)
{
	m_shader.setUniform(name, val);

	Recalculate();
}

float MandelbrotGraph::GetUniform(const std::string& name)
{
	uint shader_handle = m_shader.getNativeHandle();
	glUseProgram(shader_handle);

	GLint loc = glGetUniformLocation(shader_handle, name.c_str());
	if (loc == -1)
	{
		std::cout << "ERROR: Uniform '" << name << "' was not found!\n";
		exit(EXIT_FAILURE);
	}

	float out;
	glGetUniformfv(shader_handle, loc, &out);

	return out;
}
