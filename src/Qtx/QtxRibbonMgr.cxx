// Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
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

// File : QtxRibbonMgr.cxx

// ribbon.h / ribbonbuttongroup.h come from the QtRibbonWidget library; their
// include directories are propagated automatically via the
// QtRibbonWidget::QtRibbonWidget CMake target.
#include "QtxRibbonMgr.h"   // already includes <ribbon.h> and <ribbonbuttongroup.h>

#include <algorithm>

#include <QAction>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

/*!
  \brief Constructor.

  Creates a Ribbon widget (QtRibbonWidget library) and docks it at the top of \a parent without
  a visible title bar.  The dock has \c NoDockWidgetFeatures so it cannot be
  closed or floated by the user.

  Signals from the Ribbon are forwarded as QtxRibbonMgr signals so callers do
  not need to reach through ribbon() directly.

  \param parent the main window that owns the ribbon dock
*/
QtxRibbonMgr::QtxRibbonMgr( QMainWindow* parent )
  : QObject( parent )
  , myMainWindow( parent )
  , myDock( nullptr )
  , myRibbon( nullptr )
{
  myRibbon = new Ribbon();

  myDock = new QDockWidget( parent );
  myDock->setObjectName( "ribbonDockWidget" );
  myDock->setTitleBarWidget( new QWidget() );  // hide title bar
  myDock->setWidget( myRibbon );
  myDock->setFeatures( QDockWidget::NoDockWidgetFeatures );
  parent->addDockWidget( Qt::TopDockWidgetArea, myDock );

  connect( myRibbon, &Ribbon::ribbonModeChanged,
           this,     &QtxRibbonMgr::ribbonModeChanged );
  connect( myRibbon, &Ribbon::ribbonStyleChanged,
           this,     &QtxRibbonMgr::ribbonStyleChanged );
  connect( myRibbon, &Ribbon::actionTriggered,
           this,     &QtxRibbonMgr::actionTriggered );
}

/*!
  \brief Destructor.
*/
QtxRibbonMgr::~QtxRibbonMgr()
{
}

/*!
  \brief Return the parent main window.
*/
QMainWindow* QtxRibbonMgr::mainWindow() const
{
  return myMainWindow;
}

/*!
  \brief Return the managed Ribbon widget (QtRibbonWidget library).
*/
Ribbon* QtxRibbonMgr::ribbon() const
{
  return myRibbon;
}

// ── Tab management ──────────────────────────────────────────────────────────

/*!
  \brief Add a tab to the ribbon by name.
*/
void QtxRibbonMgr::addTab( const QString& tabName )
{
  myRibbon->addTab( tabName );
}

/*!
  \brief Add a tab with an icon to the ribbon.
*/
void QtxRibbonMgr::addTab( const QIcon& tabIcon, const QString& tabName )
{
  myRibbon->addTab( tabIcon, tabName );
}

/*!
  \brief Remove a tab by name.
*/
void QtxRibbonMgr::removeTab( const QString& tabName )
{
  myRibbon->removeTab( tabName );
  myEntries.erase(
    std::remove_if( myEntries.begin(), myEntries.end(),
                    [&]( const RibbonEntry& e ){ return e.tabName == tabName; } ),
    myEntries.end() );
}

// ── Group management ────────────────────────────────────────────────────────

/*!
  \brief Explicitly create a group without adding any content yet.
  insert() and addMenu() create groups on demand, but this is useful when
  the group needs to exist before content is added (e.g. to set its option
  button immediately).
*/
void QtxRibbonMgr::addGroup( const QString& tabName, const QString& groupName )
{
  myRibbon->addGroup( tabName, groupName );
}

// ── Action insertion ────────────────────────────────────────────────────────

/*!
  \brief Add \a action to the ribbon at \a tabName / \a groupName.

  Uses the Ribbon's action-based API so the ribbon manages the QToolButton
  internally.  The button automatically mirrors the action's icon, text,
  tooltip, shortcut and enabled/checked state.

  The tab and group are created automatically if they do not exist.
*/
void QtxRibbonMgr::insert( QAction*                      action,
                             const QString&               tabName,
                             const QString&               groupName,
                             RibbonButtonGroup::ButtonSize size )
{
  if ( !action )
    return;

  myRibbon->addAction( tabName, groupName, action, size );

  RibbonEntry entry;
  entry.action    = action;
  entry.tabName   = tabName;
  entry.groupName = groupName;
  myEntries.append( entry );
}

/*!
  \brief Add \a action with an explicit popup mode.

  \param popupMode  e.g. QToolButton::MenuButtonPopup for a split button
*/
void QtxRibbonMgr::insert( QAction*                         action,
                             const QString&                  tabName,
                             const QString&                  groupName,
                             RibbonButtonGroup::ButtonSize    size,
                             QToolButton::ToolButtonPopupMode popupMode )
{
  if ( !action )
    return;

  myRibbon->addAction( tabName, groupName, action, size, popupMode );

  RibbonEntry entry;
  entry.action    = action;
  entry.tabName   = tabName;
  entry.groupName = groupName;
  myEntries.append( entry );
}

/*!
  \brief Remove all ribbon buttons associated with \a action.
*/
void QtxRibbonMgr::remove( QAction* action )
{
  QList<RibbonEntry> remaining;
  foreach ( const RibbonEntry& e, myEntries )
  {
    if ( e.action == action )
      myRibbon->removeAction( e.tabName, e.groupName, action );
    else
      remaining.append( e );
  }
  myEntries = remaining;
}

/*!
  \brief Remove the ribbon button for \a action from a specific tab/group only.
*/
void QtxRibbonMgr::remove( QAction*       action,
                             const QString& tabName,
                             const QString& groupName )
{
  if ( !action )
    return;

  myRibbon->removeAction( tabName, groupName, action );
  myEntries.erase(
    std::remove_if( myEntries.begin(), myEntries.end(),
                    [&]( const RibbonEntry& e )
                    {
                      return e.action == action
                          && e.tabName == tabName
                          && e.groupName == groupName;
                    } ),
    myEntries.end() );
}

// ── Menu insertion ───────────────────────────────────────────────────────────

/*!
  \brief Add a QMenu as a drop-down button at \a tabName / \a groupName.

  The menu's title and icon are used for the button face.
  \param popupMode  defaults to InstantPopup (click opens the menu directly)
*/
void QtxRibbonMgr::addMenu( const QString&                  tabName,
                              const QString&                  groupName,
                              QMenu*                          menu,
                              RibbonButtonGroup::ButtonSize    size,
                              QToolButton::ToolButtonPopupMode popupMode )
{
  if ( !menu )
    return;

  myRibbon->addMenu( tabName, groupName, menu, size, popupMode );
}

// ── Widget embedding ─────────────────────────────────────────────────────────

/*!
  \brief Embed an arbitrary widget into \a tabName / \a groupName.

  The ribbon does not take ownership of \a widget.
  Use setGroupExpanding() on the group if the widget needs to stretch.
*/
void QtxRibbonMgr::addWidget( const QString&               tabName,
                                const QString&               groupName,
                                QWidget*                     widget,
                                RibbonButtonGroup::ButtonSize size )
{
  if ( !widget )
    return;

  myRibbon->addWidget( tabName, groupName, widget, size );
}

/*!
  \brief Remove a previously embedded widget from \a tabName / \a groupName.
*/
void QtxRibbonMgr::removeWidget( const QString& tabName,
                                   const QString& groupName,
                                   QWidget*       widget )
{
  if ( !widget )
    return;

  myRibbon->removeWidget( tabName, groupName, widget );
}

// ── Separators ───────────────────────────────────────────────────────────────

/*!
  \brief Add a vertical separator line between buttons in \a tabName / \a groupName.
*/
void QtxRibbonMgr::addSeparator( const QString& tabName, const QString& groupName )
{
  myRibbon->addSeparator( tabName, groupName );
}

// ── Per-group properties ─────────────────────────────────────────────────────

/*!
  \brief Control whether \a groupName stretches to fill available tab space.
  \param expanding  true = group grows horizontally (useful for galleries)
*/
void QtxRibbonMgr::setGroupExpanding( const QString& tabName,
                                       const QString& groupName,
                                       bool           expanding )
{
  myRibbon->setGroupExpanding( tabName, groupName, expanding );
}

/*!
  \brief Set or remove the option button (▾) for \a groupName.
  The option button sits in the group title bar; triggering it typically
  opens a full dialog for that group's feature set.
  Pass nullptr to hide the option button.
*/
void QtxRibbonMgr::setGroupOptionAction( const QString& tabName,
                                          const QString& groupName,
                                          QAction*       action )
{
  myRibbon->setGroupOptionAction( tabName, groupName, action );
}

// ── Context tabs ─────────────────────────────────────────────────────────────

/*!
  \brief Register a context tab that is initially hidden.

  Context tabs appear after normal tabs only when relevant content is selected
  (e.g. "Table Tools" when a table cell is active).  Call showContextTab() /
  hideContextTab() in response to selection changes.

  \param tabName  name of the context tab (also used as the key for show/hide)
  \param color    optional color strip drawn above the tab label (e.g. Qt::blue)
*/
void QtxRibbonMgr::addContextTab( const QString& tabName, const QColor& color )
{
  myRibbon->addContextTab( tabName, color );
}

/*!
  \brief Make a previously registered context tab visible.
*/
void QtxRibbonMgr::showContextTab( const QString& tabName )
{
  myRibbon->showContextTab( tabName );
}

/*!
  \brief Hide a context tab (its content is preserved for future re-show).
*/
void QtxRibbonMgr::hideContextTab( const QString& tabName )
{
  myRibbon->hideContextTab( tabName );
}

// ── Ribbon mode & style ──────────────────────────────────────────────────────

/*!
  \brief Set the button layout style (ThreeRowStyle = Office, TwoRowStyle = compact).
*/
void QtxRibbonMgr::setRibbonStyle( Ribbon::RibbonStyle style )
{
  myRibbon->setRibbonStyle( style );
}

Ribbon::RibbonStyle QtxRibbonMgr::ribbonStyle() const
{
  return myRibbon->ribbonStyle();
}

/*!
  \brief Switch between NormalMode (full ribbon) and MinimizedMode (tab bar only).
  Double-clicking a tab also toggles the mode.
*/
void QtxRibbonMgr::setRibbonMode( Ribbon::RibbonMode mode )
{
  myRibbon->setRibbonMode( mode );
}

Ribbon::RibbonMode QtxRibbonMgr::ribbonMode() const
{
  return myRibbon->ribbonMode();
}

void QtxRibbonMgr::toggleRibbonMode()
{
  myRibbon->toggleRibbonMode();
}

/*!
  \brief Control how tab labels are distributed across the tab bar.
  TabAlignLeft = packed to the left; TabAlignExpanding = fill the bar width.
*/
void QtxRibbonMgr::setTabAlignment( Ribbon::TabAlignment align )
{
  myRibbon->setTabAlignment( align );
}

Ribbon::TabAlignment QtxRibbonMgr::tabAlignment() const
{
  return myRibbon->tabAlignment();
}

// ── Theme management ─────────────────────────────────────────────────────────

/*!
  \brief Set the ribbon theme (Default, Office2013, Office2016Blue, Dark).
*/
void QtxRibbonMgr::setTheme( Ribbon::RibbonTheme theme )
{
  myRibbon->setTheme( theme );
}

Ribbon::RibbonTheme QtxRibbonMgr::theme() const
{
  return myRibbon->theme();
}

// ── Application button ───────────────────────────────────────────────────────

/*!
  \brief Set or remove the application button placed in the top-left corner.

  Typically labelled with the application name or a file/backstage icon.
  The ribbon does not take ownership; pass nullptr to remove the button.
*/
void QtxRibbonMgr::setApplicationButton( QToolButton* button )
{
  myRibbon->setApplicationButton( button );
}

QToolButton* QtxRibbonMgr::applicationButton() const
{
  return myRibbon->applicationButton();
}

// ── Quick access bar ─────────────────────────────────────────────────────────

/*!
  \brief Return the quick-access toolbar.

  The toolbar is managed by the Ribbon; the caller is responsible for placing
  it (e.g. alongside the application button or in a custom title bar widget).
*/
QToolBar* QtxRibbonMgr::quickAccessBar() const
{
  return myRibbon->quickAccessBar();
}

/*!
  \brief Add \a action to the quick-access toolbar.
*/
void QtxRibbonMgr::addQuickAccessAction( QAction* action )
{
  if ( action )
    myRibbon->addQuickAccessAction( action );
}

/*!
  \brief Remove \a action from the quick-access toolbar.
*/
void QtxRibbonMgr::removeQuickAccessAction( QAction* action )
{
  if ( action )
    myRibbon->removeQuickAccessAction( action );
}

// ── Global appearance ────────────────────────────────────────────────────────

/*!
  \brief Set the fixed height (in pixels) of all group title label rows.
*/
void QtxRibbonMgr::setPanelTitleHeight( int height )
{
  myRibbon->setPanelTitleHeight( height );
}

/*!
  \brief Show or hide the title bar of every button group.
*/
void QtxRibbonMgr::setShowPanelTitle( bool visible )
{
  myRibbon->setShowPanelTitle( visible );
}

/*!
  \brief Set the pixel spacing between adjacent button groups.
*/
void QtxRibbonMgr::setPanelSpacing( int spacing )
{
  myRibbon->setPanelSpacing( spacing );
}

/*!
  \brief Set the icon size used for LargeButton items (default 32×32).
*/
void QtxRibbonMgr::setLargeIconSize( const QSize& size )
{
  myRibbon->setLargeIconSize( size );
}

/*!
  \brief Set the icon size used for SmallButton and MediumButton items (default 16×16).
*/
void QtxRibbonMgr::setSmallIconSize( const QSize& size )
{
  myRibbon->setSmallIconSize( size );
}
