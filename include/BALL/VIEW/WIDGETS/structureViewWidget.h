// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#ifndef BALL_VIEW_WIDGETS_STRUCTUREVIEWWIDGET_H
#define BALL_VIEW_WIDGETS_STRUCTUREVIEWWIDGET_H

#ifndef BALL_VIEW_KERNEL_COMMON_H
# include <BALL/VIEW/KERNEL/common.h>
#endif

#ifndef BALL_CONCEPT_MOLECULARINFORMATION_H
#include <BALL/CONCEPT/molecularInformation.h>
#endif

#include <QtGui>


namespace BALL
{
	namespace VIEW
	{

// for internal usage only:
class CompositWidgetItem
	: public QTreeWidgetItem
{
	public:

		CompositWidgetItem(QTreeWidget* parent, QStringList& sl, Composite* composite);

		CompositWidgetItem(QTreeWidgetItem* parent, QStringList& sl, Composite* composite);

		Composite* composite;
};
/**	structureViewWidget <class description here>
*/
class BALL_VIEW_EXPORT StructureViewWidget: public QTreeWidget
{
	Q_OBJECT
	public:
	/**	@name	Constructors and Destructor
	*/	
	//@{

	/** Default Constructor.
			\param      parent the parent widget 
			\param      name the name of this widget
	*/
	StructureViewWidget(QWidget* parent = 0);
	
	void generateTree(HashSet<Composite*>& composites);
	
	//@} 
	/** @name Protected members 
	*/ 
	//@{
	protected slots:

	protected:
		bool isValidChain(Composite* comp);
		void recursiveGeneration_(Composite * composite, QString& inname);
		QString getName_(MolecularInformation &molInfo);
};

}} // namespaces

#endif // BALL_VIEW_WIDGETS_STRUCTUREVIEWWIDGET_H
