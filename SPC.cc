/*************************************************************************************************
 *
 * 文件：SPC.cc
 *
 * 描述：实现 SPC 对象
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#include <fstream>

#include <QDebug>

#include "SPC.h"

#include "snes_spc/SNES_SPC.h"
#include "pyin/Yin.h"

//-----------------------------------------------------------------------------
// SPC 函数
//-----------------------------------------------------------------------------

SPC::SPC(const QString &fileName) {
  // 打开文件
  std::ifstream ifs(fileName.toStdString(),
                    std::ios::in | std::ios::binary | std::ios::ate);
  if (!ifs.is_open())
    return;

  // 读取信息到内存
  int size = static_cast<int>(ifs.tellg());
  auto *memBlock = new quint8[size];
  ifs.seekg(0, std::ios::beg);
  ifs.read(reinterpret_cast<char *>(memBlock), size);
  ifs.close();
  qDebug() << "[ OK ] SPC::SPC: Successfully loaded file, size:" << size;

  // 使用 split700 库读取样本数据
  parseSamples(fileName.toStdString());
  // 解析并用 SNES_SPC 预渲染
  parseSPC(memBlock, size);
  info.fileName = fileName;

  // 清理
  delete[] memBlock;
}

SPC::~SPC() { clear(); }

const QString &SPC::noteName(int note) {
  initArrays();
  if (note < 0 || note >= KEYS)
    return noteNamesArr[KEYS];
  return noteNamesArr[note];
}

SPC::Note SPC::noteValue(int note) {
  initArrays();
  if (note < 0 || note >= KEYS)
    return C;
  return noteValArr[note];
}

bool SPC::isSharp(int note) {
  initArrays();
  if (note < 0 || note >= KEYS)
    return false;
  return isSharpArr[note];
}

// [minNote, maxNote) 之间的白键数量
int SPC::whiteCount(int minNote, int maxNote) {
  initArrays();
  if (minNote < 0 || minNote >= KEYS || maxNote < 0 || maxNote >= KEYS)
    return 0;
  return whiteCntArr[maxNote] - whiteCntArr[minNote];
}

QString SPC::noteNamesArr[KEYS + 1];
SPC::Note SPC::noteValArr[KEYS];
bool SPC::isSharpArr[KEYS];
int SPC::whiteCntArr[KEYS + 1];

void SPC::initArrays() {
  static bool valid = false;

  // 在第一次调用的时候建立音符名称
  if (!valid) {
    char buf[10];
    char noteChar = 'C';
    int octave = -1;
    bool isSharp = false;
    Note note = C;
    for (int i = 0; i < KEYS; i++) {
      // sprint 太慢，手动建立
      int pos = 0;
      buf[pos++] = noteChar;
      if (isSharp)
        buf[pos++] = '#';
      if (octave < 0)
        buf[pos++] = '-';
      buf[pos++] = '0' + abs(octave);
      buf[pos++] = '\0';

      noteNamesArr[i] = buf;
      noteValArr[i] = note;
      isSharpArr[i] = isSharp;

      // 推进计数
      if (note == B || note == E || isSharp)
        noteChar++;
      if (note != B && note != E)
        isSharp = !isSharp;
      if (note == B)
        octave++;
      if (note == GS) {
        noteChar = 'A';
        note = A;
      } else
        note = static_cast<Note>(note + 1);
    }
    whiteCntArr[0] = 0;
    for (int i = 1; i < KEYS + 1; i++)
      whiteCntArr[i] = whiteCntArr[i - 1] + !isSharpArr[i - 1];
    noteNamesArr[KEYS] = "Invalid";
    valid = true;
  }
}

void SPC::clear() {
  for (auto track : tracks)
    delete track;
  tracks.clear();
  sampleFreq.clear();
  for (auto event : events)
    delete event;
  events.clear();
  wave.clear();
  info.clear();
}

void SPC::parseSamples(const std::string &fileName) {
  // 先重置，只在这里重置一次
  clear();

  auto spcFile = SPCFile::Load(fileName);
  for (auto srcn : getSampleList(*spcFile)) {
    // 获取样本并解码为 wave 波形
    auto &sample = spcFile->samples[srcn];
    auto wave = SPCSampDir::decode_brr(&spcFile->ram[sample.start_address],
                                       sample.compressed_size());

    // 准备输入数组（double 类型，大容量以提升准确性）
    double in[65536] = {0};
    if (sample.looped) {
      int loopPos = sample.loop_sample();
      for (int i = 0; i < loopPos; ++i)
        in[i] = wave[i];
      int loopLen = wave.size() - loopPos;
      for (int i = 0; i < 65536 - loopPos; ++i)
        in[loopPos + i] = wave[loopPos + i % loopLen];
    } else {
      for (size_t i = 0; i < wave.size(); ++i)
        in[i] = wave[i];
    }

    // 用 pYin 算法分析基频
    Yin y(32768, 32000);
    auto yo = y.process(in + 256); // 跳过开头的干扰
    qDebug() << "[ OK ] SPC::parseSample: Successfully parsed sample" << srcn
             << "f0 =" << yo.f0;
    sampleFreq[srcn] = yo.f0;
  }

  delete spcFile;
}

void SPC::parseSPC(quint8 *data, int size) {
  char buf[128];
  int total, padding;

  // 读取签名信息
  if (parseNChars(data, 33, size, buf) != 33)
    return;
  if (parse16Bit(data + 33, size - 33, &padding) != 2)
    return;
  total = 35;

  // 检查签名信息
  if (strncmp(buf, "SNES-SPC700 Sound File Data v0.30", 33) != 0)
    return;
  if (padding != 0x1A1A)
    return;

  // 读取、检查首部
  switch (data[total]) {
  case 0x00:
  case 0x1A:
    info.hasId666Tag = true;
    break;
  case 0x1B:
    info.hasId666Tag = false;
    break;
  default:
    return;
  }
  info.minVersion = data[++total];

  total = 0x2E;
  total += parseNChars(data + total, 32, size - total, buf);
  info.songTitle = QString::fromUtf8(buf, 32);
  total += parseNChars(data + total, 32, size - total, buf);
  info.gameTitle = QString::fromUtf8(buf, 32);
  total += parseNChars(data + total, 16, size - total, buf);
  info.dumperName = QString::fromUtf8(buf, 16);
  total += parseNChars(data + total, 32, size - total, buf);
  info.comments = QString::fromUtf8(buf, 32);

  bool isTextFmt = true;
  total += parseNChars(data + total, 11, size - total, buf);
  for (int i = 0; i < 11; ++i)
    if (!isdigit(buf[i]) && buf[i] != '/' && buf[i] != '\0') {
      isTextFmt = false;
      break;
    }
  total += parseNChars(data + total, 3, size - total, buf);
  for (int i = 0; i < 3; ++i)
    if (!isdigit(buf[i]) && buf[i] != '/' && buf[i] != '\0') {
      isTextFmt = false;
      break;
    }

  total = 0x9E;
  if (isTextFmt) {
    total += parseNChars(data + total, 11, size - total, buf);
    info.dumpDate = QString::fromUtf8(buf, 11);
  } else {
    int year = data[total] | (data[total + 1] << 8); // 小端法
    int month = data[total + 2], day = data[total + 3];
    snprintf(buf, 128, "%d/%d/%d", year, month, day);
    info.dumpDate = QString::fromUtf8(buf);
    total = 0xA9;
  }

  total += parseNChars(data + total, 3, size - total, buf);
  auto playTimeStr = QString::fromUtf8(buf, 3);
  if (playTimeStr.isEmpty())
    info.playTimeSec = DEFAULT_PLAY_TIME_SEC;
  else {
    auto playTime = playTimeStr.toInt();
    info.playTimeSec = playTime == 0 ? DEFAULT_PLAY_TIME_SEC : playTime;
  }
  total += parseNChars(data + total, 5, size - total, buf);
  auto fadeoutTimeStr = QString::fromUtf8(buf, 5);
  if (fadeoutTimeStr.isEmpty())
    info.fadeoutTimeMs = DEFAULT_FADEOUT_TIME_MS;
  else {
    auto fadeoutTime = fadeoutTimeStr.toInt();
    info.fadeoutTimeMs =
        fadeoutTime == 0 ? DEFAULT_FADEOUT_TIME_MS : fadeoutTime;
  }

  total += parseNChars(data + total, 32, size - total, buf);
  info.artistName = QString::fromUtf8(buf, 32);
  info.mutedVoices = data[total++];

  switch (data[total]) {
  case '1':
    info.dumpingEmu = ZSNES;
    break;
  case '2':
    info.dumpingEmu = SNES9x;
    break;
  default:
    info.dumpingEmu = Unknown;
  }

  preRender(data, size);
}

void SPC::preRender(quint8 *data, int size) {
  // 初始化模拟器并加载数据
  SNES_SPC emu;
  emu.init();
  if (emu.load_spc(data, size))
    return;
  emu.clear_echo();

  // 获取播放时间，开始模拟
  uint len = SPCFile::MilliSecondsToXID6Ticks(info.playTimeSec * 1000);
  qDebug() << "[ OK ] SPC::preRender: Got play time:" << info.playTimeSec;
  SPCEvent *channelOn[8] = {nullptr}; // 方便连接音符事件
  bool lastKeyOn[8] = {false};
  int simultaneous = 0;

  for (uint tick = 0; tick <= len; tick += 2) {
    auto time = SPCFile::XID6TicksToMilliSeconds(tick) * 1000LL;

    // 保存 SPC 状态（save_spc 稳定但是效率低，copy_state 更快但是有 BUG）
    emu.save_spc(data);

    // 在最后强制在所有的通道触发 KOFF
    if (tick == len)
      data[0x1015C] = 0xFF;

    for (int c = 0; c < 8; ++c) {
      int spcPitch =
          (data[0x10103 + c * 0x10] & 0x3F) << 8 | data[0x10102 + c * 0x10];
      int srcn = data[0x10104 + c * 0x10];

      double freq = static_cast<double>(spcPitch) / 0x1000 * sampleFreq[srcn];
      if (freq <= 0) // 疑似鼓点，不显示音符
        continue;
      int midiPitch = floor(12.0 * log2(freq / 440.0) + 69.5);
      if (midiPitch < 0 || midiPitch > 127) // 疑似鼓点，不显示音符
        continue;

      bool keyOn = data[0x1014C] >> c & 1, keyOff = data[0x1015C] >> c & 1;
      if (keyOff) {
        // KOFF 为真。如果上一个事件为 KON，插入一个匹配的 KOFF 事件
        if (channelOn[c]) {
          auto last = channelOn[c];
          // qDebug() << "[ OK ] SPC::preRender: Tick" << tick << "Channel" << c
          //          << "KON" << keyOn << "KOFF" << keyOff << "SRCN"
          //          << last->getSample() << "Pitch" << last->getNote();
          auto event = new SPCEvent;
          event->setEvent(SPCEvent::KeyOff, last->getTrack(), tick, c,
                          last->getNote());
          event->setSister(channelOn[c]);
          event->setAbsMicroSec(time);
          event->setSimultaneous(simultaneous--);
          events.push_back(event);
          channelOn[c] = nullptr;
        }
      } else if (keyOn) {
        // KOFF 为假，KON 为真
        if (!channelOn[c]) {
          // 如果上一个事件为空，插入一个新 KON 事件.
          // qDebug() << "[ OK ] SPC::preRender: Tick" << tick << "Channel" << c
          //          << "KON" << keyOn << "KOFF" << keyOff << "SRCN" << srcn
          //          << "Pitch" << midiPitch;

          // 先检查是否存在音轨
          int found;
          for (found = 0; found < info.numTracks; found++) {
            auto &info = tracks[found]->trackInfo;
            if (info.noteCount[c] > 0 && info.srcn[c] == srcn) {
              // 找到了，在当前音轨
              info.noteCount[c]++;
              break;
            }
          }
          if (found == info.numTracks) {
            // 新增一个音轨
            auto track = new SPCTrack();
            track->trackInfo.noteCount[c] = 1;
            track->trackInfo.srcn[c] = srcn;
            track->trackInfo.numChannels = 1;
            tracks.push_back(track);
            info.numTracks++;
            info.numChannels++;
            qDebug() << "[ OK ] SPC::preRender: Added track" << info.numTracks;
          }

          auto event = new SPCEvent;
          event->setEvent(SPCEvent::KeyOn, found, tick, c, midiPitch);
          event->setAbsMicroSec(time);
          event->setSimultaneous(simultaneous++);
          events.push_back(event);
          channelOn[c] = event;

          // SPCInfo 操作
          if (events.empty()) {
            info.minNote = midiPitch;
            info.maxNote = midiPitch;
          } else {
            info.minNote = std::min(info.minNote, midiPitch);
            info.maxNote = std::max(info.maxNote, midiPitch);
          }
        } else {
          // 上一个事件为 KON 事件，如果音高改变且 KOFF 变为 KON，需要插入 KOFF
          // 和新的 KON 事件
          auto last = channelOn[c];
          if (!lastKeyOn[c] && last->getNote() != midiPitch) {
            // qDebug() << "[ OK ] SPC::preRender: Tick" << tick << "Channel" <<
            //        c << "KON" << keyOn << "KOFF" << keyOff << "SRCN"
            //          << last->getSample() << "Pitch" << last->getNote()
            //          << "Pitch now" << midiPitch;
            auto offEvent = new SPCEvent;
            offEvent->setEvent(SPCEvent::KeyOff, last->getTrack(), tick, c,
                               last->getNote());
            offEvent->setSister(channelOn[c]);
            offEvent->setAbsMicroSec(time);
            offEvent->setSimultaneous(simultaneous--);
            events.push_back(offEvent);

            int found = 0;
            for (found = 0; found < info.numTracks; found++) {
              auto &info = tracks[found]->trackInfo;
              if (info.noteCount[c] > 0 && info.srcn[c] == srcn) {
                info.noteCount[c]++;
                break;
              }
            }

            auto onEvent = new SPCEvent;
            onEvent->setEvent(SPCEvent::KeyOn, found, tick, c, midiPitch);
            onEvent->setAbsMicroSec(time);
            onEvent->setSimultaneous(simultaneous++);
            events.push_back(onEvent);
            channelOn[c] = onEvent;

            info.minNote = std::min(info.minNote, midiPitch);
            info.maxNote = std::max(info.maxNote, midiPitch);
          }
        }
      } else if (channelOn[c]) {
        // KON，KOFF 都为假，上一个事件为 KON，若 SRCN 改变，插入一个 KOFF 事件
        auto last = channelOn[c];
        if (tracks[last->getTrack()]->trackInfo.srcn[c] != srcn) {
          // qDebug() << "[ OK ] SPC::preRender: Tick" << tick << "Channel" <<
          //        c << "KON" << keyOn << "KOFF" << keyOff << "SRCN"
          //          << last->getSample() << "Pitch" << last->getNote()
          auto event = new SPCEvent;
          event->setEvent(SPCEvent::KeyOff, last->getTrack(), tick, c,
                          last->getNote());
          event->setSister(channelOn[c]);
          event->setAbsMicroSec(time);
          event->setSimultaneous(simultaneous--);
          events.push_back(event);
          channelOn[c] = nullptr;
        }
      }

      lastKeyOn[c] = keyOn;
    }

    // 运行模拟器，写入音频
    short buf[2];
    emu.play(2, buf);
    wave.append(reinterpret_cast<char *>(buf), 4);
  }

  info.eventCount = events.size();
  info.totalMicroSecs = SPCFile::XID6TicksToMilliSeconds(len) * 1000LL;
}

//-----------------------------------------------------------------------------
// 音轨函数
//-----------------------------------------------------------------------------

SPCTrack::~SPCTrack() { clear(); }

void SPCTrack::clear() { trackInfo.clear(); }

//-----------------------------------------------------------------------------
// 事件函数
//-----------------------------------------------------------------------------

void SPCEvent::setEvent(EventType eventType, int track, int absT,
                        quint8 channel, int note) {
  this->eventType = eventType;
  this->track = track;
  this->absT = absT;
  this->channel = channel;
  this->note = note;
}

//-----------------------------------------------------------------------------
// split700 的样本静态函数
//-----------------------------------------------------------------------------

bool SPC::isValidSample(const SPCFile &spc, quint8 srcn) {
  const SPCSampDir &sample = spc.samples[srcn];

  if (!sample.valid)
    return false;

  if ((sample.loop_address - sample.start_address) % SPCSampDir::BRR_CHUNK_SIZE)
    return false;

  if (sample.looped) {
    if (sample.loop_address < sample.start_address ||
        sample.loop_address >= sample.end_address)
      return false;
  }

  if (sample.start_address < 0x200)
    return false;

  quint16 dir = spc.dsp[0x5d] << 8;
  quint8 flg = spc.dsp[0x6c];
  quint16 esa = spc.dsp[0x6d] << 8;
  quint8 edl = spc.dsp[0x7d] & 15;
  bool echoEnabled = ((flg & 0x20) == 0);

  unsigned int dirEnd = dir + ((srcn + 1) * 4);
  if (dirEnd >= 0x10000)
    return false;

  if ((sample.start_address >= dir && sample.start_address < dirEnd) ||
      (sample.end_address > dir && sample.end_address <= dirEnd))
    return false;

  unsigned int numOfChunks =
      (unsigned int)(sample.compressed_size() / SPCSampDir::BRR_CHUNK_SIZE);
  const int LEAST_NUM_OF_CHUNKS = 8;
  const int LEAST_NUM_OF_CHUNKS_HARDER = 2;
  if (numOfChunks < LEAST_NUM_OF_CHUNKS) {
    if (numOfChunks < LEAST_NUM_OF_CHUNKS_HARDER)
      return false;

    if (!sample.looped) {
      if (sample.loop_address < sample.start_address ||
          sample.loop_address >= sample.end_address)
        return false;
    }
  }

  if (echoEnabled) {
    quint16 echoSize = (edl != 0) ? (2048 * edl) : 4;
    quint16 echoEnd = esa + echoSize;

    if (esa < echoEnd) {
      if ((sample.start_address >= esa && sample.start_address < echoEnd) ||
          (sample.end_address > esa && sample.end_address <= echoEnd))
        return false;
    } else {
      if (sample.start_address >= esa || sample.start_address < echoEnd ||
          sample.end_address > esa || sample.end_address <= echoEnd)
        return false;
    }
  }

  return true;
}

vector<quint8> SPC::getSampleList(const SPCFile &spc) {
  vector<quint8> srcns;

  for (int samp = 0; samp < spc.samp_dir_length; samp++) {
    quint8 srcn = (quint8)samp;
    const SPCSampDir &sample = spc.samples[srcn];

    if (!isValidSample(spc, srcn))
      continue;

    bool validAlignment = true;
    for (auto itr_srcn = srcns.begin(); itr_srcn != srcns.end(); ++itr_srcn) {
      const SPCSampDir &valid_sample = spc.samples[*itr_srcn];

      if ((sample.start_address >= valid_sample.start_address &&
           sample.start_address < valid_sample.end_address) ||
          (sample.end_address > valid_sample.start_address &&
           sample.end_address <= valid_sample.end_address)) {

        if ((sample.start_address % SPCSampDir::BRR_CHUNK_SIZE) !=
            (valid_sample.start_address % SPCSampDir::BRR_CHUNK_SIZE)) {
          validAlignment = false;
          break;
        }
      }
    }

    if (!validAlignment)
      continue;

    srcns.push_back(srcn);
  }

  return srcns;
}

//-----------------------------------------------------------------------------
// 解析函数
//-----------------------------------------------------------------------------

// 大端法解析 16 位（2 字节）
int SPC::parse16Bit(const quint8 *data, int maxSize, int *out) {
  if (!data || !out || maxSize < 2)
    return 0;

  *out = data[0];
  *out = (*out << 8) | data[1];

  return 2;
}

// 解析若干个字符
int SPC::parseNChars(const quint8 *data, int nChars, int maxSize, char *out) {
  if (!data || !out || maxSize <= 0)
    return 0;

  int size = std::min(nChars, maxSize);
  memcpy(out, data, size);

  return size;
}