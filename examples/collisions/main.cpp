#include <iostream>

#ifdef __APPLE__

#include <OpenGL/gl.h>
#include <GLUT/glut.h>

#else

#include <GL/gl.h>
#include <GL/glut.h>

#endif

#include <neu/NProgram.h>
#include <neu/NPQueue.h>
#include <neu/NPLParser.h>
#include <neu/NPLVector.h>
#include <neu/NPLObject.h>
#include <neu/NPLModule.h>
#include <neu/NRandom.h>
#include <neu/NSys.h>

using namespace std;
using namespace neu;
using namespace npl;

static const float SIZE = 0.05;
static const size_t WINDOW_WIDTH = 1024;
static const size_t WINDOW_HEIGHT = 1024;
static const float MIN_R = 0.3;
static const float MAX_R = 0.3;
static const float SLEEP = 0.00001;
static const size_t SX = 30;
static const size_t SY = 30;

static NRandom rng;

class Body : public NPLObject{
public:
  Body(const float3& color, const float3& u)
    : color(color),
      m(0.01),
      u(u),
      v({float(rng.uniform(-0.1, 0.1)),
         float(rng.uniform(-0.1, 0.1)), 0}),
      r(rng.uniform(MIN_R, MAX_R)){
    
  }

  float3 color;
  float3 u;
  float3 v;
  float r;
  float m;
};

static GLuint bodyDL;

typedef NVector<Body*> BodyVec;
BodyVec bodyVec;

static NPQueue runQueue(8);
static NPQueue interactQueue(8);

NPLModule module;

inline void toRGB(const float3& hsv, float3& rgb){
  static int i;
  static float f, p, q, t, h;
  
  if(hsv[1] == 0){
    rgb[0] = rgb[1] = rgb[2] = hsv[2];
    return;
  }

  h = hsv[0] / 60;

  i = h;
  f = h - i;
  p = hsv[2] * (1 - hsv[1]);
  q = hsv[2] * (1 - hsv[1] * f);
  t = hsv[2] * (1 - hsv[1] * (1 - f));

  switch(i){
    case 0:
      rgb[0] = hsv[2];
      rgb[1] = t;
      rgb[2] = p;
      break;
    case 1:
      rgb[0] = q;
      rgb[1] = hsv[2];
      rgb[2] = p;
      break;
    case 2:
      rgb[0] = p;
      rgb[1] = hsv[2];
      rgb[2] = t;
      break;
    case 3:
      rgb[0] = p;
      rgb[1] = q;
      rgb[2] = hsv[2];
      break;
    case 4:
      rgb[0] = t;
      rgb[1] = p;
      rgb[2] = hsv[2];
      break;
    default:
      rgb[0] = hsv[2];
      rgb[1] = p;
      rgb[2] = q;
      break;
  }
}

void animate(){
  interactQueue.run();
  runQueue.run();
  glutPostRedisplay();
}

void drawBody(float r,
              const float3& color,
              const float3& u){
  
  glPushMatrix();

  glColor4f(color[0], color[1], color[2], 1);
  
  glScalef(r, r, r);
  glTranslatef(u[0], u[1], 0);

  glCallList(bodyDL);

  glPopMatrix();
}

void display(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(0.0, 0.0, 100.0,
	    0.0, 0.0, 0.0,
	    0.0, 1.0, 0.0);

  size_t size = bodyVec.size();
  for(size_t i = 0; i < size; ++i){
    Body* b = bodyVec[i];
    drawBody(b->r, b->color, b->u);
  }
  
  NSys::sleep(SLEEP);
  glutSwapBuffers();
}

void createBody(){
  bodyDL = glGenLists(1);

  glNewList(bodyDL, GL_COMPILE);

  glutSolidSphere(1, 4, 4);

  glEndList();
}

void initGL(void){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  gluPerspective(60.0,
                 static_cast<float>(WINDOW_WIDTH)/WINDOW_HEIGHT, 0.1, 1024); 

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glClearColor(0, 0, 0, 0);

  createBody();
}

class InteractFunc : public NPLFunc{
public:
  Body* b2;
};

int main(int argc, char** argv){
  NProgram program(argc, argv);
  
  rng.timeSeed();

  glutInit(&argc, argv);

  NPLParser parser;
  nvar code = parser.parseFile("kernel.npl");

  cout << "code is: " << code << endl;
  
  if(!module.compile(code)){
    cerr << "failed to compile" << endl;
    return 1;
  }

  static const float left = -2;
  static const float top = 5;
  static const float xs = 3.0;
  static const float ys = 3.0;

  for(size_t i = 0; i < SX; ++i){
    for(size_t j = 0; j < SY; ++j){
      float3 rgb;
      float3 hsv = {j / float(SY) * 360, 1, 1};
      toRGB(hsv, rgb);
      float3 u = {left + i * xs, top + j * ys, 0};

      Body* b = new Body(rgb, u);
      bodyVec.push_back(b);
    }
  }

  for(size_t i = 0; i < bodyVec.size(); ++i){
    Body* b = bodyVec[i];

    NPLFunc* f = new NPLFunc;
    module.getFunc({"Body", "run", 0}, f);
    f->o = b;

    runQueue.add(f);

    for(size_t j = 0; j < bodyVec.size(); ++j){
      if(i == j){
        continue;
      }

      InteractFunc* fi = new InteractFunc;
      module.getFunc({"Body", "interact", 1}, fi);
      fi->o = b;
      fi->b2 = bodyVec[j];

      interactQueue.add(fi);
    }
  }

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutCreateWindow("Collisions");
  initGL();
  glutDisplayFunc(display);
  glutIdleFunc(animate);
  glutMainLoop();

  return 0;
}
