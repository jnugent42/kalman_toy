/* Copyright 2008-2010, Technische Universitaet Muenchen,
   Authors: Christian Hoeppner & Sebastian Neubert & Johannes Rauch

   This file is part of GENFIT.

   GENFIT is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GENFIT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GENFIT.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ConstField.h"
#include <iostream>
#include <TGeoManager.h>
#include <typeinfo>
#include <cstring>
#include <string>

namespace genfit {

TVector3 ConstField::get(const TVector3& pos) const {
    
    TVector3 tmp(0,0,0);
    std::string hold = "b";
    std::string name2(1, gGeoManager->FindNode(pos[0],pos[1],pos[2])->GetName()[0]);
    name2.c_str();
    if (name2.c_str()==hold) tmp.SetZ(15);
    return tmp;
    /* return field_; */
}

void ConstField::get(const double& posX, const double& posY, const double& posZ, double& Bx, double& By, double& Bz) const {

    TVector3 tmp(0,0,15);
    std::string hold = "b";
    std::string name2;
    std::string name3(1, gGeoManager->FindNode(posX,posY,posZ)->GetName()[0]);
    name2 = name3;
    name2.c_str();
    if (name2.c_str()==hold){
        Bx = tmp.X();
        By = tmp.Y();
        Bz = tmp.Z();
    }
    else { 
        Bx = 0;
        By = 0;
        Bz = 0;
    }
    /* Bx = field_.X(); */
    /* By = field_.Y(); */
    /* Bz = field_.Z(); */
}

} /* End of namespace genfit */
