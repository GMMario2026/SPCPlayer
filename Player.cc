/*************************************************************************************************
 *
 * 文件：Player.cc
 *
 * 描述：播放器的主入口，创建窗口、设置 GUI、消息循环
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#include <thread>

#include <QApplication>
#include <QGridLayout>
#include <QTranslator>

#include "Config.h"

int main(int argc, char *argv[]);
int gameThread(GameState *gameState);

//-----------------------------------------------------------------------------
// 全局变量
//-----------------------------------------------------------------------------
MainWindow *window = nullptr;
OpenGLWidget *gfxWidget = nullptr;
TSQueue<Event> eventQueue;

//-----------------------------------------------------------------------------
// 函数名: main()
// 描述: 程序入口点
//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // 加载 Qt 标准中文翻译
  QTranslator translator;
  if (translator.load(":/Translations/qtbase_zh_CN.qm"))
    if (app.installTranslator(&translator))
      qDebug() << "[ OK ] main: Successfully installed translator.";
    else {
      qCritical() << "[FAIL] main: Failed to install translator!";
      return 1;
    }
  else {
    qCritical() << "[FAIL] main: Failed to load translation file!";
    return 1;
  }

  // 导入保存的设置
  Config &config = Config::getConfig();
  qDebug() << "[ OK ] main: Successfully loaded configation.";
  auto &view = config.getViewSettings();
  auto &playback = config.getPlaybackSettings();

  // 创建程序窗体
  MainWindow windowInstance;
  window = &windowInstance; // 设置全局指针以供其他组件使用
  window->setWindowTitle(APPNAME);
  window->setWindowIcon(QIcon(":/Images/SPCPlayer Icon.ico"));
  if (view.getMainLeft() != CW_USEDEFAULT)
    window->move(view.getMainLeft(), view.getMainTop());
  window->resize(view.getMainWidth(), view.getMainHeight());
  qDebug() << "[ OK ] main: Successfully created main window.";

  // 创建工具栏和进度条
  window->createRebar();
  qDebug() << "[ OK ] main: Successfully created rebar.";

  // 创建图形窗口
  gfxWidget = new OpenGLWidget(window->centralWidget());
  gfxWidget->initialize(config.getVideoSettings().limitFPS);
  auto layout = window->findChild<QGridLayout *>("layout");
  layout->addWidget(gfxWidget, 1, 0);
  qDebug() << "[ OK ] main: Successfully created OpenGL window.";

  // 新建消息处理线程
  std::thread thread(gameThread, new IntroScreen(nullptr, nullptr));
  qDebug() << "[ OK ] main: Successfully created thread to handle message.";

  // 设置 GUI 并显示
  qDebug() << "[INFO] main: Showing main window...";
  window->setPlayMode(GameState::Intro);
  window->showControls(view.getControls());
  window->show();
  window->setOnTop(view.getOnTop());
  gfxWidget->setFocus();
  playback.setPaused(false, false);

  // 进入消息循环
  QMetaObject::connectSlotsByName(window);
  app.exec();

  // 向消息处理线程发送退出信号
  Event event = {Event::Quit, 0};
  eventQueue.forcePush(event);

  // 等待消息处理线程通知渲染，然后提前结束渲染
  // 等待消息处理线程处理退出消息后通知渲染
  // 主线程提前结束渲染，消息处理线程退出循环
  for (int i = 0; i < 2; i++) {
    while (!gfxWidget->rendering)
      ;
    gfxWidget->rendering = false;
  }
  thread.join();
  qDebug() << "[ OK ] main: Message thread quited successfully.";

  config.saveConfigValues();

  return 0;
}

int gameThread(GameState *gameState) {
  auto renderer = new OpenGLRenderer();

  // 创建游戏对象
  gameState->setWidget(gfxWidget);
  gameState->setRenderer(renderer);
  gameState->init();
  qDebug() << "[ OK ] gameThread: Successfully initialized game state.";
  GameState::GameError ge;

  // 设置和主线程的共享变量
  gfxWidget->setGameState(&gameState);
  gfxWidget->setRenderer(renderer);

  Event event = {};
  while (event.type != Event::Quit) {
    while (eventQueue.pop(event))
      gameState->event(&event);

    if ((ge = GameState::changeState(gameState->getNextState(), &gameState)) !=
        GameState::Success)
      QMetaObject::invokeMethod(window, "handleErrorMessage",
                                Qt::QueuedConnection,
                                Q_ARG(GameState::GameError, ge));
    gameState->logic();

    // 通知主线程渲染，并等待主线程渲染完成
    gfxWidget->rendering = true;
    gfxWidget->update();
    while (gfxWidget->rendering)
      ;
  }

  delete gameState;
  delete renderer;
  qDebug() << "[ OK ] gameThread: Successfully cleaned state.";

  return 0;
}