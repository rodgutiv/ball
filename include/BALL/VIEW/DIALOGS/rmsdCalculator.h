// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#ifndef BALL_VIEW_RMSDCALCULATOR_H
#define BALL_VIEW_RMSDCALCULATOR_H

#ifndef BALL_COMMON_GLOBAL_H
# include <BALL/COMMON/global.h>
#endif

#ifndef BALL_VIEW_KERNEL_MODULARWIDGET_H
# include <BALL/VIEW/KERNEL/modularWidget.h>
#endif

#ifndef BALL_STRUCTURE_SMILESPARSER_H
# include <BALL/STRUCTURE/smilesParser.h>
#endif

#include <QtGui/QDialog>

#include <map>

class QTreeWidgetItem;

namespace Ui
{
	class RmsdCalculatorData;
}

namespace BALL
{
	namespace VIEW
	{
		/** Dialog for querying PubChem and generating structures from a SMILES string
		 */
		class BALL_VIEW_EXPORT RmsdCalculator
			: public QDialog,
			  public ModularWidget
		{
			Q_OBJECT

			public:

				BALL_EMBEDDABLE ( RmsdCalculator,ModularWidget )

				/// Default Constructor.
				RmsdCalculator ( QWidget* parent = NULL, const char* name = "RmsdCalculator" );

				/// Destructor.
				virtual ~RmsdCalculator();

				///
				virtual void initializeWidget ( MainControl& main_control );

				///
				virtual void checkMenu ( MainControl& main_control );

			public slots:

				/// Show and raise dialog
				void show();

				///
				void clearEntries();

				///
				void finished();


			protected:
				Ui::RmsdCalculatorData* ui_;

				SmilesParser smiles_parser_;

				struct ParsedResult_
				{
					QString name;
					QString description;
					String smiles;
				};

				enum InfoDisplayStyle{
					LONG, SHORT
				};

				QString buildHeaderTemplate_(InfoDisplayStyle style, const char* str) const;

				std::map<QTreeWidgetItem*, System*> sd_systems_;
				std::map<QTreeWidgetItem*, System*> original_systems_;
				std::map<QTreeWidgetItem*, ParsedResult_> descriptions_;

				QAction* action1_;

				HashMap<int, QTreeWidgetItem*> esummary_request_ids_;
		};

	}
}
#endif
