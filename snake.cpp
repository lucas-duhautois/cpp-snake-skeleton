#include <iostream>
#include <vector>
#include <utility>
#include <array>
#include <thread>
#include "sys/ioctl.h"
#include "termios.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"

namespace internal
{
  // 💀💀💀💀💀💀💀💀 DON'T TOUCH THIS BLOCK 💀💀💀💀💀💀💀💀💀 ///
  int keyEvent();
  void frameSleep(const int &ms);
  const char *cmd_clear = "clear";
  void backgroundClear();
  void printFrame(const int &nx, const int &ny, const std::vector<int> &bg);
  void createFood(std::vector<int> &bg, std::array<int, 2> &food, const int &nx, const int &ny);
  bool eatFood(std::array<int, 2> &food, std::vector<std::pair<int, int>> &snake);
  // 💀💀💀💀💀💀💀💀 DON'T TOUCH THIS BLOCK 💀💀💀💀💀💀💀💀💀 ///
}

// 😇 Je suis gentil je vous ai fait le tracé du terrain 😇
std::vector<int> backgroundSetup(const int &nx, const int &ny)
{
  std::vector<int> bg(nx * ny, 0);
  int i;
  for (i = 0; i < nx; i++)
  {
    bg[i] = 1;
  }
  for (i = 0; i < nx; i++)
  {
    bg[(ny - 1) * nx + i] = 1;
  }
  for (i = 0; i < ny; i++)
  {
    bg[i * nx] = 1;
  }
  for (i = 0; i < ny; i++)
  {
    bg[i * nx + nx - 1] = 1;
  }

  return bg;
}

///////////////////////////////////////////////////////////////////////////////
/////// 🧑‍💻 FROM HERE you must complete 🧑‍💻 ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void add_snake(const std::vector<std::pair<int, int>> &snake, std::vector<int> &bg, int nx, int ny)
{
  for (auto &c : snake) {
    int index = c.second * nx + c.first ;
    bg[index] = 3;
  }
}

void remove_snake(const std::vector<std::pair<int, int>> &snake, std::vector<int> &bg, int nx, int ny)
{
  for (auto &c : snake) {
    int index = c.second * nx + c.first ;
    bg[index] = 0;
  }
}

std::array<int, 2> snake_movement(char key)
{
  std::array<int, 2> dxdy = {0, 0};
  if (key == 'z') {
    dxdy = {0, -1};
  }
  else if (key == 'q') {
    dxdy = {-1, 0};
  }
  else if (key == 's') {
    dxdy = {0, 1};
  }
  else if (key == 'd') {
    dxdy = {1, 0};
  }
  return dxdy;
}

bool verifyBorder(std::vector<std::pair<int, int>> &snake, int nx, int ny)
{
  std::pair<int, int> head = snake[0];
  for (int i = 1; i < snake.size()-1; i++) {
    if (head == snake[i]) {
      return false;
    }
  }
  return true;
}

std::vector<std::pair<int, int>> setupSnake(int snake_len)
{
  std::vector<std::pair<int, int>> snake(snake_len);
  int i;
  for (i = 0; i < snake_len; i++)
  {
    snake[i].first = 10 + i;
    snake[i].second = 10;
  }
  return snake;
}

void update_snake_coordinates(std::vector<std::pair<int, int>> &snake, bool eat, std::array<int, 2> dxdy, int nx, int ny)
{
  int size = snake.size();
  if (eat){
    snake.push_back(snake[size-1]);
  }
  for (int i = size-1; i > 0; i--) {
    snake[i] = snake[i-1];
  }
  snake[0].first += dxdy[0];
  if (snake[0].first <= 0) {
    snake[0].first = nx - 2;
  }
  else if (snake[0].first >= nx - 1) {
    snake[0].first = 1;
  }
  snake[0].second += dxdy[1];
  if (snake[0].second <= 0) {
    snake[0].second = ny - 2;
  }
  else if (snake[0].second >= ny - 1) {
    snake[0].second = 1;
  }
}

void startGame(int &lap, const int &nx, const int &ny, std::vector<std::pair<int, int>> &snake, std::vector<int> &bg, const int &lapMin, const int &lapRate)
{
  int timer = 0;
  int score = 0;
  char key;
  std::array<int, 2> dxdy = {1, 0};
  std::array<int, 2> food = {0, 0};
  internal::createFood(bg, food, nx, ny);
  while (true)
  {
    internal::frameSleep(lap);
    if (internal::keyEvent())
    {
      std::cin >> key;
      dxdy = snake_movement(key);
    }
    internal::backgroundClear();
    add_snake(snake, bg, nx, ny);
    internal::printFrame(nx, ny, bg);
    remove_snake(snake, bg, nx, ny);
    bool out = verifyBorder(snake, nx, ny);
    if (out == false)
    {
      std::cerr << "💀💀💀 Game Over 💀💀💀" << std::endl;
      std::cerr << "Your score : " << score << std::endl;
      exit(1);
    }
    bool eat = internal::eatFood(food, snake);
    if (eat)
    {
      internal::createFood(bg, food, nx, ny);
      score++;
    }
    update_snake_coordinates(snake, eat, dxdy, nx, ny);
    if (lap > lapMin) {
      if (timer == lapRate) {
      lap = lap - 1;
      timer = 0;
    }
    std::cout << "Actual lap : " << lap << std::endl;
    timer++;
    }
  }
}

int main()
{
  const int nx = 50;
  const int ny = 25;
  int lap = 200;
  int snake_len = 3;
  const int lapMin = 50;
  const int lapRate = 5;

  std::vector<int> background = backgroundSetup(nx, ny);
  internal::printFrame(nx, ny, background);

  std::vector<std::pair<int, int>> snake = setupSnake(snake_len);

  startGame(lap, nx, ny, snake, background, lapMin, lapRate);
  return 0;
}

// 💀💀💀💀💀💀💀💀 DON'T TOUCH CODE AFTER THIS LINE 💀💀💀💀💀💀💀💀💀💀 ///

namespace internal
{

  static const int STDIN = 0;
  static bool initialized = false;

  // Comment ca va

  int keyEvent()
  {
    if (!initialized)
    {
      termios term;
      tcgetattr(STDIN, &term);
      term.c_lflag &= ~ICANON;
      tcsetattr(STDIN, TCSANOW, &term);
      setbuf(stdin, NULL);
      initialized = true;
    }
    int bytesWaiting;
    // int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
  }

  void frameSleep(const int &ms)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }

  /////////////

  void backgroundClear()
  {
    int out = system(cmd_clear);
    if (out != 0)
    {
      std::cerr << "clear command failed" << std::endl;
      exit(1);
    }
  }

  void printFrame(const int &nx, const int &ny, const std::vector<int> &bg)
  {
    for (int j = 0; j < ny; j++)
    {
      for (int i = 0; i < nx; i++)
      {
        if (bg[i + j * nx] == 1)
        {
          std::cout << "#";
        }
        else if (bg[i + j * nx] == 2)
        {
          std::cout << "@";
        }
        else if (bg[i + j * nx] == 3)
        {
          std::cout << "o";
        }
        else
        {
          std::cout << " ";
        }
      }
      std::cout << std::endl;
    }
  }

  void createFood(std::vector<int> &bg, std::array<int, 2> &food, const int &nx, const int &ny)
  {
    if (food[0] == 0)
    {
      food[0] = rand() % (nx - 2) + 2;
      food[1] = rand() % (ny - 2) + 2;
      bg[food[1] * nx + food[0]] = 2;
    }
  }

  bool eatFood(std::array<int, 2> &food, std::vector<std::pair<int, int>> &snake)
  {
    if ((food[0] == std::get<0>(snake[0])) && (food[1] == std::get<1>(snake[0])))
    {
      food[0] = 0;
      food[1] = 0;
      return true;
    }
    return false;
  }

}
