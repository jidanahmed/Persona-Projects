#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <termios.h>
#include <fcntl.h>

#define WIDTH 201
#define HEIGHT 101

char screen[HEIGHT][WIDTH];

/* setup */
void setup() {
  printf("\033[48;5;0m");     // background black
  printf("\033[38;5;255m");   // foreground white
  printf("\033[?1049h");      // alternate screen buffer mode
}

/* fillBackground */
void fillBackground(char c) {
  for (int y = 0; y<HEIGHT; y++) {
    for (int x = 0; x<WIDTH; x++) {
      screen[y][x] = c;
    }
  }
}

/* fillAxes */
void fillAxes() {
  for (int y = 0; y<HEIGHT; y++) {
    for (int x = 0; x<WIDTH; x++) {
      if (x == WIDTH/2) screen[y][x] = '|';
      if (y == HEIGHT/2) screen[y][x] = '-';
      if (x == WIDTH/2 && y == HEIGHT/2) screen[y][x] = '+';
    }
  }
}

/* fillBorder */
void fillBorder() {
  for (int y = 0; y<HEIGHT; y++) {
    for (int x = 0; x<WIDTH; x++) {
      if (x == WIDTH-1 || x == 0) screen[y][x] = '#';
      if (y == HEIGHT-1 || y == 0) screen[y][x] = '=';
      if (x == WIDTH/2 && y == HEIGHT/2) screen[y][x] = '+';
    }
  }
}

/* render */
void render() {
  printf("\033[2J");  // clear
  printf("\033[H");   // home

  for (int y = 0; y < HEIGHT; y++) {
    printf("%d",y%10);
    for (int x = 0; x < WIDTH; x++) {
      // TODO: add switch case here for colors
      printf(" %c",screen[y][x]);
      // change color back to normal
    }
    printf("\n");
  }
}

/* struct vector */
struct vector {
    float x;
    float y;
    float z;
};

/* setVector */
void setVector(struct vector * v, int x, int y, int z) {
  v->x = x;
  v->y = y;
  v->z = z;
}

struct vector camera = {0,0,0};
float camera_pitch = 0;  // rotation around x-axis (look up/down)
float camera_yaw = 0;    // rotation around y-axis (look left/right)
float camera_roll = 0;   // rotation around z-axis (tilt head)
int focal_length = 90;

/* add */
void add(struct vector * v1, struct vector v2) {
  v1->x += v2.x;
  v1->y += v2.y;
  v1->z += v2.z;
}

/* subtract */
void subtract(struct vector * v1, struct vector v2) {
  v1->x -= v2.x;
  v1->y -= v2.y;
  v1->z -= v2.z;
}

/* rotate */
void rotate(struct vector * v, float ax, float ay, float az) {
  float x = v->x;
  float y = v->y;
  float z = v->z;

  float y1 = y * cos(ax) - z * sin(ax);
  float z1 = y * sin(ax) + z * cos(ax);

  float x1 = x * cos(ay) + z1 * sin(ay);
  float z2 = -x * sin(ay) + z1 * cos(ay);

  float x2 = x1 * cos(az) - y1 * sin(az);
  float y2 = x1 * sin(az) + y1 * cos(az);

  v->x = x2;
  v->y = y2;
  v->z = z2;
}

/* point */
void point(int x, int y, char c) {
  if (x < -WIDTH/2 || x > WIDTH/2 || y < -HEIGHT/2 || y > HEIGHT/2) return;

  x += WIDTH/2;
  y += HEIGHT/2;

  screen[HEIGHT-1-y][x] = c;
}

/* point3D */
void point3D(float x, float y, float z, char c) {
  if (z == 0) z = 0.01;;
  x += camera.x;
  y += camera.y;
  z += camera.z;
  x = (x*focal_length) / (z+focal_length);
  y = (y*focal_length) / (z+focal_length);

  point((int)x,(int)y,c);
}

/* point3Dv */
void point3Dv(struct vector v, char c) {
  point3D(v.x,v.y,v.z,c);
}

/* line */
void line(int x1, int y1, int x2, int y2, char c) {
  // uses that one equation
  int dx = x2 - x1;
  int dy = y2 - y1;

  int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);

  for (int i = 0; i <= steps; i++) {
    int x = x1 + i * dx / steps;
    int y = y1 + i * dy / steps;

    point(x,y,c);
  }
}

/* line3D */
void line3D(float x1, float y1, float z1, float x2, float y2, float z2, char c) {
  if (z1+focal_length + camera.z <= 0 || z2+focal_length + camera.z <= 0) return;

  if (z1 == 0) z1 = 0.01;
  if (z2 == 0) z2 = 0.01;

  x1 += camera.x;
  y1 += camera.y;
  z1 += camera.z;

  x2 += camera.x;
  y2 += camera.y;
  z2 += camera.z;


  x1 = (x1*focal_length) / (z1+focal_length);
  y1 = (y1*focal_length) / (z1+focal_length);
  x2 = (x2 *focal_length) / (z2 +focal_length);
  y2 = (y2 *focal_length) / (z2 +focal_length);

  struct vector v1 = {x1,y1,z1};
  struct vector v2 = {x2,y2,z2};
  // rotate(&v1, camera_pitch, camera_yaw, camera_roll);
  // rotate(&v1, camera_pitch, camera_yaw, camera_roll);

  line((int)v1.x,(int)v1.y,(int)v2.x,(int)v2.y,c);
}

// /* line3D */
// void line3D(float x1, float y1, float z1, float x2, float y2, float z2, char c) {
//   struct vector v1 = {x1, y1, z1};
//   struct vector v2 = {x2, y2, z2};
  
//   // rotate
//   rotate(&v1, -camera_pitch, -camera_yaw, -camera_roll);
//   rotate(&v2, -camera_pitch, -camera_yaw, -camera_roll);

//   // translate
//   v1.z /= focal_length;
//   v1.x += camera.x;
//   v1.y += camera.y;
//   v1.z += camera.z;

//   v2.z /= focal_length;
//   v2.x += camera.x;
//   v2.y += camera.y;
//   v2.z += camera.z;
  
//   // project
//   if (v1.z <= 0 || v2.z <= 0) return; // Behind camera

//   if (v1.z == 0) v1.z = 0.01;
//   if (v2.z == 0) v2.z = 0.01;
//   v1.x /= v1.z;
//   v1.y /= v1.z;
//   v2.x /= v2.z;
//   v2.y /= v2.z;

//   line((int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, c);
// }


/* line3Dv */
void line3Dv(struct vector v1, struct vector v2, char c) {
  line3D(v1.x,v1.y,v1.z,v2.x,v2.y,v2.z,c);
}

/* termios stuff */
struct termios orig_term;
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_term);
    struct termios raw = orig_term;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
}
int kbhit() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
}
int getch() {
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1) return c;
    return 0;
}
void handleKeyboard() {
  if (kbhit()) {
    char c = getch();
    if (c == 'w') camera.z -= 1;
    if (c == 's') camera.z += 1;
    if (c == 'a') camera.x += 1;
    if (c == 'd') camera.x -= 1;
    if (c == 'i') camera_pitch -= 0.1;
    if (c == 'k') camera_pitch += 0.1;
    if (c == 'j') camera_yaw -= 0.1;
    if (c == 'l') camera_yaw += 0.1;
    if (c == ' ') camera.y -= 1;
    if (c == 'A') camera.y += 1;
    if (c == 'o') focal_length+=10;
    if (c == 'i') focal_length-=10;
    if (c == 27) exit(0); // ESC
  }
}

void printStuff() {
  printf("camera pos: %f, %f, %f\n", camera.x, camera.y, camera.z);
  printf("camera ang: %f, %f, %f\n", camera_pitch, camera_yaw, camera_roll);
  printf("camera foc: %d\n", focal_length);
}

/* main */
int main() {
  setup();
  enableRawMode();

  struct vector cube[8] = {
      {  10,  10, -10  },
      { -10,  10, -10  },
      { -10, -10, -10  },
      {  10, -10, -10  },
      {  10,  10,  10  },
      { -10,  10,  10  },
      { -10, -10,  10  },
      {  10, -10,  10  }
  };

  while (1) {
    fillBackground(' ');


    // 3d axes
    line3D(-WIDTH/2,0,0, WIDTH/2,0,0,'.');
    line3D(0,-HEIGHT/2,0, 0,HEIGHT/2,0,'.');
    line3D(0,0,-50, 0,0,50,'.');

    // rotate cube
    for (int i = 0; i < 8; i++) {
      int j = (i+1)%8;
      rotate(&cube[i], 0.00, 0.01, 0.01);
    }

    // draw edges
    for (int i = 0; i < 4; i++) {
      int j = (i+1)%4;
      line3Dv(cube[i], cube[j], '#');
      line3Dv(cube[i+4], cube[j+4], '=');
      line3Dv(cube[i], cube[i+4], '*');
    }

    // draw vertices
    for (int i = 0; i < 8; i++) {
      int j = (i+1)%8;
      point3Dv(cube[i],'x');
    }



    // fillAxes();
    fillBorder();
    render();
    printStuff();
    handleKeyboard();
    usleep(1000000/60);
  }

  return 0;
}