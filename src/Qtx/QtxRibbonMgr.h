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

// File : QtxRibbonMgr.h
// Ribbon manager: owns a Ribbon (QtRibbonWidget library) dock widget and
// maps QActions / QMenus / QWidgets to ribbon tabs / groups.

#ifndef QTXRIBBONMGR_H
#define QTXRIBBONMGR_H

#include "Qtx.h"

#include <QColor>
#include <QObject>
#include <QIcon>
#include <QList>
#include <QSize>
#include <QString>
#include <QToolButton>

// Ribbon and RibbonButtonGroup come from the QtRibbonWidget library.
// Their include dirs are propagated transitively via QtRibbonWidget::QtRibbonWidget.
#include <ribbon.h>
#include <ribbonbuttongroup.h>

class QAction;
class QDockWidget;
class QMainWindow;
class QMenu;
class QToolBar;
class QWidget;

/*!
  \class QtxRibbonMgr
  \brief Manager for the ribbon bar of the SALOME desktop.

  QtxRibbonMgr creates a Ribbon widget (from the QtRibbonWidget library),
  docks it at the top of the supplied QMainWindow (without a title bar),
  and provides a complete API to populate it with tabs, groups, QActions,
  QMenus, and arbitrary widgets.

  Typical use inside an application's createActions():
  \code
    QtxRibbonMgr* rm = desktop()->ribbonMgr();

    // Application button (top-left corner)
    QToolButton* appBtn = new QToolButton;
    appBtn->setText(tr("File"));
    rm->setApplicationButton(appBtn);

    // Quick access bar
    rm->addQuickAccessAction(action(FileSaveId));

    // Normal tabs
    rm->addTab(QIcon(":/icons/home.svg"), tr("Home"));
    rm->insert(action(FileNewId),  tr("Home"), tr("File"));
    rm->insert(action(FileOpenId), tr("Home"), tr("File"));
    rm->insert(action(FileSaveId), tr("Home"), tr("File"));
    rm->addSeparator(tr("Home"), tr("File"));
    rm->insert(action(EditCutId),  tr("Home"), tr("Edit"),
               RibbonButtonGroup::SmallButton);

    // Context tab (shown only when an object is selected)
    rm->addContextTab(tr("Format"), QColor(Qt::blue));
  \endcode
*/
class QTX_EXPORT QtxRibbonMgr : public QObject
{
  Q_OBJECT

  struct RibbonEntry
  {
    QAction* action;
    QString  tabName;
    QString  groupName;
  };

public:
  explicit QtxRibbonMgr( QMainWindow* parent );
  virtual ~QtxRibbonMgr();

  QMainWindow* mainWindow() const;
  Ribbon*      ribbon()     const;

  // ── Tab management ────────────────────────────────────────────────────

  void addTab( const QString& tabName );
  void addTab( const QIcon& tabIcon, const QString& tabName );
  void removeTab( const QString& tabName );

  // ── Group management ──────────────────────────────────────────────────

  void addGroup( const QString& tabName, const QString& groupName );

  // ── Action insertion ──────────────────────────────────────────────────

  /*!
    \brief Insert \a action into \a tabName / \a groupName as a button.
    Creates the tab and group automatically if they do not already exist.
    \param action    the QAction to insert
    \param tabName   ribbon tab name
    \param groupName ribbon group name within the tab
    \param size      button size (LargeButton, MediumButton or SmallButton)
  */
  void insert( QAction*                      action,
               const QString&               tabName,
               const QString&               groupName,
               RibbonButtonGroup::ButtonSize size = RibbonButtonGroup::LargeButton );

  /*!
    \brief Insert \a action with an explicit popup mode.
    \param popupMode  controls how the drop-down arrow is shown/triggered
  */
  void insert( QAction*                         action,
               const QString&                  tabName,
               const QString&                  groupName,
               RibbonButtonGroup::ButtonSize    size,
               QToolButton::ToolButtonPopupMode popupMode );

  /*!
    \brief Remove all ribbon buttons associated with \a action.
  */
  void remove( QAction* action );

  /*!
    \brief Remove the ribbon button for \a action from a specific tab/group.
  */
  void remove( QAction*       action,
               const QString& tabName,
               const QString& groupName );

  // ── Menu insertion ────────────────────────────────────────────────────

  void addMenu( const QString&                  tabName,
                const QString&                  groupName,
                QMenu*                          menu,
                RibbonButtonGroup::ButtonSize    size      = RibbonButtonGroup::LargeButton,
                QToolButton::ToolButtonPopupMode popupMode = QToolButton::InstantPopup );

  // ── Widget embedding ──────────────────────────────────────────────────

  void addWidget( const QString&               tabName,
                  const QString&               groupName,
                  QWidget*                     widget,
                  RibbonButtonGroup::ButtonSize size = RibbonButtonGroup::LargeButton );

  void removeWidget( const QString& tabName,
                     const QString& groupName,
                     QWidget*       widget );

  // ── Separators ────────────────────────────────────────────────────────

  void addSeparator( const QString& tabName, const QString& groupName );

  // ── Per-group properties ──────────────────────────────────────────────

  /*!
    \brief Make \a groupName expand horizontally to fill available space.
    Useful for galleries, combo-boxes and other wide controls.
  */
  void setGroupExpanding( const QString& tabName,
                          const QString& groupName,
                          bool           expanding = true );

  /*!
    \brief Set (or remove) the option button (▾) action for a group.
    Pass nullptr to hide the option button.
  */
  void setGroupOptionAction( const QString& tabName,
                             const QString& groupName,
                             QAction*       action );

  // ── Context tabs ──────────────────────────────────────────────────────

  /*!
    \brief Register a context tab (hidden by default).
    \param tabName  name of the context tab
    \param color    optional color strip drawn above the tab header
  */
  void addContextTab( const QString& tabName,
                      const QColor&  color = QColor() );

  /*! \brief Make a context tab visible (appended after normal tabs). */
  void showContextTab( const QString& tabName );

  /*! \brief Hide a context tab (its content is preserved). */
  void hideContextTab( const QString& tabName );

  // ── Ribbon mode & style ───────────────────────────────────────────────

  void setRibbonStyle( Ribbon::RibbonStyle style );
  Ribbon::RibbonStyle ribbonStyle() const;

  /*! \brief Switch between NormalMode and MinimizedMode. */
  void setRibbonMode( Ribbon::RibbonMode mode );
  Ribbon::RibbonMode ribbonMode() const;

  /*! \brief Toggle between NormalMode and MinimizedMode. */
  void toggleRibbonMode();

  void setTabAlignment( Ribbon::TabAlignment align );
  Ribbon::TabAlignment tabAlignment() const;

  // ── Theme management ──────────────────────────────────────────────────

  void setTheme( Ribbon::RibbonTheme theme );
  Ribbon::RibbonTheme theme() const;

  // ── Application button ────────────────────────────────────────────────

  /*!
    \brief Set (or remove) the application button in the top-left corner.
    Pass nullptr to remove it. The ribbon does not take ownership.
  */
  void         setApplicationButton( QToolButton* button );
  QToolButton* applicationButton() const;

  // ── Quick access bar ──────────────────────────────────────────────────

  /*!
    \brief Return the managed quick-access toolbar.
    The caller is responsible for placing it (e.g. in a title bar).
  */
  QToolBar* quickAccessBar() const;

  void addQuickAccessAction( QAction* action );
  void removeQuickAccessAction( QAction* action );

  // ── Global appearance ─────────────────────────────────────────────────

  void setPanelTitleHeight( int height );
  void setShowPanelTitle( bool visible );
  void setPanelSpacing( int spacing );
  void setLargeIconSize( const QSize& size );
  void setSmallIconSize( const QSize& size );

signals:
  void ribbonModeChanged( Ribbon::RibbonMode mode );
  void ribbonStyleChanged( Ribbon::RibbonStyle style );
  void actionTriggered( QAction* action );

private:
  QMainWindow*       myMainWindow;
  QDockWidget*       myDock;
  Ribbon*            myRibbon;     // from QtRibbonWidget library
  QList<RibbonEntry> myEntries;
};

#endif // QTXRIBBONMGR_H
