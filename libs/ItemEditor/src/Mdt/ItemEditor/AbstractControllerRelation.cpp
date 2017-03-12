/****************************************************************************
 **
 ** Copyright (C) 2011-2017 Philippe Steinmann.
 **
 ** This file is part of multiDiagTools library.
 **
 ** multiDiagTools is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** multiDiagTools is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with multiDiagTools.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
#include "AbstractControllerRelation.h"
#include "AbstractController.h"

namespace Mdt{ namespace ItemEditor{

AbstractControllerRelation::AbstractControllerRelation(QObject* parent)
 : QObject(parent)
{
}

AbstractControllerRelation::AbstractControllerRelation(AbstractController *parentController, QObject* parent)
 : QObject(parent),
   mParentController(parentController)
{
  Q_ASSERT(!mParentController.isNull());

//   Q_ASSERT(parentController != nullptr);

  connect(parentController, &AbstractController::controllerStateChanged, this, &AbstractControllerRelation::setParentControllerState);
}

void AbstractControllerRelation::setParentController(AbstractController* controller)
{
  if(controller == mParentController){
    return;
  }

  if(!mParentController.isNull()){
    parentControllerAboutToChangeEvent(mParentController);
  }

  mParentController = controller;
  Q_ASSERT(!mParentController.isNull());

  if(mParentController->modelForView() != nullptr){
    parentControllerModelChangedEvent(mParentController->modelForView());
  }
  disconnect(mParentControllerModelChangedConnection);
  mParentControllerModelChangedConnection = 
    connect( mParentController, &AbstractController::modelForViewChanged, this, &AbstractControllerRelation::parentControllerModelChangedEvent );

  parentControllerChangedEvent(mParentController);
}

void AbstractControllerRelation::setChildController(AbstractController* controller)
{
  if(controller == mChildController){
    return;
  }

  if(!mChildController.isNull()){
    childControllerAboutToChangeEvent(mChildController);
  }

  mChildController = controller;
  Q_ASSERT(!mChildController.isNull());

  if(mChildController->modelForView() != nullptr){
    childControllerModelChangedEvent(mChildController->modelForView());
  }
  disconnect(mChildControllerModelChangedConnection);
  mChildControllerModelChangedConnection = 
    connect( mChildController, &AbstractController::modelForViewChanged, this, &AbstractControllerRelation::childControllerModelChangedEvent );
  
  childControllerChangedEvent(mChildController);
}

AbstractController* AbstractControllerRelation::parentController() const
{
  return mParentController;
}

AbstractController* AbstractControllerRelation::childController() const
{
  return mChildController;
}

QAbstractItemModel* AbstractControllerRelation::parentControllerModel() const
{
  return mParentController->modelForView();
}

QAbstractItemModel* AbstractControllerRelation::childControllerModel() const
{
  if(mChildController.isNull()){
    return nullptr;
  }
  return mChildController->modelForView();
}

void AbstractControllerRelation::parentControllerAboutToChangeEvent(AbstractController*)
{
}

void AbstractControllerRelation::parentControllerChangedEvent(AbstractController*)
{
}

void AbstractControllerRelation::childControllerAboutToChangeEvent(AbstractController*)
{
}

void AbstractControllerRelation::childControllerChangedEvent(AbstractController*)
{
}

void AbstractControllerRelation::parentControllerModelChangedEvent(QAbstractItemModel*)
{
}

void AbstractControllerRelation::childControllerModelChangedEvent(QAbstractItemModel*)
{
}


void AbstractControllerRelation::registerAbstractChildController(AbstractController* controller)
{
  Q_ASSERT(controller != nullptr);

  disconnect(mChildControllerStateChagedConnection);
  mChildControllerStateChagedConnection = 
    connect(controller, &AbstractController::controllerStateChanged, this, &AbstractControllerRelation::setChildControllerState);
}

void AbstractControllerRelation::parentControllerStateChangedEvent(ControllerState)
{
}

void AbstractControllerRelation::childControllerStateChangedEvent(ControllerState)
{
}

void AbstractControllerRelation::setParentControllerState(ControllerState newState)
{

}

void AbstractControllerRelation::setChildControllerState(ControllerState newState)
{

}

}} // namespace Mdt{ namespace ItemEditor{
