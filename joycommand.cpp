#include <SDL.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <boost/variant.hpp>
#include <yaml-cpp/yaml.h>

using namespace std::string_literals;

struct Button
{
  SDL_GameControllerButton button;
};

struct Axis
{
  SDL_GameControllerAxis axis;
};

using PadInput = boost::variant<Button, Axis>;

struct Mapping
{
  std::vector<PadInput> inputs;
  std::string command;
};

PadInput stringToPadInput(std::string const& s)
{
  if (s == "a") return Button{SDL_CONTROLLER_BUTTON_A};
  if (s == "b") return Button{SDL_CONTROLLER_BUTTON_B};
  if (s == "x") return Button{SDL_CONTROLLER_BUTTON_X};
  if (s == "y") return Button{SDL_CONTROLLER_BUTTON_Y};
  if (s == "leftshoulder") return Button{SDL_CONTROLLER_BUTTON_LEFTSHOULDER};
  if (s == "lefttrigger") return Axis{SDL_CONTROLLER_AXIS_TRIGGERLEFT};
  if (s == "rightshoulder") return Button{SDL_CONTROLLER_BUTTON_RIGHTSHOULDER};
  if (s == "righttrigger") return Axis{SDL_CONTROLLER_AXIS_TRIGGERRIGHT};
  if (s == "left") return Button{SDL_CONTROLLER_BUTTON_DPAD_LEFT};
  if (s == "right") return Button{SDL_CONTROLLER_BUTTON_DPAD_RIGHT};
  if (s == "up") return Button{SDL_CONTROLLER_BUTTON_DPAD_UP};
  if (s == "down") return Button{SDL_CONTROLLER_BUTTON_DPAD_DOWN};

  throw std::runtime_error("unknown button: " + s);
}

using Mappings = std::vector<Mapping>;

Mappings parse_config()
{
  Mappings mappings;

  YAML::Node config = YAML::LoadFile("config.yaml");

  for (auto const item : config)
  {
    Mapping mapping;
    for (auto const button : item["buttons"])
      mapping.inputs.push_back(stringToPadInput(button.as<std::string>()));
    mapping.command = item["command"].as<std::string>();
    mappings.push_back(mapping);
  }

  return mappings;
}

struct DeviceCloser
{
  void operator()(SDL_GameController* controller)
  {
    std::cout << "closed controller " << SDL_GameControllerName(controller) << std::endl;
    SDL_GameControllerClose(controller);
  }
};

std::map<SDL_JoystickID, std::unique_ptr<SDL_GameController, DeviceCloser>> g_gameController;

void open_controller(int index)
{
  auto const controller = SDL_GameControllerOpen(index);
  if (controller == nullptr)
  {
    std::cout << "Warning: unable to open game controller: " << SDL_GetError() << std::endl;
  }
  else
  {
    g_gameController[SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))].reset(controller);
    std::cout << "opened controller " << SDL_GameControllerName(controller) << std::endl;
  }
}

void init()
{
  if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
    throw std::runtime_error("SDL could not initialize: "s + SDL_GetError());
}

void close()
{
  g_gameController.clear();

  SDL_Quit();
}

void main_loop()
{
  auto const mappings = parse_config();

  SDL_Event e;

  while (SDL_WaitEvent(&e) != 0)
  {
    if (e.type == SDL_QUIT)
      break;

    switch (e.type) {
    //case SDL_CONTROLLERAXISMOTION:
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
      for (auto const& controller : g_gameController)
      {
        bool ok;
        for (auto const& mapping : mappings)
        {
          ok = true;
          for (auto const& input : mapping.inputs)
          {
            if (auto const button = boost::get<Button>(&input))
              if (!SDL_GameControllerGetButton(controller.second.get(), button->button))
              {
                ok = false;
                break;
              }
            if (auto const axis = boost::get<Axis>(&input))
              if (SDL_GameControllerGetAxis(controller.second.get(), axis->axis) == 0)
              {
                ok = false;
                break;
              }
          }
          if (ok)
          {
            system(mapping.command.c_str());
            break;
          }
        }
        if (ok)
          break;
      }
      break;
    case SDL_CONTROLLERDEVICEADDED:
      open_controller(e.cdevice.which);
      break;
    case SDL_CONTROLLERDEVICEREMOVED:
      g_gameController.erase(e.cdevice.which);
      break;
    //case SDL_CONTROLLERDEVICEREMAPPED:
      ;
    }
  }
}

int main(int argc, char* args[])
{
  init();

  main_loop();

  close();

  return 0;
}
