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
/// \file analysis/AnaEx02/src/HistoManager.cc
/// \brief Implementation of the HistoManager class
//
// $Id: HistoManager.cc 74272 2013-10-02 14:48:50Z gcosmo $
// GEANT4 tag $Name: geant4-09-04 $
// 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include <TH1D.h>
#include <TFile.h>
#include <TTree.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include "HistoManager.hh"
#include "G4UnitsTable.hh"
#include <vector>
#include <iostream>
#include "../shared/include/EventAction.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HistoManager::HistoManager()
:fRootFile(0), 
 fNtuple1(0), fNtuple2(0), fNtuple3(0),
 fEabs(0), fEgap(0) ,fLabs(0), fLgap(0)
{
      
  // histograms
  for (G4int k=0; k<MaxHisto; k++) fHisto[k] = 0;
    
  // ntuple
  fNtuple1 = 0;
  fNtuple2 = 0;
  fNtuple3 = 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HistoManager::~HistoManager()
{
  if ( fRootFile ) delete fRootFile;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::book()
{ 
 // Creating a tree container to handle histograms and ntuples.
 // This tree is associated to an output file.
 //
 G4String fileName = "AnaEx02.root";
 fRootFile = new TFile(fileName,"RECREATE");
 if(!fRootFile) {
   G4cout << " HistoManager::book :" 
          << " problem creating the ROOT TFile "
          << G4endl;
   return;
 }
   
 fHisto[1] = new TH1D("1", "posx", 100, -100., 100);
 if (!fHisto[1]) G4cout << "\n can't create histo 1" << G4endl;
 fHisto[2] = new TH1D("2", "posy", 100, -40., 40);
 if (!fHisto[2]) G4cout << "\n can't create histo 2" << G4endl;
 fHisto[3] = new TH1D("3", "posz", 100, -40., 40);
 if (!fHisto[3]) G4cout << "\n can't create histo 3" << G4endl;
 fHisto[4] = new TH1D("4", "trackL in gap", 100, 0., 50*CLHEP::cm);
 if (!fHisto[4]) G4cout << "\n can't create histo 4" << G4endl;  

 // create 1st ntuple in subdirectory "tuples"
 //
 fNtuple1 = new TTree("101", "Edep");
 fNtuple1->Branch("Eabs", &fEabs, "Eabs/D");
 fNtuple1->Branch("Egap", &fEgap, "Egap/D");

 // create 2nd ntuple in subdirectory "tuples"
 //
 fNtuple2 = new TTree("102", "TrackL");
 fNtuple2->Branch("Labs", &fLabs, "Labs/D");
 fNtuple2->Branch("Lgap", &fLgap, "Lgap/D");

 // create 3nd ntuple in subdirectory "tuples"
 //
 /* std::vector<float>* posx = new std::vector<float>; */
 /* std::vector<float>* posy = new std::vector<float>; */
 /* std::vector<float>* posz = new std::vector<float>; */
 fNtuple3 = new TTree("103", "pos");
 fNtuple3->Branch("_posx", "std::vector<float>", &_posx, 64000, 10);
 fNtuple3->Branch("_posy", "std::vector<float>", &_posy, 64000, 10);
 fNtuple3->Branch("_posz", "std::vector<float>", &_posz, 64000, 10);

 G4cout << "\n----> Histogram file is opened in " << fileName << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::save()
{ 
  if (fRootFile) {
    fRootFile->Write();       // Writing the histograms to the file
    fRootFile->Close();        // and closing the tree (and the file)
    G4cout << "\n----> Histogram Tree is saved \n" << G4endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::FillHisto(G4int ih, G4double xbin, G4double weight)
{
  if (ih >= MaxHisto) {
    G4cout << "---> warning from HistoManager::FillHisto() : histo " << ih
           << " does not exist. (xbin=" << xbin << " weight=" << weight << ")"
           << G4endl;
    return;
  }
 if  (fHisto[ih]) { fHisto[ih]->Fill(xbin, weight); }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::Normalize(G4int ih, G4double fac)
{
  if (ih >= MaxHisto) {
    G4cout << "---> warning from HistoManager::Normalize() : histo " << ih
           << " does not exist. (fac=" << fac << ")" << G4endl;
    return;
  }
  if (fHisto[ih]) fHisto[ih]->Scale(fac);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::FillNtuple(G4double energyAbs, G4double energyGap,
                              G4double trackLAbs , G4double trackLGap )
{
 fEabs = energyAbs;
 fEgap = energyGap;
 fLabs = trackLAbs;
 fLgap = trackLGap;

  if (fNtuple1) fNtuple1->Fill();
  if (fNtuple2) fNtuple2->Fill();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::FillNtuple2(std::vector<float>* posx, std::vector<float>* posy, std::vector<float>* posz)
{
        G4cout << "posx.size " << posx->at(5) << G4endl;     
        /* _posx.assign(0,2); */
        /* G4cout << "posx->size " << posx->size() << G4endl; */     
 for(int i=0;i<posx->size();i++){
     if (posx->at(i)!=0) _posx->push_back(posx->at(i));
        /* G4cout << "_posx->at(i) " << _posx->at(i) << G4endl; */     
}
 for(int i=0;i<posy->size();i++){
 if (posy->at(i)!=0) _posy->push_back(posy->at(i));
}
 for(int i=0;i<posz->size();i++){
 if (posz->at(i)!=0) _posz->push_back(posz->at(i));
}

  if (fNtuple3) fNtuple3->Fill();
  fNtuple3->Print();
  fNtuple3->Show(0);
  fNtuple3->GetEntry(0);
  G4cout << "_posx.size " << _posx->at(0) << G4endl;     
 _posx->clear();
 _posy->clear();
 _posz->clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HistoManager::PrintStatistic()
{
  if(fHisto[1]) {
    G4cout << "\n ----> print histograms statistic \n" << G4endl;
    
    G4cout 
    << " EAbs : mean = " << G4BestUnit(fHisto[1]->GetMean(), "Energy") 
            << " rms = " << G4BestUnit(fHisto[1]->GetRMS(),  "Energy") << G4endl;
    G4cout                
    << " EGap : mean = " << G4BestUnit(fHisto[2]->GetMean(), "Energy") 
            << " rms = " << G4BestUnit(fHisto[2]->GetRMS(),  "Energy") << G4endl;
    G4cout 
    << " LAbs : mean = " << G4BestUnit(fHisto[3]->GetMean(), "Length") 
            << " rms = " << G4BestUnit(fHisto[3]->GetRMS(),  "Length") << G4endl;
    G4cout 
    << " LGap : mean = " << G4BestUnit(fHisto[4]->GetMean(), "Length") 
            << " rms = " << G4BestUnit(fHisto[4]->GetRMS(),  "Length") << G4endl;

  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


