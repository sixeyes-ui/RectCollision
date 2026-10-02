#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <vector>


// ------------------------------------------------------------
// Check if a point is inside a rectangle
// ------------------------------------------------------------
bool isPointOnRect(
    const sf::RectangleShape& rect,
    const sf::RectangleShape& player)
{
    sf::Vector2f rectPos = rect.getPosition();
    sf::Vector2f playerPos = player.getPosition();

    return rectPos.x >= playerPos.x &&
        rectPos.y >= playerPos.y &&
        rectPos.x <= playerPos.x + player.getSize().x &&
        rectPos.y <= playerPos.y + player.getSize().y;
}


// ------------------------------------------------------------
// Rectangle vs Rectangle
// ------------------------------------------------------------
bool RectVsRect(
    const sf::RectangleShape& r1,
    const sf::RectangleShape& r2)
{
    sf::Vector2f rect1Pos = r1.getPosition();
    sf::Vector2f rect2Pos = r2.getPosition();

    return rect1Pos.x <= rect2Pos.x + r2.getSize().x &&
        rect1Pos.x + r1.getSize().x >= rect2Pos.x &&
        rect1Pos.y <= rect2Pos.y + r2.getSize().y &&
        rect1Pos.y + r1.getSize().y >= rect2Pos.y;
}


// ------------------------------------------------------------
// Ray vs Rectangle
// ------------------------------------------------------------
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


    // --------------------------------------------------------
    // Prevent division by zero
    // --------------------------------------------------------
    if (std::abs(ray_dir.x) < epsilon)
    {
        // Ray is almost vertical
        // We don't divide by ray_dir.x.
    }

    if (std::abs(ray_dir.y) < epsilon)
    {
        // Ray is almost horizontal
        // We don't divide by ray_dir.y.
    }


    // --------------------------------------------------------
    // Calculate intersection times
    // --------------------------------------------------------

    float t_nearX;
    float t_farX;

    float t_nearY;
    float t_farY;


    // X axis
    if (std::abs(ray_dir.x) < epsilon)
    {
        // Ray is parallel to X slabs.
        if (ray_origin.x < minX || ray_origin.x > maxX)
            return false;

        t_nearX = -INFINITY;
        t_farX = INFINITY;
    }
    else
    {
        t_nearX = (minX - ray_origin.x) / ray_dir.x;
        t_farX = (maxX - ray_origin.x) / ray_dir.x;
    }


    // Y axis
    if (std::abs(ray_dir.y) < epsilon)
    {
        // Ray is parallel to Y slabs.
        if (ray_origin.y < minY || ray_origin.y > maxY)
            return false;

        t_nearY = -INFINITY;
        t_farY = INFINITY;
    }
    else
    {
        t_nearY = (minY - ray_origin.y) / ray_dir.y;
        t_farY = (maxY - ray_origin.y) / ray_dir.y;
    }


    // Make sure near <= far
    if (t_nearX > t_farX)
        std::swap(t_nearX, t_farX);

    if (t_nearY > t_farY)
        std::swap(t_nearY, t_farY);


    // --------------------------------------------------------
    // Check if the X and Y intervals overlap
    // --------------------------------------------------------
    if (t_nearX > t_farY ||
        t_nearY > t_farX)
    {
        return false;
    }


    // --------------------------------------------------------
    // Collision enters rectangle at the largest near value
    // --------------------------------------------------------
    t_hit_near = std::max(t_nearX, t_nearY);

    float t_hit_far = std::min(t_farX, t_farY);


    // Rectangle is completely behind ray
    if (t_hit_far < 0.0f)
        return false;


    // --------------------------------------------------------
    // Find contact point
    // --------------------------------------------------------
    contact_point =
        ray_origin + ray_dir * t_hit_near;


    // --------------------------------------------------------
    // Find collision normal
    // --------------------------------------------------------

    if (t_nearX > t_nearY)
    {
        // Hit left/right side

        if (ray_dir.x < 0.0f)
            contact_normal = sf::Vector2f(1.0f, 0.0f);
        else
            contact_normal = sf::Vector2f(-1.0f, 0.0f);
    }
    else
    {
        // Hit top/bottom side

        if (ray_dir.y < 0.0f)
            contact_normal = sf::Vector2f(0.0f, 1.0f);
        else
            contact_normal = sf::Vector2f(0.0f, -1.0f);
    }

    return true;
}


// ------------------------------------------------------------
// Dynamic Rectangle vs Rectangle
// ------------------------------------------------------------
bool DynamicRectVsRect(
    const sf::RectangleShape& in,
    const sf::RectangleShape& target,
    sf::Vector2f& contact_point,
    sf::Vector2f& contact_normal,
    float& contact_time,
    float fElapsedTime,
    const sf::Vector2f& velocity)
{
    if (velocity.x == 0.0f &&
        velocity.y == 0.0f)
    {
        return false;
    }

    // ---------------------------------------------------------
    // Get the CENTER of the moving rectangle
    // ---------------------------------------------------------

    sf::Vector2f origin = in.getPosition();

    origin.x += in.getSize().x * 0.5f;
    origin.y += in.getSize().y * 0.5f;


    // ---------------------------------------------------------
    // Expand the target rectangle by HALF the size
    // of the moving rectangle.
    // ---------------------------------------------------------

    sf::Vector2f expandedPosition =
        target.getPosition();

    expandedPosition.x -= in.getSize().x * 0.5f;
    expandedPosition.y -= in.getSize().y * 0.5f;


    sf::Vector2f expandedSize =
        target.getSize();

    expandedSize.x += in.getSize().x;
    expandedSize.y += in.getSize().y;


    sf::RectangleShape expandedTarget;

    expandedTarget.setPosition(
        expandedPosition
    );

    expandedTarget.setSize(
        expandedSize
    );


    // ---------------------------------------------------------
    // Movement during this frame
    // ---------------------------------------------------------

    sf::Vector2f frameVelocity =
        velocity * fElapsedTime;


    // ---------------------------------------------------------
    // Cast the movement ray
    // ---------------------------------------------------------

    if (!RayVsRect(
        origin,
        frameVelocity,
        expandedTarget,
        contact_point,
        contact_normal,
        contact_time))
    {
        return false;
    }


    // ---------------------------------------------------------
    // Collision must happen during this frame.
    //
    // t = 0     -> collision immediately
    // t = 0.5   -> collision halfway through movement
    // t = 1     -> collision at the end
    // ---------------------------------------------------------

    if (contact_time < 0.0f ||
        contact_time > 1.0f)
    {
        return false;
    }


    return true;
}


// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main()
{
    sf::RenderWindow window(
        sf::VideoMode(800, 600),
        "Collision"
    );

    window.setFramerateLimit(60);


    // --------------------------------------------------------
    // Player
    // --------------------------------------------------------

    sf::RectangleShape player;

    player.setPosition(
        100.0f,
        100.0f
    );

    player.setSize(
        sf::Vector2f(50.0f, 50.0f)
    );

    player.setFillColor(
        sf::Color::Red
    );


    // --------------------------------------------------------
    // Static rectangles
    // --------------------------------------------------------

    std::vector<sf::RectangleShape> vRects;


    sf::RectangleShape rect1;

    rect1.setPosition(
        400.0f,
        500.0f
    );

    rect1.setSize(
        sf::Vector2f(100.0f, 100.0f)
    );

    rect1.setFillColor(
        sf::Color::Green
    );

    vRects.push_back(rect1);

    sf::RectangleShape rect3;

    rect3.setPosition(
        rect1.getPosition().x - rect1.getSize().x,
        500.0f
    );

    rect3.setSize(
        sf::Vector2f(100.0f, 100.0f)
    );

    rect3.setFillColor(
        sf::Color::Red
    );

    vRects.push_back(rect3);

    sf::RectangleShape rect4;

    rect4.setPosition(
        rect3.getPosition().x - rect1.getSize().x,
        500.0f
    );

    rect4.setSize(
        sf::Vector2f(100.0f, 100.0f)
    );

    rect4.setFillColor(
        sf::Color::Cyan
    );

    vRects.push_back(rect4);


    sf::RectangleShape rect2;

    rect2.setPosition(
        500.0f,
        250.0f
    );

    rect2.setSize(
        sf::Vector2f(100.0f, 50.0f)
    );

    rect2.setFillColor(
        sf::Color::Green
    );

    vRects.push_back(rect2);


    // --------------------------------------------------------
    // Movement
    // --------------------------------------------------------

    const float speed = 200.0f;

    sf::Clock clock;

    sf::Event event;


    // --------------------------------------------------------
    // Collision debug information
    // --------------------------------------------------------

    sf::Vector2f contact_point;
    sf::Vector2f contact_normal;

    float contact_time = 0.0f;


    // --------------------------------------------------------
    // Game loop
    // --------------------------------------------------------

    while (window.isOpen())
    {
        float dt =
            clock.restart().asSeconds();


        // ----------------------------------------------------
        // Events
        // ----------------------------------------------------

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
            }
        }

            // ---------------------------------------------------------
            // Get player input
            // ---------------------------------------------------------

            sf::Vector2f velocity(0.0f, 0.0f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            velocity.y -= 1.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            velocity.y += 1.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            velocity.x -= 1.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            velocity.x += 1.0f;


        // ---------------------------------------------------------
        // Normalize diagonal movement
        // ---------------------------------------------------------

        float length =
            std::sqrt(
                velocity.x * velocity.x +
                velocity.y * velocity.y
            );

        if (length > 0.0f)
        {
            velocity /= length;
        }


        // ---------------------------------------------------------
        // Convert direction into pixels per second
        // ---------------------------------------------------------

        const float speed = 200.0f;

        velocity *= speed;


        // ---------------------------------------------------------
        // Collision detection
        // ---------------------------------------------------------

        sf::Vector2f contactPoint;
        sf::Vector2f contactNormal;

        float contactTime = 1.0f;

        bool collision = false;


        // Find the earliest collision
        for (const auto& rect : vRects)
        {
            float hitTime;
            sf::Vector2f hitPoint;
            sf::Vector2f hitNormal;

            if (DynamicRectVsRect(
                player,
                rect,
                hitPoint,
                hitNormal,
                hitTime,
                dt,
                velocity))
            {
                if (!collision || hitTime < contactTime)
                {
                    collision = true;

                    contactTime = hitTime;
                    contactPoint = hitPoint;
                    contactNormal = hitNormal;
                }
            }
        }


        // ---------------------------------------------------------
        // Move player
        // ---------------------------------------------------------

        sf::Vector2f movement =
            velocity * dt;


        // ---------------------------------------------------------
        // Collision response
        // ---------------------------------------------------------

        if (collision)
        {
            // Move only until we reach the collision
            // Leave a tiny amount of space so we don't
            // become stuck inside the wall.

            float allowedTime =
                std::max(0.0f, contactTime - 0.001f);

            movement =
                velocity * dt * allowedTime;


            // -----------------------------------------------------
            // Wall sliding
            //
            // Remove the part of the velocity going INTO
            // the wall.
            // -----------------------------------------------------

            float velocityIntoWall =
                velocity.x * contactNormal.x +
                velocity.y * contactNormal.y;


            if (velocityIntoWall < 0.0f)
            {
                velocity.x -=
                    contactNormal.x * velocityIntoWall;

                velocity.y -=
                    contactNormal.y * velocityIntoWall;
            }
        }


        // ---------------------------------------------------------
        // Move player
        // ---------------------------------------------------------

        player.move(movement);


        // ----------------------------------------------------
        // Change player color when colliding
        // ----------------------------------------------------

        if (collision)
            player.setFillColor(sf::Color::Yellow);
        else
            player.setFillColor(sf::Color::Red);


        // ----------------------------------------------------
        // Draw
        // ----------------------------------------------------

        window.clear(
            sf::Color::Black
        );


        for (const auto& rect : vRects)
        {
            window.draw(rect);
        }


        window.draw(player);


        window.display();
    }


    return 0;
}