#pragma once

#include <src/Global.h>
#include <src/Event.h>

class ColorFunction
{
public:
	struct Uniform
	{
		std::string name;
		ui::Vec2f range;
		float default_val;

		Uniform(const std::string& name, const ui::Vec2f& range, float default_val)
			: name(name), range(range), default_val(default_val) {}
	};

private:
	std::vector<Uniform> m_uniforms;
	std::string m_src;

public:
	ColorFunction(const std::string& src) : m_src(src) {}

	ColorFunction& AddUniform(const Uniform& uniform)
	{
		m_uniforms.push_back(uniform);
		return *this;
	}

	ColorFunction& AddUniform(const std::string& name, const ui::Vec2f& range, float default_val)
	{
		m_uniforms.emplace_back(name, range, default_val);
		return *this;
	}

	std::vector<Uniform>& GetUniforms()
	{
		return m_uniforms;
	}

	const std::vector<Uniform>& GetUniforms() const
	{
		return m_uniforms;
	}

	const std::string& GetSource() const
	{
		return m_src;
	}
};

class MandelbrotGraph
{
private:
	ui::Vec2d m_center;
	double m_radius;
	int m_frame;
	int m_maxIters;

	ui::Vec2d m_xRange;
	ui::Vec2d m_yRange;

	ui::Vec2d m_pos;
	ui::Vec2u m_size;

	sf::Shader m_shader;
	std::string m_coreShader;
	sf::RenderTexture m_target;
	sf::RectangleShape m_shape;

	bool m_mousePressed;
	ui::Vec2d m_startPos;

	void Resize();
	void UpdateRange();

public:
	MandelbrotGraph();
	MandelbrotGraph(const ColorFunction& colorFunction);

	void Update(const sf::RenderWindow& window);
	void CheckInput(const sf::RenderWindow& window, ui::Event& e);
	void Draw(sf::RenderWindow& window);

	void SetRadius(double radius);
	void SetPosition(const ui::Vec2d& pos);
	void SetSize(const ui::Vec2u& size);
	void SetMaxIters(int maxIters);
	void SetCenter(const ui::Vec2d& center);
	void SetColorFunc(const ColorFunction& colorFunc);

	std::pair<ui::Vec2d, ui::Vec2d> GetRange();
	ui::Vec2d GetCenter();
	double GetRadius();

	void SetUniform(const std::string& name, float val);
	float GetUniform(const std::string& name);

	ui::Vec2d MapPosToCoords(const ui::Vec2d& pos);
	ui::Vec2d MapCoordsToPos(const ui::Vec2d& coords);
};

