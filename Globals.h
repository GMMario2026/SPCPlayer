/*************************************************************************************************
 *
 * 文件：Globals.h
 *
 * 描述：全局变量，如主窗口等
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#pragma once

#include "MainWindow.h"

extern MainWindow *window;
extern OpenGLWidget *gfxWidget;
extern TSQueue<Event> eventQueue; // 生产者/消费者队列，让游戏线程处理事件
