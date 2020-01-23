#include <ConstField.h>
#include <Exception.h>
#include <FieldManager.h>
#include <KalmanFitterRefTrack.h>
#include <StateOnPlane.h>
#include <Track.h>
#include <TrackCand.h>
#include <TrackPoint.h>

#include <MeasurementProducer.h>
#include <MeasurementFactory.h>

#include "mySpacepointDetectorHit.h"
#include "mySpacepointMeasurement.h"

#include <MaterialEffects.h>
#include <RKTrackRep.h>
#include <TGeoMaterialInterface.h>

#include <EventDisplay.h>

#include <HelixTrackModel.h>
#include <MeasurementCreator.h>

#include <TDatabasePDG.h>
#include <TEveManager.h>
#include <TGeoManager.h>
#include <TRandom.h>
#include <TVector3.h>
#include <vector>

#include "TDatabasePDG.h"
#include <TMath.h>
#include "TCanvas.h"
#include "TH1D.h"
#include <TFile.h>
#include <TTree.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>   // std::cout
#include <string>     // std::string, std::to_string


int main() {

  gRandom->SetSeed(14);

  // init MeasurementCreator
  genfit::MeasurementCreator measurementCreator;


  // init geometry and mag. field
  new TGeoManager("Geometry", "Geane geometry");
  TGeoManager::Import("genfitGeom.root");
  genfit::FieldManager::getInstance()->init(new genfit::ConstField(0.,0., 0)); // 15 kGauss
  genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());


  // init event display
  genfit::EventDisplay* display = genfit::EventDisplay::getInstance();


  // init fitter
  genfit::AbsKalmanFitter* fitter = new genfit::KalmanFitterRefTrack();


  double posX, posY, posZ;
  std::string hold = "b";
  std::string hold2 = "r";
  float zzz = -240;
  while (zzz<240){
      std::string name3(1, gGeoManager->FindNode(zzz,0,0)->GetName()[0]);
      if (name3.c_str()==hold || name3.c_str()==hold2){
          std::cout << zzz << std::endl;
          std::cout << name3.c_str() << std::endl;

      }
      zzz += 24;
  }
  zzz = -218;
  while (zzz<240){
      std::string name3(1, gGeoManager->FindNode(zzz,0,0)->GetName()[0]);
      if (name3.c_str()==hold || name3.c_str()==hold2){
          std::cout << zzz << std::endl;
          std::cout << name3.c_str() << std::endl;

      }
      zzz += 24;
  }
  TClonesArray myDetectorHitArray("genfit::mySpacepointDetectorHit");

  // init the factory
  int myDetId(1);
  genfit::MeasurementFactory<genfit::AbsMeasurement> factory;
  genfit::MeasurementProducer<genfit::mySpacepointDetectorHit, genfit::mySpacepointMeasurement> myProducer(&myDetectorHitArray);
  factory.addProducer(myDetId, &myProducer);


  TH1D* residual = new TH1D("residual", "residual", 99, -1, 1);
  TFile *inFile=new TFile("AnaEx02.root","READONLY");
  TTree *tree=(TTree*)inFile->Get("103");
  std::vector<float> *_posx = new std::vector<float>;
  std::vector<float> *_posy = new std::vector<float>;
  std::vector<float> *_posz = new std::vector<float>;
  tree->SetBranchAddress("_posx",&_posx);
  tree->SetBranchAddress("_posy",&_posy);
  tree->SetBranchAddress("_posz",&_posz);
  // main loop
  for (unsigned int iEvent=0; iEvent<1000; ++iEvent){

      myDetectorHitArray.Clear();

      //TrackCand
      genfit::TrackCand myCand;

      // true start values
      TVector3 mom(2.,0,0);
      tree->GetEntry(iEvent);
      if (_posx->size()==0 || _posy->size()==0 || _posz->size()==0 ) continue;
      TVector3 pos(_posx->at(0)/10, _posy->at(0)/10, _posz->at(0)/10);


      // helix track model
      const int pdg = 13;               // particle pdg code
      const double charge = TDatabasePDG::Instance()->GetParticle(pdg)->Charge()/(3.);
      genfit::HelixTrackModel* helix = new genfit::HelixTrackModel(pos, mom, charge);
      measurementCreator.setTrackModel(helix);


      unsigned int nMeasurements = 20; //gRandom->Uniform(5, 15);

      // covariance
      double resolution = 0.001;
      TMatrixDSym cov(3);
      for (int i = 0; i < 3; ++i)
          cov(i,i) = resolution*resolution;

      int size; 
      size = _posx->size();
      std::cout << "size " << size << std::endl;
      if (size>5){
          for (unsigned int i=0; i<size; ++i) {
              // "simulate" the detector
              float diff = 1;
              TVector3 currentPoshelix;
              TVector3 currentPos;
              float X = i*22; 
              std::cout << "start pos geant " << _posx->at(i)/10 << " " << _posy->at(i)/10 << " " << _posz->at(i)/10 << std::endl;
              currentPoshelix = helix->getPos(X);
              if (i==0) {
                  currentPos.SetX(_posx->at(i)/10);
                  currentPos.SetY(_posy->at(i)/10);
                  currentPos.SetZ(_posz->at(i)/10);
              }
              else {
                  currentPos.SetX((_posx->at(i)/10));
                  currentPos.SetY(_posy->at(i)/10);
                  currentPos.SetZ(_posz->at(i)/10);
              }
              /* currentPos.SetX(gRandom->Gaus(currentPos.X(), resolution)); */
              /* currentPos.SetY(gRandom->Gaus(currentPos.Y(), resolution)); */
              /* currentPos.SetZ(gRandom->Gaus(currentPos.Z(), resolution)); */

              // Fill the TClonesArray and the TrackCand
              // In a real experiment, you detector code would deliver mySpacepointDetectorHits and fill the TClonesArray.
              // The patternRecognition would create the TrackCand.
              new(myDetectorHitArray[i]) genfit::mySpacepointDetectorHit(currentPos, cov);
              myCand.addHit(myDetId, i);
          }


          // smeared start values (would come from the pattern recognition)
          const bool smearPosMom = true;   // init the Reps with smeared pos and mom
          const double posSmear = 0.1;     // cm
          const double momSmear = 3. /180.*TMath::Pi();     // rad
          const double momMagSmear = 0.1;   // relative

          TVector3 posM(pos);
          TVector3 momM(mom);
          if (smearPosMom) {
              /* posM.SetX(gRandom->Gaus(posM.X(),posSmear)); */
              /* posM.SetY(gRandom->Gaus(posM.Y(),posSmear)); */
              /* posM.SetZ(gRandom->Gaus(posM.Z(),posSmear)); */

              /* momM.SetPhi(gRandom->Gaus(mom.Phi(),momSmear)); */
              /* momM.SetTheta(gRandom->Gaus(mom.Theta(),momSmear)); */
              /* momM.SetMag(gRandom->Gaus(mom.Mag(), momMagSmear*mom.Mag())); */
          }

          // initial guess for cov
          TMatrixDSym covSeed(6);
          for (int i = 0; i < 3; ++i)
              covSeed(i,i) = resolution*resolution;
          for (int i = 3; i < 6; ++i)
              covSeed(i,i) = pow(resolution / nMeasurements / sqrt(3), 2);


          // set start values and pdg to cand
          myCand.setPosMomSeedAndPdgCode(posM, momM, pdg);
          myCand.setCovSeed(covSeed);


          // create track
          genfit::Track fitTrack(myCand, factory, new genfit::RKTrackRep(pdg));


          // do the fit
          try{
              fitter->processTrack(&fitTrack);
          }
          catch(genfit::Exception& e){
              std::cerr << e.what();
              std::cerr << "Exception, next track" << std::endl;
              continue;
          }

          TVector3 pos2;
          TVector3 mom2;
          TMatrixDSym cov2;
          fitTrack.getFittedState().getPosMomCov(pos2,mom2,cov2);
          fitTrack.checkConsistency();
          fitTrack.getFittedState().Print();


          if (iEvent < 1000) {
              // add track to event display
              display->addEvent(&fitTrack);
          }

          float res = mom2[0]-2;
          residual->Fill(res);

      }
  }// end loop over events

  TCanvas* c1 = new TCanvas();
  residual->Draw();
  c1->SaveAs("residual.pdf");
  delete c1;
  delete fitter;


  // open event display
  display->open();

}

