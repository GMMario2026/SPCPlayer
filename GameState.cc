/*************************************************************************************************
 *
 * 文件：GameState.cc
 *
 * 描述：实现界面状态和渲染项目，包含交互逻辑
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#include <thread>

#include <QAudioDevice>
#include <QMediaDevices>

#include "Config.h"

const QString GameState::errors[] = {
    "成功。", "检测到不合法的指针。这可能是程序的 BUG，欢迎反馈。",
    "程序运行内存不足。", "调用 OpenGL 时出错。这可能是程序的 BUG，欢迎反馈。"};

GameState::GameError GameState::changeState(GameState *nextState,
                                            GameState **destObj) {
  // 若 nextState 为空，意味着无需转换
  if (!nextState)
    return Success;
  if (!destObj)
    return BadPointer;

  // 抛弃旧的状态。如果需要的话，设置新的渲染器和窗口
  if (*destObj) {
    if (!nextState->widget)
      nextState->widget = (*destObj)->widget;
    if (!nextState->renderer)
      nextState->renderer = (*destObj)->renderer;
    delete *destObj;
  }
  *destObj = nextState;
  GameError result = nextState->init();
  if (result) {
    qCritical()
        << "[FAIL] GameState::changeState: Failed when initializing nextState.";
    *destObj = new IntroScreen(nextState->widget, nextState->renderer);
    delete nextState;
    (*destObj)->init();
    return result;
  }

  qDebug() << "[ OK ] GameState::changeState: Successfully changed state.";
  return Success;
}

//-----------------------------------------------------------------------------
// IntroScreen 游戏状态对象
//-----------------------------------------------------------------------------

GameState::GameError IntroScreen::event(Event *event) {
  switch (event->type) {
  case Event::ChangeState:
    nextState = reinterpret_cast<GameState *>(event->param);
    qDebug() << "[ OK ] IntroScreen::event: Successully changed state.";
    return Success;
  default:
    break;
  }

  return Success;
}

GameState::GameError IntroScreen::init() { return Success; }

GameState::GameError IntroScreen::logic() {
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  return Success;
}

GameState::GameError IntroScreen::render() {
  if (renderer->resetDeviceIfNeeded() != GL_NO_ERROR)
    return OpenGLError;

  // 黑色清屏
  renderer->clear(0xFF000000);

  renderer->drawRect(0.0f, 0.0f, static_cast<float>(renderer->getBufferWidth()),
                     static_cast<float>(renderer->getBufferHeight()),
                     0xFF000000);
  renderer->endScene();

  return Success;
}

//-----------------------------------------------------------------------------
// MainScreen 游戏状态对象
//-----------------------------------------------------------------------------

MainScreen::MainScreen(QString SPCFile, State gameMode, OpenGLWidget *widget,
                       OpenGLRenderer *renderer)
    : GameState(widget, renderer), spc(SPCFile), gameMode(gameMode) {
  // 完成 SPC 处理
  if (!spc.isValid())
    return;
  auto &events = spc.getEvents();

  // 分配
  trackSettings.resize(spc.getInfo().numTracks);
  state.reserve(128);

  // 初始化
  initNoteMap(events);
  initColors();
  initState();
  qDebug() << "[ OK ] MainScreen::MainScreen: Finished initialization.";
}

void MainScreen::initNoteMap(const std::vector<SPCEvent *> &events) {
  // 获取事件
  this->events.reserve(events.size());
  this->noteOns.reserve(events.size() / 2);
  for (auto it = events.begin(); it != events.end(); ++it) {
    auto event = *it;
    this->events.push_back(event);

    // 让随机访问更快，但不清楚是否值当
    auto eventType = event->getEventType();
    if (eventType == SPCEvent::KeyOn)
      noteOns.push_back(std::pair<qint64, int>(event->getAbsMicroSec(),
                                               this->events.size() - 1));
    else
      nonNotes.push_back(std::pair<qint64, int>(event->getAbsMicroSec(),
                                                this->events.size() - 1));
  }
}

// 显示的颜色
void MainScreen::initColors() {
  bkg.setColor(0x00464646, 0.7f, 1.3f);
  KBBkg.setColor(0x00999999, 0.4f, 0.0f);
  KBRed.setColor(0x00980A0D, 0.5f);
  KBWhite.setColor(0x00FFFFFF, 0.8f, 0.6f);
  KBSharp.setColor(0x00404040, 0.5f, 0.0f);
}

// 初始化状态变量。确保状态变量有效。
void MainScreen::initState() {
  static auto &config = Config::getConfig();
  static const auto &playback = config.getPlaybackSettings();
  static const auto &view = config.getViewSettings();

  gameMode = Practice;
  startPos = 0;
  endPos = -1;
  startTime = getMinTime();
  trackPos = trackZoom = false;
  tempZoomX = 1.0f;
  tempOffsetX = tempOffsetY = 0.0f;
  FPS = 0.0;
  FPSCount = 0;
  FPSTime = 0;
  speed = -1.0; // 强制在第一次调用 logic 时重置

  zoomX = view.getZoomX();
  offsetX = view.getOffsetX();
  offsetY = view.getOffsetY();
  paused = false;
  mute = playback.getMute();
  auto nSpeed = playback.getNSpeed();
  timeSpan = static_cast<qint64>(3.0 * nSpeed * 1000000);

  memset(noteState, -1, sizeof(noteState));
}

// 在切换到该状态之前直接执行
GameState::GameError MainScreen::init() {
  wav = Util::pcm2Wav(spc.getWave());
  buffer.setBuffer(&wav);
  if (buffer.open(QIODevice::ReadOnly)) {
    player.setAudioOutput(&audioOutput);
    player.setSourceDevice(&buffer);
  } else
    qCritical() << "[FAIL] MainScreen::initialize: Failed to open wave!";

  return Success;
}

void MainScreen::colorChannel(int track, int channel, QColor color,
                              bool random) {
  if (random)
    trackSettings[track].channels[channel].setColor();
  else
    trackSettings[track].channels[channel].setColor(color);
}

// 设置随机颜色
void ChannelSettings::setColor() { setColor(Util::randColor(), 0.6, 0.2); }

// 使用 QColor 可以减少一些颜色通道顺序的麻烦
void ChannelSettings::setColor(QColor color, double dark, double veryDark) {
  int H, S, V;
  color.getHsv(&H, &S, &V);
  primaryColor = color;
  darkColor = QColor::fromHsv(H, S, std::min(255, static_cast<int>(V * dark)),
                              color.alpha());
  veryDarkColor = QColor::fromHsv(
      H, S, std::min(255, static_cast<int>(V * veryDark)), color.alpha());
}

void MainScreen::setChannelSettings(const std::vector<bool> &hidden,
                                    const std::vector<QColor> &color) {
  auto &info = spc.getInfo();
  auto &tracks = spc.getTracks();

  bool hiddenFlag = hidden.size() > 0;
  bool colorFlag = color.size() > 0;

  size_t pos = 0;
  for (int i = 0; i < info.numTracks; i++) {
    auto &trackInfo = tracks[i]->getInfo();
    for (int j = 0; j < 8; j++)
      if (trackInfo.noteCount[j] > 0) {
        hideChannel(i, j,
                    hiddenFlag && hidden[std::min(pos, hidden.size() - 1)]);
        if (colorFlag && pos < color.size())
          colorChannel(i, j, color[pos]);
        else
          colorChannel(i, j, 0U, true);
        pos++;
      }
  }
}

GameState::GameError MainScreen::event(Event *event) {
  static auto &config = Config::getConfig();
  static auto &playback = config.getPlaybackSettings();
  static auto &view = config.getViewSettings();
  static auto &controls = config.getControlsSettings();

  switch (event->type) {
  case Event::ChangeState:
    nextState = reinterpret_cast<GameState *>(event->param);
    qDebug() << "[ OK ] MainScreen::event: Successfully changed state.";
    return Success;
  case Event::Stop:
    jumpTo(getMinTime());
    playback.setStopped(true);
    return Success;
  case Event::SkipForward:
    jumpTo(static_cast<qint64>(startTime + controls.fwdBackSecs * 1000000));
    return Success;
  case Event::SkipBackward:
    jumpTo(static_cast<qint64>(startTime - controls.fwdBackSecs * 1000000));
    return Success;
  case Event::MoveAndZoom:
    if (view.getZoomMove()) {
      view.setOffsetX(view.getOffsetX() + tempOffsetX);
      view.setOffsetY(view.getOffsetY() + tempOffsetY);
      view.setZoomX(view.getZoomX() * tempZoomX);
    } else {
      view.setZoomMove(true, true);
      return Success;
    }
  case Event::CancelMoveAndZoom:
    view.setZoomMove(false, true);
    trackPos = trackZoom = false;
    tempOffsetX = 0.0f;
    tempOffsetY = 0.0f;
    tempZoomX = 1.0f;
    return Success;
  case Event::ResetMoveAndZoom:
    view.setOffsetX(0.0f);
    view.setOffsetY(0.0f);
    view.setZoomX(1.0f);
    tempOffsetX = 0.0f;
    tempOffsetY = 0.0f;
    tempZoomX = 1.0f;
    return Success;
  case Event::SetPosition: {
    auto firstTime = getMinTime();
    auto lastTime = getMaxTime();
    jumpTo(firstTime + ((lastTime - firstTime) * event->param) / 1000, false);
    break;
  }
  case Event::LeftButtonPress:
    if (zoomMove) {
      lastPos.setX(event->param >> 32 & 0xFFFFFFFF);
      lastPos.setY(event->param & 0xFFFFFFFF);
      trackPos = true;
    }
    return Success;
  case Event::RightButtonPress:
    if (!zoomMove)
      return Success;
    lastPos.setX(event->param >> 32 & 0xFFFFFFFF);
    lastPos.setY(event->param & 0xFFFFFFFF);
    startZoom.setX(static_cast<int>(lastPos.x() - offsetX - tempOffsetX) /
                   (zoomX * tempZoomX));
    startZoom.setY(static_cast<int>(lastPos.y() - offsetY - tempOffsetY));
    trackZoom = true;
    return Success;
  case Event::LeftButtonRelease:
    trackPos = false;
    return Success;
  case Event::RightButtonRelease:
    trackZoom = false;
    return Success;
  case Event::MouseMove: {
    if (!trackPos && !trackZoom && !paused)
      return Success;
    int x = event->param >> 32 & 0xFFFFFFFF;
    int y = event->param & 0xFFFFFFFF;
    int dx = static_cast<int>(x - lastPos.x());
    int dy = static_cast<int>(y - lastPos.y());

    if (trackPos) {
      tempOffsetX += dx;
      tempOffsetY += dy;
    }
    if (trackZoom) {
      auto oldX = offsetX + tempOffsetX + startZoom.x() * zoomX * tempZoomX;
      tempZoomX *= pow(2.0f, dx / 200.0f);
      auto newX = offsetX + tempOffsetX + startZoom.x() * zoomX * tempZoomX;
      tempOffsetX = tempOffsetX - (newX - oldX);
    }

    lastPos.setX(x);
    lastPos.setY(y);
    return Success;
  }
  default:
    break;
  }

  return Success;
}

GameState::GameError MainScreen::logic() {
  static auto &config = Config::getConfig();
  static auto &playback = config.getPlaybackSettings();
  static const auto &view = config.getViewSettings();
  static auto &visual = config.getVisualSettings();
  static auto &video = config.getVideoSettings();
  auto info = spc.getInfo();

  // 检测状态的改变
  auto paused = playback.getPaused();
  auto speed = playback.getSpeed();
  auto nSpeed = playback.getNSpeed();
  auto mute = playback.getMute();
  auto timeSpan = static_cast<qint64>(3.0 * nSpeed * 1000000);
  auto pausedChanged = (paused != this->paused);

  // 设置状态
  this->paused = paused;
  this->speed = speed;
  this->mute = mute;
  this->timeSpan = timeSpan;
  volume = playback.getVolume();
  showKB = view.getKeyBoard();
  zoomMove = view.getZoomMove();
  offsetX = view.getOffsetX();
  offsetY = view.getOffsetY();
  zoomX = view.getZoomX();
  if (!zoomMove)
    trackPos = trackZoom = false;
  keysShown = visual.keysShown;
  startNote = std::min(visual.firstKey, visual.lastKey);
  endNote = std::max(visual.firstKey, visual.lastKey);
  showFPS = video.showFPS;
  if (visual.bkgColor != bkg.primaryColor)
    bkg.setColor(visual.bkgColor, 0.7f, 1.3f);

  // 时间
  auto maxTime = getMaxTime();
  auto elapsed = std::max(0LL, timer.nsecsElapsed() / 1000);
  timer.start();

  // 每半秒计算一次 FPS
  FPSTime += elapsed;
  FPSCount++;
  if (FPSTime >= 500000) {
    FPS = FPSCount / (FPSTime / 1000000.0);
    FPSTime = FPSCount = 0;
  }

  // 播放器设置
  if (pausedChanged && this->paused)
    player.pause();
  if (audioOutput.device() != QMediaDevices::defaultAudioOutput()) {
    audioOutput.setDevice(QMediaDevices::defaultAudioOutput());
    qDebug() << "[ OK ] MainScreen::logic: Sucessfully changed to"
             << QMediaDevices::defaultAudioOutput().description();
  }
  audioOutput.setVolume(playback.getVolume());
  audioOutput.setMuted(mute);

  // 确定开始和结束的时间，用于显示
  auto oldStartTime = startTime;
  auto nextStartTime = startTime + static_cast<qint64>(elapsed * speed + 0.5);

  if (!this->paused && startTime < maxTime)
    startTime = nextStartTime;
  auto endTime = startTime + this->timeSpan;

  renderGlobals();

  // 推进位置
  auto eventCount = (int)events.size();
  while (endPos + 1 < eventCount &&
         events[endPos + 1]->getAbsMicroSec() < endTime)
    endPos++;

  // 只希望在未暂停时推进开始位置，因为推进开始位置 “消耗” 事件
  if (!paused) {
    // 播放音乐
    if (!player.isPlaying() && startTime > 0) {
      player.setPosition(startTime / 1000);
      player.play();
    }
    // 更新状态
    while (startPos < eventCount &&
           events[startPos]->getAbsMicroSec() <= startTime)
      updateState(startPos++);
  }

  // 更新进度条
  auto firstTime = getMinTime();
  auto lastTime = getMaxTime();
  auto oldPos = ((oldStartTime - firstTime) * 1000) / (lastTime - firstTime);
  auto newPos = ((startTime - firstTime) * 1000) / (lastTime - firstTime);
  if (oldPos != newPos)
    playback.setPosition(static_cast<int>(newPos));

  // 音乐结束
  if (!this->paused && startTime >= maxTime)
    playback.setPaused(true, true);
  return Success;
}

void MainScreen::updateState(int pos) {
  // 事件信息
  auto *event = events[pos];
  auto eventType = event->getEventType();
  auto note = event->getNote();

  // 按下按键
  if (eventType == SPCEvent::KeyOn) {
    state.push_back(pos);
    noteState[note] = pos;
  } else {
    noteState[note] = -1;
    auto search = event->getSister();
    // 线性搜索和擦除。考虑到 N 是同时演奏的音符的数量，这不会导致卡顿
    auto it = state.begin();
    while (it != state.end()) {
      if (events[*it] == search)
        it = state.erase(it);
      else {
        if (it != state.end() && events[*it]->getNote() == note)
          noteState[note] = *it;
        ++it;
      }
    }
  }
}

void MainScreen::jumpTo(qint64 startTime, bool updateGUI) {
  // 先暂停!
  player.pause();

  // 开始时间。小菜一碟！
  auto firstTime = getMinTime();
  auto lastTime = getMaxTime();
  this->startTime = std::min(std::max(startTime, firstTime), lastTime);
  auto endTime = this->startTime + timeSpan;

  // 开始位置和当前状态：困难！
  auto begin = noteOns.begin();
  auto end = noteOns.end();
  // 用 lower bound 减少 simultaneous 复杂度
  auto middle =
      std::lower_bound(begin, end, std::pair<qint64, int>(startTime, 0));

  // 开始位置
  startPos = (int)events.size();
  if (middle != end && middle->second < startPos)
    startPos = middle->second;
  auto nonNote = std::lower_bound(nonNotes.begin(), nonNotes.end(),
                                  std::pair<qint64, int>(startTime, 0));
  if (nonNote != nonNotes.end() && nonNote->second < startPos)
    startPos = nonNote->second;

  // 找出与前一个音符同时出现的音符
  state.clear();
  memset(noteState, -1, sizeof(noteState));
  if (middle != begin) {
    auto prev = middle - 1;
    int found = 0;
    int simultaneous = events[prev->second]->getSimultaneous() + 1;
    for (eventvec_t::reverse_iterator it(middle);
         found < simultaneous && it != noteOns.rend(); ++it) {
      auto *event = events[it->second];
      auto sister = event->getSister();
      if (sister->getAbsMicroSec() > prev->first) // > 因为 middle 是时间最大值
        found++;
      if (sister->getAbsMicroSec() > startTime) // > 因为我们不关心重音结束音符
      {
        state.push_back(it->second);
        if (noteState[event->getNote()] < 0)
          noteState[event->getNote()] = it->second;
      }
    }
    std::reverse(state.begin(), state.end());
  }

  // 结束位置：有些棘手。和逻辑代码一样。只在暂停跳转时需要
  endPos = startPos - 1;
  auto eventCnt = (int)events.size();
  while (endPos + 1 < eventCnt &&
         events[endPos + 1]->getAbsMicroSec() < endTime)
    endPos++;

  player.setPosition(startTime < 0 ? 0 : startTime / 1000);

  if (updateGUI) {
    static auto &playback = Config::getConfig().getPlaybackSettings();
    auto newPos = ((startTime - firstTime) * 1000) / (lastTime - firstTime);
    playback.setPosition(static_cast<int>(newPos));
  }
}

const float MainScreen::sharpRatio = 0.65f;
const float MainScreen::KBPercent = 0.25f;
const float MainScreen::keyRatio = 0.1775f;

GameState::GameError MainScreen::render() {
  if (renderer->resetDeviceIfNeeded() != GL_NO_ERROR)
    return OpenGLError;

  renderer->clear(0xFF000000);

  renderLines();
  renderNotes();
  if (showKB)
    renderKeys();
  renderBorder();
  renderText();
  renderer->endScene();

  return Success;
}

// 这些过去是在每个 Render* 函数内部作为局部变量创建的，但代码复制太多 :/
// 依赖于 startTime、timeSpan、keysShown、startNote、endNote
void MainScreen::renderGlobals() {
  // SPC 信息
  auto &info = spc.getInfo();
  if (keysShown == VisualSettings::All) {
    startNote = std::min(startNote, SPC::A0);
    endNote = std::max(endNote, SPC::C8);
  } else if (keysShown == VisualSettings::Song) {
    startNote = info.minNote;
    endNote = info.maxNote;
  }

  // 屏幕的 X 轴信息
  notesX = offsetX + tempOffsetX;
  notesCX = renderer->getBufferWidth() * zoomX * tempZoomX;

  // 按键消息
  allWhiteKeys = SPC::whiteCount(startNote, endNote + 1);
  auto buf = (SPC::isSharp(startNote) ? sharpRatio / 2.0f : 0.0f) +
             (SPC::isSharp(endNote) ? sharpRatio / 2.0f : 0.0f);
  whiteCX = notesCX / (allWhiteKeys + buf);

  // 屏幕的 Y 轴信息
  notesY = offsetY + tempOffsetY;
  if (!showKB)
    notesCY = static_cast<float>(renderer->getBufferHeight());
  else {
    auto maxKeyCY = renderer->getBufferHeight() * KBPercent;
    auto idealKeyCY = whiteCX / keyRatio;
    // 顶部 0.95，分隔 2.0，红色过渡 0.93（估计）
    idealKeyCY = (idealKeyCY / 0.95f + 2.0f) / 0.93f;
    notesCY = floor(renderer->getBufferHeight() -
                    std::min(idealKeyCY, maxKeyCY) + 0.5f);
  }

  // 四舍五入开始时间。这仅用于渲染目的
  auto microSecsPP = static_cast<qint64>(timeSpan / notesCY + 0.5f);
  rndStartTime = startTime - (startTime < 0 ? microSecsPP : 0);
  rndStartTime = (rndStartTime / microSecsPP) * microSecsPP;
}

void MainScreen::renderLines() {
  renderer->drawRect(notesX, notesY, notesCX, notesCY, bkg.primaryColor);

  // 竖直线
  for (int i = startNote + 1; i <= endNote; i++)
    if (!SPC::isSharp(i - 1) && !SPC::isSharp(i)) {
      auto whiteKeys = SPC::whiteCount(startNote, i);
      auto startX = SPC::isSharp(startNote) * sharpRatio / 2.0f;
      auto x = notesX + whiteCX * (whiteKeys + startX);
      x = floor(x + 0.5f); // 四舍五入
      renderer->drawRect(x - 1.0f, notesY, 3.0f, notesCY, bkg.darkColor,
                         bkg.veryDarkColor, bkg.veryDarkColor, bkg.darkColor);
    }

  // SPC 文件没有 BPM 等信息，因此我们不渲染小节的横线
}

void MainScreen::renderNotes() {
  // 有没有音符需要渲染？
  if (endPos < 0 || startPos >= static_cast<int>(events.size()))
    return;

  // 渲染音符，注意先渲染白键再渲染黑键以避免被遮挡
  bool hasSharp = false;
  for (auto pos : state)
    if (!SPC::isSharp(events[pos]->getNote()))
      renderNote(pos);
    else
      hasSharp = true;

  for (int i = startPos; i <= endPos; i++) {
    auto *event = events[i];
    if (event->getEventType() == SPCEvent::KeyOn && event->getSister()) {
      if (!SPC::isSharp(event->getNote()))
        renderNote(i);
      else
        hasSharp = true;
    }
  }

  // 再对黑键做一遍
  if (hasSharp) {
    for (auto pos : state)
      if (SPC::isSharp(events[pos]->getNote()))
        renderNote(pos);

    for (int i = startPos; i <= endPos; i++) {
      auto *event = events[i];
      if (event->getEventType() == SPCEvent::KeyOn && event->getSister() &&
          SPC::isSharp(event->getNote()))
        renderNote(i);
    }
  }
}

void MainScreen::renderNote(int pos) {
  const auto *note = events[pos];
  auto pitch = note->getNote();
  auto track = note->getTrack();
  auto channel = note->getChannel();
  auto noteStart = note->getAbsMicroSec();
  auto noteEnd = note->getSister()->getAbsMicroSec();

  auto &trackSetting = trackSettings[track].channels[channel];
  if (trackSettings[track].channels[channel].hidden)
    return;

  // 计算准确位置
  auto x = getNoteX(pitch);
  auto y = notesY +
           notesCY *
               (1.0f - static_cast<float>(noteStart - rndStartTime) / timeSpan);
  auto cx = SPC::isSharp(pitch) ? whiteCX * sharpRatio : whiteCX;
  auto cy = notesCY * (static_cast<float>(noteEnd - noteStart) / timeSpan);
  float deflate = whiteCX * 0.15f / 2.0f;

  // 四舍五入，保持一致
  cy = floor(cy + 0.5f);
  y = floor(y + 0.5f);
  deflate = floor(deflate + 0.5f);
  deflate = std::max(std::min(deflate, 3.0f), 1.0f);

  // 切割 :/
  auto minY = notesY - 5.0f;
  auto maxY = notesY + notesCY + 5.0f;
  if (y > maxY) {
    cy -= (y - maxY);
    y = maxY;
  }
  if (y - cy < minY) {
    cy -= (minY - (y - cy));
    y = minY + cy;
  }

  renderer->drawRect(x, y - cy, cx, cy, trackSetting.veryDarkColor);
  renderer->drawRect(x + deflate, y - cy + deflate, cx - deflate * 2.0f,
                     cy - deflate * 2.0f, trackSetting.primaryColor,
                     trackSetting.darkColor, trackSetting.darkColor,
                     trackSetting.primaryColor);
}

float MainScreen::getNoteX(int note) {
  auto whiteKeys = SPC::whiteCount(startNote, note);
  auto startX =
      (SPC::isSharp(startNote) - SPC::isSharp(note)) * sharpRatio / 2.0f;
  if (SPC::isSharp(note)) {
    auto noteVal = SPC::noteValue(note);
    if (noteVal == SPC::CS || noteVal == SPC::FS)
      startX -= sharpRatio / 5.0f;
    else if (noteVal == SPC::AS || noteVal == SPC::DS)
      startX += sharpRatio / 5.0f;
  }
  return notesX + whiteCX * (whiteKeys + startX);
}

void MainScreen::renderKeys() {
  // 屏幕信息
  auto keysY = notesY + notesCY;
  auto keysCY = renderer->getBufferHeight() - notesCY;

  auto transitionPct = 0.02f;
  auto transitionCY = std::max(3.0f, floor(keysCY * transitionPct + 0.5f));
  auto redPct = 0.05f;
  auto redCY = floor(keysCY * redPct + 0.5f);
  auto spacerCY = 2.0f;
  auto topCY = floor((keysCY - spacerCY - redCY - transitionCY) * 0.95f + 0.5f);
  auto nearCY = keysCY - spacerCY - redCY - transitionCY - topCY;

  // 绘制背景
  renderer->drawRect(notesX, keysY, notesCX, keysCY, KBBkg.veryDarkColor);
  renderer->drawRect(notesX, keysY, notesCX, transitionCY, bkg.primaryColor,
                     bkg.primaryColor, KBBkg.veryDarkColor,
                     KBBkg.veryDarkColor);
  renderer->drawRect(notesX, keysY + transitionCY, notesCX, redCY,
                     KBRed.darkColor, KBRed.darkColor, KBRed.primaryColor,
                     KBRed.primaryColor);
  renderer->drawRect(notesX, keysY + transitionCY + redCY, notesCX, spacerCY,
                     KBBkg.darkColor, KBBkg.darkColor, KBBkg.darkColor,
                     KBBkg.darkColor);

  // 琴键信息
  auto keyGap = std::max(1.0f, floor(whiteCX * 0.05f + 0.5f));
  auto keyGap1 = keyGap - floor(keyGap / 2.0f + 0.5f);

  auto startRender = startNote - SPC::isSharp(startNote);
  auto endRender = endNote + SPC::isSharp(endNote);
  auto startX =
      (SPC::isSharp(startNote) ? whiteCX * (sharpRatio / 2.0f - 1.0f) : 0.0f);
  auto sharpCY = topCY * 0.67f;

  // 绘制白键
  auto curX = notesX + startX;
  auto curY = keysY + transitionCY + redCY + spacerCY;

  for (int i = startRender; i <= endRender; i++)
    if (!SPC::isSharp(i)) {
      if (noteState[i] == -1) {
        renderer->drawRect(curX + keyGap1, curY, whiteCX - keyGap,
                           topCY + nearCY, KBWhite.darkColor, KBWhite.darkColor,
                           KBWhite.primaryColor, KBWhite.primaryColor);
        renderer->drawRect(curX + keyGap1, curY + topCY, whiteCX - keyGap,
                           nearCY, KBWhite.darkColor, KBWhite.darkColor,
                           KBWhite.veryDarkColor, KBWhite.veryDarkColor);
        renderer->drawRect(curX + keyGap1, curY + topCY, whiteCX - keyGap, 2.0f,
                           KBBkg.darkColor, KBBkg.darkColor,
                           KBWhite.veryDarkColor, KBWhite.veryDarkColor);

        if (i == SPC::C4) {
          auto mXGap = floor(whiteCX * 0.25f + 0.5f);
          auto mCX = whiteCX - mXGap * 2.0f - keyGap;
          auto mY = std::max(curY + topCY - mCX - 5.0f, curY + sharpCY + 5.0f);
          renderer->drawRect(curX + keyGap1 + mXGap, mY, mCX,
                             curY + topCY - 5.0f - mY, KBWhite.darkColor);
        }
      } else {
        const auto *event = (noteState[i] >= 0 ? events[noteState[i]] : nullptr);
        const auto track = (event ? event->getTrack() : -1);
        const auto channel = (event ? event->getChannel() : -1);

        auto &KBWhite = trackSettings[track].channels[channel];
        renderer->drawRect(curX + keyGap1, curY, whiteCX - keyGap,
                           topCY + nearCY - 2.0f, KBWhite.darkColor,
                           KBWhite.darkColor, KBWhite.primaryColor,
                           KBWhite.primaryColor);
        renderer->drawRect(curX + keyGap1, curY + topCY + nearCY - 2.0f,
                           whiteCX - keyGap, 2.0f, KBWhite.darkColor);

        if (i == SPC::C4) {
          auto mXGap = floor(whiteCX * 0.25f + 0.5f);
          auto mCX = whiteCX - mXGap * 2.0f - keyGap;
          auto mY = std::max(curY + topCY + nearCY - mCX - 7.0f,
                             curY + sharpCY + 5.0f);
          renderer->drawRect(curX + keyGap1 + mXGap, mY, mCX,
                             curY + topCY + nearCY - 7.0f - mY,
                             KBWhite.darkColor);
        }
      }
      renderer->drawRect(curX + keyGap1 + whiteCX - keyGap, curY,
                         keyGap - keyGap1, topCY + nearCY, KBBkg.darkColor,
                         KBBkg.darkColor, KBBkg.darkColor, KBBkg.darkColor);

      curX += whiteCX;
    }

  // 绘制黑键
  startRender = startNote - (startNote != SPC::A0 && !SPC::isSharp(startNote) &&
                             startNote > 0 && SPC::isSharp(startNote - 1));
  endRender = endNote + (endNote != SPC::C8 && !SPC::isSharp(endNote) &&
                         endNote < 127 && SPC::isSharp(endNote + 1));
  startX = (SPC::isSharp(startNote) ? whiteCX * sharpRatio / 2.0f : 0.0f);

  auto sharpTop = sharpRatio * 0.7f;
  curX = notesX + startX;
  curY = keysY + transitionCY + redCY + spacerCY;
  for (int i = startRender; i <= endRender; i++)
    if (!SPC::isSharp(i))
      curX += whiteCX;
    else {
      auto nudgeX = 0.0f;
      auto note = SPC::noteValue(i);
      if (note == SPC::CS || note == SPC::FS)
        nudgeX = -sharpRatio / 5.0f;
      else if (note == SPC::AS || note == SPC::DS)
        nudgeX = sharpRatio / 5.0f;

      const auto cx = whiteCX * sharpRatio;
      const auto x = curX - whiteCX * (sharpRatio / 2.0f - nudgeX);
      const auto sharpTopX1 = x + whiteCX * (sharpRatio - sharpTop) / 2.0f;
      const auto sharpTopX2 = sharpTopX1 + whiteCX * sharpTop;

      if (noteState[i] == -1) {
        renderer->drawSkew(sharpTopX1, curY + sharpCY - nearCY, sharpTopX2,
                           curY + sharpCY - nearCY, x + cx, curY + sharpCY, x,
                           curY + sharpCY, KBSharp.primaryColor,
                           KBSharp.primaryColor, KBSharp.veryDarkColor,
                           KBSharp.veryDarkColor);
        renderer->drawSkew(sharpTopX1, curY - nearCY, sharpTopX1,
                           curY + sharpCY - nearCY, x, curY + sharpCY, x, curY,
                           KBSharp.primaryColor, KBSharp.primaryColor,
                           KBSharp.veryDarkColor, KBSharp.veryDarkColor);
        renderer->drawSkew(sharpTopX2, curY + sharpCY - nearCY, sharpTopX2,
                           curY - nearCY, x + cx, curY, x + cx, curY + sharpCY,
                           KBSharp.primaryColor, KBSharp.primaryColor,
                           KBSharp.veryDarkColor, KBSharp.veryDarkColor);
        renderer->drawRect(sharpTopX1, curY - nearCY, sharpTopX2 - sharpTopX1,
                           sharpCY, KBSharp.veryDarkColor);
        renderer->drawSkew(sharpTopX1, curY - nearCY, sharpTopX2, curY - nearCY,
                           sharpTopX2, curY - nearCY + sharpCY * 0.45f,
                           sharpTopX1, curY - nearCY + sharpCY * 0.35f,
                           KBSharp.darkColor, KBSharp.darkColor,
                           KBSharp.primaryColor, KBSharp.primaryColor);
        renderer->drawSkew(sharpTopX1, curY - nearCY + sharpCY * 0.35f,
                           sharpTopX2, curY - nearCY + sharpCY * 0.45f,
                           sharpTopX2, curY - nearCY + sharpCY * 0.65f,
                           sharpTopX1, curY - nearCY + sharpCY * 0.55f,
                           KBSharp.primaryColor, KBSharp.primaryColor,
                           KBSharp.veryDarkColor, KBSharp.veryDarkColor);
      } else {
        const auto *event = (noteState[i] >= 0 ? events[noteState[i]] : nullptr);
        const auto track = (event ? event->getTrack() : -1);
        const auto channel = (event ? event->getChannel() : -1);

        const auto newNear = nearCY * 0.25f;

        const auto &KBSharp = trackSettings[track].channels[channel];
        renderer->drawSkew(sharpTopX1, curY + sharpCY - newNear, sharpTopX2,
                           curY + sharpCY - newNear, x + cx, curY + sharpCY, x,
                           curY + sharpCY, KBSharp.primaryColor,
                           KBSharp.primaryColor, KBSharp.darkColor,
                           KBSharp.darkColor);
        renderer->drawSkew(sharpTopX1, curY - newNear, sharpTopX1,
                           curY + sharpCY - newNear, x, curY + sharpCY, x, curY,
                           KBSharp.primaryColor, KBSharp.primaryColor,
                           KBSharp.darkColor, KBSharp.darkColor);
        renderer->drawSkew(sharpTopX2, curY + sharpCY - newNear, sharpTopX2,
                           curY - newNear, x + cx, curY, x + cx, curY + sharpCY,
                           KBSharp.primaryColor, KBSharp.primaryColor,
                           KBSharp.darkColor, KBSharp.darkColor);
        renderer->drawRect(sharpTopX1, curY - newNear, sharpTopX2 - sharpTopX1,
                           sharpCY, KBSharp.darkColor);
        renderer->drawSkew(
            sharpTopX1, curY - newNear, sharpTopX2, curY - newNear, sharpTopX2,
            curY - newNear + sharpCY * 0.35f, sharpTopX1,
            curY - newNear + sharpCY * 0.25f, KBSharp.primaryColor,
            KBSharp.primaryColor, KBSharp.primaryColor, KBSharp.primaryColor);
        renderer->drawSkew(sharpTopX1, curY - newNear + sharpCY * 0.25f,
                           sharpTopX2, curY - newNear + sharpCY * 0.35f,
                           sharpTopX2, curY - newNear + sharpCY * 0.75f,
                           sharpTopX1, curY - newNear + sharpCY * 0.65f,
                           KBSharp.primaryColor, KBSharp.primaryColor,
                           KBSharp.darkColor, KBSharp.darkColor);
      }
    }
}

void MainScreen::renderBorder() {
  // 上下左右
  const QColor black(0xFF000000);
  const auto bufCY = static_cast<float>(renderer->getBufferHeight());
  renderer->drawRect(notesX - 50.0f, notesY - 50.0f, notesCX + 100.0f, 50.0f,
                     black);
  renderer->drawRect(notesX - 50.0f, notesY + bufCY, notesCX + 100.0f, 50.0f,
                     black);
  renderer->drawRect(notesX - whiteCX, notesY - 50.0f, whiteCX, bufCY + 100.0f,
                     black);
  renderer->drawRect(notesX + notesCX, notesY - 50.0f, whiteCX, bufCY + 100.0f,
                     black);

  const auto pad = 10.0f;
  const auto bkg = this->bkg.primaryColor;
  renderer->drawSkew(notesX, notesY + bufCY, notesX + notesCX, notesY + bufCY,
                     notesX + notesCX + pad, notesY + bufCY + pad, notesX - pad,
                     notesY + bufCY + pad, bkg, bkg, black, black);
  renderer->drawSkew(notesX - pad, notesY - pad, notesX + notesCX + pad,
                     notesY - pad, notesX + notesCX, notesY, notesX, notesY,
                     black, black, bkg, bkg);
  renderer->drawSkew(notesX - pad, notesY - pad, notesX, notesY, notesX,
                     notesY + bufCY, notesX - pad, notesY + bufCY + pad, black,
                     bkg, bkg, black);
  renderer->drawSkew(notesX + notesCX, notesY, notesX + notesCX + pad,
                     notesY - pad, notesX + notesCX + pad,
                     notesY + notesCY + pad, notesX + notesCX, notesY + notesCY,
                     bkg, black, black, bkg);
}

void MainScreen::renderText() {
  auto lines = 1;
  if (showFPS)
    lines++;

  // 屏幕信息
  QRect status = {QPoint(renderer->getBufferWidth() - 156, 0),
                  QPoint(renderer->getBufferWidth(), 6 + 16 * lines)};

  auto msgCY = 200;
  QRect msg;
  msg.setTopLeft(
      QPoint(0, static_cast<int>(
                    renderer->getBufferHeight() * (1.0f - KBPercent) - msgCY) /
                    2));
  msg.setRight(renderer->getBufferWidth());
  msg.setBottom(msg.top() + msgCY);

  // 绘制背景
  QColor bkgColor = QColor::fromRgba(0xC0000000);
  renderer->drawRect(static_cast<float>(status.left()),
                     static_cast<float>(status.top()),
                     static_cast<float>(status.right() - status.left()),
                     static_cast<float>(status.bottom() - status.top()),
                     QColor::fromRgba(0x80000000));
  if (zoomMove)
    renderer->drawRect(static_cast<float>(msg.left()),
                       static_cast<float>(msg.top()),
                       static_cast<float>(msg.right() - msg.left()),
                       static_cast<float>(msg.bottom() - msg.top()), bkgColor);

  // 绘制文字
  renderer->beginText();

  renderStatus(&status);
  if (zoomMove)
    renderMessage(&msg,
                  "- 单击左键并拖动以移动屏幕\n- 单击右键并拖动以水平缩放\n- "
                  "按 Esc 键取消更改\n- 按 Ctrl+V 键保存更改");

  renderer->endText();
}

void MainScreen::renderStatus(QRect *status) {
  // 设置时间字符串
  char time[128];
  const auto info = spc.getInfo();
  if (startTime >= 0)
    sprintf_s(time, "%lld:%04.1lf / %lld:%04.1lf", startTime / 60000000,
              (startTime % 60000000) / 1000000.0,
              info.totalMicroSecs / 60000000,
              (info.totalMicroSecs % 60000000) / 1000000.0);
  else
    sprintf_s(time, "\t-%lld:%04.1lf / %lld:%04.1lf", -startTime / 60000000,
              (-startTime % 60000000) / 1000000.0,
              info.totalMicroSecs / 60000000,
              (info.totalMicroSecs % 60000000) / 1000000.0);

  // 设置 FPS 字符串
  char FPSStr[128];
  sprintf_s(FPSStr, "%.1lf", FPS);

  // 显示文字
  status->adjust(6, 3, -6, -3);

  status->translate(2, 1);
  renderer->drawText("Time:", OpenGLRenderer::Small, status, 0, 0xFF404040);
  renderer->drawText(time, OpenGLRenderer::Small, status, Qt::AlignRight,
                     0xFF404040);
  status->translate(-2, -1);
  renderer->drawText("Time:", OpenGLRenderer::Small, status, 0, 0xFFFFFFFF);
  renderer->drawText(time, OpenGLRenderer::Small, status, Qt::AlignRight,
                     0xFFFFFFFF);

  if (showFPS) {
    status->translate(2, 16 + 1);
    renderer->drawText("FPS:", OpenGLRenderer::Small, status, 0, 0xFF404040);
    renderer->drawText(FPSStr, OpenGLRenderer::Small, status, Qt::AlignRight,
                       0xFF404040);
    status->translate(-2, -1);
    renderer->drawText("FPS:", OpenGLRenderer::Small, status, 0, 0xFFFFFFFF);
    renderer->drawText(FPSStr, OpenGLRenderer::Small, status, Qt::AlignRight,
                       0xFFFFFFFF);
  }
}

void MainScreen::renderMessage(QRect *msgRectPtr, QString msg) {
  QRect msgRect = {};
  auto fontSize = OpenGLRenderer::Medium;
  renderer->drawText(msg, fontSize, &msgRect, DT_CALCRECT, 0xFF000000);
  if (msgRect.right() > renderer->getBufferWidth()) {
    fontSize = OpenGLRenderer::Small;
    renderer->drawText(msg, fontSize, &msgRect, DT_CALCRECT, 0xFF000000);
  }
  msgRect.translate(
      2 + msgRectPtr->left() +
          (msgRectPtr->right() - msgRectPtr->left() - msgRect.right()) / 2,
      2 + msgRectPtr->top() +
          (msgRectPtr->bottom() - msgRectPtr->top() - msgRect.bottom()) / 2);
  renderer->drawText(msg, fontSize, &msgRect, Qt::AlignLeft | Qt::AlignTop,
                     0xFF404040);
  msgRect.translate(-2, -2);
  renderer->drawText(msg, fontSize, &msgRect, Qt::AlignLeft | Qt::AlignTop,
                     0xFFFFFFFF);
}