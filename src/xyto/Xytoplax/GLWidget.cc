//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2010  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "GLWidget.hh"
#include "GL/glext.h"
#include "../turtle.hh"
#include "../draw.hh"

#ifndef GL_TEXTURE_FILTER_CONTROL
#define GL_TEXTURE_FILTER_CONTROL         0x8500
#endif
#ifndef GL_TEXTURE_LOD_BIAS
#define GL_TEXTURE_LOD_BIAS               0x8501
#endif


class GLDisplayListMesh {
public:
        GLDisplayListMesh () {

                textures_.push_back(0);
                glGenTextures(1, &textures_[0]);
                glBindTexture(GL_TEXTURE_2D, textures_[0]);


                QImage img ("bark.jpg");

                // make it 2^n
                int w=1, h=1;
                while (img.width()>w)
                        w *= 2;
                while (img.width()>h)
                        h *= 2;
                img = img.scaled(w,h,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);


                unsigned char *foo = new unsigned char [img.width()*img.height()*3];
                for (int y=0; y<img.height(); ++y) {
                        for (int x=0; x<img.width(); ++x) {
                                QColor const & col = img.pixel(x, y);
                                unsigned char * const p = &foo[(x+img.width()*y)*3];
                                p[0] = col.red();
                                p[1] = col.green();
                                p[2] = col.blue();
                        }
                }
                gluBuild2DMipmaps(GL_TEXTURE_2D, 3, img.width(), img.height(),
                                  GL_RGB, GL_UNSIGNED_BYTE, foo);
                delete [] foo;


                glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
                glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                 GL_LINEAR_MIPMAP_LINEAR);
                glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


                displayList_ = glGenLists(1);
                glNewList(displayList_, GL_COMPILE);


                glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, -4);
                glEnable (GL_TEXTURE_2D);
        }

        ~GLDisplayListMesh () {
                glEndList();
        }

        void moveTo (Turtle state) {
                this->state = state;
        }


        void drawTo (Turtle newState) {
                using std::fabs;
                using std::acos;

                glBegin(GL_QUAD_STRIP);
                const double pi = 3.14159, pi2 = pi*2;
                int count = 5;


                const double fdot = dot (state.rotation.forward(),
                                         newState.rotation.forward());
                qWarning("fdot:" + QString::number(fdot).toAscii());
                qWarning("fwd:" + (QString::number(newState.rotation.forward().x) + "," +
                                   QString::number(newState.rotation.forward().y) + "," +
                                   QString::number(newState.rotation.forward().z)).toAscii());
                if (fabs(fdot) < 0.999) {
                        const TurtleVector axis =
                                        normalize(cross(
                                                state.rotation.forward(),
                                                newState.rotation.forward()));
                        const double angle = acos(fdot);
                        qWarning(QString::number(angle/0.0174532925).toAscii());
                        newState.rotation = state.rotation
                                            *
                                            TurtleMatrix::Rotate(angle, axis)
                                            ;
                } else {
                        newState.rotation = state.rotation;
                }

                const double oldTexV = state.pathLength    * 0.01,
                             newTexV = newState.pathLength * 0.01;
                for (int i=0; i<=count; ++i) {
                        const double f = i / (float)count;
                        const double phi = f * pi2;
                        const TurtleVector oldV = state.disk(phi) * state.scale;
                        const TurtleVector newV = newState.disk(phi) * newState.scale;
                        const TurtleVector oldN = state.normal(phi);
                        const TurtleVector newN = newState.normal(phi);

                        glTexCoord2f(f, newTexV);
                        glNormal3f(newN.x, newN.y, newN.z);
                        glVertex3f(newV.x, newV.y, newV.z);

                        glTexCoord2f(f, oldTexV);
                        glNormal3f(oldN.x, oldN.y, oldN.z);
                        glVertex3f(oldV.x, oldV.y, oldV.z);
                }
                glEnd();

                state = newState;
        }

        GLuint displayList() const {
                return displayList_;
        }
        std::vector<GLuint> textures() const {
                return textures_;
        }
private:
        Turtle state;
        GLuint displayList_;
        std::vector<GLuint> textures_;
};



int normalizeAngle(int angle) {
     while (angle < 0)
         angle += 360 * 16;
     while (angle > 360 * 16)
         angle -= 360 * 16;
     return angle;
}



GLWidget::GLWidget(QWidget *parent)
        : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{

        xRot = 0;
        yRot = 0;
        zRot = 0;

        xPos = 0;
        yPos = -20;
        zPos = -50;

        qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
        qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

        displayList = 0;

        if (!format().depth())
                 qWarning("Could not get depth buffer; results will be shitty");

}


GLWidget::~GLWidget()
{
        makeCurrent();
        if (displayList != 0)
                glDeleteLists(displayList, 1);
        while (!textures.empty()) {
                glDeleteTextures (1, &textures.back());
                textures.pop_back();
        }
}


QSize GLWidget::minimumSizeHint() const {
        return QSize(50, 50);
}


QSize GLWidget::sizeHint() const {
        return QSize(400, 400);
}


void GLWidget::setXRotation(int angle) {
      angle = normalizeAngle(angle);
      if (angle != xRot) {
          xRot = angle;
          updateGL();
      }
}
void GLWidget::setYRotation(int angle) {
      angle = normalizeAngle(angle);
      if (angle != yRot) {
          yRot = angle;
          updateGL();
      }
}
void GLWidget::setZRotation(int angle) {
      angle = normalizeAngle(angle);
      if (angle != zRot) {
          zRot = angle;
          updateGL();
      }
}



void GLWidget::initializeGL() {

        glEnable(GL_CULL_FACE);
        //glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

        glEnable(GL_DEPTH_TEST);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        //glEnable(GL_MULTISAMPLE);
        static GLfloat lightPosition[4] = { 1.0, 0.8, 2.0, 0.0 };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

        glClearColor(0.5, 0.4, 0.3, 1.0);
}


void GLWidget::resizeGL(int width, int height) {
        glViewport(0,0, width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45, width / (float)height, 1, 1000);
        //glOrtho(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);

        glMatrixMode(GL_MODELVIEW);
}


void GLWidget::paintGL() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(xPos, yPos, zPos);
        glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
        glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
        glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);
        glCallList(displayList);

        glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        glColor3f(1,0,0);
        glVertex3f(-10, 0, 10);
        glColor3f(0,0,0);
        glVertex3f(10, 0, 10);
        glColor3f(0,0,1);
        glVertex3f(10, 0, -10);
        glColor3f(1,0,1);
        glVertex3f(-10, 0, -10);
        glEnd();
}


void GLWidget::mousePressEvent(QMouseEvent *event) {
        lastPos = event->pos();
}


void GLWidget::mouseMoveEvent(QMouseEvent *event) {
        int dx = event->x() - lastPos.x();
        int dy = event->y() - lastPos.y();

        if (event->buttons() & Qt::LeftButton) {
                setXRotation(xRot + 8 * dy);
                setYRotation(yRot + 8 * dx);
        } else if (event->buttons() & Qt::RightButton) {
                xPos += dx * 0.1;
                yPos -= dy * 0.1;
                //zoom((xRot + 8 * dy)*0.01);
                //setZRotation(zRot + 8 * dx);
                updateGL();
        }
        lastPos = event->pos();
}
void GLWidget::wheelEvent (QWheelEvent *we) {
        zPos += we->delta() * 0.05;
        updateGL();
}


void GLWidget::updateData(LSystem const &lsys, Pattern const &pat) {
        if (displayList != 0)
                glDeleteLists(displayList, 1);
        while (!textures.empty()) {
                glDeleteTextures (1, &textures.back());
                textures.pop_back();
        }

        if (1) {
                {
                        GLDisplayListMesh dlm;
                        draw (lsys, pat, Turtle(), dlm);
                        displayList = dlm.displayList();
                }
                updateGL();
        } else {
                displayList = glGenLists(1);
                glNewList(displayList, GL_COMPILE);

                glBegin(GL_LINES);
                glColor3f(0.5,0.7,0.9);
                glVertex3f(0,0,0);
                glVertex3f(10,50,0);
                glEnd();

                glEndList();

                updateGL();
        }
}



void GLWidget::setWireframe (bool enable) {
        glPolygonMode (GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL);
        updateGL();
}
