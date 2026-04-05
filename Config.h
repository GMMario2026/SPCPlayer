/*************************************************************************************************
 *
 * 文件：Config.h
 *
 * 描述：定义配置对象
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#pragma once

#include <QXmlStreamWriter>

#include "GameState.h"
#include "Globals.h"

#define APPNAME "SPC 播放器"
#define APPNAMENOSPACES "SPCPlayer"
#define MINWIDTH 640
#define MINHEIGHT 469

class Settings {
public:
  virtual void loadDefaultValues() = 0;
  virtual void loadConfigValues(QXmlStreamReader &reader) = 0;
  virtual bool saveConfigValues(QXmlStreamWriter &writer) = 0;
};

struct VisualSettings : public Settings {
  void loadDefaultValues() override;
  void loadConfigValues(QXmlStreamReader &reader) override;
  bool saveConfigValues(QXmlStreamWriter &writer) override;

  enum KeysShown { All, Song, Custom } keysShown;
  int firstKey, lastKey;
  bool alwaysShowControls;
  QColor colors[16], bkgColor;
};

struct VideoSettings : public Settings {
  void loadDefaultValues() override;
  void loadConfigValues(QXmlStreamReader &reader) override;
  bool saveConfigValues(QXmlStreamWriter &writer) override;

  enum Renderer { DirectX, OpenGL, Vulkan } renderer;
  bool showFPS, limitFPS;
};

struct ControlsSettings : public Settings {
  void loadDefaultValues() override;
  void loadConfigValues(QXmlStreamReader &reader) override;
  bool saveConfigValues(QXmlStreamWriter &writer) override;

  double fwdBackSecs, speedUpPct;
};

class PlaybackSettings : public Settings {
public:
  void loadDefaultValues() override;
  void loadConfigValues(QXmlStreamReader &reader) override;
  bool saveConfigValues(QXmlStreamWriter &writer) override;

  void toggleMute(bool updateGUI = false) { setMute(!mute, updateGUI); }
  void togglePaused(bool updateGUI = false) { setPaused(!paused, updateGUI); }

  void setPosition(int position) { window->setPosition(position); }

  // 设置函数，根据需要更新 GUI
  void setPlayMode(GameState::State playMode, bool updateGUI = false) {
    if (updateGUI)
      window->setPlayMode(playMode);
    this->playMode = playMode;
  }
  void setPlayable(bool playable, bool updateGUI = false) {
    if (updateGUI)
      window->setPlayable(playable);
    this->playable = playable;
  }
  void setPaused(bool paused, bool updateGUI = false) {
    if (updateGUI)
      window->setPlayPauseStop(!paused, paused, false);
    this->paused = paused;
  }
  void setStopped(bool updateGUI = false) {
    if (updateGUI)
      window->setPlayPauseStop(false, false, true);
    paused = true;
  }
  void setSpeed(double speed, bool updateGUI = false) {
    if (updateGUI)
      window->setSpeed(speed);
    this->speed = speed;
  }
  void setNSpeed(double nSpeed, bool updateGUI = false) {
    nSpeed = std::max(std::min(nSpeed, 10.0), 0.005);
    if (updateGUI)
      window->setNSpeed(nSpeed);
    this->nSpeed = nSpeed;
  }

  void setVolume(double volume, bool updateGUI = false) {
    if (updateGUI)
      window->setVolume(volume);
    this->volume = volume;
  }
  void setMute(bool mute, bool updateGUI = false) {
    if (updateGUI)
      window->setMute(mute);
    this->mute = mute;
  }

  // 很简单的访问函数
  auto getPlayMode() const { return playMode; }
  auto getPlayable() const { return playable; }
  auto getPaused() const { return paused; }
  auto getMute() const { return mute; }
  auto getSpeed() const { return speed; }
  auto getNSpeed() const { return nSpeed; }
  auto getVolume() const { return volume; }

private:
  GameState::State playMode;
  bool playable, paused;
  bool mute;
  double speed, nSpeed, volume;
};

class ViewSettings : public Settings {
public:
  void loadDefaultValues() override;
  void loadConfigValues(QXmlStreamReader &reader) override;
  bool saveConfigValues(QXmlStreamWriter &writer) override;

  void toggleControls(bool updateGUI = false) {
    setControls(!controls, updateGUI);
  }
  void toggleKeyboard(bool updateGUI = false) {
    setKeyboard(!keyBoard, updateGUI);
  }
  void toggleOnTop(bool updateGUI = false) { setOnTop(!onTop, updateGUI); }
  void toggleFullScreen(bool updateGUI = false) {
    setFullScreen(!fullScreen, updateGUI);
  }

  void setMainPos(int mainLeft, int mainTop) {
    this->mainLeft = mainLeft;
    this->mainTop = mainTop;
  }
  void setMainSize(int mainWidth, int mainHeight) {
    this->mainWidth = mainWidth;
    this->mainHeight = mainHeight;
  }
  void setOffsetX(float offsetX) { this->offsetX = offsetX; }
  void setOffsetY(float offsetY) { this->offsetY = offsetY; }
  void setZoomX(float zoomX) { this->zoomX = zoomX; }
  void setControls(bool controls, bool updateGUI = false) {
    this->controls = controls;
    if (updateGUI)
      window->showControls(controls);
  }
  void setKeyboard(bool keyBoard, bool updateGUI = false) {
    this->keyBoard = keyBoard;
    if (updateGUI)
      window->showKeyboard(keyBoard);
  }
  void setOnTop(bool onTop, bool updateGUI = false) {
    this->onTop = onTop;
    if (updateGUI)
      window->setOnTop(onTop);
  }
  void setFullScreen(bool fullScreen, bool updateGUI = false) {
    this->fullScreen = fullScreen;
    if (updateGUI)
      window->setFullScreen(fullScreen);
  }
  void setZoomMove(bool zoomMove, bool updateGUI = false) {
    this->zoomMove = zoomMove;
    if (updateGUI)
      window->setZoomMove(zoomMove);
  }

  auto getMainLeft() const { return mainLeft; }
  auto getMainTop() const { return mainTop; }
  auto getMainWidth() const { return mainWidth; }
  auto getMainHeight() const { return mainHeight; }
  auto getZoomX() const { return zoomX; }
  auto getOffsetX() const { return offsetX; }
  auto getOffsetY() const { return offsetY; }
  auto getControls() const { return controls; }
  auto getKeyBoard() const { return keyBoard; }
  auto getOnTop() const { return onTop; }
  auto getFullScreen() const { return fullScreen; }
  auto getZoomMove() const { return zoomMove; }

private:
  bool controls, keyBoard, onTop, fullScreen, zoomMove;
  float zoomX, offsetX, offsetY;
  int mainLeft, mainTop, mainWidth, mainHeight;
};

class Config : public Settings {
public:
  // 单例模式
  static Config &getConfig();
  static QString getFolder();

  // 接口
  void loadDefaultValues() override;
  void loadConfigValues();
  void loadConfigValues(QXmlStreamReader &reader) override;
  bool saveConfigValues();
  bool saveConfigValues(QXmlStreamWriter &writer) override;

  auto &getVisualSettings() const { return visualSettings; }
  auto &getVideoSettings() const { return videoSettings; }
  auto &getControlsSettings() const { return controlsSettings; }
  auto &getPlaybackSettings() { return playbackSettings; }
  auto &getViewSettings() { return viewSettings; }

  void setVisualSettings(const VisualSettings &visualSettings) {
    this->visualSettings = visualSettings;
  }
  void setVideoSettings(const VideoSettings &videoSettings) {
    this->videoSettings = videoSettings;
  }
  void setControlsSettings(const ControlsSettings &controlsSettings) {
    this->controlsSettings = controlsSettings;
  }

private:
  // 单例模式
  Config();

  VisualSettings visualSettings;
  VideoSettings videoSettings;
  ControlsSettings controlsSettings;
  PlaybackSettings playbackSettings;
  ViewSettings viewSettings;
};