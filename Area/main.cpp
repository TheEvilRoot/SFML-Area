#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <sstream>
#include "ResourcePath.hpp"

using namespace sf;

struct {
  std::unique_ptr<VideoMode> videoMode;
  std::unique_ptr<RenderWindow> window;
  ContextSettings *settings;
  Font font;
  Color backgroundColor;
  Vector2f origin;
  Vector2f size;
  float scale;
  float originStep;
  float scaleStep;
  float maxFps;
  bool vSync;
  bool overlay;
  bool originWindowBox;
  
  void setMaxFps(float mfps) {
    maxFps = mfps;
    if (window) window->setFramerateLimit(maxFps);
  }
  
  void setVSync(bool vsync) {
    vSync = vsync;
    if (window) window->setVerticalSyncEnabled(vSync);
  }
} context;

Transform adjustOrigin(std::unique_ptr<RenderWindow>& window, Vector2f &origin, float &scale) {
  Transform trf = Transform::Identity;
  trf.translate(origin.x, origin.y);
  trf.scale(scale, scale);
  
  return trf;
}

void drawBackground(std::unique_ptr<RenderWindow>& win) {
  RectangleShape background(Vector2f(win->getSize().x, win->getSize().y));
  background.setFillColor(context.backgroundColor);
  background.setPosition(0, 0);
  
  win->draw(background);
}

void drawForeground(std::unique_ptr<RenderWindow>& window, Transform &transform) {
  CircleShape circle(20);
  circle.setPosition(0, 0);
  circle.setFillColor(Color::Red);

  window->draw(circle, transform);
}

void drawOriginWindowBox(std::unique_ptr<RenderWindow>& window, Transform &transform) {
  Vertex verteces[5] = {
    Vertex(Vector2f(0, 0), Color::Green),
    Vertex(Vector2f(window->getSize().x, 0), Color::Green),
    Vertex(Vector2f(window->getSize().x, window->getSize().y), Color::Green),
    Vertex(Vector2f(0, window->getSize().y), Color::Green),
    Vertex(Vector2f(0, 0), Color::Green)
  };
  
  window->draw(verteces, 5, PrimitiveType::LineStrip, transform);
}

void moveOrigin(int direction) {
  if (direction & 1) { // left
    if (context.size.x - context.window->getSize().x > context.origin.x) {
      context.origin.x += context.originStep;
    }
  }
  if (direction & 2) { // top
    if (context.size.y - context.window->getSize().y > context.origin.y) {
      context.origin.y += context.originStep;
    }
  }
  if (direction & 4) { // right
    if (context.origin.x > 0) {
      context.origin.x -= context.originStep;
    }
  }
  if (direction & 8) { // down
    if (context.origin.y > 0) {
      context.origin.y -= context.originStep;
    }
  }
  
  std::cout << "Origin: " << context.origin.x << " " << context.origin.y << "\n";
}

void setScale(int mod) {
  float delta = mod * context.scaleStep;
  float newScale = context.scale + delta;
  if (newScale <= 0.1) newScale = 0.1;
  if (newScale >= 4) newScale = 4;
  
  context.scale = newScale;
 
  std::cout << "Scale: " << context.scale << "\n";
}

void resetView(bool resetScale = false, bool resetOrigin = false) {
  if (resetScale) context.scale = 1;
  if (resetOrigin) {
    context.origin.x = 0;
    context.origin.y = 0;
  }
}

void dispatchEvent(Event &event) {
  if (event.type == Event::EventType::KeyPressed) {
    switch(event.key.code) {
      case Keyboard::Right: {
        moveOrigin(4);
        break;
      }
      case Keyboard::Left: {
        moveOrigin(1);
        break;
      }
      case Keyboard::Up: {
        moveOrigin(2);
        break;
      }
      case Keyboard::Down: {
        moveOrigin(8);
        break;
      }
      case Keyboard::Add: case Keyboard::PageUp: {
        setScale(1);
        break;
      }
      case Keyboard::Subtract: case Keyboard::PageDown: {
        setScale(-1);
        break;
      }
      case Keyboard::O: {
        context.overlay = !context.overlay;
        break;
      }
      case Keyboard::B: {
        context.originWindowBox = !context.originWindowBox;
        break;
      }
      case Keyboard::R: {
        resetView(true, true);
        break;
      }
    }
  }
}

void drawOverlay(std::unique_ptr<RenderWindow>& win) {
  std::ostringstream text;
  text << "originX: " << context.origin.x << " originY: " << context.origin.y;
  Text originText(text.str(), context.font, 12);
  originText.setFillColor(Color::White);
  text.str("");
  text.clear();
  
  text << "scale: " << context.scale;
  Text scaleText(text.str(), context.font, 12);
  scaleText.setFillColor(Color::White);
  text.str("");
  text.clear();
  
  text << "mouseX: " << Mouse::getPosition(*win).x - context.origin.x << " mouseY: " << Mouse::getPosition(*win).y - context.origin.y;
  Text mouseText(text.str(), context.font, 12);
  mouseText.setFillColor(Color::White);
  text.str("");
  text.clear();
  
  int y = 0;
  int x = 1;
  originText.setPosition(x, y++ * 12);
  scaleText.setPosition(x, y++ * 12);
  mouseText.setPosition(x, y++ * 12);
  
  win->draw(originText);
  win->draw(scaleText);
  win->draw(mouseText);
}

void mainLoop(std::unique_ptr<RenderWindow>& win) {
  while (win->isOpen()) {
    sf::Event event;
    while (win->pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        win->close();
      }
      if(event.type == sf::Event::Resized){
        win->setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
      }
      dispatchEvent(event);
    }
    win->clear();
    drawBackground(context.window);
    Transform transform = adjustOrigin(context.window, context.origin, context.scale);
    
    if (context.originWindowBox) drawOriginWindowBox(context.window, transform);
    drawForeground(context.window, transform);
    if (context.overlay) drawOverlay(context.window);
    win->display();
  }
}

int main(int, char const**) {
  context.videoMode = std::unique_ptr<VideoMode>(new VideoMode(800, 600));
  context.settings = new ContextSettings(2,0, 20);
  context.window = std::unique_ptr<RenderWindow>(new RenderWindow(*context.videoMode, "SFML window", Style::Default, *context.settings));
  context.font.loadFromFile(resourcePath() + "ubuntu.ttf");
  context.backgroundColor = Color::Black;
  context.origin = Vector2f(0, 0);
  context.size = Vector2f(5000, 5000);
  context.scale = 1;
  context.originStep = 10;
  context.scaleStep = 0.1;
  context.originWindowBox = false;
  context.overlay = true;
  context.setVSync(true);
  context.setMaxFps(60);
  
  mainLoop(context.window);
  
  return EXIT_SUCCESS;
}
