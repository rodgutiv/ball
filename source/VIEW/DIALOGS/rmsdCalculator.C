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
#include <BALL/STRUCTURE/atomBijection.h>
#include <BALL/KERNEL/atomContainer.h>
#include <BALL/KERNEL/forEach.h>
#include <BALL/KERNEL/protein.h>
#include <BALL/KERNEL/molecule.h>
#include <BALL/KERNEL/system.h>
#include <BALL/KERNEL/PTE.h>

#include <BALL/VIEW/WIDGETS/structureViewWidget.h>

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
			
			setObjectName ( name );

			// register the widget with the MainControl
			ModularWidget::registerWidget ( this );
			
			// setup the button box:
			btn_calculate = ui_->buttonBox->addButton ( tr ( " Calculate " ), QDialogButtonBox::ActionRole );
			connect( btn_calculate, SIGNAL(clicked()), this, SLOT(calc_rmsd()) );
			QPushButton* btn_cancel = ui_->buttonBox->addButton ( tr ( " Cancel " ), QDialogButtonBox::RejectRole );
			connect( btn_cancel, SIGNAL(clicked()), this, SLOT(finished()) );
			
			// Setup the 2x4 add/remove arrow-buttons:
			// single:
			connect( ui_->btn_toRef, SIGNAL( clicked()), this, SLOT(addTo_ref()) );
			connect( ui_->btn_fromRef, SIGNAL(clicked()), this, SLOT(removeFrom_ref()) );
			connect( ui_->btn_toProbes, SIGNAL(clicked()), this, SLOT(addTo_probe()) );
			connect( ui_->btn_fromProbes, SIGNAL(clicked()), this, SLOT(removeFrom_probe()) );
			// all:
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
			// NOT YET 100% SURE IF I NEED TO DELETE SOMETHING ELSE HERE

			delete ui_;
		}
		
		
		void RmsdCalculator::show()
		{
			// Clear data of a possible previous session:
			//ui_->textEdit_results->clear();
			
			// reset options:
			btn_calculate->setEnabled(false);
			ui_->vbox_opt_modify->setEnabled(false);
			
			// reset previously selected structures:
			removeAllFrom_probe();
			removeAllFrom_ref();
			
			// generate flat names from current composite set:
			HashSet<Composite*> composites = getMainControl()->getCompositeManager().getComposites();
			ui_->treeWidget_workspace->clear();
			ui_->treeWidget_workspace->generateTree(composites);
			
			QDialog::show();
			raise();
		}
		
		/* Response to clicking "cancel"
		 */
		void RmsdCalculator::finished()
		{
			hide();
		}
		
		/* How and where this dialog will be
		 * registered
		 */
		void RmsdCalculator::initializeWidget ( MainControl& )
		{
			action1_ = insertMenuEntry ( MainControl::TOOLS, tr ( "&Superpose && RMSD" ), this,
			                             SLOT ( show() ), "Shortcut|Tools|Superpose_RMSD", QKeySequence(),
			                             tr ( "Dialog for Superimposing and RMSD calculations" ),
			                             UIOperationMode::MODE_ADVANCED );
		}

		/*
		 * Disable/Lock specific parts of the GUI
		 * while this tool is active
		 */
		void RmsdCalculator::checkMenu ( MainControl& main_control )
		{// disable tool menu-entry when structure/composite window is locked:
			bool busy = main_control.compositesAreLocked();

			if ( action1_ )
				action1_->setEnabled ( !busy );
			// TODO: 
			// better lock composites, while the dialog is open
			QMenu* menu = getMainControl()->initPopupMenu(MainControl::TOOLS_GRID, UIOperationMode::MODE_ADVANCED);
			if (menu)
				menu->setEnabled(false);
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
		void RmsdCalculator::radioExclusiveDeselect1(){radioExclusiveDeselect(ui_->rad_modi_copy);}
		void RmsdCalculator::radioExclusiveDeselect2(){radioExclusiveDeselect(ui_->rad_modi_yes);}
		
		void RmsdCalculator::radioExclusiveDeselect(QRadioButton* btn)
		{
			if (btn == ui_->rad_modi_copy)
			{
				ui_->rad_modi_yes->setChecked(false);
			}
			else
				ui_->rad_modi_copy->setChecked(false);
		}

		
		/*
		 * Calculate RMSD for proteins and small-molecules
		 */
		void RmsdCalculator::calc_rmsd()
		{
			// TODO: perhaps some pre-checks if rmsd-calculation makes sense
			// e.g.: smolecule  and proteins together in the lists
			
			getUserSettings(); // get user settings
			
			// setup loop variables:
			QString result_output = "";
			
			StructureMapper mapper;
			AtomBijection atom_selection;
			
			double rmsd_result = -1;
			Matrix4x4 trans_matrix;
			
			// loop in a many-to-many manner over references and probes:
			list<Composite*>::iterator refIt = ref_molecules_.begin();
			for (; refIt != ref_molecules_.end(); refIt++)
			{
				list<Composite*>::iterator proIt = probe_molecules_.begin();
				for (; proIt != probe_molecules_.end(); proIt++)
				{
//------------------------------------------------------------------------------
					// get molecules:
					AtomContainer* ref_ac = dynamic_cast<AtomContainer*>(*refIt);
					AtomContainer* probe_ac = dynamic_cast<AtomContainer*>(*proIt);

					// User selected "superpose":
					if (opt_superpose_ == OPT_CALC_ALIGN)
					{
						getMapping(probe_ac, ref_ac, mapper, trans_matrix);// get transformation matrix
						mapper.setTransformation(trans_matrix);
						
						// modify the probes/queries:
						if (opt_modify_ == OPT_MODIFY_YES)
						{
							probe_ac->apply(mapper);

							applyBijection(atom_selection, ref_ac, probe_ac);
							rmsd_result = mapper.calculateRMSD(atom_selection); //transformation == identity
							// update the changed structure:
							getMainControl()->updateRepresentationsOf(*probe_ac,true,true);
						}
						// create a modified copy:
						else if (opt_modify_ == OPT_MODIFY_COPY)
						{
							AtomContainer* temp_probe_ac = insertCopyIntoWorkspace(probe_ac);
							temp_probe_ac->apply(mapper);
							
							applyBijection(atom_selection, ref_ac, temp_probe_ac);
							rmsd_result = mapper.calculateRMSD(atom_selection); //transformation == identity
						}
						// align but without affecting workspace:
						else
						{
							AtomContainer* temp_probe_ac = new AtomContainer(*probe_ac, true);
							temp_probe_ac->apply(mapper);
							
							applyBijection(atom_selection, ref_ac, temp_probe_ac);
							rmsd_result = mapper.calculateRMSD(atom_selection); //transformation == identity
							
							delete temp_probe_ac;
						}
					}
					// User selected: "no superposition":
					else
					{
						applyBijection(atom_selection, ref_ac, probe_ac);
						rmsd_result = mapper.calculateRMSD(atom_selection); //transformation == identity
					}
					// selected atoms for RMSD:
					ui_->textEdit_results->appendPlainText(QString("got bijection size ")+
																								 QString(QString::number(atom_selection.size())));
					// DEBUG START
					AtomBijection::PairVector::iterator it = atom_selection.begin();
					for (;it != atom_selection.end(); it++)
					{
						ui_->textEdit_results->appendPlainText(QString(it->first->getFullName().c_str())+
																									 QString(" with ")+QString(it->second->getFullName().c_str()));
					}
					ui_->textEdit_results->appendPlainText(QString("RMSD: ") + QString::number(rmsd_result));
					
					// TODO
					// * get the original names, as displayed in the widget
					//   add to Result-object (perhaps a string/qustring?)
					// * format RMSD meaningful: float with (max) 4 digits

//			// DEBUG:
//			ui_->textEdit_results->appendPlainText(QString("OPTIONS:"));
//			ui_->textEdit_results->appendPlainText(QString("opt_type_:      ") + QString::number(opt_type_));
//			ui_->textEdit_results->appendPlainText(QString("opt_superpose_: ") + QString::number(opt_superpose_));
//			ui_->textEdit_results->appendPlainText(QString("opt_atoms_:     ") + QString::number(opt_atoms_));
//			ui_->textEdit_results->appendPlainText(QString("opt_modify_:      ") + QString::number(opt_modify_));
//			ui_->textEdit_results->appendPlainText(QString("......................................."));
//      // selected atoms for RMSD:
//					ui_->textEdit_results->appendPlainText(QString("got bijection size ")+
//																								 QString(QString::number(atom_selection.size())));
//					// DEBUG START
//					AtomBijection::PairVector::iterator it = atom_selection.begin();
//					for (;it != atom_selection.end(); it++)
//					{
//						ui_->textEdit_results->appendPlainText(QString(it->first->getFullName().c_str())+
//																									 QString(" - ")+QString(it->second->getFullName().c_str()));
//					}
//					// DEBUG END

//------------------------------------------------------------------------------
				}
				ui_->textEdit_results->appendPlainText(QString("")); // next reference
			} // END many-to-many LOOP
			
			//TODO:
			// Display the results for this (multi)-calculation
		}
		
		/* 
		 * Create the correct bijection object, depending on
		 * the users choice.
		 * This selects certain atoms from the two input atom containers for the
		 * RMSD calculation later. AtomBijection is a vector<pair<atom*, atom*>>
		 */
		void RmsdCalculator::applyBijection(AtomBijection& atm_bij, AtomContainer* ref, AtomContainer* probe)
		{
			switch( opt_atoms_)
			{
			case OPT_RMSD_HEAVY:
			{
				// select all non hydrogen atoms
				AtomIterator iter, iter2;
				iter = ref->beginAtom(); iter2 = probe->beginAtom();
				for(; (iter != ref->endAtom() && iter2 != probe->endAtom()); iter++, iter2++)
				{
					if (iter->getElement().getSymbol() == "H"){
						iter->deselect();
					}
					else{
						iter->select();
					}
					
					if (iter2->getElement().getSymbol() == "H") {
						iter2->deselect();
					}
					else{
						iter2->select();
					}
				}

				if (opt_type_ == OPT_TYPE_MOLECULE)// if we have a smolecule we can only use trivial-assignment
					atm_bij.assignTrivial(*probe, *ref, true);
				else
					atm_bij.assignByName(*probe, *ref, true);
				break;
			}
			case OPT_RMSD_ALL:
			{
				if (opt_type_ == OPT_TYPE_MOLECULE)
					atm_bij.assignTrivial(*probe, *ref);
				else
					atm_bij.assignByName(*probe, *ref);
			}
				break;
			case OPT_RMSD_CA:
				atm_bij.assignCAlphaAtoms(*probe, *ref);
				break;
			case OPT_RMSD_BB:
				atm_bij.assignBackboneAtoms(*probe, *ref);
				break;
			default:
				// TODO trow error?
				break;
			}
		}
		
		/* 
		 * Attain the transformation matrix that maps the probe on the reference.
		 * Currently this can only be done for proteins and the alignment is 
		 * calculated only according to their C-alpha atoms.
		 */
		void RmsdCalculator::getMapping(AtomContainer* probe, AtomContainer* ref, StructureMapper &mapper, Matrix4x4 &trans_matrix)
		{
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
			
			// default box values
			double upper = 8.0;
			double lower = 4.0;
			double tolerance = 0.6;
			double rmsd=-1;
			Size matched_ca;
			
			//TODO: use dynamic cast and catch error! (the chains could be incorrectly molecules)
			Protein* orig_ref = (Protein*)ref->getParent();
			Protein* orig_prob= (Protein*)probe->getParent();

			trans_matrix = mapper.mapProteins(*orig_prob,*orig_ref,type_map,matched_ca,rmsd,upper,lower,tolerance);
		}
		
		
		/*
		 * Creates a deep copy of the given atomContainer and inserts it into
		 * the workspace.
		 */
		AtomContainer* RmsdCalculator::insertCopyIntoWorkspace(AtomContainer *ac)
		{
			// create a new system:
			System* new_sys = new System();
			
			if (opt_type_ == OPT_TYPE_PROTEIN)
			{
				AtomContainer *new_ac = new AtomContainer(*ac, true);
				Protein* prot = new Protein();
				prot->insert(*new_ac);
				new_sys->insert(*prot);
				getMainControl()->insert(*new_sys, "copy_"+ac->getName());
				return new_ac;
			}
			else
			{
				Molecule * mol = new Molecule(*ac, true);
				new_sys->insert(*mol);
				getMainControl()->insert(*new_sys, "copy_"+ac->getName());
				return mol;
			}
		}
		
		
		/*
		 * Derive the current user settings from the state of the radio buttons
		 * and the selected structures
		 */
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
		

		/*
		 * Response to selecting "Superpose" or "In place".
		 * Only enable the modification settings, if the user actually decides to
		 * align the structures. Otherwise disable the modification settings.
		 */
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
		
		/*
		 * "Add to list" and "Remove from List" buttons:
		 */
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
