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

static const size_t N = 512*10;
static const float SIZE = 0.05;
static const size_t WINDOW_WIDTH = 1024;
static const size_t WINDOW_HEIGHT = 1024;
static const float MIN_R = 0.01;
static const float MAX_R = 0.10;
static const float MAX_V = 0.50;
static const float MAX_SPIN = 30.0;
static const float REPEL = -2e-1;
static const float SLEEP = 0.001;
static const size_t RESET_INTERVAL = 250;

static GLuint cubeDL;
static NRandom rng;

class Cube : public NPLObject{
public:
  Cube(){
    
  }

  void reset(const float3& u){
    this->u = u;
   
    v[0] = rng.uniform(-MAX_V, MAX_V);
    v[1] = rng.uniform(-MAX_V, MAX_V);
    v[2] = rng.uniform(-MAX_V, MAX_V);

    a = {0.0, 0.0, 0.0};

    av[0] = rng.uniform(-MAX_SPIN, MAX_SPIN);
    av[1] = rng.uniform(-MAX_SPIN, MAX_SPIN);
    av[2] = rng.uniform(-MAX_SPIN, MAX_SPIN);
    
    r = rng.uniform(MIN_R, MAX_R);
  }

  float3 u;
  float3 v;
  float3 a;
  float3 av;
  float r;
};

typedef NVector<Cube*> CubeVec;
CubeVec cubeVec;

static NPQueue queue(8);
size_t t = 0;

NPLModule module;

void animate(){
  if(t % RESET_INTERVAL == 0){
    float3 u = {float(rng.uniform(-20, 20)),
                float(rng.uniform(0, 50)),
                float(rng.uniform(-50, 10))};

    for(size_t i = 0; i < cubeVec.size(); ++i){
      Cube* cube = cubeVec[i];
      cube->reset(u);
    }
  }

  queue.run();

  glutPostRedisplay();
  ++t;
}

void createCube(){
  cubeDL = glGenLists(1);

  glNewList(cubeDL, GL_COMPILE);

  static GLfloat v0[] = {-1, -1, 1};
  static GLfloat v1[] = {1, -1, 1};
  static GLfloat v2[] = {1, 1, 1};
  static GLfloat v3[] = {-1, 1, 1};
  static GLfloat v4[] = {-1, -1, -1};
  static GLfloat v5[] = {1, -1, -1};
  static GLfloat v6[] = {1, 1, -1};
  static GLfloat v7[] = {-1, 1, -1};
  static GLubyte red[]    = {255, 0, 0, 255};
  static GLubyte green[]  = {0, 255, 0, 255};
  static GLubyte blue[]   = {0, 0, 255, 255};
  static GLubyte white[]  = {255, 255, 255, 255};
  static GLubyte yellow[] = {0, 255, 255, 255};
  static GLubyte black[]  = {0, 0, 0, 255};
  static GLubyte orange[] = {255, 255, 0, 255};
  static GLubyte purple[] = {255, 0, 255, 0};

  glBegin(GL_TRIANGLES);

  glColor4ubv(red);
  glVertex3fv(v0);
  glColor4ubv(green);
  glVertex3fv(v1);
  glColor4ubv(blue);
  glVertex3fv(v2);
  
  glColor4ubv(red);
  glVertex3fv(v0);
  glColor4ubv(blue);
  glVertex3fv(v2);
  glColor4ubv(white);
  glVertex3fv(v3);
  
  glColor4ubv(green);
  glVertex3fv(v1);
  glColor4ubv(black);
  glVertex3fv(v5);
  glColor4ubv(orange);
  glVertex3fv(v6);
  
  glColor4ubv(green);
  glVertex3fv(v1);
  glColor4ubv(orange);
  glVertex3fv(v6);
  glColor4ubv(blue);
  glVertex3fv(v2);
  
  glColor4ubv(black);
  glVertex3fv(v5);
  glColor4ubv(yellow);
  glVertex3fv(v4);
  glColor4ubv(purple);
  glVertex3fv(v7);
  
  glColor4ubv(black);
  glVertex3fv(v5);
  glColor4ubv(purple);
  glVertex3fv(v7);
  glColor4ubv(orange);
  glVertex3fv(v6);
   
  glColor4ubv(yellow);
  glVertex3fv(v4);
  glColor4ubv(red);
  glVertex3fv(v0);
  glColor4ubv(white);
  glVertex3fv(v3);
  
  glColor4ubv(yellow);
  glVertex3fv(v4);
  glColor4ubv(white);
  glVertex3fv(v3);
  glColor4ubv(purple);
  glVertex3fv(v7);
  
  glColor4ubv(white);
  glVertex3fv(v3);
  glColor4ubv(blue);
  glVertex3fv(v2);
  glColor4ubv(orange);
  glVertex3fv(v6);
  
  glColor4ubv(white);
  glVertex3fv(v3);
  glColor4ubv(orange);
  glVertex3fv(v6);
  glColor4ubv(purple);
  glVertex3fv(v7);
  
  glColor4ubv(green);
  glVertex3fv(v1);
  glColor4ubv(red);
  glVertex3fv(v0);
  glColor4ubv(yellow);
  glVertex3fv(v4);
  
  glColor4ubv(green);
  glVertex3fv(v1);
  glColor4ubv(yellow);
  glVertex3fv(v4);
  glColor4ubv(black);
  glVertex3fv(v5);
  
  glEnd();

  glEndList();
}

void drawCube(float size,
	      const float3& u,
              const float3& a){
  
  glPushMatrix();

  glTranslatef(u[0], u[1], u[2]);
  glScalef(size, size, size);

  glRotatef(a[0], 1.0, 0.0, 0.0);
  glRotatef(a[1], 0.0, 1.0, 0.0);
  glRotatef(a[2], 0.0, 0.0, 1.0);

  glCallList(cubeDL);

  glPopMatrix();
}

void display(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(0.0, 5.0, 25.0,
	    0.0, 10.0, 0.0,
	    0.0, 1.0, 0.0);

  for(CubeVec::iterator itr = cubeVec.begin(),
	itrEnd = cubeVec.end(); itr != itrEnd; ++itr){
    Cube* cube = *itr;
    drawCube(cube->r, cube->u, cube->a);
  }
  
  NSys::sleep(SLEEP);
  glutSwapBuffers();
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

  createCube();
}

int main(int argc, char** argv){
  NProgram program(argc, argv);

  glutInit(&argc, argv);

  NPLParser parser;
  nvar code = parser.parseFile("kernel.npl");

  cout << "code is: " << code << endl;
  
  if(!module.compile(code)){
    cerr << "failed to compile" << endl;
    return 1;
  }

  for(size_t i = 0; i < N; ++i){
    Cube* c = new Cube;
    cubeVec.push_back(c);

    NPLFunc* f = new NPLFunc;
    module.getFunc({"Cube", "run", 0}, f);
    f->o = c;

    queue.add(f);
  }

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutCreateWindow("Cubes");
  initGL();
  glutDisplayFunc(display);
  glutIdleFunc(animate);
  glutMainLoop(); 

  return 0;
}
