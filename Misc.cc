/*************************************************************************************************
 *
 * 文件：Misc.cc
 *
 * 描述：实现一些辅助对象
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#include "Misc.h"

//-----------------------------------------------------------------------------
// 简单的函数
//-----------------------------------------------------------------------------

QColor Util::randColor() {
  return QColor::fromHsv(rand() % 360, (rand() % 40 + 60) * 255 / 100,
                         (rand() % 20 + 80) * 255 / 100);
}

QByteArray Util::pcm2Wav(const QByteArray &pcm, int sampleRate,
                         short bitsPerSample, short channels) {
  // 获取波形数据的信息
  const uint dataSize = pcm.size();
  const uint fileSize = 44 + dataSize; // 首部 44 字节
  const int fmtSize = 16;              // PCM 格式大小为 16 字节
  const short audioFmt = 1;            // PCM 格式
  const uint byteRate = sampleRate * channels * (bitsPerSample / 8); // 字节比率
  const ushort blockAlign = channels * (bitsPerSample / 8); // 块对齐

  QByteArray wav;
  // RIFF 块
  wav.append("RIFF");
  wav.append(reinterpret_cast<const char *>(&fileSize), 4);
  wav.append("WAVE");
  // fmt 块
  wav.append("fmt ");
  wav.append(reinterpret_cast<const char *>(&fmtSize), 4);
  wav.append(reinterpret_cast<const char *>(&audioFmt), 2);
  wav.append(reinterpret_cast<const char *>(&channels), 2);
  wav.append(reinterpret_cast<const char *>(&sampleRate), 4);
  wav.append(reinterpret_cast<const char *>(&byteRate), 4);
  wav.append(reinterpret_cast<const char *>(&blockAlign), 2);
  wav.append(reinterpret_cast<const char *>(&bitsPerSample), 2);
  // data 块
  wav.append("data");
  wav.append(reinterpret_cast<const char *>(&dataSize), 4);
  wav.append(pcm);
  return wav;
}