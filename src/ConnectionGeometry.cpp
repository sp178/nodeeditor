#include "ConnectionGeometry.hpp"

#include <cmath>

#include "StyleCollection.hpp"

using QtNodes::ConnectionGeometry;
using QtNodes::PortType;

ConnectionGeometry::
ConnectionGeometry()
  : _in(0, 0)
  , _out(0, 0)
  //, _animationPhase(0)
  , _lineWidth(3.0)
  , _hovered(false)
{ }

QPointF const&
ConnectionGeometry::
getEndPoint(PortType portType)
{
  Q_ASSERT(portType != PortType::None);
  return (portType == PortType::Out ?
          _out :
          _in);
}


void
ConnectionGeometry::
setEndPoint(PortType portType, QPointF const& point)
{
  switch (portType)
  {
    case PortType::Out:
      _out = point;
      break;

    case PortType::In:
      _in = point;
      break;

    default:
      break;
  }
  initPath();
}


void
ConnectionGeometry::
moveEndPoint(PortType portType, QPointF const &offset)
{
  switch (portType)
  {
    case PortType::Out:
      _out += offset;
      break;

    case PortType::In:
      _in += offset;
      break;

    default:
      break;
  }
}

void
ConnectionGeometry::
initPath()
{
    if(_in.x()>_out.x())
    {
     if(_paintpath.size()!=2)
     {
         if(_in!=QPointF(0,0)||_out!=QPointF(0,0))
         {
         _paintpath.resize(2);
         _paintpath[0] = _in;
         _paintpath[1] = _out;
         _paintpath[1].rx()= 0.5*(_in.x()+_out.x());
         _paintpath[0].rx()= 0.5*(_in.x()+_out.x());
         }
     }else{
         if(_in!=QPointF(0,0)||_out!=QPointF(0,0))
         {
             _paintpath[0].ry()= _in.y();
             _paintpath[1].ry()= _out.y();
         }else{
             _paintpath[0] = _in;
             _paintpath[1] = _out;
             _paintpath[1].rx()= 0.5*(_in.x()+_out.x());
             _paintpath[0].rx()= 0.5*(_in.x()+_out.x());
         }

     }

    }else
    {
        if(_paintpath.size()!=4)
        {
            if(_in!=QPointF(0,0)||_out!=QPointF(0,0))
            {
                _paintpath.resize(4);
                _paintpath[0] = _in;
                _paintpath[0].rx()-=10;
                _paintpath[3] = _out;
                _paintpath[3].rx()+=10;

                _paintpath[1] = _paintpath[0];
                _paintpath[2] = _paintpath[3];
                _paintpath[1].ry() =0.5*(_in.ry()+_out.ry());
                _paintpath[2].ry()=0.5*(_in.ry()+_out.ry());
            }

        }else{
            if(_in!=QPointF(0,0)&&_out!=QPointF(0,0))
            {

            }else{
                _paintpath[0].ry()= _in.y();
                _paintpath[3].rx()= _out.y();
            }

        }
    }
}
QRectF
ConnectionGeometry::
boundingRect()
{
  auto points = pointsC1C2();


  QRectF basicRect = QRectF(_out, _in).normalized();

  QRectF c1c2Rect = QRectF(points.first, points.second).normalized();

  auto const &connectionStyle =
    StyleCollection::connectionStyle();

  float const diam = connectionStyle.pointDiameter();

  QRectF commonRect = basicRect.united(c1c2Rect);

  QPointF const cornerOffset(diam, diam);

  commonRect.setTopLeft(commonRect.topLeft() - cornerOffset);
  commonRect.setBottomRight(commonRect.bottomRight() + 2 * cornerOffset);

  if(_paintpath.size()==0)
  {
      commonRect.setTopLeft(QPointF(-1024,-1027));
      commonRect.setBottomRight(QPointF(1024,1024));

  }else
  {
      double top = _in.y(),left=_in.x(),
              bottom=_in.y(),right=_in.x();
      for(auto point:_paintpath)
      {
        if(top>(point.y()-cornerOffset.y()))
            top = point.y()-cornerOffset.y();
        if(left>(point.x()-cornerOffset.x()))
            left = point.x()-cornerOffset.x();
        if(bottom<=(point.y()+2*cornerOffset.y()))
            bottom = point.y()+2*cornerOffset.y();
        if(right<=(point.x()+2*cornerOffset.x()))
            right = point.x()+2*cornerOffset.x();
      }
      if(top>(_out.y()-cornerOffset.y()))
          top = _out.y()-cornerOffset.y();
      if(left>(_out.x()-cornerOffset.x()))
          left = _out.x()-cornerOffset.x();
      if(bottom<=(_out.y()+2*cornerOffset.y()))
          bottom = _out.y()+2*cornerOffset.y();
      if(right<=(_out.x()+2*cornerOffset.x()))
          right = _out.x()+2*cornerOffset.x();
      commonRect.setTopLeft(QPointF(left,top));
      commonRect.setBottomRight(QPointF(right,bottom));
  }
      return commonRect;
}
QPointF operator /(QPointF _l,QPointF _r)
{
    if(fabs(_r.rx())<1E-4) _r.rx()=1E-4;
    if(fabs(_r.ry())<1E-4) _r.ry()=1E-4;
    QPointF tmp(_l.rx()/_r.rx(),_l.ry()/_r.ry());
    return tmp;
}
int
ConnectionGeometry::addPath(QPointF _path)
{

    if(_paintpath.size()<_maxpath)
        _paintpath.push_back(_path);
    return _paintpath.size();
}

std::vector<QPointF>&
ConnectionGeometry::getPath()
{
    return _paintpath;
}
std::pair<QPointF, QPointF>
ConnectionGeometry::
pointsC1C2() const
{
  double xDistance = _in.x() - _out.x();
  //double yDistance = _in.y() - _out.y() - 100;

  double defaultOffset = 200;

  double minimum = qMin(defaultOffset, std::abs(xDistance));

  double verticalOffset = 0;

  double ratio1 = 0.5;

  if (xDistance <= 0)
  {
    verticalOffset = -minimum;

    ratio1 = 1.0;
  }

  //double verticalOffset2 = verticalOffset;
  //if (xDistance <= 0)
  //verticalOffset2 = qMin(defaultOffset, std::abs(yDistance));
  //auto sign = [](double d) { return d > 0.0 ? +1.0 : -1.0; };
  //verticalOffset2 = 0.0;

  QPointF c1(_out.x() + minimum * ratio1,
             _out.y() + verticalOffset);

  QPointF c2(_in.x() - minimum * ratio1,
             _in.y() + verticalOffset);


  return std::make_pair(c1, c2);
}
