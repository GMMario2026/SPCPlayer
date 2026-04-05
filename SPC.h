/*************************************************************************************************
 *
 * 文件: SPC.h
 *
 * 描述: 定义 SPC 对象
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#pragma once

#include <QString>

#include "split700/SPCFile.h"

// 定义的类
class SPC;
class SPCTrack;
class SPCEvent;

//
// SPC 文件类
//

// SPC 数据
class SPC {
public:
  enum Emulator { Unknown = 0, ZSNES, SNES9x };
  enum Note { A, AS, B, C, CS, D, DS, E, F, FS, G, GS };

  static const int DEFAULT_PLAY_TIME_SEC = 120;
  static const int DEFAULT_FADEOUT_TIME_MS = 10000;

  static const int KEYS = 129; // 因为第 128 个键为黑键
  static constexpr int C8 = 108;
  static const int C4 = C8 - 4 * 12;
  static constexpr int A0 = C8 - 7 * 12 - 3;

  static const QString &noteName(int note);
  static Note noteValue(int note);
  static bool isSharp(int note);
  static int whiteCount(int minNote, int maxNote);

  // 需要使用的解析函数
  static int parse16Bit(const quint8 *data, int maxSize, int *out);
  static int parseNChars(const quint8 *data, int nChars, int maxSize,
                         char *out);

  SPC(const QString &fileName);
  ~SPC();

  // split700 的样本静态函数
  static bool isValidSample(const SPCFile &spc, quint8 srcn);
  static std::vector<quint8> getSampleList(const SPCFile &spc);

  // 解析函数，将数据加载到实例
  void parseSamples(const std::string &fileName);
  void parseSPC(quint8 *data, int size);
  void preRender(quint8 *data, int size);
  bool isValid() const { return (tracks.size() > 0 && info.eventCount > 0); }

  void clear();

  struct SPCInfo {
    SPCInfo() { clear(); }
    void clear() {
      minVersion = playTimeSec = fadeoutTimeMs = mutedVoices = totalMicroSecs =
          firstNote = numTracks = numChannels = minNote = maxNote = eventCount =
              0;
      hasId666Tag = false;
      fileName.clear();
      songTitle.clear();
      gameTitle.clear();
      dumperName.clear();
      comments.clear();
      dumpDate.clear();
      artistName.clear();
      dumpingEmu = Unknown;
    }

    QString fileName;
    int minVersion;
    bool hasId666Tag;
    QString songTitle, gameTitle, dumperName, comments;
    QString dumpDate;
    int playTimeSec, fadeoutTimeMs;
    QString artistName;
    quint8 mutedVoices;
    Emulator dumpingEmu;
    int numTracks, numChannels;
    int minNote, maxNote, eventCount;
    qint64 totalMicroSecs, firstNote;
  };

  auto &getInfo() const { return info; }
  auto &getTracks() const { return tracks; }
  auto &getEvents() const { return events; }
  const auto &getWave() { return wave; }

private:
  static void initArrays();
  static QString noteNamesArr[KEYS + 1];
  static Note noteValArr[KEYS];
  static bool isSharpArr[KEYS];
  static int whiteCntArr[KEYS + 1];

  SPCInfo info;
  std::vector<SPCTrack *> tracks;
  std::unordered_map<quint8, double> sampleFreq;
  std::vector<SPCEvent *> events;
  QByteArray wave;
};

// SPC 中，我们为 (DSP 通道，样本) 分配音轨
class SPCTrack {
public:
  ~SPCTrack();

  void clear();

  friend class SPC;

  struct SPCTrackInfo {
    SPCTrackInfo() { clear(); }
    void clear() {
      numChannels = 0;
      memset(noteCount, 0, sizeof(noteCount));
      memset(srcn, 0, sizeof(srcn));
    }

    int noteCount[8], srcn[8], numChannels;
  };
  const SPCTrackInfo &getInfo() const { return trackInfo; }

private:
  SPCTrackInfo trackInfo;
};

// 通道事件：按下和松开
class SPCEvent {
public:
  SPCEvent() : sister(nullptr), simultaneous(0) {}

  enum EventType { KeyOff, KeyOn };
  void setEvent(EventType eventType, int track, int absT, quint8 channel,
                int note);

  // 访问器
  auto getAbsT() const { return absT; }
  auto getAbsMicroSec() const { return absMicroSec; }
  void setAbsMicroSec(qint64 absMicroSec) { this->absMicroSec = absMicroSec; }

  auto getEventType() const { return eventType; }
  auto getChannel() const { return channel; }
  auto getTrack() const { return track; }
  auto getNote() const { return note; }
  auto getSister() const { return sister; }
  auto getSimultaneous() const { return simultaneous; }

  void setSister(SPCEvent *sister) {
    this->sister = sister;
    sister->sister = this;
  }
  void setSimultaneous(int simultaneous) { this->simultaneous = simultaneous; }

private:
  EventType eventType;
  int track;
  int absT;
  qint64 absMicroSec;

  quint8 channel;
  int note;
  SPCEvent *sister;
  int simultaneous;
};