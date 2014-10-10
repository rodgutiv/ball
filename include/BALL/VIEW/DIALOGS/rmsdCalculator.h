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
#include <QtGui>


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
		/** Dialog for calculating the RMSD between 'many to many' structures
		 */
		class BALL_VIEW_EXPORT RmsdCalculator
			: public QDialog,
			  public ModularWidget
		{
			Q_OBJECT

			public:
				BALL_EMBEDDABLE ( RmsdCalculator,ModularWidget )

				/// Default Constructor & Destructor.
				RmsdCalculator ( QWidget* parent = NULL, const char* name = "RmsdCalculator" );
				virtual ~RmsdCalculator();

				///
				virtual void initializeWidget ( MainControl& main_control );

				///
				virtual void checkMenu ( MainControl& main_control );

			public slots:
				void radioExclusiveDeselect1();
				void radioExclusiveDeselect2();
				
				void calc_rmsd();
				
				void addTo_ref();
				void removeFrom_ref();
				void addTo_probe();
				void removeFrom_probe();
				
				void addAllTo_probe();
				void addAllTo_ref();
				void removeAllFrom_probe();
				void removeAllFrom_ref();
				
				void checkRadioAlign();
				
				/// Show and raise dialog
				void show();

				// TODO: check if we need to reimplement clear, destroy, initializeWidet etc...
				///
				void finished();


			protected:
				// helper methods:
				void moveBetween2Views_(QTreeWidget *from, list<Composite *> *fromLst, QTreeWidget *to, list<Composite *> *toLst);
				bool checkProteinsOnly_(); // see if molecule lists contain only protein chains
				void radioExclusiveDeselect(QRadioButton *btn, int pos);
				
				// gui stuff
				Ui::RmsdCalculatorData* ui_;
				QAction* action1_;
				QPushButton* btn_calculate;
				
				// model stuff:
				// molecule lists
				list<Composite*> ref_molecules_;
				list<Composite*> probe_molecules_;
				
				// settings:
				enum CalculationSettings
				{
					/// Structure Types
					OPT_TYPE_PROTEIN,
					OPT_TYPE_MOLECULE,
					
					/// Atoms to use for RMSD calculation
					OPT_RMSD_ALL,
					OPT_RMSD_HEAVY,
					OPT_RMSD_CA,
					OPT_RMSD_BB,
			
					/// Superposeing options:
					OPT_CALC_ALIGN,
					OPT_CALC_INPLACE,
					
					/// Molecule modification:
					OPT_MODIFY_NO,
					OPT_MODIFY_YES,
					OPT_MODIFY_COPY
				};
				CalculationSettings opt_type_, opt_atoms_, opt_superpose_, opt_modify_;
				void getUserSettings();
				
		};

	}
}
#endif
