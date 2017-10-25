/**********************************************************************************************

  DSP Solutions GmbH & Co. KG
  http://www.dspsolutions.de/

  Author:      Oliver Eichler
  Email:       oliver.eichler@dspsolutions.de
  Phone:       +49-941-83055-1
  Fax:         +49-941-83055-79

  File:        main.cpp

  Module:

  Description:

  Created:     08/16/2013

  (C) 2013 DSP Solutions. All rights reserved.


**********************************************************************************************/

#include "CMainWidget.h"

int main(int argc, char ** argv)
{

    QApplication theApp(argc,argv);

    QCoreApplication::setApplicationName("TrackComp");
    QCoreApplication::setOrganizationName("DSPSolutions");
    QCoreApplication::setOrganizationDomain("dspsolutions.eu");

    CMainWidget w;
    w.show();

    int res  = theApp.exec();

    return res;

}
