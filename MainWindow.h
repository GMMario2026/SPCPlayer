/*************************************************************************************************
 *
 * 文件：MainWindow.h
 *
 * 描述：窗体的行为定义
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#pragma once

#include <QDialog>
#include <QMainWindow>
#include <QOpenGLFunctions_4_5_Compatibility>
#include <QOpenGLWidget>

#include "Misc.h"

#define CW_USEDEFAULT ((int)0x80000000)

class GameState;
class OpenGLRenderer;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget *parent = nullptr);

  void createMenu();
  void createRebar();

  void handOffEvent(Event::Type type, qint64 param);
  void showControls(bool show);
  void showKeyboard(bool show);
  void setOnTop(bool onTop);
  void setFullScreen(bool fullScr);
  void setZoomMove(bool zoomMove);
  void setMute(bool mute);
  void setSpeed(double speed);
  void setNSpeed(double nSpeed);
  void setVolume(double vol);
  void setPosition(int posn);
  void setPlayable(bool playable);
  void setPlayMode(int playMode);
  void setPlayPauseStop(bool play, bool pause, bool stop);
  bool playFile(QString file, bool customSettings);
  void checkActivity(bool isActive, QPoint *newPt = nullptr,
                     bool toggleEnable = false);

protected:
  void moveEvent(QMoveEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void on_openAct_triggered();
  void on_openCustomAct_triggered();
  void on_closeAct_triggered();

  void on_playPauseAct_triggered();
  void on_stopAct_triggered();
  void on_skipFwdAct_triggered();
  void on_skipBackAct_triggered();
  void on_increaseRateAct_triggered();
  void on_decreaseRateAct_triggered();
  void on_resetRateAct_triggered();
  void on_nFasterAct_triggered();
  void on_nSlowerAct_triggered();
  void on_muteAct_triggered();
  void on_volumeUpAct_triggered();
  void on_volumeDownAct_triggered();

  void on_controlsAct_triggered();
  void on_keyboardAct_triggered();
  void on_alwaysOnTopAct_triggered();
  void on_fullScreenAct_triggered();
  void on_moveAndZoomAct_triggered();
  void on_resetMoveAndZoomAct_triggered();

  void on_perferenceAct_triggered();
  void on_aboutAct_triggered();

  void on_toolPlayAct_triggered();
  void on_toolPauseAct_triggered();
  void on_toolStopAct_triggered();
  void on_toolSkipFwdAct_triggered();
  void on_toolSkipBackAct_triggered();
  void on_toolMuteAct_triggered();
  void on_volumeSlider_valueChanged(int value);
  void on_volumeSlider_sliderPressed();
  void on_speedSlider_valueChanged(int value);
  void on_nSpeedSlider_valueChanged(int value);

  void on_positionSlider_sliderMoved(int value);
  void on_positionSlider_sliderPressed();

  void on_nResetAct_triggered();
  void on_noFullScreenAct_triggered();

  void handleErrorMessage(qint64 param);
};

class OpenGLWidget : public QOpenGLWidget,
                     protected QOpenGLFunctions_4_5_Compatibility {
  friend class OpenGLRenderer;

public:
  OpenGLWidget(QWidget *parent);

  void initialize(bool limitFPS);
  void setGameState(GameState **gameState) { this->gameState = gameState; }
  void setRenderer(OpenGLRenderer *renderer) { this->renderer = renderer; }

  // 共享变量，主线程需要在渲染完成或阻塞时设为 false 让子线程进入下一个循环
  // 子线程设为 true 并监听以保证主线程渲染完成
  volatile bool rendering = false;

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void timerEvent(QTimerEvent *event) override;

  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

private:
  // OpenGLWidget 必须在主线程中处理绘制逻辑
  // 而 GameState 和 OpenGLRenderer 的对象实例均在子线程中，因此需要共享
  GameState **gameState;
  OpenGLRenderer *renderer;
};

class AboutDialog : public QDialog {
public:
  AboutDialog(QWidget *parent);
};