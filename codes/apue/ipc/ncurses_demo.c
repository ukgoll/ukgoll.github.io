#include <ncurses.h>

int main()
{
  initscr();            // 初始化 ncurses
  noecho();             // 禁止键盘输入自动打印
  cbreak();             // 立即响应按键，不缓冲
  curs_set(0);          // 隐藏光标
  keypad(stdscr, TRUE); // 启用方向键

  int height, width;
  getmaxyx(stdscr, height, width);

  int title_h = 3;
  int input_h = 3;
  int body_h = height - title_h - input_h;
  int left_w = width / 3;
  int right_w = width - left_w;

  // 创建窗口
  WINDOW *title_win = newwin(title_h, width, 0, 0);
  WINDOW *left_win = newwin(body_h, left_w, title_h, 0);
  WINDOW *right_win = newwin(body_h, right_w, title_h, left_w);
  WINDOW *input_win = newwin(input_h, width, height - input_h, 0);

  // 边框 & 标题
  box(title_win, 0, 0);
  mvwprintw(title_win, 1, (width - 20) / 2, "My Terminal Layout(%d, %d)", width, height);

  box(left_win, 0, 0);
  mvwprintw(left_win, 0, 2, " Left Panel ");

  box(right_win, 0, 0);
  mvwprintw(right_win, 0, 2, " Right Panel ");

  box(input_win, 0, 0);
  mvwprintw(input_win, 1, 2, "Input: ");

  // 刷新窗口
  wrefresh(title_win);
  wrefresh(left_win);
  wrefresh(right_win);
  wrefresh(input_win);

  getch(); // 等待输入后退出

  // 清理
  delwin(title_win);
  delwin(left_win);
  delwin(right_win);
  delwin(input_win);
  endwin();

  return 0;
}
