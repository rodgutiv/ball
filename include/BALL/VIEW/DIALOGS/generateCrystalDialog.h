// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:

#ifndef BALL_VIEW_DIALOGS_GENERATECRYSTALDIALOG_H
#define BALL_VIEW_DIALOGS_GENERATECRYSTALDIALOG_H

#include <BALL/VIEW/UIC/ui_generateCrystalDialog.h>

#ifndef BALL_VIEW_KERNEL_MODULARWIDGET_H
# include <BALL/VIEW/KERNEL/modularWidget.h>
#endif

#ifndef BALL_DATATYPE_HASHSET_H
# include <BALL/DATATYPE/hashSet.h>
#endif

#ifndef BALL_KERNEL_SYSTEM_H
# include <BALL/KERNEL/system.h>
#endif

#ifndef BALL_XRAY_CRYSTALGENERATOR_H
# include <BALL/XRAY/crystalGenerator.h>
#endif

namespace BALL
{

	namespace VIEW
	{

		/** Dialog for generating crystallographic objects like the ASU, the unit cell
				or an arbitrarily spaced crystal packing
				\ingroup ViewDialogs
		*/
		class BALL_VIEW_EXPORT GenerateCrystalDialog
			: public QDialog,
				public Ui_GenerateCrystalDialogData,
				public ModularWidget
		{ 
				Q_OBJECT
			
			public:
				BALL_EMBEDDABLE(GenerateCrystalDialog, ModularWidget)
				
				///
				GenerateCrystalDialog(QWidget* parent = 0, const char* name = "GenerateCrystalDialog", 
												bool modal = FALSE, Qt::WFlags fl = 0 );

				///
				~GenerateCrystalDialog();

				///
				//void checkMenu(MainControl& mc);
				
				///
				bool initSpaceGroupList();
				
				///
				//virtual void initializeWidget(MainControl& main_control);
				
			public slots:
				
				///
				void slotOk();
				
				///
				void slotCancel();


			protected:


				HashMap<String, QImage> image_cache_;

				// e.g. gif images if not supported
				HashSet<String> 				unsupported_images_;

				//QAction* menu_id_;
				CrystalGenerator* cryst_gen_;
				System* selectedSystem_;
		};

	} 
} //namespaces
#endif