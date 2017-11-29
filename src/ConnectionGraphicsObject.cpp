#include "ConnectionGraphicsObject.hpp"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsBlurEffect>
#include <QtWidgets/QStyleOptionGraphicsItem>
#include <QtWidgets/QGraphicsView>

#include "FlowScene.hpp"

#include "Connection.hpp"
#include "ConnectionGeometry.hpp"
#include "ConnectionPainter.hpp"
#include "ConnectionState.hpp"
#include "ConnectionBlurEffect.hpp"

#include "NodeGraphicsObject.hpp"

#include "NodeConnectionInteraction.hpp"

#include "Node.hpp"

using QtNodes::ConnectionGraphicsObject;
using QtNodes::Connection;
using QtNodes::FlowScene;

ConnectionGraphicsObject::
ConnectionGraphicsObject(FlowScene &scene,
                         Connection &connection)
  : _scene(scene)
  , _connection(connection)
{
  _scene.addItem(this);
    _firstclick = false;
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);

  setAcceptHoverEvents(true);

  // addGraphicsEffect();
    _qwidget = nullptr;
  setZValue(-1.0);
}


ConnectionGraphicsObject::
~ConnectionGraphicsObject()
{
  _scene.removeItem(this);
}


QtNodes::Connection&
ConnectionGraphicsObject::
connection()
{
  return _connection;
}


QRectF
ConnectionGraphicsObject::
boundingRect() const
{

        return _connection.connectionGeometry().boundingRect();
}


QPainterPath
ConnectionGraphicsObject::
shape() const
{
#ifdef DEBUG_DRAWING

  //QPainterPath path;

  //path.addRect(boundingRect());
  //return path;

#else
  auto const &geom =
    _connection.connectionGeometry();

  return ConnectionPainter::getPainterStroke(geom);

#endif
}


void
ConnectionGraphicsObject::
setGeometryChanged()
{
  prepareGeometryChange();
}


void
ConnectionGraphicsObject::
move()
{

  auto moveEndPoint =
  [this] (PortType portType)
  {
    if (auto node = _connection.getNode(portType))
    {
      auto const &nodeGraphics = node->nodeGraphicsObject();

      auto const &nodeGeom = node->nodeGeometry();

      QPointF scenePos =
        nodeGeom.portScenePosition(_connection.getPortIndex(portType),
                                   portType,
                                   nodeGraphics.sceneTransform());

      {
        QTransform sceneTransform = this->sceneTransform();

        QPointF connectionPos = sceneTransform.inverted().map(scenePos);

        _connection.connectionGeometry().setEndPoint(portType,
                                                     connectionPos);

        _connection.getConnectionGraphicsObject().setGeometryChanged();
        _connection.getConnectionGraphicsObject().update();
      }
    }
  };

  //adjest connect
  std::vector<QPointF>& path = _connection.connectionGeometry().getPath();
  auto& geom = _connection.connectionGeometry();
  if(4==path.size())
  {
      path[0].ry() = geom.sink().ry();
      path[1].rx() = path[0].rx();
      path[3].ry() = geom.source().ry();
      path[2].rx() = path[3].rx();
  }
  moveEndPoint(PortType::In);
  moveEndPoint(PortType::Out);
}

void ConnectionGraphicsObject::lock(bool locked)
{
  setFlag(QGraphicsItem::ItemIsMovable, !locked);
  setFlag(QGraphicsItem::ItemIsFocusable, !locked);
  setFlag(QGraphicsItem::ItemIsSelectable, !locked);
}


void
ConnectionGraphicsObject::
paint(QPainter* painter,
      QStyleOptionGraphicsItem const* option,
      QWidget* _widget)
{
  painter->setClipRect(option->exposedRect);
  _qwidget =  _widget;
  ConnectionPainter::paint(painter,
                           _connection);
}


void
ConnectionGraphicsObject::
mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    _firstclick = true;
  QGraphicsItem::mousePressEvent(event);
  //event->ignore();
}

double length(QPointF _l,QPointF _r)
{
    QPointF length = _l-_r;
    return sqrt(pow(length.rx(),2)+pow(length.ry(),2));
}
double pointToline(const QPointF& _l1,const QPointF& _l2,const QPointF& _p)
{
//    return length(_l1,_p)+length(_l2,_p);
//    double a,b,c;
//    if(fabs(_l1.x()-_l2.x())<1E-6)
//    {
//        a=1;b=0;c=-_l1.x();
//    }else if(fabs(_l1.y()-_l2.y())<1E-6)
//    {
//        a=0;b=_l1.y();c=-_l1.y();
//    }else{
//        a = 1/(_l2.x()-_l1.x());
//        b = -1/(_l2.y()-_l1.y());
//        c = -_l1.x()/a-_l1.y()/b;
//    }
//    return fabs(a*_p.x()+b*_p.y()+c)/sqrt(a*a+b*b+0.1);

       if(fabs(_l1.x()-_l2.x())<5)
           return fabs(_l1.x()-_p.x());
       if(fabs(_l1.y()-_l2.y())<5)
           return fabs(_l1.y()-_p.y());
}
#include"QDebug"

void
ConnectionGraphicsObject::
mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  prepareGeometryChange();
  std::vector<QPointF>& path = _connection.connectionGeometry().getPath();
  auto& geom = _connection.connectionGeometry();
  int chose = 1;
  if(_firstclick)
  {
      if(path.size()==4)
      {
          double position[3];
          position[0] = pointToline(path[0],path[1],event->pos());
          position[1] = pointToline(path[1],path[2],event->pos());
          position[2] = pointToline(path[2],path[3],event->pos());
          double small= std::min(position[2],std::min(position[1],position[0]));
          for(int index_ = 0;index_<3;++index_)
          {
              if(fabs(small-position[index_])<1E-9)
              {
                  _choisPoint = index_;
                  break;
              }
          }
//          if(0==_choisPoint&&fabs(path[0].y()-path[1].y())<5)
//          {
//              _choisPoint=1;
//          }
//          if(2==_choisPoint&&fabs(path[3].y()-path[2].y())<5)
//          {
//              _choisPoint=1;
//          }
          switch (_choisPoint) {
          case 0:
                path[0]=QPointF(event->pos().x(),geom.sink().y());
                path[1].rx()=event->pos().x();
              break;
          case 1:
                path[1].ry() = event->pos().y();
                path[2].ry() = event->pos().y();
              break;
          case 2:
                path[2].rx() = event->pos().x();
                path[3].rx() = event->pos().x();
              break;
          default:
              break;
          }
      }
      if(path.size()==2)
      {
          path[0].rx()=event->pos().x();
          path[1].rx()=event->pos().x();
      }
      _firstclick = false;
  }else
  {
      if(path.size()==4)
      {
          double position[3];
          position[0] = pointToline(path[0],path[1],event->pos());
          position[1] = pointToline(path[1],path[2],event->pos());
          position[2] = pointToline(path[2],path[3],event->pos());
          double small= std::min(position[2],std::min(position[1],position[0]));
          for(int index_ = 0;index_<3;++index_)
          {
              if(fabs(small-position[index_])<1E-9)
              {
                  _choisPoint = index_;
                  break;
              }
          }
//          if(0==_choisPoint&&fabs(path[0].y()-path[1].y())<5)
//          {
//              _choisPoint=1;
//          }
//          if(2==_choisPoint&&fabs(path[3].y()-path[2].y())<5)
//          {
//              _choisPoint=1;
//          }
          switch (_choisPoint) {
          case 0:
                path[0]=QPointF(event->pos().x(),geom.sink().y());
                path[1].rx()=event->pos().x();
              break;
          case 1:
                path[1].ry() = event->pos().y();
                path[2].ry() = event->pos().y();
              break;
          case 2:
                path[2].rx() = event->pos().x();
                path[3].rx() = event->pos().x();
              break;
          default:
              break;
          }
      }
      if(path.size()==2)
      {
          path[0].rx()=event->pos().x();
          path[1].rx()=event->pos().x();
      }
  }
  qDebug()<<event->pos()<<event->scenePos();



  auto view = static_cast<QGraphicsView*>(event->widget());
  auto node = locateNodeAt(event->scenePos(),
                           _scene,
                           view->transform());

  auto &state = _connection.connectionState();

  state.interactWithNode(node);
  if (node)
  {
    node->reactToPossibleConnection(state.requiredPort(),
                                    _connection.dataType(),
                                    event->scenePos());
  }

  //-------------------

  QPointF offset = event->pos() - event->lastPos();

  auto requiredPort = _connection.requiredPort();

  if (requiredPort != PortType::None)
  {
    _connection.connectionGeometry().moveEndPoint(requiredPort, offset);
  }

  //-------------------

  update();

  event->accept();
}

#include<QDebug>
void
ConnectionGraphicsObject::
mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  ungrabMouse();
  event->accept();

  auto node = locateNodeAt(event->scenePos(), _scene,
                           _scene.views()[0]->transform());
  NodeConnectionInteraction interaction(*node, _connection, _scene);

  if (node && interaction.tryConnect())
  {
    node->resetReactionToConnection();
    _connection.connectionGeometry().initPath();
  }
  else if (_connection.connectionState().requiresPort())
  {

    _scene.deleteConnection(_connection);
  }
}


void
ConnectionGraphicsObject::
hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  _connection.connectionGeometry().setHovered(true);

  update();
  _scene.connectionHovered(connection(), event->screenPos());
  event->accept();
}


void
ConnectionGraphicsObject::
hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  _connection.connectionGeometry().setHovered(false);

  update();
  _scene.connectionHoverLeft(connection());
  event->accept();
}


void
ConnectionGraphicsObject::
addGraphicsEffect()
{
  auto effect = new QGraphicsBlurEffect;

  effect->setBlurRadius(5);
  setGraphicsEffect(effect);

  //auto effect = new QGraphicsDropShadowEffect;
  //auto effect = new ConnectionBlurEffect(this);
  //effect->setOffset(4, 4);
  //effect->setColor(QColor(Qt::gray).darker(800));
}
