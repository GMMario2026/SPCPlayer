/*************************************************************************************************
 *
 * 文件：ConfigDialog.cc
 *
 * 描述：实现配置的对话框
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#include <QColorDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QMouseEvent>
#include <QStyledItemDelegate>
#include <QTableWidget>

#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ConfigDialog) {
  ui->setupUi(this);

  initVisual(ui->visualTab);
  initVideo(ui->videoTab);
  initControls(ui->controlsTab);

  auto applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
  applyButton->setText("应用 (&A)");
  applyButton->setEnabled(false);
}

ConfigDialog::~ConfigDialog() { delete ui; }

void ConfigDialog::initVisual(QWidget *widget) {
  auto firstKeyComboBox = widget->findChild<QComboBox *>("firstKeyComboBox");
  auto lastKeyComboBox = widget->findChild<QComboBox *>("lastKeyComboBox");

  // 插入按键符号
  for (int i = SPC::A0; i <= SPC::C8; i++) {
    firstKeyComboBox->addItem(SPC::noteName(i));
    lastKeyComboBox->addItem(SPC::noteName(i));
  }
  firstKeyComboBox->setCurrentIndex(-1);
  lastKeyComboBox->setCurrentIndex(-1);

  // 获取填充的配置
  auto &config = Config::getConfig();
  setVisualValues(widget, config.getVisualSettings());
}

void ConfigDialog::setVisualValues(QWidget *widget,
                                   const VisualSettings visual) {
  auto allKeysButton = widget->findChild<QRadioButton *>("allKeysButton");
  auto songKeysButton = widget->findChild<QRadioButton *>("songKeysButton");
  auto customKeysButton = widget->findChild<QRadioButton *>("customKeysButton");
  auto showControlsCheckBox =
      widget->findChild<QCheckBox *>("showControlsCheckBox");
  auto firstKeyComboBox = widget->findChild<QComboBox *>("firstKeyComboBox");
  auto lastKeyComboBox = widget->findChild<QComboBox *>("lastKeyComboBox");

  // 设置值
  allKeysButton->setChecked(visual.keysShown == visual.All);
  songKeysButton->setChecked(visual.keysShown == visual.Song);
  customKeysButton->setChecked(visual.keysShown == visual.Custom);
  showControlsCheckBox->setChecked(visual.alwaysShowControls);
  firstKeyComboBox->setEnabled(visual.keysShown == visual.Custom);
  lastKeyComboBox->setEnabled(visual.keysShown == visual.Custom);
  firstKeyComboBox->setCurrentIndex(visual.firstKey - SPC::A0);
  lastKeyComboBox->setCurrentIndex(visual.lastKey - SPC::A0);

  // 颜色
  for (int i = 0; i < 6; i++) {
    auto colorButton =
        widget->findChild<QToolButton *>(QString("color%1Button").arg(i + 1));
    colorButton->setProperty("userData", visual.colors[i]);
    colorButton->setStyleSheet(
        QString("border: 2px inset gray; background-color: %1;")
            .arg(colorButton->property("userData").value<QColor>().name()));
  }
  auto bkgColorButton =
      widget->findChild<QToolButton *>("bkgColorButton");
  bkgColorButton->setProperty("userData", visual.bkgColor);
  bkgColorButton->setStyleSheet(
      QString("border: 2px inset gray; background-color: %1;")
          .arg(bkgColorButton->property("userData")
                   .value<QColor>()
                   .name()));
}

void ConfigDialog::on_allKeysButton_clicked() {
  ui->firstKeyComboBox->setEnabled(false);
  ui->lastKeyComboBox->setEnabled(false);
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_songKeysButton_clicked() { on_allKeysButton_clicked(); }

void ConfigDialog::on_customKeysButton_clicked() {
  ui->firstKeyComboBox->setEnabled(true);
  ui->lastKeyComboBox->setEnabled(true);
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_firstKeyComboBox_currentIndexChanged() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_lastKeyComboBox_currentIndexChanged() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_showControlsCheckBox_checkStateChanged() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_color1Button_clicked() {
  QColorDialog colorDialog;
  for (int i = 0; i < 16; i++)
    colorDialog.setCustomColor(i, customColor[i]);
  colorDialog.setCurrentColor(
      ui->color1Button->property("userData").value<QColor>());
  if (colorDialog.exec() == QDialog::Accepted) {
    ui->color1Button->setProperty("userData", colorDialog.currentColor());
    ui->color1Button->setStyleSheet(
        QString("border: 2px inset gray; background-color: %1;")
            .arg(colorDialog.currentColor().name()));
  }
  for (int i = 0; i < 16; i++)
    customColor[i] = colorDialog.customColor(i);

  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_color2Button_clicked() {
  QColorDialog colorDialog;
  for (int i = 0; i < 16; i++)
    colorDialog.setCustomColor(i, customColor[i]);
  colorDialog.setCurrentColor(
      ui->color2Button->property("userData").value<QColor>());
  if (colorDialog.exec() == QDialog::Accepted) {
    ui->color2Button->setProperty("userData", colorDialog.currentColor());
    ui->color2Button->setStyleSheet(
        QString("border: 2px inset gray; background-color: %1;")
            .arg(colorDialog.currentColor().name()));
  }
  for (int i = 0; i < 16; i++)
    customColor[i] = colorDialog.customColor(i);

  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_color3Button_clicked() {
  QColorDialog colorDialog;
  for (int i = 0; i < 16; i++)
    colorDialog.setCustomColor(i, customColor[i]);
  colorDialog.setCurrentColor(
      ui->color3Button->property("userData").value<QColor>());
  if (colorDialog.exec() == QDialog::Accepted) {
    ui->color3Button->setProperty("userData", colorDialog.currentColor());
    ui->color3Button->setStyleSheet(
        QString("border: 2px inset gray; background-color: %1;")
            .arg(colorDialog.currentColor().name()));
  }
  for (int i = 0; i < 16; i++)
    customColor[i] = colorDialog.customColor(i);

  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_color4Button_clicked() {
  QColorDialog colorDialog;
  for (int i = 0; i < 16; i++)
    colorDialog.setCustomColor(i, customColor[i]);
  colorDialog.setCurrentColor(
      ui->color4Button->property("userData").value<QColor>());
  if (colorDialog.exec() == QDialog::Accepted) {
    ui->color4Button->setProperty("userData", colorDialog.currentColor());
    ui->color4Button->setStyleSheet(
        QString("border: 2px inset gray; background-color: %1;")
            .arg(colorDialog.currentColor().name()));
  }
  for (int i = 0; i < 16; i++)
    customColor[i] = colorDialog.customColor(i);

  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_color5Button_clicked() {
  QColorDialog colorDialog;
  for (int i = 0; i < 16; i++)
    colorDialog.setCustomColor(i, customColor[i]);
  colorDialog.setCurrentColor(
      ui->color5Button->property("userData").value<QColor>());
  if (colorDialog.exec() == QDialog::Accepted) {
    ui->color5Button->setProperty("userData", colorDialog.currentColor());
    ui->color5Button->setStyleSheet(
        QString("border: 2px inset gray; background-color: %1;")
            .arg(colorDialog.currentColor().name()));
  }
  for (int i = 0; i < 16; i++)
    customColor[i] = colorDialog.customColor(i);

  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_color6Button_clicked() {
  QColorDialog colorDialog;
  for (int i = 0; i < 16; i++)
    colorDialog.setCustomColor(i, customColor[i]);
  colorDialog.setCurrentColor(
      ui->color6Button->property("userData").value<QColor>());
  if (colorDialog.exec() == QDialog::Accepted) {
    ui->color6Button->setProperty("userData", colorDialog.currentColor());
    ui->color6Button->setStyleSheet(
        QString("border: 2px inset gray; background-color: %1;")
            .arg(colorDialog.currentColor().name()));
  }
  for (int i = 0; i < 16; i++)
    customColor[i] = colorDialog.customColor(i);

  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_bkgColorButton_clicked() {
  QColorDialog colorDialog;
  for (int i = 0; i < 16; i++)
    colorDialog.setCustomColor(i, customColor[i]);
  colorDialog.setCurrentColor(
      ui->bkgColorButton->property("userData").value<QColor>());
  if (colorDialog.exec() == QDialog::Accepted) {
    ui->bkgColorButton->setProperty("userData", colorDialog.currentColor());
    ui->bkgColorButton->setStyleSheet(
        QString("border: 2px inset gray; background-color: %1;")
            .arg(colorDialog.currentColor().name()));
  }
  for (int i = 0; i < 16; i++)
    customColor[i] = colorDialog.customColor(i);

  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_restoreDefaultsButton_clicked() {
  VisualSettings visualSettings;
  visualSettings.loadDefaultValues();

  setVisualValues(ui->visualTab, visualSettings);
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::applyVisual(QWidget *widget) {
  // 获取配置拷贝，覆盖设置
  auto &config = Config::getConfig();
  auto visual = config.getVisualSettings();
  auto &view = config.getViewSettings();

  auto allKeysButton = widget->findChild<QRadioButton *>("allKeysButton");
  auto songKeysButton = widget->findChild<QRadioButton *>("songKeysButton");
  auto customKeysButton = widget->findChild<QRadioButton *>("customKeysButton");
  auto showControlsCheckBox =
      widget->findChild<QCheckBox *>("showControlsCheckBox");
  auto firstKeyComboBox = widget->findChild<QComboBox *>("firstKeyComboBox");
  auto lastKeyComboBox = widget->findChild<QComboBox *>("lastKeyComboBox");

  // VisualSettings 结构体
  bool alwaysShowControls = visual.alwaysShowControls;
  visual.keysShown = (allKeysButton->isChecked()      ? visual.All
                      : songKeysButton->isChecked()   ? visual.Song
                      : customKeysButton->isChecked() ? visual.Custom
                                                      : visual.Song);
  visual.alwaysShowControls = showControlsCheckBox->isChecked();
  if (firstKeyComboBox->currentIndex() != -1)
    visual.firstKey = firstKeyComboBox->currentIndex() + SPC::A0;
  if (lastKeyComboBox->currentIndex() != -1)
    visual.lastKey = lastKeyComboBox->currentIndex() + SPC::A0;
  for (int i = 0; i < 6; i++) {
    auto colorButton =
        widget->findChild<QToolButton *>(QString("color%1Button").arg(i + 1));
    visual.colors[i] = colorButton->property("userData").value<QColor>();
  }
  auto bkgColorButton = widget->findChild<QToolButton *>("bkgColorButton");
  visual.bkgColor = bkgColorButton->property("userData").value<QColor>();

  config.setVisualSettings(visual);
  if (view.getFullScreen() && alwaysShowControls != visual.alwaysShowControls)
    view.setControls(view.getControls(), true);
}

void ConfigDialog::initVideo(QWidget *widget) {
  // 获取填充的配置
  auto &config = Config::getConfig();
  auto &video = config.getVideoSettings();

  auto openGLButton = widget->findChild<QRadioButton *>("openGLButton");
  auto directXButton = widget->findChild<QRadioButton *>("directXButton");
  auto vulkanButton = widget->findChild<QRadioButton *>("vulkanButton");
  auto displayFPSCheckBox =
      widget->findChild<QCheckBox *>("displayFPSCheckBox");
  auto limitFPSCheckBox = widget->findChild<QCheckBox *>("limitFPSCheckBox");

  openGLButton->setChecked(video.renderer == VideoSettings::OpenGL);
  directXButton->setChecked(video.renderer == VideoSettings::DirectX);
  vulkanButton->setChecked(video.renderer == VideoSettings::Vulkan);
  displayFPSCheckBox->setChecked(video.showFPS);
  limitFPSCheckBox->setChecked(video.limitFPS);
}

void ConfigDialog::on_openGLButton_clicked() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_directXButton_clicked() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_vulkanButton_clicked() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_displayFPSCheckBox_checkStateChanged() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_limitFPSCheckBox_checkStateChanged() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::applyVideo(QWidget *widget) {
  // 获取配置拷贝，覆盖设置
  auto &config = Config::getConfig();
  auto video = config.getVideoSettings();

  auto openGLButton = widget->findChild<QRadioButton *>("openGLButton");
  auto directXButton = widget->findChild<QRadioButton *>("directXButton");
  auto vulkanButton = widget->findChild<QRadioButton *>("vulkanButton");
  auto displayFPSCheckBox =
      widget->findChild<QCheckBox *>("displayFPSCheckBox");
  auto limitFPSCheckBox = widget->findChild<QCheckBox *>("limitFPSCheckBox");

  video.renderer = (openGLButton->isChecked()    ? video.OpenGL
                    : directXButton->isChecked() ? video.DirectX
                    : vulkanButton->isChecked()  ? video.Vulkan
                                                 : video.OpenGL);
  video.showFPS = displayFPSCheckBox->isChecked();
  video.limitFPS = limitFPSCheckBox->isChecked();

  config.setVideoSettings(video);
}

void ConfigDialog::initControls(QWidget *widget) {
  // 获取填充的配置文件
  auto &config = Config::getConfig();
  auto &ctrls = config.getControlsSettings();

  auto lrArrowsSpinBox = widget->findChild<QDoubleSpinBox *>("lrArrowsSpinBox");
  auto udArrowsSpinBox = widget->findChild<QDoubleSpinBox *>("udArrowsSpinBox");

  lrArrowsSpinBox->setValue(ctrls.fwdBackSecs);
  udArrowsSpinBox->setValue(ctrls.speedUpPct);
}

void ConfigDialog::on_lrArrowsSpinBox_valueChanged() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::on_udArrowsSpinBox_valueChanged() {
  ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void ConfigDialog::applyControls(QWidget *widget) {
  // 获取配置拷贝，覆盖设置
  auto &config = Config::getConfig();
  auto ctrls = config.getControlsSettings();

  ctrls.fwdBackSecs =
      widget->findChild<QDoubleSpinBox *>("lrArrowsSpinBox")->value();
  ctrls.speedUpPct =
      widget->findChild<QDoubleSpinBox *>("udArrowsSpinBox")->value();

  config.setControlsSettings(ctrls);
}

void ConfigDialog::on_buttonBox_clicked(QAbstractButton *button) {
  if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Apply) {
    applyVisual(ui->visualTab);
    applyVideo(ui->videoTab);
    applyControls(ui->controlsTab);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
  }
}

void ConfigDialog::accept() {
  applyVisual(ui->visualTab);
  applyVideo(ui->videoTab);
  applyControls(ui->controlsTab);
  QDialog::accept();
}

void doPreferences(QWidget *window) {
  ConfigDialog configDialog(window);
  configDialog.exec();
}

class CustomDrawDelegate : public QStyledItemDelegate {
public:
  CustomDrawDelegate(TrackDialog *dialog) : dialog(dialog) {}
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;

private:
  const TrackDialog *dialog;
};

TrackDialog::TrackDialog(QWidget *parent) : QDialog(parent) {
  // 设置对话框大小和标题
  setFixedSize(450, 300);
  setWindowTitle("音轨设置");
  setUpdatesEnabled(true);

  // 设置网格布局
  auto layout = new QGridLayout(this);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setHorizontalSpacing(25);
  layout->setVerticalSpacing(5);

  // 添加文本标签
  auto fileStaticLabel = new QLabel("文件：", this);
  layout->addWidget(fileStaticLabel, 0, 0);

  auto fileLabel = new QLabel(this);
  fileLabel->setObjectName("fileLabel");
  fileLabel->setTextInteractionFlags(
      Qt::TextInteractionFlag::TextSelectableByMouse);
  layout->addWidget(fileLabel, 0, 1);

  auto dirStaticLabel = new QLabel("目录：", this);
  layout->addWidget(dirStaticLabel, 1, 0);

  auto dirLabel = new QLabel(this);
  dirLabel->setObjectName("dirLabel");
  dirLabel->setTextInteractionFlags(
      Qt::TextInteractionFlag::TextSelectableByMouse);
  layout->addWidget(dirLabel, 1, 1);

  auto noteCountStaticLabel = new QLabel("音符数：", this);
  layout->addWidget(noteCountStaticLabel, 2, 0);

  auto noteCountLabel = new QLabel(this);
  noteCountLabel->setObjectName("noteCountLabel");
  noteCountLabel->setTextInteractionFlags(
      Qt::TextInteractionFlag::TextSelectableByMouse);
  layout->addWidget(noteCountLabel, 2, 1);

  auto lengthStaticLabel = new QLabel("时长：", this);
  layout->addWidget(lengthStaticLabel, 3, 0);

  auto lengthLabel = new QLabel(this);
  lengthLabel->setObjectName("lengthLabel");
  lengthLabel->setTextInteractionFlags(
      Qt::TextInteractionFlag::TextSelectableByMouse);
  layout->addWidget(lengthLabel, 3, 1);

  // 添加音轨列表
  auto trackTableWidget = new QTableWidget(this);
  trackTableWidget->setObjectName("trackTableWidget");
  trackTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  trackTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
  trackTableWidget->setFocusPolicy(Qt::NoFocus);
  trackTableWidget->setStyleSheet(
      "QTableWidget::item { border: none; padding: 5px }"
      "QHeaderView::section { padding: 5px 8px; }");
  trackTableWidget->setShowGrid(false);
  trackTableWidget->setItemDelegate(new CustomDrawDelegate(this));

  trackTableWidget->viewport()->installEventFilter(this);
  trackTableWidget->horizontalHeader()->setObjectName("header");
  trackTableWidget->verticalHeader()->setVisible(false);
  trackTableWidget->verticalHeader()->setDefaultSectionSize(20);

  layout->addWidget(trackTableWidget, 4, 0, 1, 2);

  // 添加确认、取消按钮
  auto buttonBox = new QDialogButtonBox(this);
  buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Ok |
                                QDialogButtonBox::StandardButton::Cancel);
  layout->addWidget(buttonBox, 5, 1);

  QMetaObject::connectSlotsByName(this);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

bool getCustomSettings(MainScreen *gameState) {
  TrackDialog dialog;
  dialog.init(gameState);
  return dialog.exec() == QDialog::Accepted;
}

void TrackDialog::init(MainScreen *gameState) {
  static const auto &visual = Config::getConfig().getVisualSettings();
  char buf[255];

  // 获取数据
  auto &spc = gameState->getSPC();
  auto &info = spc.getInfo();
  auto &tracks = spc.getTracks();
  this->gameState = gameState;

  // 填充标签中的文本
  auto fileLabel = findChild<QLabel *>("fileLabel");
  auto dirLabel = findChild<QLabel *>("dirLabel");
  auto noteCountLabel = findChild<QLabel *>("noteCountLabel");
  auto lengthLabel = findChild<QLabel *>("lengthLabel");

  fileLabel->setText(QFileInfo(info.fileName).fileName());
  dirLabel->setText(QFileInfo(info.fileName).absolutePath());
  sprintf_s(buf, "%d", info.eventCount / 2);
  noteCountLabel->setText(buf);
  sprintf_s(buf, "%lld:%02.0lf", info.totalMicroSecs / 60000000,
            (info.totalMicroSecs % 60000000) / 1000000.0);
  lengthLabel->setText(buf);

  // 初始化状态变量
  hidden.resize(info.numChannels);
  colors.resize(info.numChannels);
  int colorCount = sizeof(visual.colors) / sizeof(visual.colors[0]);
  for (int i = 0; i < info.numChannels; i++) {
    hidden[i] = false;
    if (i < colorCount)
      colors[i] = visual.colors[i];
    else
      colors[i] = Util::randColor();
  }

  // 设置表格的列
  QList<QString> text = {"音轨", "样本", "音符数", "隐藏", "颜色"};
  auto trackTableWidget = findChild<QTableWidget *>("trackTableWidget");
  trackTableWidget->setHorizontalHeaderLabels(text);
  trackTableWidget->setColumnCount(5);
  trackTableWidget->setColumnWidth(0, 50);
  trackTableWidget->setColumnWidth(1, 50);
  trackTableWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft |
                                                              Qt::AlignVCenter);
  trackTableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignRight |
                                                              Qt::AlignVCenter);
  trackTableWidget->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignRight |
                                                              Qt::AlignVCenter);

  int pos = 0;
  for (int i = 0; i < info.numTracks; i++) {
    auto &trackInfo = tracks[i]->getInfo();
    for (int j = 0; j < 8; j++) {
      if (trackInfo.noteCount[j] > 0) {
        trackTableWidget->insertRow(pos);

        sprintf_s(buf, "%d", pos + 1);
        trackTableWidget->setItem(pos, 0, new QTableWidgetItem(buf));
        trackTableWidget->item(pos, 0)->setTextAlignment(Qt::AlignLeft |
                                                         Qt::AlignVCenter);

        sprintf_s(buf, "%d", trackInfo.srcn[j]);
        trackTableWidget->setItem(pos, 1, new QTableWidgetItem(buf));
        trackTableWidget->item(pos, 1)->setTextAlignment(Qt::AlignRight |
                                                         Qt::AlignVCenter);

        sprintf_s(buf, "%d", trackInfo.noteCount[j]);
        trackTableWidget->setItem(pos, 2, new QTableWidgetItem(buf));
        trackTableWidget->item(pos, 2)->setTextAlignment(Qt::AlignRight |
                                                         Qt::AlignVCenter);

        pos++;
      }
    }
  }
}

void TrackDialog::on_header_sectionClicked(int index) {
  static const auto &visual = Config::getConfig().getVisualSettings();

  bool allChecked = true;
  auto colorCount = sizeof(visual.colors) / sizeof(visual.colors[0]);
  switch (index) {
  case 3:
    for (size_t i = 0; i < hidden.size(); i++)
      allChecked &= hidden[i];
    for (size_t i = 0; i < hidden.size(); i++)
      hidden[i] = !allChecked;
    findChild<QTableWidget *>("trackTableWidget")->reset();
    return;
  case 4:
    for (size_t i = 0; i < colors.size(); i++)
      if (i < colorCount)
        colors[i] = visual.colors[i];
      else
        colors[i] = Util::randColor();
    findChild<QTableWidget *>("trackTableWidget")->reset();
  }
}

bool TrackDialog::eventFilter(QObject *obj, QEvent *event) {
  auto trackTableWidget = findChild<QTableWidget *>("trackTableWidget");
  if (obj == trackTableWidget->viewport()) {
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: {
      auto mouseEvent = static_cast<QMouseEvent *>(event);
      auto index = trackTableWidget->indexAt(mouseEvent->pos());
      if (index.isValid()) {
        switch (index.column()) {
        case 3:
          hidden[index.row()] = !hidden[index.row()];
          trackTableWidget->reset();
          return true;
        case 4: {
          static QColor customColor[16] = {
              0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
              0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
              0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
              0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
          };
          QColorDialog colorDialog;
          for (int i = 0; i < 16; i++)
            colorDialog.setCustomColor(i, customColor[i]);
          colorDialog.setCurrentColor(colors[index.row()]);
          if (colorDialog.exec() == QDialog::Accepted)
            colors[index.row()] = colorDialog.currentColor();
          for (int i = 0; i < 16; i++)
            customColor[i] = colorDialog.customColor(i);
          trackTableWidget->reset();
          return true;
        }
        }
      }
      break;
    }
    case QEvent::Paint:
      return false;
    default:
      return true;
    }
  }
  return false;
}

void CustomDrawDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const {
  if (index.column() >= 3 && index.column() <= 4) {
    // Qt 的复选框大小固定
    QRect out;
    int bmpSize = QApplication::style()->pixelMetric(QStyle::PM_IndicatorWidth);
    out.setLeft(option.rect.left() +
                (option.rect.right() - option.rect.left() - bmpSize) / 2);
    out.setTop(option.rect.top() +
               (option.rect.bottom() - option.rect.top() - bmpSize) / 2);
    out.setRight(out.left() + bmpSize);
    out.setBottom(out.top() + bmpSize);

    // 绘制复选框和颜色选择框
    if (index.column() == 3) {
      QStyleOptionButton button;
      button.state =
          QStyle::State_Enabled |
          (dialog->hidden[index.row()] ? QStyle::State_On : QStyle::State_Off);
      button.rect = out;
      QApplication::style()->drawControl(QStyle::CE_CheckBox, &button, painter);
    } else if (index.column() == 4) {
      painter->fillRect(option.rect, QApplication::palette().window());
      auto innerRect = option.rect.adjusted(1, 1, -1, -1);
      painter->fillRect(innerRect, dialog->colors[index.row()]);
      QStyleOptionFrame optionFrame;
      optionFrame.rect = innerRect;
      optionFrame.state = QStyle::State_Sunken;
      QApplication::style()->drawPrimitive(QStyle::PE_Frame, &optionFrame,
                                           painter);
    }
  } else {
    QStyledItemDelegate::paint(painter, option, index);
  }
}

void TrackDialog::accept() {
  gameState->setChannelSettings(hidden, colors);
  QDialog::accept();
}