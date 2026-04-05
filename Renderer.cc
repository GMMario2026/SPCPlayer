/*************************************************************************************************
 *
 * 文件：Renderer.cc
 *
 * 描述：实现渲染对象（OpenGL）
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#include "Renderer.h"

#include "GameState.h"
#include "MainWindow.h"

void OpenGLRenderer::destroyDeviceObjects() {
  if (vbo.isCreated())
    vbo.release();

  isDeviceValid = false;
}

GLenum OpenGLRenderer::init(OpenGLWidget *gfxWidget) {
  GLenum result;

  // 绑定 Widget 对象
  this->gfxWidget = gfxWidget;

  // 初始化 OpenGL 函数
  gfxWidget->initializeOpenGLFunctions();

  // 创建文字绘图器
  smallFont.setPixelSize(13);
  smallFont.setWeight(QFont::Normal);

  mediumFont.setPixelSize(25);
  mediumFont.setWeight(QFont::Normal);

  if ((result = restoreDeviceObjects()) != GL_NO_ERROR)
    return result;

  bufWidth = gfxWidget->width();
  bufHeight = gfxWidget->height();
  isDeviceValid = true;

  return GL_NO_ERROR;
}

GLenum OpenGLRenderer::restoreDeviceObjects() {
  GLenum result;

  gfxWidget->glEnableClientState(GL_VERTEX_ARRAY);
  gfxWidget->glEnableClientState(GL_COLOR_ARRAY);
  vbo.create();
  vbo.bind();
  vbo.allocate(vboSize);
  vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  gfxWidget->glVertexPointer(3, GL_FLOAT, sizeof(SCREEN_VERTEX), (void *)0);
  gfxWidget->glColorPointer(4, GL_FLOAT, sizeof(SCREEN_VERTEX),
                            (void *)offsetof(SCREEN_VERTEX, r));

  if ((result = gfxWidget->glGetError()) != GL_NO_ERROR)
    return result;
  triangle = 0;

  gfxWidget->glEnable(GL_BLEND);
  gfxWidget->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  return gfxWidget->glGetError();
}

GLenum OpenGLRenderer::resetDeviceIfNeeded() {
  if (!isDeviceValid) {
    auto result = gfxWidget->glGetError();
    if (result == GL_CONTEXT_LOST)
      result = resetDevice();
    if (result != GL_NO_ERROR)
      return result;
  }
  return GL_NO_ERROR;
}

GLenum OpenGLRenderer::resetDevice() {
  GLenum result;

  // 销毁对象，并重新初始化
  if (isDeviceValid)
    destroyDeviceObjects();

  // 恢复对象
  if ((result = restoreDeviceObjects()) != GL_NO_ERROR)
    return result;

  bufWidth = gfxWidget->width();
  bufHeight = gfxWidget->height();
  isDeviceValid = true;
  return GL_NO_ERROR;
}

GLenum OpenGLRenderer::clear(QColor color) {
  gfxWidget->glClearColor(color.redF(), color.greenF(), color.blueF(),
                          color.alphaF());
  return gfxWidget->glGetError();
}

GLenum OpenGLRenderer::endScene() {
  flushBuffer();
  return gfxWidget->glGetError();
}

GLenum OpenGLRenderer::beginText() {
  flushBuffer();
  textPainter.begin(gfxWidget);
  return gfxWidget->glGetError();
}

GLenum OpenGLRenderer::drawText(const QString text, FontSize fontSize,
                                QRect *pos, int format, QColor color) {
  auto &font = (fontSize == Small    ? smallFont
                : fontSize == Medium ? mediumFont
                                     : mediumFont);

  // 计算区域
  if (format == DT_CALCRECT) {
    QFontMetrics fontMetrics(font);
    *pos = fontMetrics.boundingRect(*pos, Qt::AlignLeft, text);
    return GL_NO_ERROR;
  }

  textPainter.setFont(font);
  textPainter.setPen(color);
  textPainter.drawText(*pos, format, text);

  return gfxWidget->glGetError();
}

GLenum OpenGLRenderer::endText() {
  textPainter.end();
  return gfxWidget->glGetError();
}

GLenum OpenGLRenderer::drawRect(float x, float y, float cx, float cy,
                                QColor color) {
  return drawRect(x, y, cx, cy, color, color, color, color);
}

GLenum OpenGLRenderer::drawRect(float x, float y, float cx, float cy, QColor c1,
                                QColor c2, QColor c3, QColor c4) {
  x -= 0.5f;
  y -= 0.5f;

  SCREEN_VERTEX vertices[6] = {
      {x, y, 0.5f, c1.redF(), c1.greenF(), c1.blueF(), c1.alphaF()},
      {x + cx, y, 0.5f, c2.redF(), c2.greenF(), c2.blueF(), c2.alphaF()},
      {x + cx, y + cy, 0.5f, c3.redF(), c3.greenF(), c3.blueF(), c3.alphaF()},
      {x, y, 0.5f, c1.redF(), c1.greenF(), c1.blueF(), c1.alphaF()},
      {x + cx, y + cy, 0.5f, c3.redF(), c3.greenF(), c3.blueF(), c3.alphaF()},
      {x, y + cy, 0.5f, c4.redF(), c4.greenF(), c4.blueF(), c4.alphaF()}};

  return bilt(vertices, 2);
}

GLenum OpenGLRenderer::drawSkew(float x1, float y1, float x2, float y2,
                                float x3, float y3, float x4, float y4,
                                QColor c1, QColor c2, QColor c3, QColor c4) {
  SCREEN_VERTEX vertices[6] = {{x1 - 0.5f, y1 - 0.5f, 0.5f, c1.redF(),
                                c1.greenF(), c1.blueF(), c1.alphaF()},
                               {x2 - 0.5f, y2 - 0.5f, 0.5f, c2.redF(),
                                c2.greenF(), c2.blueF(), c2.alphaF()},
                               {x3 - 0.5f, y3 - 0.5f, 0.5f, c3.redF(),
                                c3.greenF(), c3.blueF(), c3.alphaF()},
                               {x1 - 0.5f, y1 - 0.5f, 0.5f, c1.redF(),
                                c1.greenF(), c1.blueF(), c1.alphaF()},
                               {x3 - 0.5f, y3 - 0.5f, 0.5f, c3.redF(),
                                c3.greenF(), c3.blueF(), c3.alphaF()},
                               {x4 - 0.5f, y4 - 0.5f, 0.5f, c4.redF(),
                                c4.greenF(), c4.blueF(), c4.alphaF()}};

  return bilt(vertices, 2);
}

GLenum OpenGLRenderer::bilt(SCREEN_VERTEX *vertices, int triangles) {
  prepBuffer(triangles);
  memcpy(vertexData + triangle * 3 * sizeof(SCREEN_VERTEX), vertices,
         triangles * 3 * sizeof(SCREEN_VERTEX));
  vbo.unmap();
  this->triangle += triangles;
  return GL_NO_ERROR;
}

GLenum OpenGLRenderer::prepBuffer(int triangles) {
  if (triangle > maxTriangles)
    return GL_INVALID_OPERATION;
  if (triangle == 0) {
    vbo.bind();
    vertexData = reinterpret_cast<quint8 *>(vbo.map(QOpenGLBuffer::WriteOnly));
    return gfxWidget->glGetError();
  }
  if (triangle + triangles <= maxTriangles)
    return GL_NO_ERROR;

  flushBuffer();
  vbo.bind();
  vertexData = reinterpret_cast<quint8 *>(vbo.map(QOpenGLBuffer::WriteOnly));
  return gfxWidget->glGetError();
}

GLenum OpenGLRenderer::flushBuffer() {
  if (triangle == 0)
    return GL_NO_ERROR;

  vbo.bind();
  gfxWidget->glDrawArrays(GL_TRIANGLES, 0, triangle * 3);
  auto result = gfxWidget->glGetError();
  triangle = 0;
  return result;
}

void OpenGLWidget::initializeGL() {
  renderer->init(this);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, renderer->getBufferWidth(), renderer->getBufferHeight(), 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
}

void OpenGLWidget::resizeGL(int w, int h) {
  renderer->resetDevice();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, h, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
}

void OpenGLWidget::paintGL() {
  // 确保游戏状态有效的情况下渲染，否则不渲染
  if (rendering)
    (*gameState)->render();
  rendering = false;
}