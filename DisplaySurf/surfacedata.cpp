#include "surfacedata.h"
#include <QFile>

void SurfaceData::fromFile(const QString &filePath, const float dx,
                           const float dy) {
  QFile ifile(filePath);
  ifile.open(QFile::ReadOnly);
  float fsize[2];
  ifile.read((char *)fsize, 2 * sizeof(float));
  const int rows = fsize[0];
  const int cols = fsize[1];

  // qDebug() << rows << " " << cols;
  m_data.resize(rows);
  for (int i = 0; i < rows; ++i) {
    auto &&ll = m_data[i];
    ll.resize(cols);
    ifile.read((char *)ll.data(), cols * sizeof(float));
  }

  m_dx = dx;
  m_dy = dy;
  m_h = rows;
  m_w = cols;
}

void SurfaceData::populate(QSurfaceDataArray *dataArray, const int reduce_by) {
  dataArray->reserve(m_h / reduce_by);
  int index = 0;
  // dataArray->reserve(sampleCountZ);
  // for (int i = 0 ; i < sampleCountZ ; ++i) {
  //     auto *newRow = new QSurfaceDataRow;
  //     newRow->reserve(sampleCountX);
  //     // Keep values within range bounds, since just adding step can cause
  //     minor drift due
  //     // to the rounding errors.
  //     float z = qMin(sampleMax, (i * stepZ + sampleMin));
  //     for (int j = 0; j < sampleCountX; ++j) {
  //         float x = qMin(sampleMax, (j * stepX + sampleMin));
  //         float R = qSqrt(z * z + x * x) + 0.01f;
  //         float y = (qSin(R) / R + 0.24f) * 1.61f;
  //         newRow->append(QSurfaceDataItem({x, y, z}));
  //     }
  //     dataArray->append(newRow);
  // }
  for (int i = 0; i < m_h; i += reduce_by) {
    auto *newRow = new QSurfaceDataRow;
    newRow->reserve(m_w / reduce_by);
    const float z = i * m_dy;
    auto &&data = m_data[i];
    for (int j = 0; j < m_w; j += reduce_by) {
      const float x = j * m_dx;
      newRow->append(QSurfaceDataItem({x, data[j], z}));
    }
    dataArray->append(newRow);
  }
}
