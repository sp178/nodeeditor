#include "ConnectionPainter.hpp"

//#define DEBUG_DRAWING 1

#include "ConnectionGeometry.hpp"
#include "ConnectionState.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "Connection.hpp"

#include "NodeData.hpp"

#include "StyleCollection.hpp"

using QtNodes::ConnectionPainter;
using QtNodes::ConnectionGeometry;
using QtNodes::Connection;

ConnectionPainter::
ConnectionPainter()
{}

QPainterPath
ConnectionPainter::
cubicPath(ConnectionGeometry const& geom)
{
  QPointF const& source = geom.source();
  QPointF const& sink   = geom.sink();

  auto c1c2 = geom.pointsC1C2();

  // cubic spline
  QPainterPath cubic(source);

  cubic.cubicTo(c1c2.first, c1c2.second, sink);

  return cubic;
}


QPainterPath
ConnectionPainter::
getPainterStroke(ConnectionGeometry const& geom)
{
  auto cubic = cubicPath(geom);

  QPointF const& source = geom.source();
  QPainterPath result(source);

  unsigned segments = 20;

  const auto& path =  ((ConnectionGeometry*)&geom)->getPath();
  if(path.size()>0)
  {
      result.moveTo(geom.sink());
      for(auto& point:path)
      {
         result.lineTo(point);
      }
      result.lineTo(geom.source());
  }else
  {
     result.moveTo(geom.sink());
     result.lineTo(geom.source());
//      for (auto i = 0ul; i < segments; ++i)
//      {
//        double ratio = double(i + 1) / segments;
//        result.lineTo(cubic.pointAtPercent(ratio));
//      }
  }


  QPainterPathStroker stroker; stroker.setWidth(10.0);

  return stroker.createStroke(result);
}


#include <limits>

#include <QDebug>

void
ConnectionPainter::
paint(QPainter* painter,
      Connection const &connection)
{
    //sp178 found line draw
  auto const &connectionStyle =
    StyleCollection::connectionStyle();

  QColor normalColor   = connectionStyle.normalColor();
  QColor hoverColor    = connectionStyle.hoveredColor();
  QColor selectedColor = connectionStyle.selectedColor();

//  QColor normalColor   = QColor(120,0,0);
//  QColor hoverColor    = QColor(0,120,0);
//  QColor selectedColor = QColor(0,0,120);
  auto dataType = connection.dataType();

  if (connectionStyle.useDataDefinedColors())
  {

    normalColor   = connectionStyle.normalColor(dataType.id);
    hoverColor    = normalColor.lighter(200);
    selectedColor = normalColor.darker(200);
  }

  ConnectionGeometry const& geom =
    connection.connectionGeometry();

  ConnectionState const& state =
    connection.connectionState();

  double const lineWidth     = connectionStyle.lineWidth();
  double const pointDiameter = connectionStyle.pointDiameter();

#ifdef DEBUG_DRAWING

  {
    QPointF const& source = geom.source();
    QPointF const& sink   = geom.sink();

    auto points = geom.pointsC1C2();

    painter->setPen(Qt::red);
    painter->setBrush(Qt::red);

    painter->drawLine(QLineF(source, points.first));
    painter->drawLine(QLineF(points.first, points.second));
    painter->drawLine(QLineF(points.second, sink));
    painter->drawEllipse(points.first, 3, 3);
    painter->drawEllipse(points.second, 3, 3);

    painter->setBrush(Qt::NoBrush);

    painter->drawPath(cubicPath(geom));
  }

  {
    painter->setPen(Qt::yellow);

    painter->drawRect(geom.boundingRect());
  }
#endif

  bool const hovered = geom.hovered();

  auto const& graphicsObject =
    connection.getConnectionGraphicsObject();

  bool const selected = graphicsObject.isSelected();

  if (hovered || selected)
  {
    QPen p;

    p.setWidth(2 * lineWidth);
    p.setColor(selected ?
               connectionStyle.selectedHaloColor() :
               hoverColor);

    painter->setPen(p);
    painter->setBrush(Qt::NoBrush);

    // cubic spline

    auto& sppath =  ((ConnectionGeometry*)&geom)->getPath();
    if(sppath.size()!=0)
    {
        painter->drawLine(geom.sink(),sppath[0]);
        for(int index_=0;index_<sppath.size()-1;++index_)
            painter->drawLine(sppath[index_],sppath[index_+1]);
        painter->drawLine(sppath[sppath.size()-1],geom.source());
    }else{
            painter->drawLine(geom.sink(),geom.source());
    }

    //painter->drawPath(cubic);
  }

  // draw normal line
  {
    QPen p;

    p.setWidth(lineWidth);

    if (selected)
      p.setColor(selectedColor);
    else
      p.setColor(normalColor);

    if (state.requiresPort())
    {
      p.setWidth(connectionStyle.constructionLineWidth());
      p.setColor(connectionStyle.constructionColor());
      p.setStyle(Qt::DashLine);
    }

    painter->setPen(p);
    painter->setBrush(Qt::NoBrush);

    //sp178
    // cubic spline
//    painter->drawPath(cubic);

    //sp178
    const auto&sppath =  ((ConnectionGeometry*)&geom)->getPath();

    if(sppath.size()!=0)
    {
        painter->drawLine(geom.sink(),sppath[0]);
        for(int index_=0;index_<sppath.size()-1;++index_)
            painter->drawLine(sppath[index_],sppath[index_+1]);
        painter->drawLine(sppath[sppath.size()-1],geom.source());
    }else{
            painter->drawLine(geom.sink(),geom.source());
    }
  }

  QPointF const& source = geom.source();
  QPointF const& sink   = geom.sink();

  painter->setPen(connectionStyle.constructionColor());
  painter->setBrush(connectionStyle.constructionColor());
  double const pointRadius = pointDiameter / 2.0;
  painter->drawEllipse(source, pointRadius, pointRadius);
  painter->drawEllipse(sink, pointRadius, pointRadius);
}
