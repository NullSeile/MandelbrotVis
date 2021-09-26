#if 1
#include <UITools.h>
#include <iomanip>

#include "MandelbrotGraph.h"

int main()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	ui::Vec2u windowSize = { 900, 900 };

	sf::RenderWindow window({ windowSize.x, windowSize.y }, "Graph", sf::Style::Default, settings);
	window.setFramerateLimit(120);

	std::vector<ColorFunction> colors;

	colors.push_back(ColorFunction(R"(
vec3 colors[] = vec3[](
  vec3(0, 0, 0),
  vec3(0.13, 0.142, 0.8),
  vec3(1, 1, 1),
  vec3(1, 0.667, 0),
  vec3(0, 0, 0)
);
vec3 get_color(int iters)
{
	float x  = iters / colorMult;
    x = mod(x, colors.length() - 1);

    if (x == colors.length())
        x = 0;

    if (floor(x) == x)
        return colors[int(x)];

    int i = int(floor(x));
    float t = mod(x, 1);
    return mix(colors[i], colors[i+1], t);
}
	)").AddUniform("colorMult", { 1, 300 }, 200));

	colors.push_back(ColorFunction(R"(
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 get_color(int i)
{
    return hsv2rgb(vec3(i / colorMult, 1, 1));
}
	)").AddUniform("colorMult", { 1.1f, 5000 }, 1000));

	colors.push_back(ColorFunction(R"(
vec3 get_color(int i)
{
    float t = exp(-i / colorMult);
    return mix(vec3(1, 1, 1), vec3(0.1, 0.1, 1), t);
}
	)").AddUniform("colorMult", { 1, 1000 }, 200));

	colors.push_back(ColorFunction(R"(
vec3 get_color(int i)
{
    float x = i / colorMult;
    float n1 = sin(x) * 0.5 + 0.5;
    float n2 = cos(x) * 0.5 + 0.5;
    return vec3(n1, n2, 1.0) * 1;
}
	)").AddUniform("colorMult", { 1, 300 }, 200));


	size_t currentColorIndex = 0;

	MandelbrotGraph graph(colors[currentColorIndex]);
	graph.SetPosition({ 0, 0 });
	graph.SetSize(windowSize);
	graph.SetMaxIters(1500);

	sf::Font font;
	font.loadFromFile("rsc/Consolas.ttf");

	{
		graph.SetCenter({ -0.5, 0 });
		graph.SetRadius(1.1);

		//graph.SetCenter({ 0.270925, 0.004725 });
		//graph.SetRadius(0.0001);

		//graph.SetCenter({ -0.745428, 0.113009 });
		//graph.SetRadius(3.0e-5);

		//graph.SetCenter({ -1.25223118015508028122, 0.03755885941558481655 });
		//graph.SetRadius(3.6e-8);

		//graph.SetCenter({ -1.2519620871808931906, 0.037393550920969360896 });
		//graph.SetRadius(1.45e-08);

		//graph.SetCenter({-0.747747, 0.124517});
		//graph.SetRadius(1.0e-4);

		//graph.SetCenter({ -1.25066, 0.02012 });
		//graph.SetRadius(1.7e-4);

		//graph.SetCenter({ -0.748, 0.1 });
		//graph.SetRadius(0.0014);

		//graph.SetCenter({ -0.514814, 0.6111110539 });
		//graph.SetRadius(0.1);

		//graph.SetCenter({ -0.7461860152692163517, 0.095926522548036297078 });
		//graph.SetRadius(0.0035131274618377607462);

		//graph.SetCenter({ -0.74656412896776469523, 0.098865810107694587772 });
		//graph.SetRadius(8.2212188006580699331e-12);
	}

	ui::Vec2u toolsSize = { 600, 300 };
	sf::RenderWindow toolsWindow({ toolsSize.x, toolsSize.y }, "Tools :)", sf::Style::Close);


	ui::Widget widget;

	auto UpdateSliders = [&graph, &colors, &currentColorIndex, &widget, &toolsSize, &font]()
	{
		widget.Clear<ui::Slider>();

		//float space = 10;

		size_t i = 0;
		for (auto& u : colors[currentColorIndex].GetUniforms())
		{
			auto slider = new ui::Slider(u.name, font);
			slider->SetPosition({ 10, (float)(i * 40 + 80) });
			slider->SetSize({ (float)(toolsSize.x - 100), 30 });
			slider->SetRange(u.range);
			slider->SetValue(u.default_val);
			slider->ShowValue(true);
			slider->SetTextPrecision(0);

			auto& text = slider->GetText();
			text.setCharacterSize(10);
			text.setFillColor(sf::Color::White);
			text.setScale(0.7f, 0.7f);

			slider->SetReleasedFunction([&u, &graph](ui::UIObject* obj)
			{
				auto self = dynamic_cast<ui::Slider*>(obj);
				float val = self->GetValue();
				graph.SetUniform(u.name, val);
				u.default_val = val;
			});

			widget.AddObject(slider);
			i++;
		}
	};

	{
		float xSize = toolsSize.x / (float)colors.size();
		float space = 10;
		//float width = 50;
		ui::Vec2f size = { xSize - 2 * space, 50 };
		for (size_t i = 0; i < colors.size(); i++)
		{
			auto button = new ui::Button(std::to_string(i));

			auto texture = new sf::RenderTexture();
			texture->create((uint)size.x, (uint)size.y);

			auto shader = new sf::Shader();

			std::stringstream ss;

			ss << "#version 460\n\n";

			for (const auto& u : colors[i].GetUniforms())
				ss << "uniform float " << u.name << ";\n";

			ss << colors[i].GetSource() << '\n';
			ss << R"(
uniform int range;
uniform int size;

out vec4 outColor;

void main()
{
    int i = int((gl_FragCoord.x / size) * range);
    outColor = vec4(get_color(i).xyz, 1);
}
		)";

			shader->loadFromMemory(ss.str(), sf::Shader::Fragment);
			shader->setUniform("size", (int)size.x);
			shader->setUniform("range", 1000);

			for (const auto& u : colors[i].GetUniforms())
				shader->setUniform(u.name, u.default_val);

			sf::RectangleShape shape(size);
			texture->draw(shape, shader);
			texture->display();

			button->shape.setTexture(&texture->getTexture());

			//button->shape.setFillColor(sf::Color::White);
			button->shape.setPosition((xSize * i) + space, space);
			button->shape.setSize(size);
			button->SetClickFunction([i, &UpdateSliders, &currentColorIndex, &graph, &colors](ui::UIObject* obj)
				{
					currentColorIndex = i;
					graph.SetColorFunc(colors[currentColorIndex]);
					UpdateSliders();
				});

			widget.AddObject(button);
		}
	}

	UpdateSliders();

	sf::Clock c;

	while (window.isOpen())
	{
		ui::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();

			graph.CheckInput(window, e);

			if (e.type == sf::Event::Resized)
			{
				windowSize = window.getSize();

				sf::View view = window.getView();
				view.setSize((ui::Vec2f)windowSize);
				view.setCenter((ui::Vec2f)windowSize / 2.f);
				window.setView(view);

				graph.SetSize(windowSize);
			}

			if (e.type == sf::Event::KeyPressed)
			{
				if (e.key.code == sf::Keyboard::Space)
				{
					auto [xRange, yRange] = graph.GetRange();

					std::cout << std::setprecision(20) << "Center: " << graph.GetCenter() << " Radius: " << graph.GetRadius() << '\n';
				}
				if (e.key.code == sf::Keyboard::Return)
				{
					std::cout << "New max iters: ";
					int maxIters;
					std::cin >> maxIters;
					graph.SetMaxIters(maxIters);
				}
				if (e.key.code == sf::Keyboard::C)
				{
					graph.SetCenter({ 0, 0 });
				}
			}
		}

		graph.Update(window);

		while (toolsWindow.pollEvent(e))
		{
			widget.CheckInput(toolsWindow, e);
		}

		toolsWindow.clear();

		widget.Update(toolsWindow);
		widget.Draw(toolsWindow);

		toolsWindow.display();

		
		window.clear();
		graph.Draw(window);

		window.display();
	}
}

#endif