// Disciplina de Computação Gráfica
// Aluno: Mateus Jaques Winter
// Trabalho 2

// Botão esquerdo do mouse - Rotacionar
// Botão direito do mouse - Mover
// Espaço - Ativar luzes

#include <GL/freeglut.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtx/component_wise.hpp"

using namespace std;
using namespace glm;

struct Vertex {
    vec3 normal;
    vec2 texture;
    vec3 position;
};

struct VertRef {
    VertRef(int v, int vt, int vn) : v(v), vt(vt), vn(vn) {}
    int v, vt, vn;
};

vector<Vertex> LoadOBJ(istream& in) {
    vector<Vertex> verts;
    vector<vec4> positions(1, vec4(0, 0, 0, 0));
    vector<vec3> textures(1, vec3(0, 0, 0));
    vector<vec3> normals(1, vec3(0, 0, 0));
    string lineStr;
    while (getline(in, lineStr)) {
        istringstream lineSS(lineStr);
        string lineType;
        lineSS >> lineType;

        if (lineType == "v") {
            float x = 0, y = 0, z = 0, w = 1;
            lineSS >> x >> y >> z >> w;
            positions.push_back(vec4(x, y, z, w));
        }

        if (lineType == "vt") {
            float u = 0, v = 0, w = 0;
            lineSS >> u >> v >> w;
            textures.push_back(vec3(u, v, w));
        }

        if (lineType == "vn") {
            float i = 0, j = 0, k = 0;
            lineSS >> i >> j >> k;
            normals.push_back(normalize(vec3(i, j, k)));
        }

        if (lineType == "f") {
            vector<VertRef> refs;
            string refStr;
            while (lineSS >> refStr) {
                istringstream ref(refStr);
                string vStr, vtStr, vnStr;
                getline(ref, vStr, '/');
                getline(ref, vtStr, '/');
                getline(ref, vnStr, '/');
                int v = atoi(vStr.c_str());
                int vt = atoi(vtStr.c_str());
                int vn = atoi(vnStr.c_str());
                v = (v >= 0 ? v : positions.size() + v);
                vt = (vt >= 0 ? vt : textures.size() + vt);
                vn = (vn >= 0 ? vn : normals.size() + vn);
                refs.push_back(VertRef(v, vt, vn));
            }

            if (refs.size() < 3) {
                continue;
            }

            VertRef* p[3] = { &refs[0], NULL, NULL };
            for (size_t i = 1; i + 1 < refs.size(); ++i) {
                p[1] = &refs[i + 0];
                p[2] = &refs[i + 1];

                vec3 U(positions[p[1]->v] - positions[p[0]->v]);
                vec3 V(positions[p[2]->v] - positions[p[0]->v]);
                vec3 faceNormal = normalize(cross(U, V));

                for (size_t j = 0; j < 3; ++j) {
                    Vertex vert;
                    vert.position = vec3(positions[p[j]->v]);
                    vert.texture = vec2(textures[p[j]->vt]);
                    vert.normal = (p[j]->vn != 0 ? normals[p[j]->vn] : faceNormal);
                    verts.push_back(vert);
                }
            }
        }
    }

    return verts;
}

bool light = true;

void esc(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        exit(0);
        break;
    };
}

void lightSwitch(unsigned char key, int x, int y) {
    switch (key) {
    case 32:
        light = light ? false : true;
        break;
    };
}

int button;
ivec2 startMouse;
ivec2 startRotation, curRotation;
ivec2 startTranslation, curTranslation;
void mouse(int btn, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        button = btn;
        startMouse = ivec2(x, glutGet(GLUT_WINDOW_HEIGHT) - y);
        startRotation = curRotation;
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        button = btn;
        startMouse = ivec2(x, glutGet(GLUT_WINDOW_HEIGHT) - y);
        startTranslation = curTranslation;
    }
}

void mouseMovement(int x, int y) {
    ivec2 curMouse(x, glutGet(GLUT_WINDOW_HEIGHT) - y);
    if (button == GLUT_LEFT_BUTTON) {
        curRotation = startRotation + (curMouse - startMouse);
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        curTranslation = startTranslation + (curMouse - startMouse);
    }
    glutPostRedisplay();
}

vector<Vertex> modelo;
void display() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double w = glutGet(GLUT_WINDOW_WIDTH);
    double h = glutGet(GLUT_WINDOW_HEIGHT);
    double ar = w / h;
    glTranslatef(curTranslation.x / w * 2, curTranslation.y / h * 2, 0);
    gluPerspective(60, ar, 0.1, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -10);

    glPushMatrix();
    {
        glRotatef(curRotation.x % 360, 0, 1, 0);
        glRotatef(-curRotation.y % 360, 1, 0, 0);

        glColor3ub(0, 0, 255);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &modelo[0].position);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &modelo[0].texture);
        glNormalPointer(GL_FLOAT, sizeof(Vertex), &modelo[0].normal);
        glDrawArrays(GL_TRIANGLES, 0, modelo.size());

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);

        if (light) {
            glEnable(GL_LIGHTING);
        }
        else {
            glDisable(GL_LIGHTING);
        }
    }
    glPopMatrix();

    glutSwapBuffers();
}

template <typename Vec>
pair<Vec, Vec> getEdges(const Vec* pts, size_t stride, size_t count) {
    unsigned char* base = (unsigned char*)pts;
    Vec pmin(*(Vec*)base);
    Vec pmax(*(Vec*)base);
    for (size_t i = 0; i < count; ++i, base += stride) {
        const Vec& pt = *(Vec*)base;
        pmin = glm::min(pmin, pt);
        pmax = glm::max(pmax, pt);
    }

    return make_pair(pmin, pmax);
}

template <typename Vec>
void centralize(Vec* pts, size_t stride, size_t count,
    const typename Vec::value_type& size) {
    typedef typename Vec::value_type Scalar;

    pair<Vec, Vec> exts = getEdges(pts, stride, count);

    const Vec center = (exts.first * Scalar(0.5)) + (exts.second * Scalar(0.5f));

    const Scalar factor = size / glm::compMax(exts.second - exts.first);
    unsigned char* base = (unsigned char*)pts;
    for (size_t i = 0; i < count; ++i, base += stride) {
        Vec& pt = *(Vec*)base;
        pt = ((pt - center) * factor);
    }
}

int main(int argc, char** argv) {
    ifstream ifile("data/porsche.obj");
    modelo = LoadOBJ(ifile);
    centralize(&modelo[0].position, sizeof(Vertex), modelo.size(), 7);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(640, 480);
    glutCreateWindow("Trabalho 2");
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMovement);
    glutKeyboardFunc(esc);
    glutKeyboardUpFunc(lightSwitch);

    glEnable(GL_DEPTH_TEST);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLfloat diffuse[] = { 0.9f, 0.8f, 0.8f, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat positional[] = { 0, 0, 1, 0 };
    GLfloat localView[] = { 0.0 };

    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, positional);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, localView);

    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_LINE);
    glutMainLoop();
    return 0;
}