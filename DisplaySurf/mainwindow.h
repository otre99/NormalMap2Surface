#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "colormapping.h"
#include "surfacedata.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Q3DSurface;
class QSurface3DSeries;
class QSurfaceDataProxy;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void OpenFileFromCMD(const QString &file_name);

private slots:
  void on_pushButton_clicked();
  void on_comboBoxReduceSize_currentIndexChanged(int index);
  void on_horizontalSliderXmin_sliderMoved(int position);
  void on_horizontalSliderXmax_sliderMoved(int position);
  void on_horizontalSliderYmin_sliderMoved(int position);
  void on_horizontalSliderYmax_sliderMoved(int position);
  void on_checkBoxOrthoProjection_clicked(bool checked);
  void on_comboBoxGradients_currentTextChanged(const QString &arg1);
  void on_pushButtonScreenShot_clicked();
  void on_doubleSpinBoxAspectRatio_valueChanged(double arg1);
  void on_checkBoxCMapInvert_clicked();
  void on_toolButtonExpandX_clicked();
  void on_toolButtonExpandY_clicked();

  private:
  void displayData(int reduce_dy = 1);
  void upateColorScale(const ColorMapper::GradientPreset &gp, bool inverted);

  Ui::MainWindow *ui;
  Q3DSurface *m_surfaceGraph{};
  QSurface3DSeries *m_series{};
  QSurfaceDataProxy *m_dataProxy{};
  SurfaceData m_surfaceData;
  ColorMapper m_colorMapper;
};
#endif // MAINWINDOW_H
