/*************************************************************************************************
 *
 * 文件：MainWindow.cc
 *
 * 描述：窗体的行为实现
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSlider>
#include <QToolBar>
#include <QToolTip>
#include <QWindow>

#include "ConfigDialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setMinimumSize(MINWIDTH, MINHEIGHT);

  createMenu();
  qDebug() << "[ OK ] MainWindow::MainWindow: Successfully created menu.";

  auto centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
  auto layout = new QGridLayout(centralWidget);
  layout->setObjectName("layout");
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  qDebug()
      << "[ OK ] MainWindow::MainWindow: Successfully created central widget.";
}

void MainWindow::createMenu() {
  menuBar()->setContextMenuPolicy(Qt::CustomContextMenu);

  auto openAct = new QAction("打开 SPC 文件... (&O)", this);
  openAct->setObjectName("openAct");
  openAct->setShortcut(QKeySequence::Open);

  auto openCustomAct = new QAction("使用自定义设置打开... (&P)", this);
  openCustomAct->setObjectName("openCustomAct");

  auto closeAct = new QAction("关闭文件 (&C)", this);
  closeAct->setObjectName("closeAct");
  closeAct->setEnabled(false);
  closeAct->setShortcut(QKeySequence::Close);

  auto playPauseAct = new QAction("播放/暂停 (&P)", this);
  playPauseAct->setObjectName("playPauseAct");
  playPauseAct->setEnabled(false);
  playPauseAct->setShortcut(QKeySequence("Space"));

  auto stopAct = new QAction("停止 (&S)", this);
  stopAct->setObjectName("stopAct");
  stopAct->setEnabled(false);
  stopAct->setShortcut(QKeySequence("."));

  auto skipFwdAct = new QAction("快进 (&F)", this);
  skipFwdAct->setObjectName("skipFwdAct");
  skipFwdAct->setEnabled(false);
  skipFwdAct->setShortcut(QKeySequence("Right"));

  auto skipBackAct = new QAction("快退 (&B)", this);
  skipBackAct->setObjectName("skipBackAct");
  skipBackAct->setEnabled(false);
  skipBackAct->setShortcut(QKeySequence("Left"));

  // TODO: 增加速度调节功能
  auto increaseRateAct = new QAction("加速 (&A)", this);
  increaseRateAct->setObjectName("increaseRateAct");
  increaseRateAct->setEnabled(false);
  increaseRateAct->setShortcut(QKeySequence("Down"));

  auto decreaseRateAct = new QAction("减速 (&O)", this);
  decreaseRateAct->setObjectName("decreaseRateAct");
  decreaseRateAct->setEnabled(false);
  decreaseRateAct->setShortcut(QKeySequence("Up"));

  auto resetRateAct = new QAction("重置速度 (&R)", this);
  resetRateAct->setObjectName("resetRateAct");
  resetRateAct->setEnabled(false);
  resetRateAct->setShortcut(QKeySequence("R"));

  auto nFasterAct = new QAction("缩短音符 (&N)", this);
  nFasterAct->setObjectName("nFasterAct");
  nFasterAct->setShortcut(QKeySequence("Shift+Up"));

  auto nSlowerAct = new QAction("拉长音符 (&T)", this);
  nSlowerAct->setObjectName("nSlowerAct");
  nSlowerAct->setShortcut(QKeySequence("Shift+Down"));

  auto volumeUpAct = new QAction("提高音量 (&U)", this);
  volumeUpAct->setObjectName("volumeUpAct");
  volumeUpAct->setShortcut(QKeySequence("Alt+Up"));

  auto volumeDownAct = new QAction("降低音量 (&D)", this);
  volumeDownAct->setObjectName("volumeDownAct");
  volumeDownAct->setShortcut(QKeySequence("Enter"));

  auto muteAct = new QAction("静音 (&M)", this);
  muteAct->setObjectName("muteAct");
  muteAct->setCheckable(true);
  muteAct->setShortcut(QKeySequence("M"));

  auto controlsAct = new QAction("播放工具栏 (&P)", this);
  controlsAct->setObjectName("controlsAct");
  controlsAct->setCheckable(true);
  controlsAct->setChecked(true);
  controlsAct->setShortcut(QKeySequence("Ctrl+T"));

  auto keyboardAct = new QAction("键盘 (&K)", this);
  keyboardAct->setObjectName("keyboardAct");
  keyboardAct->setCheckable(true);
  keyboardAct->setChecked(true);
  keyboardAct->setShortcut(QKeySequence("Ctrl+K"));

  auto alwaysOnTopAct = new QAction("置于顶层 (&A)", this);
  alwaysOnTopAct->setObjectName("alwaysOnTopAct");
  alwaysOnTopAct->setCheckable(true);
  alwaysOnTopAct->setShortcut(QKeySequence("Ctrl+A"));

  auto fullScreenAct = new QAction("全屏 (&F)", this);
  fullScreenAct->setObjectName("fullScreenAct");
  fullScreenAct->setCheckable(true);
  fullScreenAct->setShortcut(QKeySequence::FullScreen);

  auto moveAndZoomAct = new QAction("移动和缩放", this);
  moveAndZoomAct->setObjectName("moveAndZoomAct");
  moveAndZoomAct->setEnabled(false);
  moveAndZoomAct->setCheckable(true);

  auto resetMoveAndZoomAct = new QAction("重置移动与缩放", this);
  resetMoveAndZoomAct->setObjectName("resetMoveAndZoomAct");
  moveAndZoomAct->setShortcut(QKeySequence("Ctrl+V"));
  resetMoveAndZoomAct->setShortcut(QKeySequence("Ctrl+Z"));

  auto perferenceAct = new QAction("偏好设置...\tCtrl+P", this);
  perferenceAct->setObjectName("perferenceAct");
  perferenceAct->setShortcut(QKeySequence::Preferences);

  auto aboutAct = new QAction("关于 SPC 播放器", this);
  aboutAct->setObjectName("aboutAct");

  // 仅能通过快捷键触发的功能
  auto nResetAct = new QAction(this);
  nResetAct->setObjectName("nResetAct");
  nResetAct->setShortcut(QKeySequence("Shift+R"));

  auto noFullScreenAct = new QAction(this);
  noFullScreenAct->setObjectName("noFullScreenAct");
  noFullScreenAct->setShortcut(QKeySequence::Cancel);

  auto fileMenu = menuBar()->addMenu("文件 (&F)");
  fileMenu->setObjectName("fileMenu");
  fileMenu->addAction(openAct);
  fileMenu->addAction(openCustomAct);
  fileMenu->addAction(closeAct);

  auto playMenu = menuBar()->addMenu("播放 (&P)");
  playMenu->setObjectName("playMenu");
  playMenu->addAction(playPauseAct);
  playMenu->addAction(stopAct);
  playMenu->addAction(skipFwdAct);
  playMenu->addAction(skipBackAct);
  playMenu->addSeparator();
  playMenu->addAction(increaseRateAct);
  playMenu->addAction(decreaseRateAct);
  playMenu->addAction(resetRateAct);
  playMenu->addSeparator();
  playMenu->addAction(nFasterAct);
  playMenu->addAction(nSlowerAct);
  playMenu->addSeparator();
  playMenu->addAction(volumeUpAct);
  playMenu->addAction(volumeDownAct);
  playMenu->addAction(muteAct);

  auto viewMenu = menuBar()->addMenu("视图 (&V)");
  viewMenu->setObjectName("viewMenu");
  viewMenu->addAction(controlsAct);
  viewMenu->addAction(keyboardAct);
  viewMenu->addSeparator();
  viewMenu->addAction(alwaysOnTopAct);
  viewMenu->addSeparator();
  viewMenu->addAction(fullScreenAct);
  viewMenu->addAction(moveAndZoomAct);
  viewMenu->addAction(resetMoveAndZoomAct);

  auto optionsMenu = menuBar()->addMenu("选项 (&O)");
  optionsMenu->addAction(perferenceAct);

  auto helpMenu = menuBar()->addMenu("帮助 (&H)");
  helpMenu->addAction(aboutAct);

  this->addAction(openAct);
  this->addAction(closeAct);
  this->addAction(playPauseAct);
  this->addAction(stopAct);
  this->addAction(skipFwdAct);
  this->addAction(skipBackAct);
  this->addAction(increaseRateAct);
  this->addAction(decreaseRateAct);
  this->addAction(resetRateAct);
  this->addAction(nFasterAct);
  this->addAction(nSlowerAct);
  this->addAction(volumeUpAct);
  this->addAction(volumeDownAct);
  this->addAction(muteAct);
  this->addAction(controlsAct);
  this->addAction(keyboardAct);
  this->addAction(alwaysOnTopAct);
  this->addAction(fullScreenAct);
  this->addAction(moveAndZoomAct);
  this->addAction(resetMoveAndZoomAct);
  this->addAction(perferenceAct);
  this->addAction(nResetAct);
  this->addAction(noFullScreenAct);
}

void MainWindow::createRebar() {
  // 新建工具栏
  auto toolBar = new QToolBar(this);
  toolBar->setObjectName("toolBar");
  toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
  toolBar->setMovable(false);

  auto toolPlayAct = new QAction(
      QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart), "播放", this);
  toolPlayAct->setObjectName("toolPlayAct");
  toolPlayAct->setEnabled(false);
  toolPlayAct->setCheckable(true);

  auto toolPauseAct = new QAction(
      QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause), "暂停", this);
  toolPauseAct->setObjectName("toolPauseAct");
  toolPauseAct->setEnabled(false);
  toolPauseAct->setCheckable(true);

  auto toolStopAct = new QAction(
      QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStop), "停止", this);
  toolStopAct->setObjectName("toolStopAct");
  toolStopAct->setEnabled(false);
  toolStopAct->setCheckable(true);

  auto toolSkipBackAct = new QAction(
      QIcon::fromTheme(QIcon::ThemeIcon::MediaSkipBackward), "快退", this);
  toolSkipBackAct->setObjectName("toolSkipBackAct");
  toolSkipBackAct->setEnabled(false);

  auto toolSkipFwdAct = new QAction(
      QIcon::fromTheme(QIcon::ThemeIcon::MediaSkipForward), "快进", this);
  toolSkipFwdAct->setObjectName("toolSkipFwdAct");
  toolSkipFwdAct->setEnabled(false);

  auto toolMuteAct = new QAction(
      QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeHigh), "静音", this);
  toolMuteAct->setObjectName("toolMuteAct");

  auto spacer0 = new QWidget(this);
  spacer0->setFixedWidth(5);

  auto volumeSlider = new QSlider(Qt::Horizontal, this);
  volumeSlider->setObjectName("volumeSlider");
  volumeSlider->setFixedSize(75, 20);
  volumeSlider->setRange(0, 100);

  auto spacer1 = new QWidget(this);
  spacer1->setFixedWidth(5);

  auto speedLabel = new QLabel(" 播放速度：", this);

  auto speedSlider = new QSlider(Qt::Horizontal, this);
  speedSlider->setObjectName("speedSlider");
  speedSlider->setEnabled(false); // Fusion 风格有 BUG
  speedSlider->setFixedSize(75, 20);
  speedSlider->setRange(5, 195);

  auto spacer2 = new QWidget(this);
  spacer2->setFixedWidth(5);

  auto nSpeedLabel = new QLabel(" 音符长度：", this);

  auto nSpeedSlider = new QSlider(Qt::Horizontal, this);
  nSpeedSlider->setObjectName("nSpeedSlider");
  nSpeedSlider->setFixedSize(75, 20);
  nSpeedSlider->setRange(5, 195);

  toolBar->addAction(toolPlayAct);
  toolBar->addAction(toolPauseAct);
  toolBar->addAction(toolStopAct);
  toolBar->addSeparator();
  toolBar->addAction(toolSkipBackAct);
  toolBar->addAction(toolSkipFwdAct);
  toolBar->addSeparator();
  toolBar->addAction(toolMuteAct);
  toolBar->addWidget(spacer0);
  toolBar->addWidget(volumeSlider);
  toolBar->addWidget(spacer1);
  toolBar->addSeparator();
  toolBar->addWidget(speedLabel);
  toolBar->addWidget(speedSlider);
  toolBar->addWidget(spacer2);
  toolBar->addSeparator();
  toolBar->addWidget(nSpeedLabel);
  toolBar->addWidget(nSpeedSlider);

  // 控件创建完成，接下来设置音量条、播放速度条和音符长度条的滑块位置
  auto &config = Config::getConfig();
  const auto &playback = config.getPlaybackSettings();
  setMute(playback.getMute());
  speedSlider->setValue(100 * playback.getSpeed() + 0.5);
  nSpeedSlider->setValue(100 * (2 - playback.getNSpeed()) + 0.5);
  volumeSlider->setValue(100 * playback.getVolume() + 0.5);

  // QToolBar 是透明的，为了兼容浅色和深色颜色模式，我们不允许 “悬浮”
  addToolBar(toolBar);

  // 新建进度条
  auto centralWidget = this->centralWidget();
  auto positionSlider = new QSlider(Qt::Horizontal, centralWidget);
  positionSlider->setObjectName("positionSlider");
  positionSlider->setEnabled(false);
  positionSlider->setRange(0, 1000);
  auto layout = findChild<QGridLayout *>("layout");
  layout->addWidget(positionSlider, 0, 0);

  qDebug() << "[ OK ] MainWindow::MainWindow: Successfully created rebar.";
}

void MainWindow::moveEvent(QMoveEvent *event) {
  static auto &view = Config::getConfig().getViewSettings();
  view.setMainPos(event->pos().x(), event->pos().y());
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  static auto &view = Config::getConfig().getViewSettings();
  if ((windowState() & Qt::WindowMaximized) && !view.getFullScreen())
    view.setMainSize(event->size().width(), event->size().height());
}

// GUI 交互的帮助函数

void MainWindow::handOffEvent(Event::Type type, qint64 param) {
  Event eventGameThread = {type, param};
  eventQueue.forcePush(eventGameThread);
}

void MainWindow::showControls(bool show) {
  static const auto &view = Config::getConfig().getViewSettings();
  static auto &visual = Config::getConfig().getVisualSettings();

  auto toolBar = findChild<QToolBar *>("toolBar");
  auto positionSlider = findChild<QSlider *>("positionSlider");
  if (!view.getFullScreen() || visual.alwaysShowControls) {
    toolBar->setVisible(show);
    positionSlider->setVisible(show);
  } else {
    toolBar->setVisible(false);
    positionSlider->setVisible(false);
  }

  auto controlsAct = findChild<QAction *>("controlsAct");
  controlsAct->setChecked(show);
}

void MainWindow::showKeyboard(bool show) {
  auto keyboardAct = findChild<QAction *>("keyboardAct");
  keyboardAct->setChecked(show);
}

void MainWindow::setOnTop(bool onTop) {
  static const auto &view = Config::getConfig().getViewSettings();
  if (!view.getFullScreen()) {
    auto flags = windowHandle()->flags();
    if (onTop)
      flags |= Qt::WindowStaysOnTopHint; // 添加置顶标志
    else
      flags &= ~Qt::WindowStaysOnTopHint; // 移除置顶标志
    windowHandle()->setFlags(flags);
  }

  auto alwaysOnTopAct = findChild<QAction *>("alwaysOnTopAct");
  alwaysOnTopAct->setChecked(onTop);
}

void MainWindow::setFullScreen(bool fullScreen) {
  static const auto &view = Config::getConfig().getViewSettings();
  static auto &visual = Config::getConfig().getVisualSettings();
  auto menu = menuBar();
  auto fullScreenAct = findChild<QAction *>("fullScreenAct");
  auto toolBar = findChild<QToolBar *>("toolBar");
  auto positionSlider = findChild<QSlider *>("positionSlider");

  if (fullScreen) {
    menu->hide();
    if (!visual.alwaysShowControls) {
      toolBar->hide();
      positionSlider->hide();
    }
    auto flags = windowHandle()->flags() | Qt::WindowStaysOnTopHint;
    windowHandle()->setFlags(flags);
    showFullScreen();
    fullScreenAct->setChecked(true);
  } else {
    menu->show();
    if (view.getControls()) {
      toolBar->show();
      positionSlider->show();
    }
    showNormal();
    setOnTop(view.getOnTop());
    fullScreenAct->setChecked(false);
  }
}

void MainWindow::setZoomMove(bool zoomMove) {
  auto moveAndZoomAct = findChild<QAction *>("moveAndZoomAct");
  moveAndZoomAct->setChecked(zoomMove);
}

void MainWindow::setMute(bool mute) {
  auto toolMuteAct = findChild<QAction *>("toolMuteAct");
  auto muteAct = findChild<QAction *>("muteAct");
  toolMuteAct->setIcon(
      mute ? QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeMuted)
           : QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeHigh));
  muteAct->setChecked(mute);
}

void MainWindow::setSpeed(double speed) {
  auto speedSlider = findChild<QSlider *>("speedSlider");
  speedSlider->setValue(100 * speed + 0.5);
}

void MainWindow::setNSpeed(double nSpeed) {
  auto nSpeedSlider = findChild<QSlider *>("nSpeedSlider");
  nSpeedSlider->setValue(100 * (2.0 - nSpeed) + 0.5);
}

void MainWindow::setVolume(double volume) {
  auto volumeSlider = findChild<QSlider *>("volumeSlider");
  volumeSlider->setValue(100 * volume + 0.5);
}

void MainWindow::setPosition(int position) {
  auto positionSlider = findChild<QSlider *>("positionSlider");
  positionSlider->setValue(position);
}

void MainWindow::setPlayable(bool playable) {
  auto toolPlayAct = findChild<QAction *>("toolPlayAct");
  toolPlayAct->setEnabled(playable);
}

void MainWindow::setPlayMode(int playMode) {
  bool practice = (playMode == GameState::Practice);

  setZoomMove(false);

  auto closeAct = findChild<QAction *>("closeAct");
  auto playPauseAct = findChild<QAction *>("playPauseAct");
  auto stopAct = findChild<QAction *>("stopAct");
  auto skipFwdAct = findChild<QAction *>("skipFwdAct");
  auto skipBackAct = findChild<QAction *>("skipBackAct");
  auto moveAndZoomAct = findChild<QAction *>("moveAndZoomAct");
  closeAct->setEnabled(playMode);
  playPauseAct->setEnabled(practice);
  stopAct->setEnabled(practice);
  skipFwdAct->setEnabled(practice);
  skipBackAct->setEnabled(practice);
  moveAndZoomAct->setEnabled(practice);

  auto toolPlayAct = findChild<QAction *>("toolPlayAct");
  auto toolPauseAct = findChild<QAction *>("toolPauseAct");
  auto toolStopAct = findChild<QAction *>("toolStopAct");
  auto toolSkipFwdAct = findChild<QAction *>("toolSkipFwdAct");
  auto toolSkipBackAct = findChild<QAction *>("toolSkipBackAct");
  toolPlayAct->setEnabled(practice);
  toolPlayAct->setCheckable(practice);
  toolPlayAct->setChecked(true);
  toolStopAct->setEnabled(practice);
  toolPauseAct->setEnabled(practice);
  toolPauseAct->setCheckable(practice);
  toolSkipFwdAct->setEnabled(practice);
  toolSkipBackAct->setEnabled(practice);

  auto positionSlider = findChild<QSlider *>("positionSlider");
  positionSlider->setEnabled(practice);
}

void MainWindow::setPlayPauseStop(bool play, bool pause, bool stop) {
  auto toolPlayAct = findChild<QAction *>("toolPlayAct");
  auto toolPauseAct = findChild<QAction *>("toolPauseAct");
  auto toolStopAct = findChild<QAction *>("toolStopAct");
  toolPlayAct->setChecked(play);
  toolPauseAct->setChecked(pause);
  toolStopAct->setChecked(stop);
}

bool MainWindow::playFile(QString file, bool customSettings) {
  auto &config = Config::getConfig();
  auto &visual = config.getVisualSettings();
  auto &playback = config.getPlaybackSettings();
  auto &view = config.getViewSettings();

  // 先停止渲染，并暂停音乐播放
  playback.setPaused(true);
  gfxWidget->rendering = false;

  const auto playMode = GameState::Practice;

  // 尝试加载文件
  MainScreen *gameState = nullptr;
  gameState = new MainScreen(file, playMode, nullptr, nullptr);
  if (!gameState->isValid()) {
    QMessageBox::critical(this, "错误", "载入文件 " + file + " 失败");
    return false;
  }
  qDebug() << "[ OK ] MainWindow::playFile: Successfully created main screen.";

  // 设置样本颜色
  if (customSettings) {
    if (!getCustomSettings(gameState))
      return false;
  } else {
    gameState->setChannelSettings(
        std::vector<bool>(),
        std::vector<QColor>(visual.colors,
                            visual.colors + sizeof(visual.colors) /
                                                sizeof(visual.colors[0])));
  }
  qDebug() << "[ OK ] MainWindow::playFile: Successfully set colors.";

  // 成功！设置 GUI 播放
  if (!playback.getPlayable())
    playback.setPlayable(true, true);
  if (playback.getPlayMode() != playMode)
    playback.setPlayMode(playMode, true);
  playback.setPaused(playMode != GameState::Practice, true);
  playback.setPosition(0);
  view.setZoomMove(false, true);
  setWindowTitle(QFileInfo(file).fileName());
  qDebug() << "[ OK ] MainWindow::playFile: Successfully set GUI.";

  // 切换游戏状态
  handOffEvent(Event::ChangeState, (qint64)gameState);
  return true;
}

void MainWindow::checkActivity(bool isActive, QPoint *newPoint,
                               bool toggleEnable) {
  static const auto &view = Config::getConfig().getViewSettings();
  static auto &visual = Config::getConfig().getVisualSettings();
  static bool enabled = true;
  static bool wasActive = true;
  static bool mouseHidden = false;
  static QPoint oldPoint;

  if (!enabled && !toggleEnable)
    return;
  if (toggleEnable)
    enabled = !enabled;

  // 鼠标位置是否变化？
  bool samePoint;
  if (newPoint) {
    samePoint =
        (newPoint->x() == oldPoint.x() && newPoint->y() == oldPoint.y());
    oldPoint = *newPoint;
  } else {
    auto point = QCursor::pos();
    samePoint = (point.x() == oldPoint.x() && point.y() == oldPoint.y());
    oldPoint = point;
  }

  auto toolBar = findChild<QToolBar *>("toolBar");
  if ((isActive && !newPoint) || !samePoint || !view.getFullScreen()) {
    wasActive = true;
    setCursor(Qt::ArrowCursor);
    mouseHidden = false;
  } else if (!isActive && focusWidget() == gfxWidget &&
             (!toolBar->isVisible() || visual.alwaysShowControls)) {
    if (wasActive)
      wasActive = false;
    else if (!mouseHidden) {
      setCursor(Qt::BlankCursor);
      mouseHidden = true;
    }
  }
}

void MainWindow::on_openAct_triggered() {
  checkActivity(true);
  auto filename = QFileDialog::getOpenFileName(this, "选择一个 SPC 文件并打开",
                                               "", "SPC 文件 (*.spc)");
  qDebug() << "[INFO] MainWindow::on_openAct_triggered: Filename:" << filename;
  if (!filename.isEmpty())
    playFile(filename, false);
}

void MainWindow::on_openCustomAct_triggered() {
  checkActivity(true);
  auto filename = QFileDialog::getOpenFileName(this, "选择一个 SPC 文件并打开",
                                               "", "SPC 文件 (*.spc)");
  qDebug() << "[INFO] MainWindow::on_openCustomAct_triggered: Filename:"
           << filename;
  if (!filename.isEmpty())
    playFile(filename, true);
}

void MainWindow::on_closeAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (!playback.getPlayMode())
    return;
  playback.setPlayMode(GameState::Intro, true);
  playback.setPosition(0);
  setWindowTitle(APPNAME);
  handOffEvent(Event::ChangeState, (qint64) new IntroScreen(nullptr, nullptr));
  qDebug()
      << "[ OK ] MainWindow::on_closeAct_triggered: Successfully closed file.";
}

void MainWindow::on_playPauseAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (playback.getPlayMode())
    playback.togglePaused(true);
  qDebug() << "[ OK ] MainWindow::on_playPauseAct_triggered: Successfully "
              "toggled play/pause state.";
}

void MainWindow::on_stopAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (playback.getPlayMode())
    handOffEvent(Event::Stop, 0);
  qDebug() << "[ OK ] MainWindow::on_stopAct_triggered: Successfully stopped.";
}

void MainWindow::on_skipFwdAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (playback.getPlayMode())
    handOffEvent(Event::SkipForward, 0);
  qDebug() << "[ OK ] MainWindow::on_skipFwdAct_triggered: Successfully "
              "skipped forward.";
}

void MainWindow::on_skipBackAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (playback.getPlayMode())
    handOffEvent(Event::SkipBackward, 0);
  qDebug() << "[ OK ] MainWindow::on_skipBackAct_triggered: Successfully "
              "skipped backward.";
}

void MainWindow::on_increaseRateAct_triggered() {
  static auto &controls = Config::getConfig().getControlsSettings();
  static auto &playback = Config::getConfig().getPlaybackSettings();
  playback.setSpeed(playback.getSpeed() * (1.0 + controls.speedUpPct / 100.0),
                    true);
  qDebug() << "[ OK ] MainWindow::on_increaseRateAct_triggered: Successfully "
              "increased rate.";
}

void MainWindow::on_decreaseRateAct_triggered() {
  static auto &controls = Config::getConfig().getControlsSettings();
  static auto &playback = Config::getConfig().getPlaybackSettings();
  playback.setSpeed(playback.getSpeed() / (1.0 + controls.speedUpPct / 100.0),
                    true);
  qDebug() << "[ OK ] MainWindow::on_decreaseRateAct_triggered: Successfully "
              "decreased rate.";
}

void MainWindow::on_resetRateAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  playback.setSpeed(1.0, true);
  qDebug() << "[ OK ] MainWindow::on_resetRateAct_triggered: Successfully "
              "reset rate.";
}

void MainWindow::on_muteAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  playback.toggleMute(true);
  qDebug()
      << "[ OK ] MainWindow::on_muteAct_triggered: Successfully toggled mute.";
}

void MainWindow::on_nFasterAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  static auto &controls = Config::getConfig().getControlsSettings();
  playback.setNSpeed(playback.getNSpeed() * (1.0 + controls.speedUpPct / 100.0),
                     true);
  qDebug() << "[ OK ] MainWindow::on_nFasterAct_triggered: Successfully "
              "extended notes.";
}

void MainWindow::on_nSlowerAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  static auto &controls = Config::getConfig().getControlsSettings();
  playback.setNSpeed(playback.getNSpeed() / (1.0 + controls.speedUpPct / 100.0),
                     true);
  qDebug() << "[ OK ] MainWindow::on_nSlowerAct_triggered: Successfully "
              "shortened notes.";
}

void MainWindow::on_volumeUpAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  playback.setVolume(std::min(playback.getVolume() + 0.1, 1.0), true);
  qDebug() << "[ OK ] MainWindow::on_volumeUpAct_triggered: Successfully "
              "volumed up.";
}

void MainWindow::on_volumeDownAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  playback.setVolume(std::max(playback.getVolume() - 0.1, 0.0), true);
  qDebug() << "[ OK ] MainWindow::on_volumeDownAct_triggered: Successfully "
              "volumed down.";
}

void MainWindow::on_controlsAct_triggered() {
  static auto &view = Config::getConfig().getViewSettings();
  view.toggleControls(true);
  qDebug() << "[ OK ] MainWindow::on_controlsAct_triggered: Successfully "
              "toggled controls bar.";
}

void MainWindow::on_keyboardAct_triggered() {
  static auto &view = Config::getConfig().getViewSettings();
  view.toggleKeyboard(true);
  qDebug() << "[ OK ] MainWindow::on_keyboardAct_triggered: Successfully "
              "toggled keyboard.";
}

void MainWindow::on_alwaysOnTopAct_triggered() {
  static auto &view = Config::getConfig().getViewSettings();
  view.toggleOnTop(true);
  qDebug() << "[ OK ] MainWindow::on_alwaysOnTopAct_triggered: Successfully "
              "toggled on top.";
}

void MainWindow::on_fullScreenAct_triggered() {
  static auto &view = Config::getConfig().getViewSettings();
  view.toggleFullScreen(true);
  qDebug() << "[ OK ] MainWindow::on_fullScreenAct_triggered: Successfully "
              "toggled full screen.";
}

void MainWindow::on_moveAndZoomAct_triggered() {
  handOffEvent(Event::MoveAndZoom, 0);
  qDebug() << "[ OK ] MainWindow::on_moveAndZoomAct_triggered: Successfully "
              "toggled move and zoom.";
}

void MainWindow::on_resetMoveAndZoomAct_triggered() {
  handOffEvent(Event::ResetMoveAndZoom, 0);
  qDebug()
      << "[ OK ] MainWindow::on_resetMoveAndZoomAct_triggered: Successfully "
         "reset move and zoom.";
}

void MainWindow::on_perferenceAct_triggered() {
  checkActivity(true);
  doPreferences(this);
}

void MainWindow::on_aboutAct_triggered() {
  AboutDialog aboutDialog(this);
  aboutDialog.exec();
}

void MainWindow::on_toolPlayAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (playback.getPlayMode())
    playback.setPaused(false, true);
  qDebug() << "[ OK ] MainWindow::on_toolPlayAct_triggered: "
              "Successfully set played.";
}

void MainWindow::on_toolPauseAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  playback.setPaused(true, true);
  qDebug() << "[ OK ] MainWindow::on_toolPauseAct_triggered: "
              "Successfully set paused.";
}

void MainWindow::on_toolStopAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (playback.getPlayMode())
    handOffEvent(Event::Stop, 0);
  qDebug() << "[ OK ] MainWindow::on_toolStopAct_triggered: "
              "Successfully stopped.";
}

void MainWindow::on_toolSkipFwdAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (playback.getPlayMode())
    handOffEvent(Event::SkipForward, 0);
  qDebug() << "[ OK ] MainWindow::on_toolSkipFwdAct_triggered: Successfully "
              "skipped forward.";
}

void MainWindow::on_toolSkipBackAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (playback.getPlayMode())
    handOffEvent(Event::SkipBackward, 0);
  qDebug() << "[ OK ] MainWindow::on_toolSkipBackAct_triggered: Successfully "
              "skipped backward.";
}

void MainWindow::on_toolMuteAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  playback.toggleMute(true);
  qDebug() << "[ OK ] MainWindow::on_toolMuteAct_triggered: Successfully "
              "toggled mute.";
}

// 观察音量
void MainWindow::on_volumeSlider_valueChanged(int value) {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  auto volumeSlider = findChild<QSlider *>("volumeSlider");
  QToolTip::showText(QCursor::pos(), QString::number(value), volumeSlider);
  playback.setVolume(value / 100.0, false);
}

void MainWindow::on_volumeSlider_sliderPressed() {
  auto volumeSlider = findChild<QSlider *>("volumeSlider");
  QToolTip::showText(QCursor::pos(), QString::number(volumeSlider->value()),
                     volumeSlider);
}

void MainWindow::on_speedSlider_valueChanged(int value) {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (value < 108 && value > 92 && value != 100)
    playback.setSpeed(1.0, true);
  else
    playback.setSpeed(value / 100.0, false);
}

void MainWindow::on_nSpeedSlider_valueChanged(int value) {
  static auto &playback = Config::getConfig().getPlaybackSettings();
  if (value < 108 && value > 92 && value != 100)
    playback.setNSpeed(1.0, true);
  else
    playback.setNSpeed((200 - value) / 100.0, false);
}

void MainWindow::on_positionSlider_sliderMoved(int value) {
  handOffEvent(Event::SetPosition, value);
}

void MainWindow::on_positionSlider_sliderPressed() {
  auto positionSlider = findChild<QSlider *>("positionSlider");
  int value = positionSlider->value();
  handOffEvent(Event::SetPosition, value);
}

void MainWindow::on_nResetAct_triggered() {
  static auto &playback = Config::getConfig().getPlaybackSettings();

  playback.setNSpeed(1.0, true);
  qDebug() << "[ OK ] MainWindow::on_nResetAct_triggered: Successfully "
              "reset note speed.";
}

void MainWindow::on_noFullScreenAct_triggered() {
  static auto &view = Config::getConfig().getViewSettings();
  if (view.getZoomMove())
    handOffEvent(Event::CancelMoveAndZoom, 0);
  else if (view.getFullScreen())
    view.setFullScreen(false, true);
}

void MainWindow::handleErrorMessage(qint64 param) {
  QMessageBox::critical(this, "错误", GameState::errors[param]);
}

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), gameState(nullptr), renderer(nullptr) {
  setMouseTracking(true);
  startTimer(2500);
}

void OpenGLWidget::initialize(bool limitFPS) {
  auto fmt = this->format();
  fmt.setSwapInterval(limitFPS);
  QSurfaceFormat::setDefaultFormat(fmt);
}

void OpenGLWidget::contextMenuEvent(QContextMenuEvent *event) {
  static const auto &view = Config::getConfig().getViewSettings();

  if (view.getZoomMove())
    return;

  auto fileMenu = ::window->findChild<QMenu *>("fileMenu");
  auto playMenu = ::window->findChild<QMenu *>("playMenu");
  auto viewMenu = ::window->findChild<QMenu *>("viewMenu");

  QMenu popUpMenu;
  popUpMenu.addMenu(fileMenu);
  popUpMenu.addMenu(viewMenu);
  popUpMenu.addAction(::window->findChild<QAction *>("perferenceAct"));
  popUpMenu.addSeparator();
  popUpMenu.addActions(playMenu->actions());

  // 显示菜单
  ::window->checkActivity(true, nullptr, true);
  popUpMenu.exec(event->globalPos());
  ::window->checkActivity(true, nullptr, true);
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
  // 全屏下显示控制条的静态变量
  static const auto &view = Config::getConfig().getViewSettings();
  static auto &visual = Config::getConfig().getVisualSettings();
  static bool showBar;
  static int barHeight;

  auto toolBar = ::window->findChild<QToolBar *>("toolBar");
  auto positionSlider = ::window->findChild<QSlider *>("positionSlider");

  qint64 param = (qint64)event->pos().x() << 32 | event->pos().y();
  ::window->handOffEvent(Event::MouseMove, param);

  // 如果全屏且鼠标位置靠上，显示控制条
  QPoint p = event->pos();
  ::window->checkActivity(true, &p);

  if (view.getFullScreen()) {
    if (!barHeight)
      barHeight = toolBar->height() + positionSlider->height();

    if (!visual.alwaysShowControls) {
      if (event->globalPosition().y() < barHeight && view.getControls()) {
        toolBar->show();
        positionSlider->show();
        showBar = true;
      } else if (showBar) {
        toolBar->hide();
        positionSlider->hide();
        setFocus();
        showBar = false;
      }
    }
  }
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event) {
  qint64 param = (qint64)event->pos().x() << 32 | event->pos().y();
  switch (event->button()) {
  case Qt::LeftButton:
    ::window->handOffEvent(Event::LeftButtonPress, param);
    setFocus();
    break;
  case Qt::RightButton:
    ::window->handOffEvent(Event::RightButtonPress, param);
    setFocus();
  default:
    break;
  }
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event) {
  switch (event->button()) {
  case Qt::LeftButton:
    ::window->handOffEvent(Event::LeftButtonRelease, 0);
    break;
  case Qt::RightButton:
    ::window->handOffEvent(Event::RightButtonRelease, 0);
  default:
    break;
  }
}

void OpenGLWidget::timerEvent(QTimerEvent *event) {
  event->id(); // 让编译器不要报变量未使用的警告
  ::window->checkActivity(false);
}

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
  setFixedSize(350, 300);
  setWindowTitle(APPNAME);

  auto layout = new QGridLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->setColumnMinimumWidth(2, 7);

  auto imageLabel = new QLabel(this);
  imageLabel->setFixedWidth(150);
  imageLabel->setPixmap(QPixmap(":/Images/Mirror.png"));
  imageLabel->setScaledContents(true);
  layout->addWidget(imageLabel, 0, 0, 2, 1);

  auto vSpacer = new QSpacerItem(0, 1, QSizePolicy::Policy::Minimum,
                                 QSizePolicy::Policy::Fixed);
  layout->addItem(vSpacer, 0, 1, 1, 2);

  auto textLabel = new QLabel(this);
  textLabel->setSizePolicy(QSizePolicy::Policy::Expanding,
                           QSizePolicy::Policy::Expanding);
  textLabel->setAlignment(Qt::AlignmentFlag::AlignLeft |
                          Qt::AlignmentFlag::AlignTop);
  textLabel->setMargin(10);
  textLabel->setText("SPC 播放器\n版本：1.01\nQt 版本：6.8.3\n基于 MIT "
                     "协议开源\nCopyright (C) GMMario2026");
  layout->addWidget(textLabel, 1, 1, 1, 2);

  auto frame = new QFrame(this);
  frame->setFrameShape(QFrame::Shape::HLine);
  frame->setFrameShadow(QFrame::Shadow::Sunken);
  layout->addWidget(frame, 2, 0, 1, 3);

  auto buttonBox = new QDialogButtonBox(this);
  buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Ok);
  buttonBox->setFixedHeight(35);
  layout->addWidget(buttonBox, 3, 0, 1, 2);

  auto hSpacer = new QSpacerItem(7, 0, QSizePolicy::Policy::Fixed,
                                 QSizePolicy::Policy::Minimum);
  layout->addItem(hSpacer, 3, 2);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}
