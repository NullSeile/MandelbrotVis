#include "MandelbrotGraph.h"
#include <glad/gl.h>
#include <string>
#include <fstream>
#include <streambuf>

static const sf::BlendMode BlendAlpha(sf::BlendMode::SrcAlpha, sf::BlendMode::OneMinusSrcAlpha, sf::BlendMode::Add,
	sf::BlendMode::Zero, sf::BlendMode::One, sf::BlendMode::Add);
static const sf::BlendMode BlendIgnoreAlpha(sf::BlendMode::One, sf::BlendMode::Zero, sf::BlendMode::Add,
	sf::BlendMode::Zero, sf::BlendMode::One, sf::BlendMode::Add);


MandelbrotGraph::MandelbrotGraph()
	: m_pos(0, 0)
	, m_size(100, 100)
	, m_radius(2)
	, m_center(0, 0)
	, m_mousePressed(false)
	, m_maxIters(2048)
{
	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));

	std::ifstream file("rsc/mandelbrot.frag");
	m_coreShader = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	SetColorFunc(ColorFunction("vec3 get_color(int i) { return vec3(1, 1, 1); }"));
}

MandelbrotGraph::MandelbrotGraph(const ColorFunction& colorFunc)
	: m_pos(0, 0)
	, m_size(100, 100)
	, m_radius(2)
	, m_center(0, 0)
	, m_mousePressed(false)
	, m_maxIters(2048)
{
	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));

	std::ifstream file("rsc/mandelbrot.frag");
	m_coreShader = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	SetColorFunc(colorFunc);
}

void MandelbrotGraph::Resize()
{

}

void MandelbrotGraph::UpdateRange()
{
	double aspect = (double)m_size.x / (double)m_size.y;

	m_yRange = { m_center.y - m_radius, m_center.y + m_radius };
	m_xRange = { m_center.x - aspect * m_radius, m_center.x + aspect * m_radius };

	uint shader_handle = m_shader.getNativeHandle();
	glUseProgram(shader_handle);

	GLint xRange_loc = glGetUniformLocation(shader_handle, "xRange");
	GLint yRange_loc = glGetUniformLocation(shader_handle, "yRange");

	glUniform2d(xRange_loc, m_xRange.x, m_xRange.y);
	glUniform2d(yRange_loc, m_yRange.x, m_yRange.y);

	m_frame = 0;
}

std::pair<ui::Vec2d, ui::Vec2d> MandelbrotGraph::GetRange()
{
	return { m_xRange, m_yRange };
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
	m_shape.setPosition((ui::Vec2f)pos);
}

void MandelbrotGraph::SetSize(const ui::Vec2u& size)
{
	m_size = size;

	m_target.create(m_size.x, m_size.y);
	m_frame = 0;

	m_shape.setSize((ui::Vec2f)m_size);

	uint shader_handle = m_shader.getNativeHandle();
	glUseProgram(shader_handle);

	GLint size_loc = glGetUniformLocation(shader_handle, "size");
	glUniform2ui(size_loc, m_size.x, m_size.y);

	UpdateRange();
}

void MandelbrotGraph::SetMaxIters(int maxIters)
{
	m_maxIters = maxIters;
	m_shader.setUniform("maxIters", m_maxIters);
	m_frame = 0;
}

void MandelbrotGraph::Update(const sf::RenderWindow& window)
{
	if (m_mousePressed)
	{
		ui::Vec2d mousePos = MapCoordsToPos((ui::Vec2d)sf::Mouse::getPosition(window));
		ui::Vec2d delta = MapCoordsToPos(m_startPos) - mousePos;

		m_center += delta;

		m_startPos = (ui::Vec2d)sf::Mouse::getPosition(window);

		UpdateRange();

		m_frame = 0;
	}
}

void MandelbrotGraph::CheckInput(const sf::RenderWindow& window, ui::Event& e)
{
	if (e.type == sf::Event::MouseWheelMoved)
	{
		m_radius /= std::pow(1.1f, e.mouseWheel.delta);

		// Zoom where the mouse is
		ui::Vec2d iMousePos = MapCoordsToPos((ui::Vec2d)sf::Mouse::getPosition(window));

		UpdateRange();

		ui::Vec2d fMousePos = MapCoordsToPos((ui::Vec2d)sf::Mouse::getPosition(window));

		ui::Vec2d delta = fMousePos - iMousePos;

		m_center -= delta;

		UpdateRange();

		m_frame = 0;
	}

	if (e.type == sf::Event::MouseButtonPressed && e.key.code == sf::Mouse::Left && !e.handled)
	{
		m_mousePressed = true;
		m_startPos = (ui::Vec2d)sf::Mouse::getPosition(window);
	}

	if (e.type == sf::Event::MouseButtonReleased && e.key.code == sf::Mouse::Left)
	{
		m_mousePressed = false;
		m_frame = 0;
	}
}

void MandelbrotGraph::SetRadius(double radius)
{
	m_radius = radius;
	UpdateRange();
}

void MandelbrotGraph::SetCenter(const ui::Vec2d& center)
{
	m_center = center;
	UpdateRange();
}

void MandelbrotGraph::Draw(sf::RenderWindow& window)
{
	m_shader.setUniform("frame", m_frame);

	sf::RenderStates states = sf::RenderStates::Default;
	states.blendMode = (m_frame > 0 ? BlendAlpha : BlendIgnoreAlpha);
	states.shader = &m_shader;

	m_shape.setSize(sf::Vector2f((double)m_size.x, (double)m_size.y));
	m_shape.setFillColor(sf::Color::Magenta);

	m_target.draw(m_shape, states);
	m_target.display();

	sf::Sprite sprite(m_target.getTexture());
	window.clear();
	window.draw(sprite, sf::RenderStates(BlendIgnoreAlpha));

	m_frame += 1;
}

ui::Vec2d MandelbrotGraph::GetCenter()
{
	return m_center;
}

double MandelbrotGraph::GetRadius()
{
	return m_radius;
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

	// Set default uniforms
	SetSize(m_size);
	SetMaxIters(m_maxIters);

	m_frame = 0;
}

void MandelbrotGraph::SetUniform(const std::string& name, float val)
{
	m_shader.setUniform(name, val);

	m_frame = 0;
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
