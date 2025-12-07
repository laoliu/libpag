/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2025 Tencent. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "WindowManager.h"
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QQuickStyle>
#include <QTranslator>
#include <QtGui/QFont>
#include <QtQuick/QQuickWindow>
#include <fstream>
#include <memory>
#include "AlertInfoModel.h"
#include "PAGViewerInstallModel.h"
#include "alert/AlertWindow.h"
#include "config/ConfigFile.h"
#include "export/PAGExport.h"
#include "platform/PlatformHelper.h"
#include "utils/AEHelper.h"
#include "utils/FileHelper.h"
#include "utils/StringHelper.h"

namespace exporter {

WindowManager& WindowManager::GetInstance() {
  static WindowManager instance;
  return instance;
}

WindowManager::WindowManager() {
  RunScriptPreWarm();
  initializeQtEnvironment();
  translator = std::make_unique<QTranslator>();
}

void WindowManager::showExportPanelWindow() {
  init();
  
  std::ofstream log("/tmp/pag_panel_debug.log", std::ios::app);
  log << "=== showExportPanelWindow called ===" << std::endl;
  
  // 获取活动合成
  AEGP_ItemH activeItemHandle = GetActiveCompositionItem();
  if (!activeItemHandle) {
    log << "No active composition" << std::endl;
    log.close();
    showSimpleError("请先选择一个合成");
    return;
  }
  log << "Got active composition handle" << std::endl;
  
  // 获取保存路径
  log << "Opening file dialog..." << std::endl;
  log.close();
  
  QString filePath = QFileDialog::getSaveFileName(
      nullptr,
      "导出PAG文件 (Panel)",
      "",
      "PAG Files (*.pag)");
  
  log.open("/tmp/pag_panel_debug.log", std::ios::app);
  log << "File dialog closed, path: " << filePath.toUtf8().toStdString() << std::endl;
  
  if (filePath.isEmpty()) {
    log << "User cancelled" << std::endl;
    log.close();
    return;
  }
  
  // 导出配置
  log << "Creating export config..." << std::endl;
  PAGExportConfigParam configParam;
  configParam.activeItemHandle = activeItemHandle;
  configParam.outputPath = filePath.toUtf8().toStdString();
  configParam.exportAudio = true;
  configParam.hardwareEncode = false;
  configParam.exportActually = true;
  configParam.showAlertInfo = true;
  
  // 执行导出
  log << "Creating PAGExport object..." << std::endl;
  log.close();
  
  try {
    PAGExport exporter(configParam);
    
    log.open("/tmp/pag_panel_debug.log", std::ios::app);
    log << "PAGExport created, calling exportFile()..." << std::endl;
    log.close();
    
    bool success = exporter.exportFile();
    
    log.open("/tmp/pag_panel_debug.log", std::ios::app);
    log << "exportFile() returned: " << (success ? "true" : "false") << std::endl;
    log.close();
    
    if (success) {
      QMessageBox::information(nullptr, "导出成功", "PAG文件导出成功！");
    } else {
      showSimpleError("导出失败，请检查日志");
    }
  } catch (const std::exception& e) {
    log.open("/tmp/pag_panel_debug.log", std::ios::app);
    log << "Exception caught: " << e.what() << std::endl;
    log.close();
    showSimpleError(QString("导出异常: ") + e.what());
  } catch (...) {
    log.open("/tmp/pag_panel_debug.log", std::ios::app);
    log << "Unknown exception caught" << std::endl;
    log.close();
    showSimpleError("导出时发生未知错误");
  }
}

void WindowManager::showPAGConfigWindow() {
  init();
  if (configWindow == nullptr) {
    configWindow = std::make_unique<ConfigWindow>(app.get());
  }
  configWindow->show();
  app->exec();
}

void WindowManager::showExportPreviewWindow() {
  init();
  
  // 获取活动合成
  AEGP_ItemH activeItemHandle = GetActiveCompositionItem();
  if (!activeItemHandle) {
    showSimpleError("请先选择一个合成");
    return;
  }
  
  // 获取保存路径用于预览
  QString filePath = QFileDialog::getSaveFileName(
      nullptr,
      "预览PAG文件",
      "",
      "PAG Files (*.pag)");
  
  if (filePath.isEmpty()) {
    return;
  }
  
  // 导出配置
  PAGExportConfigParam configParam;
  configParam.activeItemHandle = activeItemHandle;
  configParam.outputPath = filePath.toUtf8().toStdString();
  configParam.exportAudio = true;
  configParam.hardwareEncode = false;
  configParam.exportActually = true;
  configParam.showAlertInfo = true;
  
  // 执行导出
  PAGExport exporter(configParam);
  bool success = exporter.exportFile();
  
  if (success) {
    QMessageBox::information(nullptr, "导出成功", "PAG文件已导出，您可以在PAG Viewer中预览。");
  } else {
    showSimpleError("导出失败，请检查日志");
  }
}

void WindowManager::showExportWindow() {
  init();
  
  std::ofstream log("/tmp/pag_export_debug.log", std::ios::app);
  log << "=== showExportWindow called ===" << std::endl;
  
  // 获取活动合成
  AEGP_ItemH activeItemHandle = GetActiveCompositionItem();
  if (!activeItemHandle) {
    log << "No active composition" << std::endl;
    log.close();
    showSimpleError("请先选择一个合成");
    return;
  }
  log << "Got active composition handle" << std::endl;
  
  // 调试：检查合成时长
  pag::Frame duration = GetItemDuration(activeItemHandle);
  A_FpLong frameRate = GetItemFrameRate(activeItemHandle);
  log << "Composition duration: " << duration << " frames" << std::endl;
  log << "Frame rate: " << frameRate << " fps" << std::endl;
  log << "Duration in seconds: " << (duration / frameRate) << std::endl;
  
  // 获取保存路径
  log << "Opening file dialog..." << std::endl;
  log.close();
  
  QString filePath = QFileDialog::getSaveFileName(
      nullptr,
      "导出PAG文件",
      "",
      "PAG Files (*.pag)");
  
  log.open("/tmp/pag_export_debug.log", std::ios::app);
  log << "File dialog closed, path: " << filePath.toUtf8().toStdString() << std::endl;
  
  if (filePath.isEmpty()) {
    log << "User cancelled" << std::endl;
    log.close();
    return;
  }
  
  // 导出配置
  log << "Creating export config..." << std::endl;
  PAGExportConfigParam configParam;
  configParam.activeItemHandle = activeItemHandle;
  configParam.outputPath = filePath.toUtf8().toStdString();
  configParam.exportAudio = true;
  configParam.hardwareEncode = false;
  configParam.exportActually = true;
  configParam.showAlertInfo = true;
  
  // 执行导出
  log << "Creating PAGExport object..." << std::endl;
  log.close();
  
  try {
    PAGExport exporter(configParam);
    
    log.open("/tmp/pag_export_debug.log", std::ios::app);
    log << "PAGExport created, calling exportFile()..." << std::endl;
    log.close();
    
    bool success = exporter.exportFile();
    
    log.open("/tmp/pag_export_debug.log", std::ios::app);
    log << "exportFile() returned: " << (success ? "true" : "false") << std::endl;
    log.close();
    
    if (success) {
      QMessageBox::information(nullptr, "导出成功", "PAG文件导出成功！");
    } else {
      showSimpleError("导出失败，请检查日志");
    }
  } catch (const std::exception& e) {
    log.open("/tmp/pag_export_debug.log", std::ios::app);
    log << "Exception caught: " << e.what() << std::endl;
    log.close();
    showSimpleError(QString("导出异常: ") + e.what());
  } catch (...) {
    log.open("/tmp/pag_export_debug.log", std::ios::app);
    log << "Unknown exception caught" << std::endl;
    log.close();
    showSimpleError("导出时发生未知错误");
  }
}

bool WindowManager::showWarnings(const std::vector<AlertInfo>& infos) {
  if (infos.empty()) {
    return true;
  }
  init();
  auto alertWindow = AlertWindow(app.get());
  return alertWindow.showWarnings(infos);
}

bool WindowManager::showErrors(const std::vector<AlertInfo>& infos) {
  if (infos.empty()) {
    return true;
  }
  init();
  auto alertWindow = AlertWindow(app.get());
  return alertWindow.showErrors(infos);
}

bool WindowManager::showSimpleError(const QString& errorMessage) {
  if (errorMessage.isEmpty()) {
    return false;
  }
  init();
  auto alertWindow = AlertWindow(app.get());
  return alertWindow.showErrors({}, errorMessage);
}

bool WindowManager::showPAGViewerInstallDialog(const std::string& pagFilePath) {
  PAGViewerInstallModel installModel;
  return installModel.showInstallDialog(pagFilePath);
}

void WindowManager::initializeQtEnvironment() {
  QApplication::setAttribute(Qt::AA_PluginApplication, true);
  QSurfaceFormat defaultFormat = QSurfaceFormat();
  defaultFormat.setRenderableType(QSurfaceFormat::RenderableType::OpenGL);
  defaultFormat.setVersion(3, 2);
  defaultFormat.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(defaultFormat);

#ifdef WIN32
  QFont defaultFonts("Microsoft Yahei");
  defaultFonts.setStyleHint(QFont::SansSerif);
  QApplication::setFont(defaultFonts);
#else
  QFont defaultFonts("Helvetica Neue,PingFang SC");
  QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);
  defaultFonts.setStyleHint(QFont::SansSerif);
  QApplication::setFont(defaultFonts);
#endif
  app = std::make_unique<QApplication>(argc, argv);
  app->setObjectName("PAG-Exporter");
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
  QQuickStyle::setStyle("Universal");
}

void WindowManager::init() {
  ConfigParam config;
  ReadConfigFile(&config);
  bool result = translator->load(":/translation/Chinese.qm");
  if (result) {
    if (config.language == Language::Chinese) {
      app->installTranslator(translator.get());
    } else {
      app->removeTranslator(translator.get());
    }
  }

  if (configWindow != nullptr && configWindow->isWaitToDestory()) {
    configWindow.reset();
  }

  AlertInfoManager::GetInstance().warningList.clear();
  AlertInfoManager::GetInstance().saveWarnings.clear();
}

}  // namespace exporter
