/*************************************************************************************************
 *
 * 文件：ConfigDlg.h
 *
 * 描述：定义配置的对话框
 *
 * SPDX-License-Identifier: MIT
 *
 *************************************************************************************************/
#pragma once

#include <QAbstractButton>

#include "Config.h"

namespace Ui {
class ConfigDialog;
} // namespace Ui

class ConfigDialog : public QDialog {
  Q_OBJECT

public:
  ConfigDialog(QWidget *parent = nullptr);
  ~ConfigDialog();

private:
  void initVisual(QWidget *widget);
  void setVisualValues(QWidget *widget, const VisualSettings visual);
  void applyVisual(QWidget *widget);

  void initVideo(QWidget *widget);
  void applyVideo(QWidget *widget);

  void initControls(QWidget *widget);
  void applyControls(QWidget *widget);

  Ui::ConfigDialog *ui;
  QColor customColor[16] = {
      0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
      0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
      0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF,
  };

private slots:
  void on_allKeysButton_clicked();
  void on_songKeysButton_clicked();
  void on_customKeysButton_clicked();
  void on_firstKeyComboBox_currentIndexChanged();
  void on_lastKeyComboBox_currentIndexChanged();
  void on_showControlsCheckBox_checkStateChanged();
  void on_color1Button_clicked();
  void on_color2Button_clicked();
  void on_color3Button_clicked();
  void on_color4Button_clicked();
  void on_color5Button_clicked();
  void on_color6Button_clicked();
  void on_bkgColorButton_clicked();
  void on_restoreDefaultsButton_clicked();

  void on_openGLButton_clicked();
  void on_directXButton_clicked();
  void on_vulkanButton_clicked();
  void on_displayFPSCheckBox_checkStateChanged();
  void on_limitFPSCheckBox_checkStateChanged();

  void on_lrArrowsSpinBox_valueChanged();
  void on_udArrowsSpinBox_valueChanged();

  void on_buttonBox_clicked(QAbstractButton *button);
  void accept() override;
};

class TrackDialog : public QDialog {
  Q_OBJECT

public:
  TrackDialog(QWidget *parent = nullptr);

  void init(MainScreen *gameState);
  // TODO：置于控件中
  std::vector<bool> hidden;
  std::vector<QColor> colors;

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  MainScreen *gameState;

private slots:
  void on_header_sectionClicked(int index);
  void accept() override;
};

void doPreferences(QWidget *window);

bool getCustomSettings(MainScreen *gameState);