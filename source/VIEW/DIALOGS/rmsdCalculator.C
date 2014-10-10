/* TRANSLATOR BALL::VIEW::RmsdCalculator

		Necessary for lupdate.
*/

// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#include <BALL/VIEW/DIALOGS/rmsdCalculator.h>
#include <BALL/VIEW/UIC/ui_rmsdCalculator.h>
#include <BALL/VIEW/KERNEL/mainControl.h>
#include <BALL/VIEW/KERNEL/compositeManager.h>
#include <BALL/STRUCTURE/structureMapper.h>
#include <BALL/STRUCTURE/sdGenerator.h>
#include <BALL/FORMAT/SDFile.h>

#include <BALL/VIEW/WIDGETS/structureViewWidget.h>

#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QProgressBar>
#include <QtGui/QMessageBox>

#include <algorithm>
#include <map>


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
			
			// setup the 3 windows:
			// ui_->widget->setFocusPolicy(Qt::NoFocus);
			setObjectName ( name );

			// register the widget with the MainControl
			ModularWidget::registerWidget ( this );
			
			// setup the button box:
			btn_calculate = ui_->buttonBox->addButton ( tr ( " Calculate " ), QDialogButtonBox::ActionRole );
			connect( btn_calculate, SIGNAL(clicked()), this, SLOT(calc_rmsd()) );
			QPushButton* btn_cancel = ui_->buttonBox->addButton ( tr ( " Cancel " ), QDialogButtonBox::RejectRole );
			connect( btn_cancel, SIGNAL(clicked()), this, SLOT(finished()) );
			
			// Setup the add/remove arrow-buttons:
			connect( ui_->btn_toRef, SIGNAL( clicked()), this, SLOT(addTo_ref()) );
			connect( ui_->btn_fromRef, SIGNAL(clicked()), this, SLOT(removeFrom_ref()) );
			connect( ui_->btn_toProbes, SIGNAL(clicked()), this, SLOT(addTo_probe()) );
			connect( ui_->btn_fromProbes, SIGNAL(clicked()), this, SLOT(removeFrom_probe()) );
			
			connect( ui_->btn_allToRef, SIGNAL(clicked()), this, SLOT(addAllTo_ref()) );
			connect( ui_->btn_allFromRef, SIGNAL(clicked()), this, SLOT(removeAllFrom_ref()) );
			connect( ui_->btn_allToProbes, SIGNAL(clicked()), this, SLOT(addAllTo_probe()) );
			connect( ui_->btn_allFromProbes, SIGNAL(clicked()), this, SLOT(removeAllFrom_probe()) );
			
			// Setup Radio-buttons:
			connect( ui_->rad_align_yes, SIGNAL(toggled(bool)), this, SLOT(checkRadioAlign()) );
			connect( ui_->rad_align_no, SIGNAL(toggled(bool)), this, SLOT(checkRadioAlign()) );
			// exclusive and deselectable radio-buttons: (a hack to give the senders adress as slot arg)
			connect( ui_->rad_modi_copy, SIGNAL(pressed()), this, SLOT(radioExclusiveDeselect1()));
			connect( ui_->rad_modi_yes, SIGNAL(pressed()), this, SLOT(radioExclusiveDeselect2()));
			
		}
		
		
		/// Destructor:
		RmsdCalculator::~RmsdCalculator()
		{
#ifdef BALL_VIEW_DEBUG
			Log.error() << "deleting RmsdCalculator " << this << std::endl;
#endif
			// NOT YET SURE IF I NEED TO DELETE SOMETHING HERE

			delete ui_;
		}
		
		
		void RmsdCalculator::show()
		{
			btn_calculate->setEnabled(false);
			ui_->vbox_opt_modify->setEnabled(false);
			HashSet<Composite*> composites = getMainControl()->getCompositeManager().getComposites();
			ui_->treeWidget_workspace->clear();
			ui_->treeWidget_workspace->generateTree(composites);
			ui_->treeWidget_workspace->update();
			QDialog::show();
			raise();
		}
		
		void RmsdCalculator::finished()
		{
			removeAllFrom_probe(); // reset the selections
			removeAllFrom_ref();
			hide();
		}
		
		void RmsdCalculator::initializeWidget ( MainControl& )
		{
			action1_ = insertMenuEntry ( MainControl::TOOLS, tr ( "&Superpose && RMSD" ), this,
			                             SLOT ( show() ), "Shortcut|Tools|Superpose_RMSD", QKeySequence(),
			                             tr ( "Dialog for Superimposing and RMSD calculations" ),
			                             UIOperationMode::MODE_ADVANCED );
		}

		void RmsdCalculator::checkMenu ( MainControl& main_control )
		{// disable tool menu-entry when structure/composite window is locked:
			bool busy = main_control.compositesAreLocked();

			if ( action1_ )
				action1_->setEnabled ( !busy );
		}

		/*
		 * Returns true only if all selected structures are proteinogenic structures
		 * in references and probes.
		 */
		bool RmsdCalculator::checkProteinsOnly_()
		{
			if ( ref_molecules_.empty() || probe_molecules_.empty() )
			{
				return false;
			}
				
			bool res = true;
			Composite* tmp;
			list<Composite*>::iterator it;
			
			// check probe molecules:
			it = ref_molecules_.begin();
			for (; it != ref_molecules_.end(); it++)
			{
				tmp = *it;
				if ( !(tmp->isChain() || tmp->isResidue() || tmp->isProtein()) )
				{
					res = false;
					break;
				}
			}
			
			// early exit? (if first test was already negative)
			if (!res){ return res;}
			
			// check reference molecules:
			it = probe_molecules_.begin();
			for (; it != probe_molecules_.end(); it++)
			{
				tmp = *it;
				if ( !(tmp->isChain() || tmp->isResidue() || tmp->isProtein()) )
				{
					res = false;
					break;
				}
			}
			return res;
		}
		
		/*
		 * Makes it possible to deselect radiobuttons that are
		 * also mutual exclusive
		 */
		void RmsdCalculator::radioExclusiveDeselect1(){radioExclusiveDeselect(ui_->rad_modi_copy, 1);}
		void RmsdCalculator::radioExclusiveDeselect2(){radioExclusiveDeselect(ui_->rad_modi_yes, 2);}
		
		void RmsdCalculator::radioExclusiveDeselect(QRadioButton* btn, int pos)
		{
			if (pos == 1)
			{
				ui_->rad_modi_yes->setChecked(false);
			}
			else
				ui_->rad_modi_copy->setChecked(false);
		}
		
		void RmsdCalculator::getUserSettings()
		{
			// opt_superpose_
			if (ui_->rad_align_no->isChecked())
				opt_superpose_ = OPT_CALC_INPLACE;
			else
				opt_superpose_ = OPT_CALC_ALIGN;
			
			// opt_modify_
			if (ui_->rad_modi_yes->isChecked())
				opt_modify_ = OPT_MODIFY_YES;
			else if (ui_->rad_modi_copy->isChecked()) 
				opt_modify_ = OPT_MODIFY_COPY;
			else
				opt_modify_ = OPT_MODIFY_NO;
			
			// opt_atoms_
			if (ui_->rad_use_heavy->isChecked())
				opt_atoms_ = OPT_RMSD_HEAVY;
			else if (ui_->rad_use_all->isChecked())
				opt_atoms_ = OPT_RMSD_ALL;
			else if (ui_->rad_use_ca->isChecked())
				opt_atoms_ = OPT_RMSD_CA;
			else
				opt_atoms_ = OPT_RMSD_BB;
			
			// opt_type_
			if (checkProteinsOnly_())
				opt_type_ = OPT_TYPE_PROTEIN;
			else
				opt_type_ = OPT_TYPE_MOLECULE;
		}
		
		


		// Calculate RMSD for small-molecules and peptides:
		void RmsdCalculator::calc_rmsd()
		{
			// parse settings and start gui-decoupled functions:

			std::map<String, Position> type_map;
			type_map["ALA"] = 0;
			type_map["GLY"] = 1;
			type_map["VAL"] = 2;
			type_map["LEU"] = 3;
			type_map["ILE"] = 4;
			type_map["SER"] = 5;
			type_map["CYS"] = 6;
			type_map["THR"] = 7;
			type_map["MET"] = 8;
			type_map["PHE"] = 9;
			type_map["TYR"] = 10;
			type_map["TRP"] = 11;
			type_map["PRO"] = 12;
			type_map["HIS"] = 13;
			type_map["LYS"] = 14;
			type_map["ARG"] = 15;
			type_map["ASP"] = 16;
			type_map["GLU"] = 17;
			type_map["ASN"] = 18;
			type_map["GLN"] = 19;
			
			// map the two static chains
			Matrix4x4       transMatrix;
			StructureMapper sMapper;
			double					rmsd;
			
			// default values
			double upper = 8.0;
			double lower = 4.0;
			double tolerance = 0.6;
			
			// Print settings:
			getUserSettings();
			ui_->textEdit_results->clear();
			ui_->textEdit_results->appendPlainText(QString("OPTIONS:"));
			ui_->textEdit_results->appendPlainText(QString("opt_type_:      ") + QString::number(opt_type_));
			ui_->textEdit_results->appendPlainText(QString("opt_superpose_: ") + QString::number(opt_superpose_));
			ui_->textEdit_results->appendPlainText(QString("opt_atoms_:     ") + QString::number(opt_atoms_));
			ui_->textEdit_results->appendPlainText(QString("opt_modify_:      ") + QString::number(opt_modify_));
		}
		
		
		
		// modifications on probe structures are only possible
		// if a structural alignment is done.
		// Thus only enable these radio buttens, when "Superpose" radiobutton
		// aka 'rad_align_yes' is checked
		void RmsdCalculator::checkRadioAlign()
		{
			if (ui_->rad_align_yes->isChecked())
			{
				ui_->vbox_opt_modify->setEnabled(true);
			}
			else
			{
				ui_->vbox_opt_modify->setEnabled(false);
				ui_->rad_modi_copy->setChecked(false);
				ui_->rad_modi_yes->setChecked(false);
			}
		}
		
		
		
		// Handles the transfer of QTreeWidgetItmes and saved composites between
		// the lists and QTreeViewWidgets.
		void RmsdCalculator::moveBetween2Views_(QTreeWidget* from, list<Composite*>* fromLst, QTreeWidget* to, list<Composite*>* toLst)
		{
			QList<QTreeWidgetItem*> qlst = from->selectedItems();
			QList<QTreeWidgetItem*>::Iterator it = qlst.begin();
			
			for (; it != qlst.end(); it++)
			{
				// transfer view components:
				int idx = from->indexOfTopLevelItem(*it);
				QTreeWidgetItem* tmp = from->takeTopLevelItem(idx);
				to->addTopLevelItem(tmp);
				
				// 'transfer to' or 'remove from' lists:
				Composite* comp = static_cast <CompositWidgetItem*>(tmp)->composite;
				if(fromLst)
				{
					fromLst->remove(comp); // never delete from heap: the composites are always still referenced in the mainframe
				}
				if(toLst)
				{
					toLst->push_back(comp);
				}
			}
			
			// if all selections include only proteinic molecules, enable backbone
			// and calpha rmsd selection:
			bool areProteins = checkProteinsOnly_();
			ui_->rad_use_ca->setEnabled(areProteins);
			ui_->rad_use_bb->setEnabled(areProteins);
			ui_->rad_align_yes->setEnabled(areProteins);
			if (!areProteins)
			{
				ui_->rad_align_no->setChecked(true);
				if(ui_->rad_use_ca->isChecked())
				{
					ui_->rad_use_ca->setChecked(false);
					ui_->rad_use_heavy->setChecked(true);
				}
				if(ui_->rad_use_bb->isChecked())
				{
					ui_->rad_use_bb->setChecked(false);
					ui_->rad_use_heavy->setChecked(true);
				}
			}
			
			// check if we generally can calculate RMSD (each list containing at least 1 enty)
			btn_calculate->setEnabled(true);
			if ( ref_molecules_.empty() || probe_molecules_.empty() )
			{
				btn_calculate->setEnabled(false);
			}
		}
		
		//
		// "Add to list" and "Remove from List" buttons:
		//
		void RmsdCalculator::removeFrom_ref()
		{moveBetween2Views_(ui_->treeWidget_ref, &ref_molecules_, ui_->treeWidget_workspace, 0);}
		
		void RmsdCalculator::addTo_ref()
		{moveBetween2Views_(ui_->treeWidget_workspace, 0, ui_->treeWidget_ref, &ref_molecules_);}
		
		void RmsdCalculator::removeFrom_probe()
		{moveBetween2Views_(ui_->treeWidget_probe, &probe_molecules_, ui_->treeWidget_workspace, 0);}
		
		void RmsdCalculator::addTo_probe()
		{moveBetween2Views_(ui_->treeWidget_workspace, 0, ui_->treeWidget_probe, &probe_molecules_);}

		void RmsdCalculator::removeAllFrom_ref()
		{
			ui_->treeWidget_ref->selectAll();
			removeFrom_ref();
		}
		
		void RmsdCalculator::addAllTo_ref()
		{
			ui_->treeWidget_workspace->selectAll();
			addTo_ref();
		}
		
		void RmsdCalculator::removeAllFrom_probe()
		{
			ui_->treeWidget_probe->selectAll();
			removeFrom_probe();
		}
		
		void RmsdCalculator::addAllTo_probe()
		{
			ui_->treeWidget_workspace->selectAll();
			addTo_probe();
		}
	}// END OF CLASS/NAMESAPCES
}
