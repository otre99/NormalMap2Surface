#ifndef SURFACEDATA_H
#define SURFACEDATA_H
#include "qsurfacedataproxy.h"
#include <QString>
#include <QVector>

class SurfaceData {
  QVector<QVector<float>> m_data{};
  int m_h{-1}, m_w{-1};
  float m_dx{1.0}, m_dy{1.0};

public:
  SurfaceData() = default;
  int width() const { return m_w; }
  int height() const { return m_h; }
  float dx() const { return m_dx; }
  float dy() const { return m_dy; }
  void fromFile(const QString &filePath, const float dx = 1.0f,
                const float dy = 1.0f);
  void populate(QSurfaceDataArray *dataArray, const int reduce_by = 1);
};

#endif // SURFACEDATA_H
