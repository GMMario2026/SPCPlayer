/*************************************************************************************************
 *
 * 文件：Renderer.h
 *
 * 描述：定义渲染对象
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#pragma once

#include <QOpenGLBuffer>
#include <QPainter>

#define DT_CALCRECT -1

class OpenGLWidget;

class OpenGLRenderer {
  friend class OpenGLWidget;

public:
  enum FontSize { Small, Medium };

  struct SCREEN_VERTEX {
    float x, y, z;
    float r, g, b, a;
  };

  static const int maxTriangles = 200;
  static const int vboSize = sizeof(SCREEN_VERTEX) * 3 * maxTriangles;

  OpenGLRenderer()
      : bufWidth(0), bufHeight(0), isDeviceValid(false), triangle(0) {}

  GLenum init(OpenGLWidget *widget);

  // 下列函数只能在主线程中执行
  GLenum resetDeviceIfNeeded();
  GLenum resetDevice();
  GLenum clear(QColor color);
  GLenum endScene();
  GLenum beginText();
  GLenum drawText(const QString text, FontSize font, QRect *pos, int format,
                  QColor color); // 不需要区分 ANSI / Unicode
  GLenum endText();
  GLenum drawRect(float x, float y, float cx, float cy, QColor color);
  GLenum drawRect(float x, float y, float cx, float cy, QColor c1, QColor c2,
                  QColor c3, QColor c4);
  GLenum drawSkew(float x1, float y1, float x2, float y2, float x3, float y3,
                  float x4, float y4, QColor c1, QColor c2, QColor c3,
                  QColor c4);

  auto getBufferWidth() const { return bufWidth; }
  auto getBufferHeight() const { return bufHeight; }

protected:
  int bufWidth, bufHeight;

private:
  GLenum restoreDeviceObjects();
  void destroyDeviceObjects();

  OpenGLWidget *gfxWidget;
  QPainter textPainter;
  QFont smallFont;
  QFont mediumFont;
  bool isDeviceValid;

  GLenum prepBuffer(int triangles);
  GLenum flushBuffer();
  QOpenGLBuffer vbo;
  int triangle;
  quint8 *vertexData;

  GLenum bilt(SCREEN_VERTEX *data, int triangles);
};