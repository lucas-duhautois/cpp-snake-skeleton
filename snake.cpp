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

class board {
  public :
  const int nx;
  const int ny;
  std::vector<int> bg;

  board (const int nx, const int ny) : nx{nx}, ny{ny}, bg(nx*ny, 0)
  {
  }

  ~board() = default;

  void setup () {
    int i;
    for (i = 0; i < nx; i++){
      bg[i] = 1;
    }
    for (i = 0; i < nx; i++){
      bg[(ny - 1) * nx + i] = 1;
    }
    for (i = 0; i < ny; i++){
      bg[i * nx] = 1;
    }
    for (i = 0; i < ny; i++){
      bg[i * nx + nx - 1] = 1;
    }
  }

  void clear () {
    int out = system("clear");
    if (out != 0)
    {
      std::cerr << "clear command failed" << std::endl;
      exit(1);
    }
  }

  void print() {
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

  void sleep (const int &ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }
};

class snake {
  public:
  std::pair<int, int> head;
  std::vector<std::pair<int, int>> body;

  snake (const int snake_len) {
    head = std::make_pair(10, 10);
    for (int i = 1; i < snake_len; i++){
      body.push_back(std::make_pair(10+i, 10));
    }
  }

  ~snake () = default;

  void add (board &bg) {
    bg.bg[head.second * bg.nx + head.first] = 3;
    for (auto &c : body) {
      int index = c.second * bg.nx + c.first ;
      bg.bg[index] = 3;
    }
  }

  void remove (board &bg) {
    bg.bg[head.second * bg.nx + head.first] = 0;
    for (auto &c : body) {
      int index = c.second * bg.nx + c.first ;
      bg.bg[index] = 0;
    }
  }

  bool verify () {
    for (auto &c : body) {
      if (head == c) {
        return false;
      } 
    }
    return true;
  }

  void move (char key, bool eat, const board &bg) {
    int size = body.size();
    if (eat){
      body.push_back(body[size-1]);
    }
    for (int i = size-1; i > 0; i--) {
      body[i] = body[i-1];
    }
    body[0] = head;

    if (key == 'z') {
      head.second--;
    }
    else if (key == 'q') {
      head.first--;
    }
    else if (key == 's') {
      head.second++;
    }
    else if (key == 'd') {
      head.first++;
    }
    
    if (head.first <= 0) {
      head.first = bg.nx - 2;
    }
    else if (head.first >= bg.nx - 1) {
      head.first = 1;
    }

    if (head.second <= 0) {
      head.second = bg.ny - 2;
    }
    else if (head.second >= bg.ny - 1) {
      head.second = 1;
    }
  }
};

class food {
  public:
  std::array<int, 2> apple;

  food () : apple {0, 0}
  {
  }

  ~food() = default;

  void renew (const snake &player, const board &bg) {
    bool test = true;
    while (test) {
      apple[0] = rand() % (bg.nx - 2) + 2;
      apple[1] = rand() % (bg.ny - 2) + 2;
      test = false;
      for (auto &c : player.body) {
        if (~test & ((c.first == apple[0]) & (c.second == apple[1]))) {
          test = true;
        }
      }
      if (~test & ((player.head.first == apple[0]) & (player.head.second == apple[1]))) {
        test = true;
      }
    }
  }

  void print (board &bg) {
    bg.bg[apple[1] * bg.nx + apple[0]] = 2;
  }

  bool eat (const snake &player) {
    if ((apple[0] == std::get<0>(player.body[0])) && (apple[1] == std::get<1>(player.body[0]))){
      return true;
    }
    if ((apple[0] == player.head.first) && (apple[1] == player.head.second)){
      return true;
    }
    return false;
  }
};

///////////////////////////////////////////////////////////////////////////////
/////// 🧑‍💻 FROM HERE you must complete 🧑‍💻 ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static const int STDIN = 0;
static bool initialized = false;

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

void startGame(const int &snake_len, const int &nx, const int &ny, int &lap, const int &lapMin, const int &lapRate)
{
  board background (nx, ny);
  snake player (snake_len);
  food yum;

  background.setup();
  background.print();

  int timer = 0;
  int score = 0;
  char key = 'd';

  yum.renew(player, background);
  yum.print(background);
  while (true)
  {
    background.sleep(lap);
    if (keyEvent())
    {
      std::cin >> key;
    }
    background.clear();
    player.add(background);
    background.print();
    player.remove(background);
    bool out = player.verify();
    if ((out == false) & (score > 0))
    {
      std::cerr << "💀💀💀 Game Over 💀💀💀" << std::endl;
      std::cerr << "Your score : " << score << std::endl;
      exit(1);
    }
    bool eat = yum.eat(player);
    if (eat)
    {
      yum.renew(player, background);
      yum.print(background);
      score++;
    }
    player.move(key, eat, background);
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

  startGame(snake_len, nx, ny, lap, lapMin, lapRate);
  return 0;
}