/*************************************************************************************************
 *
 * 文件：GameState.h
 *
 * 描述：定义界面状态和渲染项目
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#pragma once

#include <QAudioOutput>
#include <QBuffer>
#include <QElapsedTimer>
#include <QMediaPlayer>

#include "Misc.h"
#include "Renderer.h"
#include "SPC.h"

// 抽象基类
class GameState {
public:
  enum GameError { Success = 0, BadPointer, OutOfMemory, OpenGLError };
  enum State { Intro = 0, Practice };

  // 静态方法
  static const QString errors[];
  static GameError changeState(GameState *nextState, GameState **destObj);

  // 构造器
  GameState(OpenGLWidget *widget, OpenGLRenderer *renderer)
      : widget(widget), renderer(renderer), nextState(nullptr) {}
  virtual ~GameState() {}

  // 在其他游戏状态被删除之后初始化
  virtual GameError init() = 0;

  // 处理事件
  virtual GameError event(Event *event) = 0;

  // 运行逻辑
  virtual GameError logic() = 0;

  // 渲染场景
  virtual GameError render() = 0;

  // 相同状态下为空
  GameState *getNextState() { return nextState; }

  void setWidget(OpenGLWidget *widget) { this->widget = widget; }
  void setRenderer(OpenGLRenderer *renderer) { this->renderer = renderer; }

protected:
  // 渲染窗口
  OpenGLWidget *widget;

  // 渲染对象
  OpenGLRenderer *renderer;

  GameState *nextState;
};

struct ChannelSettings {
  ChannelSettings() {
    hidden = false;
    setColor(0xFF000000);
  }
  void setColor();
  void setColor(QColor color, double dark = 0.5, double veryDark = 0.2);

  bool hidden;
  QColor primaryColor, darkColor, veryDarkColor;
};

struct TrackSettings {
  ChannelSettings channels[8];
};

class IntroScreen : public GameState {
public:
  IntroScreen(OpenGLWidget *widget, OpenGLRenderer *render)
      : GameState(widget, render) {}

  GameError event(Event *event);
  GameError init();
  GameError logic();
  GameError render();
};

class MainScreen : public GameState {
public:
  static const float KBPercent;

  MainScreen(QString SPCFile, State gameMode, OpenGLWidget *widget,
             OpenGLRenderer *renderer);

  // 游戏状态函数
  GameError event(Event *event);
  GameError init();
  GameError logic();
  GameError render();

  // 信息
  auto isValid() const { return spc.isValid(); }
  const auto &getSPC() const { return spc; }

  // 设置
  // 对 SPC 的样本通道静音在目前的实现方式上非常麻烦
  // 可以对 SPC 通道静音，并且需要重新加载文件生效
  void hideChannel(int track, int channel, bool hidden) {
    trackSettings[track].channels[channel].hidden = hidden;
  }
  void colorChannel(int track, int channel, QColor color, bool random = false);
  void setChannelSettings(const std::vector<bool> &hidden,
                          const std::vector<QColor> &color);

private:
  typedef std::vector<std::pair<qint64, int>> eventvec_t;

  // 初始化
  void initNoteMap(const std::vector<SPCEvent *> &events);
  void initColors();
  void initState();

  // 游戏逻辑
  void updateState(int pos);
  void jumpTo(qint64 startTime, bool updateGUI = true);

  // SPC 辅助函数
  qint64 getMinTime() const { return -3000000LL; }
  auto getMaxTime() const { return spc.getInfo().totalMicroSecs + 500000; }

  // 渲染函数
  void renderGlobals();
  void renderLines();
  void renderNotes();
  void renderNote(int pos);
  float getNoteX(int note);
  void renderKeys();
  void renderBorder();
  void renderText();
  void renderStatus(QRect *pos);
  void renderMessage(QRect *pos, QString msg);

  // SPC 信息
  SPC spc;                        // 播放的 SPC
  std::vector<SPCEvent *> events; // 音乐事件
  eventvec_t
      noteOns; // 映射：音符->时间->事件位置，用于对歌曲进行（更）快的随机访问
  eventvec_t nonNotes; // 用于跳转

  // 播放信息
  State gameMode;
  int startPos, endPos; // 当前窗口中开始事件和结束事件的位置
  qint64 startTime, timeSpan; // 当前窗口的开始事件和结束事件的时间
  std::vector<int> state;      // startTime 存在的音符数量
  int noteState[128];          // 上一个开启的音符
  double speed;                // 速度比例
  bool paused;                 // 暂停状态
  QElapsedTimer timer;         // 帧计时器
  bool mute;
  double volume;

  // FPS 变量
  bool showFPS;
  int FPSCount;
  qint64 FPSTime;
  double FPS;

  // 播放设备
  QAudioOutput audioOutput;
  QBuffer buffer;
  QMediaPlayer player;
  QByteArray wav;

  // 视觉信息
  static const float sharpRatio;
  static const float keyRatio;
  bool showKB;
  int keysShown;
  ChannelSettings bkg;
  ChannelSettings KBRed, KBWhite, KBSharp, KBBkg;
  std::vector<TrackSettings> trackSettings;

  float zoomX, offsetX, offsetY;
  float tempZoomX, tempOffsetX, tempOffsetY;
  bool zoomMove, trackPos, trackZoom;
  QPoint startZoom, lastPos;

  // 在 renderGlobal 中计算
  int startNote, endNote; // 音乐中的最小音高和最大音高
  float notesX, notesY, notesCX, notesCY; // 音符位置
  int allWhiteKeys;                       // 屏幕上的白键数量
  float whiteCX;                          // 白键的宽度
  qint64 rndStartTime; // 四舍五入开始时间，保持同步
};