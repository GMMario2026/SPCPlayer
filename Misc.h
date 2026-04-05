/*************************************************************************************************
 *
 * 文件：Misc.h
 *
 * 描述：定义一些辅助对象
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#pragma once

#include <QColor>

//-----------------------------------------------------------------------------
// 简单的函数
//-----------------------------------------------------------------------------

class Util {
public:
  static QColor randColor();
  static QByteArray pcm2Wav(const QByteArray &pcm, int sampleRate = 32000,
                             short bitsPerSample = 16, short channels = 2);
};

//-----------------------------------------------------------------------------
// 线程安全队列类。只在单生产者和单消费者的情况下安全
//-----------------------------------------------------------------------------

template <typename T> class TSQueue {
public:
  TSQueue() : write(0), read(0) {}
  bool push(const T &element);
  bool pop(T &element);

  void forcePush(const T &element) {
    while (!push(element))
      ;
  }

private:
  static const int queueSize = 1024;
  T queue[queueSize]; // 需要 volatile 吗？
  volatile int write;
  volatile int read;
};

template <class T> bool TSQueue<T>::push(const T &element) {
  int nextElement = (write + 1) % queueSize;

  // 队列满了吗？
  if (nextElement == read)
    return false;

  // 插入数据（注意顺序）
  queue[write] = element;
  write = nextElement;

  return true;
}

template <class T> bool TSQueue<T>::pop(T &element) {
  // 队列为空吗？
  if (write == read)
    return false;

  // 计算偏置，读取数据，更新指针（注意顺序）
  int nextElement = (read + 1) % queueSize;
  element = queue[read];
  read = nextElement;

  return true;
}

//-----------------------------------------------------------------------------
// 事件处理
//-----------------------------------------------------------------------------

struct Event {
  enum Type {
    CancelMoveAndZoom,
    ChangeState,
    MoveAndZoom,
    ResetMoveAndZoom,
    SkipBackward,
    SkipForward,
    Stop,
    SetPosition,
    LeftButtonPress,
    RightButtonPress,
    LeftButtonRelease,
    RightButtonRelease,
    MouseMove,
    Quit
  };

  Type type;
  qint64 param;
};