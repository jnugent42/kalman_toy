//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file analysis/shared/src/DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class
//
//
// $Id: DetectorConstruction.cc 77206 2013-11-22 01:33:52Z adotti $
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"
#include "G4SolidStore.hh"

#include "F03FieldSetup.hh"
#include "F03FieldMessenger.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"
#include "B4cCalorimeterSD.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4ChordFinder.hh"
#include <iostream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreadLocal
G4GlobalMagFieldMessenger* DetectorConstruction::fMagFieldMessenger = 0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
:G4VUserDetectorConstruction(),
 fAbsorberMaterial(0),fGapMaterial(0),fDefaultMaterial(0),
 fSolidWorld(0),fLogicWorld(0),fPhysiWorld(0),
 fSolidCalor(0),fLogicCalor(0),fPhysiCalor(0),
 fSolidLayer(0),fLogicLayer(0),fPhysiLayer(0),
 fSolidAbsorber(0),fLogicAbsorber(0),fPhysiAbsorber(0),
 fSolidGap (0),fLogicGap (0),fPhysiGap (0),
 fDetectorMessenger(0)
 /* fSolidAbsorber2(0),fLogicAbsorber2(0),fPhysiAbsorber2(0), fAbsorber2Material(0), */

{
  // default parameter values of the calorimeter
  fAbsorberThickness = 20.*mm;
  fAbsorber2Thickness = 20.*mm;
  fGapThickness      = 200.*mm;
  fNbOfLayers        = 20;
  fCalorSizeYZ       = 200.*cm;
  fWorldSizeX       = 10000.*cm;
  fWorldSizeYZ       = 10000.*cm;
  fCalorThickness       = 5000.*cm;
  ComputeCalorParameters();
  
  // materials
  DefineMaterials();
  SetAbsorberMaterial("G4_POLYPROPYLENE");
  SetAbsorber2Material("G4_Fe");
  SetGapMaterial("G4_AIR");
  
  // create commands for interactive definition of the calorimeter
  fDetectorMessenger = new DetectorMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{ delete fDetectorMessenger;}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  return ConstructCalorimeter();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineMaterials()
{ 
// use G4-NIST materials data base
//
G4NistManager* man = G4NistManager::Instance();
fDefaultMaterial = man->FindOrBuildMaterial("AIR");
man->FindOrBuildMaterial("G4_Pb");
man->FindOrBuildMaterial("G4_Fe");
man->FindOrBuildMaterial("G4_lAr");
man->FindOrBuildMaterial("G4_Galactic");
man->FindOrBuildMaterial("G4_POLYPROPYLENE");
man->FindOrBuildMaterial("G4_AIR");

// print table
//
G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::ConstructCalorimeter()
{

  // Clean old geometry, if any
  //
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  // complete the Calor parameters definition
  ComputeCalorParameters();
   
  //     
  // World
  //
  fSolidWorld = new G4Box("World",                                //its name
                   fWorldSizeX/2,fWorldSizeYZ/2,fWorldSizeYZ/2);  //its size
                         
  fLogicWorld = new G4LogicalVolume(fSolidWorld,            //its solid
                                   fDefaultMaterial,        //its material
                                   "World");                //its name
                                   
  fPhysiWorld = new G4PVPlacement(0,                        //no rotation
                                 G4ThreeVector(),           //at (0,0,0)
                                 fLogicWorld,             //its logical volume
                                 "World",                   //its name
                                 0,                         //its mother  volume
                                 false,                  //no boolean operation
                                 0);                        //copy number
  
  //                               
  // Calorimeter
  //  
  fSolidCalor=0; fLogicCalor=0; fPhysiCalor=0;
  fSolidLayer=0; fLogicLayer=0; fPhysiLayer=0;
  
  if (fCalorThickness > 0.)  
    { fSolidCalor = new G4Box("Calorimeter",                //its name
                    fCalorThickness/2,fCalorSizeYZ/2,fCalorSizeYZ/2);//size
                                 
      fLogicCalor = new G4LogicalVolume(fSolidCalor,        //its solid
                                        fDefaultMaterial,   //its material
                                        "Calorimeter");     //its name
                                           
      fPhysiCalor = new G4PVPlacement(0,                    //no rotation
                                     G4ThreeVector(),       //at (0,0,0)
                                     fLogicCalor,           //its logical volume
                                     "Calorimeter",         //its name
                                     fLogicWorld,           //its mother  volume
                                     false,              //no boolean operation
                                     0);                    //copy number
  
  //                                 
  // Layer
  //
      fSolidLayer = new G4Box("Layer",                        //its name
                       fLayerThickness/2,fCalorSizeYZ/2,fCalorSizeYZ/2); //size
                       
      fLogicLayer = new G4LogicalVolume(fSolidLayer,        //its solid
                                       fDefaultMaterial,    //its material
                                       "Layer");            //its name
      if (fNbOfLayers > 1)                                      
        fPhysiLayer = new G4PVReplica("Layer",              //its name
                                     fLogicLayer,           //its logical volume
                                     fLogicCalor,           //its mother
                                     kXAxis,              //axis of replication
                                     fNbOfLayers,           //number of replica
                                     fLayerThickness);      //witdth of replica
      else
        fPhysiLayer = new G4PVPlacement(0,                  //no rotation
                                     G4ThreeVector(),       //at (0,0,0)
                                     fLogicLayer,           //its logical volume
                                     "Layer",               //its name
                                     fLogicCalor,           //its mother  volume
                                     false,               //no boolean operation
                                     0);                    //copy number     
    }                                   
  
  //                               
  // Absorber
  //
  fSolidAbsorber=0; fLogicAbsorber=0; fPhysiAbsorber=0;  
  
  if (fAbsorberThickness > 0.) 
    { fSolidAbsorber = new G4Box("Absorber",                //its name
                          fAbsorberThickness/2,fCalorSizeYZ/2,fCalorSizeYZ/2); 
                          
      fLogicAbsorber = new G4LogicalVolume(fSolidAbsorber,    //its solid
                                            fAbsorberMaterial, //its material
                                            "poly");//fAbsorberMaterial->GetName());//name
                                                
      fPhysiAbsorber = new G4PVPlacement(0,                   //no rotation
                          G4ThreeVector(-fGapThickness/2,0.,0.),  //its position
                          /* G4ThreeVector(-100,0.,0.),  //its position */
                                        fLogicAbsorber,     //its logical volume
                                        fAbsorberMaterial->GetName(), //its name
                                        fLogicLayer,          //its mother
                                        false,               //no boulean operat
                                        0);                   //copy number
                                        
    }
  //                               
  // Absorber 2
  //
  /* fSolidAbsorber2=0; fLogicAbsorber2=0; fPhysiAbsorber2=0; */  
  
  /* if (fAbsorber2Thickness > 0.) */ 
  /*   { fSolidAbsorber2 = new G4Box("Absorber2",                //its name */
  /*                         fAbsorber2Thickness/2,fCalorSizeYZ/2,fCalorSizeYZ/2); */ 
                          
  /*     fLogicAbsorber2 = new G4LogicalVolume(fSolidAbsorber2,    //its solid */
  /*                                           fAbsorber2Material, //its material */
  /*                                           "iron");//fAbsorber2Material->GetName());//name */
                                                
  /*     fPhysiAbsorber2 = new G4PVPlacement(0,                   //no rotation */
  /*                         G4ThreeVector(0,0.,0.),  //its position */
  /*                                       fLogicAbsorber2,     //its logical volume */
  /*                                       fAbsorber2Material->GetName(), //its name */
  /*                                       fLogicLayer,          //its mother */
  /*                                       false,               //no boulean operat */
  /*                                       0);                   //copy number */
                                        
  /*   } */
 
    /* const G4String* a; */
    /* const G4String* b; */ 
    /* G4int* c; */
    /* B4cCalorimeterSD* absoSD = new B4cCalorimeterSD(*a, *b, *c); */
    /*   /1* = new B4cCalorimeterSD("AbsorberSD", "AbsorberHitsCollection", fNbOfLayers); *1/ */
    /* SetSensitiveDetector("AbsoLV",absoSD); */ 
  //                                 
  // Gap
  //
  /* fSolidGap=0; fLogicGap=0; fPhysiGap=0; */ 
  
  /* if (fGapThickness > 0.) */
  /*   { fSolidGap = new G4Box("Gap", */
  /*                              fGapThickness/2,fCalorSizeYZ/2,fCalorSizeYZ/2); */
                               
  /*     fLogicGap = new G4LogicalVolume(fSolidGap, */
  /*                                          fGapMaterial, */
  /*                                          fGapMaterial->GetName()); */
                                           
  /*     fPhysiGap = new G4PVPlacement(0,                      //no rotation */
  /*              G4ThreeVector(fAbsorberThickness/2,0.,0.),   //its position */
  /*                                  fLogicGap,               //its logical volume */
  /*                                  fGapMaterial->GetName(), //its name */
  /*                                  fLogicLayer,             //its mother */
  /*                                  false,                   //no boulean operat */
  /*                                  0);                      //copy number */
  /*   } */
    
  PrintCalorParameters();     
  
  //                                        
  // Visualization attributes
  //
  fLogicWorld->SetVisAttributes (G4VisAttributes::Invisible);

  G4VisAttributes* simpleBoxVisAtt= new G4VisAttributes(G4Colour(1.0,1.0,1.0));
  simpleBoxVisAtt->SetVisibility(true);
  fLogicCalor->SetVisAttributes(simpleBoxVisAtt);

  //
  //always return the physical World
  //
  return fPhysiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::PrintCalorParameters()
{
  G4cout << "\n------------------------------------------------------------"
         << "\n---> The calorimeter is " << fNbOfLayers << " layers of: [ "
         << fAbsorberThickness/mm << "mm of " << fAbsorberMaterial->GetName() 
         << " + "
         /* << fAbsorber2Thickness/mm << "mm of " << fAbsorber2Material->GetName() */ 
         << " + "
         << fGapThickness/mm << "mm of " << fGapMaterial->GetName() << " ] " 
         << "\n------------------------------------------------------------\n";
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetAbsorberMaterial(G4String materialChoice)
{
  // search the material by its name   
  G4Material* pttoMaterial =
  G4NistManager::Instance()->FindOrBuildMaterial(materialChoice);      
  if (pttoMaterial)
  {
      fAbsorberMaterial = pttoMaterial;
      if ( fLogicAbsorber )
      {
          fLogicAbsorber->SetMaterial(fAbsorberMaterial);
          G4RunManager::GetRunManager()->PhysicsHasBeenModified();
      }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetAbsorber2Material(G4String materialChoice)
{
  // search the material by its name   
  G4Material* pttoMaterial =
  G4NistManager::Instance()->FindOrBuildMaterial(materialChoice);      
  if (pttoMaterial)
  {
      fAbsorber2Material = pttoMaterial;
      if ( fLogicAbsorber2 )
      {
          G4cout << "bnsfgnsvn" << G4endl;
          /* fLogicAbsorber2->SetMaterial(fAbsorber2Material); */
          G4cout << "bnsfgnsvn" << G4endl;
          G4RunManager::GetRunManager()->PhysicsHasBeenModified();
      }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::SetGapMaterial(G4String materialChoice)
{
  // search the material by its name
  G4Material* pttoMaterial =  
  G4NistManager::Instance()->FindOrBuildMaterial(materialChoice);   
  if (pttoMaterial)
  {
      fGapMaterial = pttoMaterial;
      if ( fLogicGap )
      {
          fLogicGap->SetMaterial(fGapMaterial);
          G4RunManager::GetRunManager()->PhysicsHasBeenModified();
      }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetAbsorberThickness(G4double val)
{
  // change Absorber thickness and recompute the calorimeter parameters
  fAbsorberThickness = val;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetGapThickness(G4double val)
{
  // change Gap thickness and recompute the calorimeter parameters
  fGapThickness = val;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetCalorSizeYZ(G4double val)
{
  // change the transverse size and recompute the calorimeter parameters
  fCalorSizeYZ = val;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetNbOfLayers(G4int val)
{
  fNbOfLayers = val;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField()
  {
    // G4SDManager::GetSDMpointer()->SetVerboseLevel(1);
  
    // 
    // Sensitive detectors
    //
    B4cCalorimeterSD* absoSD = new B4cCalorimeterSD("AbsorberSD", "AbsorberHitsCollection", fNbOfLayers); 
    /* SetSensitiveDetector(fAbsorberMaterial->GetName(),absoSD); */ 
    SetSensitiveDetector("poly",absoSD); 
  
    /* G4UniformMagField* magField = new G4UniformMagField( G4ThreeVector( 0.0, 0.0, 1.5*tesla  )  ); */
    /* G4FieldManager* fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager(); */
    /* fieldMgr->SetDetectorField( magField  ); */
    /* fieldMgr->CreateChordFinder( magField  ); */   
    /* G4cout << fieldMgr->GetChordFinder()->GetDeltaChord() << G4endl; */
    /* fieldMgr->GetChordFinder()->SetDeltaChord( 0.00001 ); */
    /*   fieldMgr->SetMinimumEpsilonStep( 1.0e-18  ); */
    /*   fieldMgr->SetAccuraciesWithDeltaOneStep(0.5e-15); */ 
    /*       fieldMgr->SetDeltaIntersection(0.5e-15*mm); */
    /*     fieldMgr->SetMaximumEpsilonStep( 1.0e-17  ); */
    /*       fieldMgr->SetDeltaOneStep( 0.5e-15 * mm  );  // 0.5 micrometer */
    /*   fieldMgr->SetFieldChangesEnergy(false); */
    // 
    // Magnetic field
    //
    // Create global magnetic field messenger.
    // Uniform magnetic field is then created automatically if
    // the field value is not zero.
    G4ThreeVector fieldValue = G4ThreeVector();
    fMagFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
    fMagFieldMessenger->SetVerboseLevel(1);
  
    // Register the field messenger for deleting
    G4AutoDelete::Register(fMagFieldMessenger);

   // Construct the field creator - this will register the field it creates
 
   /* if (!fEmFieldSetup.Get()) { */
   /*   F03FieldSetup* emFieldSetup = new F03FieldSetup(); */
 
   /*   fEmFieldSetup.Put(emFieldSetup); */
   /*   G4AutoDelete::Register(emFieldSetup); //Kernel will delete the messenger */
   /* } */
   // Set local field manager and local field in radiator and its daughters:
   G4bool allLocal = false;

   /* fLogicAbsorber2->SetFieldManager(fEmFieldSetup.Get()->GetLocalFieldManager(), */
   /*                                 allLocal ); */
   /* fLogicCalor->SetFieldManager(fEmFieldSetup.Get()->GetLocalFieldManager(), */
                                   /* allLocal ); */


  }