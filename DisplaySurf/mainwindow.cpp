#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
// #include <QSurfaceDataItem>
// #include <QSurfaceDataProxy>
// #include <QtGraphs/q3dsurface.h>
#include <QtDataVisualization/q3dsurface.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  m_surfaceGraph = new Q3DSurface();
  QSize screenSize = m_surfaceGraph->screen()->size();
  m_surfaceGraph->setMinimumSize(
      QSize(screenSize.width() / 4, (screenSize.height() / 2) / 1.75));
  m_surfaceGraph->setMaximumSize(screenSize);

  m_surfaceGraph->setOrthoProjection(false);

  // m_surfaceGraph->setOptimizationHints(Q3DSurface::OptimizationDefault);
  // m_surfaceGraph->setSizePolicy(QSizePolicy::Expanding,
  // QSizePolicy::Expanding); m_surfaceGraph->setFocusPolicy(Qt::StrongFocus);
  // m_surfaceGraph->setResizeMode(QQuickWidget::SizeRootObjectToView);

  // ThemePrimaryColors
  m_surfaceGraph->activeTheme()->setType(Q3DTheme::ThemeQt);
  m_surfaceGraph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
  m_surfaceGraph->scene()->activeCamera()->setCameraPreset(
      Q3DCamera::CameraPresetFront);
  m_surfaceGraph->scene()->activeLight()->setAutoPosition(true);
  m_surfaceGraph->activeTheme()->setBackgroundEnabled(false);
  m_surfaceGraph->axisX()->setTitle("X-Axis");
  m_surfaceGraph->axisY()->setTitle("Z-Axis");
  m_surfaceGraph->axisZ()->setTitle("Y-Axis");

  m_surfaceGraph->axisZ()->setTitleVisible(true);
  m_surfaceGraph->axisY()->setTitleVisible(true);
  m_surfaceGraph->axisX()->setTitleVisible(true);

  m_dataProxy = new QSurfaceDataProxy();
  m_series = new QSurface3DSeries(m_dataProxy);
  m_surfaceGraph->addSeries(m_series);
  auto w = QWidget::createWindowContainer(m_surfaceGraph);
  setCentralWidget(w);

  ui->doubleSpinBoxAspectRatio->setValue(m_surfaceGraph->aspectRatio());
  m_series->setDrawMode(QSurface3DSeries::DrawSurface);
  on_comboBoxGradients_currentTextChanged("gradGrayscale");
  ui->dockWidget->setMaximumWidth(screenSize.width() / 3);
  ui->dockWidget->setMinimumWidth(0);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_pushButton_clicked() {
  const QString file_name = QFileDialog::getOpenFileName(this, "Open x32 file");
  if (file_name.isEmpty())
    return;

  m_dataProxy->resetArray(nullptr);
  m_surfaceData.fromFile(file_name);
  m_surfaceGraph->axisX()->setRange(0, m_surfaceData.width());
  m_surfaceGraph->axisZ()->setRange(0, m_surfaceData.height());
  displayData(ui->comboBoxReduceSize->currentIndex() + 1);
  on_toolButtonExpandX_clicked();
  on_toolButtonExpandY_clicked();
  setWindowTitle("File:" + QFileInfo(file_name).baseName().toUpper());
}

void MainWindow::OpenFileFromCMD(const QString &file_name) {
  m_dataProxy->resetArray(nullptr);
  m_surfaceData.fromFile(file_name);
  m_surfaceGraph->axisX()->setRange(0, m_surfaceData.width());
  m_surfaceGraph->axisZ()->setRange(0, m_surfaceData.height());
  displayData(ui->comboBoxReduceSize->currentIndex() + 1);
  on_toolButtonExpandX_clicked();
  on_toolButtonExpandY_clicked();
  setWindowTitle("File:" + QFileInfo(file_name).baseName().toUpper());
}

void MainWindow::displayData(int reduce_dy) {
  auto *dataArray = new QSurfaceDataArray;
  m_surfaceData.populate(dataArray, reduce_dy);
  m_dataProxy->resetArray(dataArray);
}

void MainWindow::on_comboBoxReduceSize_currentIndexChanged(int index) {
  displayData(index + 1);
  // ui->horizontalSliderXmin->setValue(0);
  // ui->horizontalSliderYmin->setValue(0);
  // ui->horizontalSliderXmax->setValue(ui->horizontalSliderXmax->maximum());
  // ui->horizontalSliderYmax->setValue(ui->horizontalSliderYmax->maximum());
}

void MainWindow::on_horizontalSliderXmin_sliderMoved(int position) {
  const float maxVal = m_surfaceGraph->axisX()->max();
  const float dx =
      float(m_surfaceData.width()) / ui->horizontalSliderXmin->maximum();
  const float minVal = qMin(dx * position, maxVal - 10 * m_surfaceData.dx());
  m_surfaceGraph->axisX()->setRange(minVal, maxVal);
  ui->lbXMin->setNum(minVal);
}

void MainWindow::on_horizontalSliderXmax_sliderMoved(int position) {
  const float minVal = m_surfaceGraph->axisX()->min();
  const float dx =
      float(m_surfaceData.width()) / ui->horizontalSliderXmin->maximum();
  const float maxVal = qMax(dx * position, minVal + 10 * m_surfaceData.dx());
  m_surfaceGraph->axisX()->setRange(minVal, maxVal);
  ui->lbXMax->setNum(maxVal);
}

void MainWindow::on_horizontalSliderYmin_sliderMoved(int position) {
  const float maxVal = m_surfaceGraph->axisZ()->max();
  const float dy =
      float(m_surfaceData.height()) / ui->horizontalSliderYmin->maximum();
  const float minVal = qMin(dy * position, maxVal - 10 * m_surfaceData.dy());
  m_surfaceGraph->axisZ()->setRange(minVal, maxVal);
  ui->lbYMin->setNum(minVal);
}

void MainWindow::on_horizontalSliderYmax_sliderMoved(int position) {
  const float minVal = m_surfaceGraph->axisZ()->min();
  const float dy =
      float(m_surfaceData.height()) / ui->horizontalSliderYmin->maximum();
  const float maxVal = qMax(dy * position, minVal + 10 * m_surfaceData.dy());
  m_surfaceGraph->axisZ()->setRange(minVal, maxVal);
  ui->lbYMax->setNum(maxVal);
}

void MainWindow::on_checkBoxOrthoProjection_clicked(bool checked) {
  m_surfaceGraph->setOrthoProjection(checked);
}

void MainWindow::upateColorScale(const ColorMapper::GradientPreset &gp,
                                 bool inverted) {
  m_colorMapper.loadPreset(gp);
  auto &&cl = inverted ? m_colorMapper.inverted() : m_colorMapper;
  const QMap<double, QColor> cstop = cl.colorStops();
  QLinearGradient grd;
  auto &&keys = cstop.keys();
  for (auto pos : keys) {
    grd.setColorAt(pos, cstop[pos]);
  }
  m_series->setBaseGradient(grd);
  m_series->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

void MainWindow::on_comboBoxGradients_currentTextChanged(const QString &arg1) {

  if (arg1 == "gradGrayscale") {
    upateColorScale(ColorMapper::gpGrayscale,
                    ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradHot") {
    upateColorScale(ColorMapper::gpHot, ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradCold") {
    upateColorScale(ColorMapper::gpCold, ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradNight") {
    upateColorScale(ColorMapper::gpNight, ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradCandy") {
    upateColorScale(ColorMapper::gpCandy, ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradGeography") {
    upateColorScale(ColorMapper::gpGeography,
                    ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradIon") {
    upateColorScale(ColorMapper::gpIon, ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradThermal") {
    upateColorScale(ColorMapper::gpThermal,
                    ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradPolar") {
    upateColorScale(ColorMapper::gpPolar, ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradSpectrum") {
    upateColorScale(ColorMapper::gpSpectrum,
                    ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradJet") {
    upateColorScale(ColorMapper::gpJet, ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradHues") {
    upateColorScale(ColorMapper::gpHues, ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradYellow") {
    upateColorScale(ColorMapper::gpYellow, ui->checkBoxCMapInvert->isChecked());
  } else if (arg1 == "gradBlue") {
    upateColorScale(ColorMapper::gpBlue, ui->checkBoxCMapInvert->isChecked());
  }

  // if (arg1 == "Yellow" || arg1 == "Green" || arg1 == "Gray") {
  //   m_series->setColorStyle(Q3DTheme::ColorStyleUniform);
  //   if (arg1 == "Yellow")
  //     m_series->setBaseColor(QColor(Qt::yellow).lighter());
  //   if (arg1 == "Green")
  //     m_series->setBaseColor(QColor(Qt::green).lighter());
  //   if (arg1 == "Gray")
  //     m_series->setBaseColor(QColor(Qt::gray).lighter());
  // } else {
  //   m_series->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
  //   if (arg1 == "Gradient") {
  //     QLinearGradient gr;
  //     gr.setColorAt(0.f, Qt::black);
  //     gr.setColorAt(0.33f, Qt::blue);
  //     gr.setColorAt(0.67f, Qt::red);
  //     gr.setColorAt(1.f, Qt::yellow);
  //     m_series->setBaseGradient(gr);
  //   }
  // }
}

void MainWindow::on_pushButtonScreenShot_clicked() {
  const QString file_name =
      QFileDialog::getSaveFileName(this, "Save screenshot");
  if (file_name.isEmpty())
    return;

  const QImage image = m_surfaceGraph->renderToImage();
  image.save(file_name);
}

void MainWindow::on_doubleSpinBoxAspectRatio_valueChanged(double arg1) {
  m_surfaceGraph->setAspectRatio(arg1);
}

void MainWindow::on_checkBoxCMapInvert_clicked() {
  on_comboBoxGradients_currentTextChanged(ui->comboBoxGradients->currentText());
}

void MainWindow::on_toolButtonExpandX_clicked() {
  ui->horizontalSliderXmin->setValue(0);
  ui->horizontalSliderXmax->setValue(ui->horizontalSliderXmax->maximum());
  m_surfaceGraph->axisX()->setRange(0, m_surfaceData.width());
}

void MainWindow::on_toolButtonExpandY_clicked() {
  ui->horizontalSliderYmin->setValue(0);
  ui->horizontalSliderYmax->setValue(ui->horizontalSliderYmax->maximum());
  m_surfaceGraph->axisZ()->setRange(0, m_surfaceData.height());
}
