/* TRANSLATOR BALL::VIEW::RmsdCalculator

		Necessary for lupdate.
*/

// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#include <BALL/VIEW/DIALOGS/rmsdCalculator.h>
#include <BALL/VIEW/KERNEL/mainControl.h>
#include <BALL/STRUCTURE/sdGenerator.h>
#include <BALL/FORMAT/SDFile.h>

#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QProgressBar>
#include <QtGui/QMessageBox>

#include <algorithm>
#include <BALL/VIEW/UIC/ui_rmsdCalculator.h>

namespace BALL
{
	namespace VIEW
	{
		RmsdCalculator::RmsdCalculator ( QWidget* parent, const char* name )
			: QDialog ( parent ),
			  ModularWidget ( name ),
			  action1_ ( 0 )
		{
#ifdef BALL_VIEW_DEBUG
			Log.error() << "new RmsdCalculator " << this << std::endl;
#endif
			ui_ = new Ui::RmsdCalculatorData;
			ui_->setupUi ( this );

			setObjectName ( name );

			// register the widget with the MainControl
			ModularWidget::registerWidget ( this );
		}

		RmsdCalculator::~RmsdCalculator()
		{
#ifdef BALL_VIEW_DEBUG
			Log.error() << "deleting RmsdCalculator " << this << std::endl;
#endif
			clearEntries();

			delete ui_;
		}

		void RmsdCalculator::finished()
		{
			hide();
		}

		void RmsdCalculator::clearEntries()
		{
			std::map<QTreeWidgetItem*, System*>::iterator it = sd_systems_.begin();

			for ( ; it != sd_systems_.end(); it++ )
			{
				delete it->second;
			}

			it = original_systems_.begin();

			for ( ; it != original_systems_.end(); it++ )
			{
				delete it->second;
			}

			sd_systems_.clear();
			original_systems_.clear();
			descriptions_.clear();
		}

		

		void RmsdCalculator::show()
		{
			QDialog::show();
			raise();
//			add_button_->setDefault ( false );
		}


		QString RmsdCalculator::buildHeaderTemplate_(InfoDisplayStyle style, const char* str) const
		{
			return "<b>" + tr(str) + "</b>" + ((style == LONG) ? "<br/>" : " ") + "%1<br/><br/>";
		}


		void RmsdCalculator::initializeWidget ( MainControl& )
		{
			
			action1_ = insertMenuEntry ( MainControl::TOOLS, tr ( "&RMSD Calculator" ), this,
			                             SLOT ( show() ), "Shortcut|Tools|RMSD_Calculator", QKeySequence(),
			                             tr ( "Dialog for RMSD calculations" ),
			                             UIOperationMode::MODE_ADVANCED );
		}

		void RmsdCalculator::checkMenu ( MainControl& main_control )
		{
			bool busy = main_control.compositesAreLocked();

			if ( action1_ )
				action1_->setEnabled ( !busy );
		}

	}
}
