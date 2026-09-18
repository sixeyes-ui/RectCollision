#include <SFML/Graphics.hpp>

bool isPointOnRect(const sf::RectangleShape& rect, const sf::RectangleShape& player)
{
	sf::Vector2f rectPos = rect.getPosition();
	sf::Vector2f playerPos = player.getPosition();
	return rectPos.x >= playerPos.x &&
		   rectPos.y >= playerPos.y &&
		   rectPos.x <= playerPos.x + player.getSize().x &&
		   rectPos.y <= playerPos.y + player.getSize().y;
}

bool RectVsRect(const sf::RectangleShape& r1, const sf::RectangleShape& r2)
{
	sf::Vector2f rect1Pos = r1.getPosition();
	sf::Vector2f rect2Pos = r2.getPosition();
	return rect1Pos.x <= rect2Pos.x + r2.getSize().x && rect1Pos.x + r1.getSize().x >= rect2Pos.x &&
		   rect1Pos.y <= rect2Pos.y + r2.getSize().y && rect1Pos.y + r1.getSize().y >= rect2Pos.y;
}

bool RayVsRect(
	const sf::Vector2f& ray_origin,
	const sf::Vector2f& ray_dir,
	const sf::RectangleShape& target,
	sf::Vector2f& contact_point,
	sf::Vector2f& contact_normal,
	float& t_hit_near)
{
	const float epsilon = 0.000001f;

	float minX = target.getPosition().x;
	float minY = target.getPosition().y;

	float maxX = minX + target.getSize().x;
	float maxY = minY + target.getSize().y;

	// Avoid division by zero
	if (std::abs(ray_dir.x) < epsilon ||
		std::abs(ray_dir.y) < epsilon)
	{
		return false;
	}

	float t_nearX = (minX - ray_origin.x) / ray_dir.x;
	float t_farX = (maxX - ray_origin.x) / ray_dir.x;

	float t_nearY = (minY - ray_origin.y) / ray_dir.y;
	float t_farY = (maxY - ray_origin.y) / ray_dir.y;

	if (t_nearX > t_farX)
		std::swap(t_nearX, t_farX);

	if (t_nearY > t_farY)
		std::swap(t_nearY, t_farY);

	// No intersection
	if (t_nearX > t_farY ||
		t_nearY > t_farX)
	{
		return false;
	}

	t_hit_near = std::max(t_nearX, t_nearY);

	float t_hit_far = std::min(t_farX, t_farY);

	// Rectangle is behind ray
	if (t_hit_far < 0.0f)
		return false;

	contact_point =
		ray_origin + t_hit_near * ray_dir;

	// Find collision normal
	if (t_nearX > t_nearY)
	{
		if (ray_dir.x < 0)
			contact_normal = sf::Vector2f(1.f, 0.f);
		else
			contact_normal = sf::Vector2f(-1.f, 0.f);
	}
	else
	{
		if (ray_dir.y < 0)
			contact_normal = sf::Vector2f(0.f, 1.f);
		else
			contact_normal = sf::Vector2f(0.f, -1.f);
	}

	return true;
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(800, 600), "Collision");

	float playerX = 400.f;
	float playerY = 300.f;

	float speed = 100.f;

	sf::RectangleShape player;
	player.setPosition(playerX, playerY);
	player.setFillColor(sf::Color::Red);
	player.setSize(sf::Vector2f(100.f, 70.f));

	sf::RectangleShape rect;
	rect.setPosition(200.f, 200.f);
	rect.setFillColor(sf::Color::Green);
	rect.setSize(sf::Vector2f(20.f, 70.f));

	sf::Vector2f ray_point(20.f, 20.f);
	sf::Vector2f ray_direction;

	sf::Event event;
	sf::Clock clock;
	float dt;
	sf::VertexArray line(
		sf::Lines,
		2
	);
	line[0].position = ray_point;
	line[0].color = sf::Color::White;
	while (window.isOpen())
	{
		dt = clock.restart().asSeconds();
		playerX = 0.f, playerY = 0.f;

		sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
		ray_direction =
			sf::Vector2f(
				static_cast<float>(mousePosition.x),
				static_cast<float>(mousePosition.y)
			) - ray_point;

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
					window.close();
			}
		}

		sf::Vector2f cp, cn;
		float t = 0;

		if (RayVsRect(ray_point, ray_direction, player, cp, cn, t) && t < 1.0f)
		{
			player.setOutlineThickness(1.f);
			player.setOutlineColor(sf::Color::White);
		}
		else
		{
			player.setOutlineColor(sf::Color::Red);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			playerY = -1;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			playerY = 1;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			playerX = 1;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			playerX = -1;
		}

		line[1].position = ray_point + ray_direction;
		line[1].color = sf::Color::White;

		player.move(playerX * speed * dt, playerY * speed * dt);
		//rect.setPosition(mousePosition.x, mousePosition.y);

		window.clear();
		window.draw(line);
		window.draw(player);
		window.display();
	}

	return 0;
}