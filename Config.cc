/*************************************************************************************************
 *
 * 文件：Config.cc
 *
 * 描述：配置对象的实现
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#include <QDir>
#include <QStandardPaths>

#include "Config.h"

//-----------------------------------------------------------------------------
// 主设置类
//-----------------------------------------------------------------------------

Config &Config::getConfig() {
  static Config instance;
  return instance;
}

Config::Config() {
  loadDefaultValues();
  loadConfigValues();
}

QString Config::getFolder() {
  auto appData =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir appDataDir(appData);
  if (!appDataDir.exists())
    if (!appDataDir.mkpath(".")) {
      qCritical() << "[FAIL] Config::getFolder: Cannot get app data directory!";
      return QString();
    }

  return appData;
}

void Config::loadDefaultValues() {
  visualSettings.loadDefaultValues();
  videoSettings.loadDefaultValues();
  controlsSettings.loadDefaultValues();
  playbackSettings.loadDefaultValues();
  viewSettings.loadDefaultValues();
}

void Config::loadConfigValues() {
  // 读取哪里？
  auto path = getFolder();
  if (path.length() == 0)
    return;

  // 加载文档
  QFile file(path + "/Config.xml");
  if (!file.open(QIODevice::ReadOnly)) {
    qCritical() << "[FAIL] Config::loadConfigValues: Cannot open xml!";
    return;
  }

  QXmlStreamReader reader(&file);
  // 跳过起始头
  reader.readNext();
  reader.readNext();

  loadConfigValues(reader);

  reader.readNext();
  file.close();
}

void Config::loadConfigValues(QXmlStreamReader &reader) {
  visualSettings.loadConfigValues(reader);
  videoSettings.loadConfigValues(reader);
  controlsSettings.loadConfigValues(reader);
  playbackSettings.loadConfigValues(reader);
  viewSettings.loadConfigValues(reader);
}

bool Config::saveConfigValues() {
  // 保存在哪里？
  auto path = getFolder();
  if (path.length() == 0)
    return false;

  // 创建 XML 文件
  QFile file(path + "/Config.xml");
  if (!file.open(QIODevice::WriteOnly)) {
    qCritical() << "[FAIL] Config::saveConfigValues: Cannot write xml!";
    return false;
  }

  QXmlStreamWriter writer(&file);
  writer.writeStartDocument();
  writer.writeStartElement(APPNAMENOSPACES);

  // 保存各个配置
  saveConfigValues(writer);

  writer.writeEndElement();
  writer.writeEndDocument();
  file.close();

  qDebug() << "[ OK ] Config::saveConfigValues: Successfully saved config.";
  return true;
}

bool Config::saveConfigValues(QXmlStreamWriter &writer) {
  bool saved = true;
  saved &= visualSettings.saveConfigValues(writer);
  saved &= videoSettings.saveConfigValues(writer);
  saved &= controlsSettings.saveConfigValues(writer);
  saved &= playbackSettings.saveConfigValues(writer);
  saved &= viewSettings.saveConfigValues(writer);
  return saved;
}

//-----------------------------------------------------------------------------
// loadDefaultValues
//-----------------------------------------------------------------------------

void VisualSettings::loadDefaultValues() {
  keysShown = All;
  alwaysShowControls = false;
  firstKey = SPC::A0;
  lastKey = SPC::C8;

  bkgColor = 0xFF303030;
  int S = 80, V = 100;
  int colors = sizeof(this->colors) / sizeof(this->colors[0]);
  for (int i = 10, count = 0; count < colors; i = (i + 7) % colors, count++) {
    this->colors[count] = QColor::fromHsv(360 * i / colors, S * 255 / 100,
                                          V * 255 / 100); // 注意范围
  }
  std::swap(this->colors[2], this->colors[4]);
}

void VideoSettings::loadDefaultValues() {
  limitFPS = true;
  showFPS = false;
  renderer = OpenGL;
}

void ControlsSettings::loadDefaultValues() {
  fwdBackSecs = 3.0;
  speedUpPct = 10.0;
}

void PlaybackSettings::loadDefaultValues() {
  playMode = GameState::Intro;
  mute = false;
  playable = false;
  paused = true;
  speed = 1.0;
  nSpeed = 1.0;
  volume = 1.0;
}

void ViewSettings::loadDefaultValues() {
  controls = true;
  keyBoard = true;
  onTop = false;
  fullScreen = false;
  zoomMove = false;
  offsetX = 0.0f;
  offsetY = 0.0f;
  zoomX = 1.0f;
  mainLeft = CW_USEDEFAULT;
  mainTop = CW_USEDEFAULT;
  mainWidth = 960;
  mainHeight = 589;
}

//-----------------------------------------------------------------------------
// loadConfigValues
//-----------------------------------------------------------------------------

void VisualSettings::loadConfigValues(QXmlStreamReader &reader) {
  reader.readNext();
  auto name = reader.name().toString();
  if (name != "visual") {
    qCritical()
        << "[FAIL] VisualSettings::loadConfigValues: Cannot find visual block!";
    return;
  }

  // 属性
  auto attr = reader.attributes();
  keysShown = static_cast<KeysShown>(attr.value("keys-shown").toInt());
  alwaysShowControls = attr.value("always-show-controls").toInt();
  firstKey = attr.value("first-key").toInt();
  lastKey = attr.value("last-key").toInt();

  // 颜色
  int r, g, b;
  size_t i = 0;
  reader.readNext();
  name = reader.name().toString();
  if (name == "colors")
    for (reader.readNext(), name = reader.name().toString();
         name == "color" && i < sizeof(colors) / sizeof(colors[0]);
         reader.readNext(), name = reader.name().toString(), i++) {
      attr = reader.attributes();
      r = attr.value("r").toInt();
      g = attr.value("g").toInt();
      b = attr.value("b").toInt();
      colors[i] = QColor(r, g, b);
      reader.readNext();
    }
  else
    qCritical()
        << "[FAIL] VisualSettings::loadConfigValues: Cannot find colors block!";

  reader.readNext();
  name = reader.name().toString();
  if (name == "bkg-color") {
    attr = reader.attributes();
    r = attr.value("r").toInt();
    g = attr.value("g").toInt();
    b = attr.value("b").toInt();
    bkgColor = QColor(r, g, b);
  } else
    qCritical() << "[FAIL] VisualSettings::loadConfigValues: Cannot find "
                   "bkg-color block!";

  reader.readNext();
  reader.readNext();
}

void VideoSettings::loadConfigValues(QXmlStreamReader &reader) {
  reader.readNext();
  auto name = reader.name().toString();
  if (name != "video") {
    qCritical()
        << "[FAIL] VideoSettings::loadConfigValues: Cannot find video block!";
    return;
  }

  auto attr = reader.attributes();
  showFPS = attr.value("show-FPS").toInt();
  limitFPS = attr.value("limit-FPS").toInt();
  renderer = static_cast<Renderer>(attr.value("renderer").toInt());
  reader.readNext();
}

void ControlsSettings::loadConfigValues(QXmlStreamReader &reader) {
  reader.readNext();
  auto name = reader.name().toString();
  if (name != "controls") {
    qCritical() << "[FAIL] ControlsSettings::loadConfigValues: Cannot find "
                   "controls block!";
    return;
  }

  auto attr = reader.attributes();
  fwdBackSecs = attr.value("fwd-back-secs").toDouble();
  speedUpPct = attr.value("speed-up-pct").toDouble();
  reader.readNext();
}

void PlaybackSettings::loadConfigValues(QXmlStreamReader &reader) {
  reader.readNext();
  auto name = reader.name().toString();
  if (name != "playback") {
    qCritical() << "[FAIL] PlaybackSettings::loadConfigValues: Cannot find "
                   "playback block!";
    return;
  }

  auto attr = reader.attributes();
  mute = attr.value("mute").toInt();
  nSpeed = attr.value("note-speed").toDouble();
  volume = attr.value("vol").toDouble();
  reader.readNext();
}

void ViewSettings::loadConfigValues(QXmlStreamReader &reader) {
  reader.readNext();
  auto name = reader.name().toString();
  if (name != "view") {
    qCritical()
        << "[FAIL] ViewSettings::loadConfigValues: Cannot find view block!";
    return;
  }

  auto attr = reader.attributes();
  controls = attr.value("controls").toInt();
  keyBoard = attr.value("keyboard").toInt();
  onTop = attr.value("on-top").toInt();
  offsetX = attr.value("offset-x").toFloat();
  offsetY = attr.value("offset-y").toFloat();
  zoomX = attr.value("zoom-x").toFloat();
  mainLeft = attr.value("main-left").toInt();
  mainTop = attr.value("main-top").toInt();
  mainWidth = attr.value("main-width").toInt();
  mainHeight = attr.value("main-height").toInt();
  reader.readNext();
}

//-----------------------------------------------------------------------------
// saveConfigValues
//-----------------------------------------------------------------------------

bool VisualSettings::saveConfigValues(QXmlStreamWriter &writer) {
  writer.writeStartElement("visual");
  writer.writeAttribute("keys-shown", QString::number(keysShown));
  writer.writeAttribute("always-show-controls",
                        QString::number(alwaysShowControls));
  writer.writeAttribute("first-key", QString::number(firstKey));
  writer.writeAttribute("last-key", QString::number(lastKey));

  writer.writeStartElement("colors");
  for (size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
    writer.writeStartElement("color");
    writer.writeAttribute("r", QString::number(colors[i].red()));
    writer.writeAttribute("g", QString::number(colors[i].green()));
    writer.writeAttribute("b", QString::number(colors[i].blue()));
    writer.writeEndElement();
  }
  writer.writeEndElement();

  writer.writeStartElement("bkg-color");
  writer.writeAttribute("r", QString::number(bkgColor.red()));
  writer.writeAttribute("g", QString::number(bkgColor.green()));
  writer.writeAttribute("b", QString::number(bkgColor.blue()));
  writer.writeEndElement();

  writer.writeEndElement();
  return true;
}

bool VideoSettings::saveConfigValues(QXmlStreamWriter &writer) {
  writer.writeStartElement("video");
  writer.writeAttribute("renderer", QString::number(renderer));
  writer.writeAttribute("show-FPS", QString::number(showFPS));
  writer.writeAttribute("limit-FPS", QString::number(limitFPS));
  writer.writeEndElement();
  return true;
}

bool ControlsSettings::saveConfigValues(QXmlStreamWriter &writer) {
  writer.writeStartElement("controls");
  writer.writeAttribute("fwd-back-secs", QString::number(fwdBackSecs));
  writer.writeAttribute("speed-up-pct", QString::number(speedUpPct));
  writer.writeEndElement();
  return true;
}

bool PlaybackSettings::saveConfigValues(QXmlStreamWriter &writer) {
  writer.writeStartElement("playback");
  writer.writeAttribute("mute", QString::number(mute));
  writer.writeAttribute("vol", QString::number(volume));
  writer.writeAttribute("note-speed", QString::number(speed));
  writer.writeEndElement();
  return true;
}

bool ViewSettings::saveConfigValues(QXmlStreamWriter &writer) {
  writer.writeStartElement("view");
  writer.writeAttribute("controls", QString::number(controls));
  writer.writeAttribute("keyboard", QString::number(keyBoard));
  writer.writeAttribute("on-top", QString::number(onTop));
  writer.writeAttribute("offset-x", QString::number(offsetX));
  writer.writeAttribute("offset-y", QString::number(offsetY));
  writer.writeAttribute("zoom-x", QString::number(zoomX));
  writer.writeAttribute("main-left", QString::number(mainLeft));
  writer.writeAttribute("main-top", QString::number(mainTop));
  writer.writeAttribute("main-width", QString::number(mainWidth));
  writer.writeAttribute("main-height", QString::number(mainHeight));
  writer.writeEndElement();
  return true;
}