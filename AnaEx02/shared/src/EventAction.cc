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
/// \file analysis/shared/src/EventAction.cc
/// \brief Implementation of the EventAction class
//
//
// $Id: EventAction.cc 67226 2013-02-08 12:07:18Z ihrivnac $
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "EventAction.hh"

#include "RunAction.hh"
#include "HistoManager.hh"
#include "G4SDManager.hh"
#include "B4cCalorimeterSD.hh"
#include "B4cCalorHit.hh"
#include "B4Analysis.hh"

#include "G4Event.hh"

#include "globals.hh"

 #include "B4cCalorimeterSD.hh"
 
 #include "G4RunManager.hh"
 #include "G4HCofThisEvent.hh"
 #include "G4UnitsTable.hh"
 
 #include "Randomize.hh"
 #include <iomanip>


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction(RunAction* run, HistoManager* histo)
:G4UserEventAction(),
 fRunAct(run),fHistoManager(histo),
 fEnergyAbs(0.), fEnergyGap(0.),
 fTrackLAbs(0.), fTrackLGap(0.),
 fPrintModulo(0)                             
{
 fPrintModulo = 100; }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{ }

B4cCalorHitsCollection*  EventAction::GetHitsCollection(G4int hcID, const G4Event* event) const                                           
{                                                                 
    /* G4cout << hcID << G4endl; */ 
    /* G4cout << "sdbsdgb " << event->GetHCofThisEvent()->GetHC(hcID)->PrintAllHits() << G4endl; */ 
    /* for (int j=0;j<hcID+1;j++){ */
    /* G4cout << j << " " << event->GetHCofThisEvent()->GetHC(j)->GetSDname() << G4endl; */
    /* } */

    /* G4cout << event->GetHCofThisEvent()->GetHC(hcID)->GetSize() << G4endl; */
    B4cCalorHitsCollection* hitsCollection = static_cast<B4cCalorHitsCollection*>(event->GetHCofThisEvent()->GetHC(0));                                                        
    /* event->GetHCofThisEvent()->GetHC(hcID)); */                                                        
    /* G4cout << hcID << G4endl; */ 

    if ( ! hitsCollection ) {                                                                             
        G4ExceptionDescription msg;                                                                         
        msg << "Cannot access hitsCollection ID " << hcID;                                                  
        G4Exception("B4cEventAction::GetHitsCollection()",                                                  
                "MyCode0003", FatalException, msg);                                                               
    }                                                                                                     

    return hitsCollection;                                                                                
}                                                                                                       
                                                                                                             

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

    void EventAction::BeginOfEventAction(const G4Event* evt)
    {  
      G4int evtNb = evt->GetEventID();
      if (evtNb%fPrintModulo == 0) 
        G4cout << "\n---> Begin of event: " << evtNb << G4endl;
     
     // initialisation per event
     fEnergyAbs = fEnergyGap = 0.;
     fTrackLAbs = fTrackLGap = 0.;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

    void EventAction::EndOfEventAction(const G4Event* event)
    {
        if ( fAbsHCID == -1 ) {
            fAbsHCID 
                = G4SDManager::GetSDMpointer()->GetCollectionID("AbsorberHitsCollection");
        }

        // Get hits collections
        B4cCalorHitsCollection* absoHC = GetHitsCollection(fAbsHCID, event);

        // Fill histograms, ntuple
        //

        // get analysis manager
        G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

        for (G4int i=0;i<absoHC->entries();i++) {
            G4cout << "1" << G4endl;
            /* if ((*absoHC)[i]->GetParentID()==0){ */
                G4ThreeVector position = (*absoHC)[i]->GetPosition();
                G4ThreeVector momentum = (*absoHC)[i]->GetMomentum(); 
                std::cout << position.x() << std::endl;
                posx->push_back(position.x());
                posy->push_back(position.y());
                posz->push_back(position.z());
                fHistoManager->FillHisto(1, position.x());
                fHistoManager->FillHisto(2, position.y());
                fHistoManager->FillHisto(3, position.z());
            /* } */
        }

        //accumulates statistic
        //
        fRunAct->fillPerEvent(fEnergyAbs, fEnergyGap, fTrackLAbs, fTrackLGap);

        //fill histograms
        //
        fHistoManager->FillHisto(4, fTrackLGap);

        //fill ntuple
        //
        fHistoManager->FillNtuple(fEnergyAbs, fEnergyGap, fTrackLAbs, fTrackLGap);
        fHistoManager->FillNtuple2(posx,posy,posz);
        /* analysisManager->AddNtupleRow(); */
        posx->clear();
        posy->clear();
        posz->clear();
    }  

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
