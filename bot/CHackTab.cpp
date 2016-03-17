#include "stdafx.h"

#include "CHackTab.hpp"
#include "loot.h"

CHackTab::CHackTab(__in QWidget *pParent) : QWidget(pParent)
{
	QCheckBox *fmlCheckBox = new QCheckBox(tr("Full Map Loot"), this);
	connect(fmlCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnFMLCheckBoxStateChanged(int)));

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(fmlCheckBox);
	mainLayout->addStretch(1);
	setLayout(mainLayout);
}

CHackTab::~CHackTab()
{

}

void CHackTab::OnFMLCheckBoxStateChanged(int state)
{
	ToggleFullMapLoot(state);
}