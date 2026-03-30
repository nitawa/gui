// Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#include "SUIT_Desktop.h"

#include "SUIT_ViewWindow.h"

#include <QtxLogoMgr.h>
#include <QtxActionMenuMgr.h>
#include <QtxActionToolMgr.h>
#include <QtxRibbonMgr.h>
#include <QMenuBar>

#include <QPointer>
#include <QCloseEvent>
#include <QApplication>

/*!\class SUIT_Desktop
 * Provide desktop management:\n
 * \li menu manager
 * \li tool manager
 * \li windows
 */

class SUIT_Desktop::ReparentEvent : public QEvent
{
public:
  ReparentEvent( Type t, QObject* obj ) : QEvent( t ), myObj( obj ) {};

  QObject* object() const { return myObj; }

private:
  QPointer<QObject> myObj;
};

/*!
  Constructor.
*/
SUIT_Desktop::SUIT_Desktop()
: QtxMainWindow()
{
  myMenuMgr   = new QtxActionMenuMgr( this );
  myToolMgr   = new QtxActionToolMgr( this );
  myLogoMgr   = new QtxLogoMgr( menuBar() );
  myRibbonMgr = new QtxRibbonMgr( this );

  // Enable ribbon redirection for standard menu and toolbar managers
  myMenuMgr->setRibbonMgr( myRibbonMgr );
  myToolMgr->setRibbonMgr( myRibbonMgr );

  // Set default modern theme
  myRibbonMgr->setTheme( Ribbon::Office2016BlueTheme );

  // Hide the standard menu bar to ensure all menus go to the ribbon
  if ( menuBar() )
    menuBar()->hide();

  // Hide all standard toolbars to ensure only the ribbon is visible
  QList<QToolBar*> toolbars = findChildren<QToolBar*>();
  for ( QToolBar* tb : toolbars )
  {
    if ( tb->parentWidget() == this )
      tb->hide();
  }
}

/*!
  Overriden to hide toolbar if ribbon is present.
*/
void SUIT_Desktop::addToolBar( QToolBar* tb )
{
  QMainWindow::addToolBar( tb );
  if ( myRibbonMgr && tb )
    tb->hide();
}

/*!
  Overriden to hide toolbar if ribbon is present.
*/
void SUIT_Desktop::addToolBar( Qt::ToolBarArea area, QToolBar* tb )
{
  QMainWindow::addToolBar( area, tb );
  if ( myRibbonMgr && tb )
    tb->hide();
}

/*!
  Destructor.
*/
SUIT_Desktop::~SUIT_Desktop()
{
}

/*!
  Override to hide toolbars as soon as they are added as children.
*/
void SUIT_Desktop::childEvent( QChildEvent* e )
{
  if ( e && e->type() == QEvent::ChildAdded )
  {
    if ( myRibbonMgr && qobject_cast<QToolBar*>( e->child() ) )
    {
      QToolBar* tb = static_cast<QToolBar*>( e->child() );
      tb->hide();
    }

    if ( e->child()->isWidgetType() )
    {
      // The following line is a workaround to avoid showing view window as a top-level window
      // before re-parenting it to workstack (issue #23467).
      // See SUIT_ViewWindow::setVisible() and SUIT_Desktop::customEvent().
      e->child()->setProperty("blockShow", true );
      QApplication::postEvent( this, new ReparentEvent( QEvent::Type( Reparent ), e->child() ) );
      return;
    }
  }

  QtxMainWindow::childEvent( e );
}

/*!
  Emit on event \a e.
*/
bool SUIT_Desktop::event( QEvent* e )
{
  if ( !e )
    return false;

  switch ( e->type() )
  {
  case QEvent::WindowActivate:
    emit activated();
    break;
  case QEvent::WindowDeactivate:
    emit deactivated();
    break;
 /*case QEvent::Move:
    emit moved();
    break;*/
  default:
    break;
  }

  return QtxMainWindow::event( e );
}

/*!
  Close event \a e.
*/
void SUIT_Desktop::closeEvent( QCloseEvent* e )
{
  emit closing( this, e );
  e->ignore();
}

void SUIT_Desktop::customEvent( QEvent* e )
{
  if ( (int)e->type() != Reparent )
    return;

  ReparentEvent* re = (ReparentEvent*)e;
  SUIT_ViewWindow* wid = ::qobject_cast<SUIT_ViewWindow*>( re->object() );
  if ( wid )
  {
    bool invis = wid->testAttribute( Qt::WA_WState_ExplicitShowHide ) &&
                 wid->testAttribute( Qt::WA_WState_Hidden );

    // The following line is a workaround to avoid showing view window as a top-level window
    // before re-parenting it to workstack (issue #23467).
    // See SUIT_ViewWindow::setVisible() and SUIT_Desktop::childEvent().
    wid->setProperty("blockShow", false);
    addWindow( wid );
    wid->setVisible( !invis );
  }
}

/*!
  Gets menu manager.
*/
QtxActionMenuMgr* SUIT_Desktop::menuMgr() const
{
  return myMenuMgr;
}

/*!
  Gets tool manager.
*/
QtxActionToolMgr* SUIT_Desktop::toolMgr() const
{
  return myToolMgr;
}

/*!
  Gets logo manager.
*/
QtxLogoMgr* SUIT_Desktop::logoMgr() const
{
  return myLogoMgr;
}

/*!
  Gets ribbon manager.
*/
QtxRibbonMgr* SUIT_Desktop::ribbonMgr() const
{
  return myRibbonMgr;
}

/*!
  Returns the count of the existed logos.
*/
int SUIT_Desktop::logoCount() const
{
  return 0;

  if ( !myLogoMgr )
    return 0;
  else
    return myLogoMgr->count();
}

/*!
  Adds new logo to the menu bar area
*/
void SUIT_Desktop::logoInsert( const QString& logoID, QMovie* logo, const int idx )
{
  if ( myLogoMgr )
    myLogoMgr->insert( logoID, logo, idx );
}

/*!
  Adds new logo to the menu bar area
*/
void SUIT_Desktop::logoInsert( const QString& logoID, const QPixmap& logo, const int idx )
{
  if ( myLogoMgr )
    myLogoMgr->insert( logoID, logo, idx );
}

/*!
  Removes a logo
*/
void SUIT_Desktop::logoRemove( const QString& logoID )
{
  if ( myLogoMgr )
    myLogoMgr->remove( logoID );
}

/*!
  Removes all logos 
*/
void SUIT_Desktop::logoClear()
{
  if ( myLogoMgr )
    myLogoMgr->clear();
}

/*!
  Emits activated signal
*/
void SUIT_Desktop::emitActivated()
{
  emit activated();
}

/*!
  Emits message signal
*/
void SUIT_Desktop::emitMessage( const QString& theMessage )
{
  emit message( theMessage );
}

/*!
  Activate window (default implementation just sets focus to the window.
*/
void SUIT_Desktop::setActiveWindow(SUIT_ViewWindow* wnd)
{
  if (wnd) wnd->setFocus();
}
