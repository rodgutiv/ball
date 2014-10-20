/* TRANSLATOR BALL::VIEW::StructureViewWidget

		Necessary for lupdate.
*/

// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#include <BALL/VIEW/WIDGETS/structureViewWidget.h>
#include <BALL/CONCEPT/composite.h>
#include <BALL/KERNEL/atomContainer.h>
#include <BALL/KERNEL/chain.h>
#include <BALL/KERNEL/residue.h>
//#include <BALL/VIEW/KERNEL/mainControl.h>

#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QToolTip>
#include <QtGui/QGridLayout>
#include <QtGui/QCompleter>

#include <set>
namespace BALL
{
	namespace VIEW
	{

		CompositWidgetItem::CompositWidgetItem(QTreeWidget* parent,
																												 QStringList& sl, 
																												 Composite* c)
			: QTreeWidgetItem(parent, sl),
				composite(c)
		{
			setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}



		CompositWidgetItem::CompositWidgetItem(QTreeWidgetItem* parent, 
																												 QStringList& sl, 
																												 Composite* c)
			: QTreeWidgetItem(parent, sl),
				composite(c)
		{
			setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}



		StructureViewWidget::StructureViewWidget(QWidget* parent):
			 QTreeWidget(parent)
		{
		#ifdef BALL_VIEW_DEBUG
			Log.error() << "new MolecularControl " << this << std::endl;
#endif
		}
		
		CompositWidgetItem *StructureViewWidget::getWidgetItem(Composite * comp)
		{
			if (comp_to_widget.has(comp))
			{
				return comp_to_widget[comp];
			}
			else
			{
				return 0;
			}
		}
		
		
		
		void StructureViewWidget::generateTree(HashSet<Composite *> &composites)
		{
			HashSet<Composite *>::Iterator it = composites.begin();
			MolecularInformation molInfo;
			for (; it != composites.end(); ++it)
			{
				// every workspace entry starts with a system having a system name
				// get that one and start recursion:
				molInfo.visit(**it);
				QString qnam = molInfo.getName().c_str();
				recursiveGeneration_(*it, qnam);
			}
			this->resizeColumnToContents(0);
		}
		
		/*
		 * This function differentiates protein chains from water chains (or chains
		 * containing ions, ligands etc...). A "valid Chain" contains (mostly) amino
		 * acids.
		 */
		bool StructureViewWidget::isValidChain(Composite *comp)
		{
			Chain* totest = dynamic_cast<Chain*>(comp);
			if (!totest)
				return false;
			
			Residue * a_res=0;
			int yes=0;
			int min = 7;
			int num_res = totest->countResidues();
			
			// check number of residues:
			if (num_res == 0)
			{
				return false;
			}
			else if(num_res < 7)
			{
				min = num_res;
			}
			
			// check that most residues are amino acids
			for (int i = 0; i < num_res; i++)
			{
				a_res = totest->getResidue(i);
				if (!a_res)
					return false;
				
				if(a_res->isAminoAcid())
					yes++;
			}
			return (min-yes <= 0);
		}
		
		
		
		void StructureViewWidget::recursiveGeneration_(Composite * composite, QString& inname)
		{
			// create a new list item
			CompositWidgetItem* new_item = 0;
			MolecularInformation molInfo;
			
			// iterate over all children of the composite:
			Composite::ChildCompositeIterator it = composite->beginChildComposite();
			for (;it!= composite->endChildComposite(); it++)
			{
				// get name of composite:
				molInfo.visit(*it);
				QString name = inname + "_" + getName_(molInfo);

				// if it is a single molecule, make an entry
				MolecularInformation::Type typ = molInfo.getType();
				if (typ == MolecularInformation::TYPE__CHAIN  ||
						typ == MolecularInformation::TYPE__MOLECULE)
				{
					if( typ == MolecularInformation::TYPE__CHAIN && !isValidChain(&(*it)) )
					{
						continue; // excludes non protein chains, we could alternatively
											// create a specific entry for this...
					}
					QStringList sl;
					sl << name;
					new_item = new CompositWidgetItem(this, sl, &((*it)));
					comp_to_widget[&*it] = new_item;
				}
				// else if not ended: descend further
				else if(it->getDegree() != 0)
				{
					recursiveGeneration_(&(*it), name);
				}
				// else do nothing (path to entry did describe not a molecule)
			}
		}



		QString StructureViewWidget::getName_(MolecularInformation &molInfo)
		{
			QString res = "";
			if (molInfo.getType() == MolecularInformation::TYPE__CHAIN)
			{
				res += "chain_";
			}
			else if (molInfo.getType() == MolecularInformation::TYPE__MOLECULE)
			{
				res += "molec_";
			}
			res += molInfo.getName().c_str();
			
			return res.remove(" ");
		}
}} // END NAME SPACES
