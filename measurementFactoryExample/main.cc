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



int main() {

  gRandom->SetSeed(14);
  // init MeasurementCreator
  genfit::MeasurementCreator measurementCreator;


  // init geometry and mag. field
  new TGeoManager("Geometry", "Geant geometry");
  TGeoManager::Import("genfitGeom.root");
  genfit::FieldManager::getInstance()->init(new genfit::ConstField(0.,0., 20)); // 15 kGauss
  genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());


  // init event display
  genfit::EventDisplay* display = genfit::EventDisplay::getInstance();


  // init fitter
  genfit::AbsKalmanFitter* fitter = new genfit::KalmanFitterRefTrack();


  TClonesArray myDetectorHitArray("genfit::mySpacepointDetectorHit");

  // init the factory
  int myDetId(1);
  genfit::MeasurementFactory<genfit::AbsMeasurement> factory;
  genfit::MeasurementProducer<genfit::mySpacepointDetectorHit, genfit::mySpacepointMeasurement> myProducer(&myDetectorHitArray);
  factory.addProducer(myDetId, &myProducer);

  TH1D* residual = new TH1D("residual", "residual", 100, -.5, .5);
  TH1D* posresidual = new TH1D("posresidual", "posresidual", 100, -1000, 1000);
  TFile *inFile=new TFile("AnaEx02.root","READONLY");
  TTree *tree=(TTree*)inFile->Get("103");
  tree->Show(0);

  std::vector<float> *_posx = new std::vector<float>;
  std::vector<float> *_posy = new std::vector<float>;
  std::vector<float> *_posz = new std::vector<float>;
  /* Float_t *_posx = 0; */
  /* Float_t *_posy = 0; */
  /* Float_t *_posz = 0; */
  tree->SetBranchAddress("_posx",&_posx);
  tree->SetBranchAddress("_posy",&_posy);
  tree->SetBranchAddress("_posz",&_posz);
  tree->Print();
  // main loop
  for (unsigned int iEvent=0; iEvent<100; ++iEvent){

    myDetectorHitArray.Clear();

    //TrackCand
    genfit::TrackCand myCand;

    // true start values

    TVector3 mom(0.5,0,0);
    /* mom.SetPhi(0.); */
    /* mom.SetTheta(1.); */
    /* mom.SetMag(0.5); */
    std::cout << "true " << mom[0] << std::endl;

    tree->GetEntry(iEvent);
    TVector3 pos(_posx->at(0), _posy->at(0), _posz->at(0));
    /* TVector3 pos(50, -40, 10); */

    // helix track model
    const int pdg = 13;               // particle pdg code
    const double charge = TDatabasePDG::Instance()->GetParticle(pdg)->Charge()/(3.);
    genfit::HelixTrackModel* helix = new genfit::HelixTrackModel(pos, mom, charge);
    measurementCreator.setTrackModel(helix);


    unsigned int nMeasurements = gRandom->Uniform(5, 15);

    // covariance
    double resolution = 0.01;
    TMatrixDSym cov(3);
    for (int i = 0; i < 3; ++i)
      cov(i,i) = resolution*resolution;

        /* std::cout << "_posx->size() " << _posx->size() << std::endl; */ 
        /* std::cout << "_posy->size() " << _posy->size() << std::endl; */ 
        /* std::cout << "_posz->size() " << _posz->size() << std::endl; */ 
        /* std::cout << "Start Geant" << std::endl; */ 
    for(unsigned int i=0; i<_posz->size(); i++){  
        TVector3 currentPos;
        double posX = _posx->at(i);
        double posY = _posy->at(i);
        double posZ = _posz->at(i);
        /* std::cout << "posx " << posX << std::endl; */ 
        /* std::cout << "posy " << posY << std::endl; */ 
        /* std::cout << "posz " << posZ << std::endl; */ 
        currentPos.SetX(posX);
        currentPos.SetY(posY);
        currentPos.SetZ(posZ);
      new(myDetectorHitArray[i]) genfit::mySpacepointDetectorHit(currentPos, cov);
      myCand.addHit(myDetId, i);
    }
        /* std::cout << "End Geant" << std::endl; */ 
        /* std::cout << "sgfbsg" << std::endl; */ 
    /* for (unsigned int i=0; i<nMeasurements; ++i) { */
    /*   // "simulate" the detector */
    /*   TVector3 currentPos = helix->getPos(i*2.); */
    /*   currentPos.SetX(gRandom->Gaus(2*currentPos.X(), resolution)); */
    /*   currentPos.SetY(gRandom->Gaus(currentPos.Y(), resolution)); */
    /*   currentPos.SetZ(gRandom->Gaus(currentPos.Z(), resolution)); */
        /* std::cout << "posx " << currentPos.X() << std::endl; */ 
        /* std::cout << "posy " << currentPos.Y() << std::endl; */ 
        /* std::cout << "posz " << currentPos.Z() << std::endl; */ 

      // Fill the TClonesArray and the TrackCand
      // In a real experiment, you detector code would deliver mySpacepointDetectorHits and fill the TClonesArray.
      // The patternRecognition would create the TrackCand.
      /* new(myDetectorHitArray[i]) genfit::mySpacepointDetectorHit(currentPos, cov); */
      /* myCand.addHit(myDetId, i); */
    /* } */


    // smeared start values (would come from the pattern recognition)
    const bool smearPosMom = true;   // init the Reps with smeared pos and mom
    const double posSmear = 0.1;     // cm
    const double momSmear = 3. /180.*TMath::Pi();     // rad
    const double mo500MagSmear = 0.1;   // relative

    TVector3 posM(pos);
    TVector3 momM(mom);
    /* if (smearPosMom) { */
    /*   posM.SetX(gRandom->Gaus(posM.X(),posSmear)); */
    /*   posM.SetY(gRandom->Gaus(posM.Y(),posSmear)); */
    /*   posM.SetZ(gRandom->Gaus(posM.Z(),posSmear)); */

    /*   momM.SetPhi(gRandom->Gaus(mom.Phi(),momSmear)); */
    /*   momM.SetTheta(gRandom->Gaus(mom.Theta(),momSmear)); */
    /*   momM.SetMag(gRandom->Gaus(mom.Mag(), momMagSmear*mom.Mag())); */
    /* } */
    std::cout << "smear " << momM[0] << std::endl;

    // initial guess for cov
    TMatrixDSym covSeed(6);
    for (int i = 0; i < 3; ++i)
      covSeed(i,i) = resolution*resolution;
    for (int i = 3; i < 6; ++i)
      covSeed(i,i) = pow(resolution / _posz->size() / sqrt(3), 2);


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

    fitTrack.checkConsistency();

    TVector3 pos2;
    TVector3 mom2;
    TMatrixDSym cov2;
    fitTrack.getFittedState().getPosMomCov(pos2,mom2,cov2);
    std::cout << "kalman " << mom2[0] << std::endl;
    /* fitTrack.getPoint(0)->Print(); */

    if (iEvent < 1000) {
      // add track to event display
      display->addEvent(&fitTrack);
    }
    float res = mom2.Mag()-mom[0];
    float posres = pos2[0]-pos[0];
    std::cout << "res " << res << std::endl;
    residual->Fill(res);
    /* residual->Fill(mom2[0]); */
    posresidual->Fill(posres);


  }// end loop over events

  TCanvas* c1 = new TCanvas();
  residual->Draw();
  c1->SaveAs("residual.pdf");
  c1->Clear();
  posresidual->Draw();
  c1->SaveAs("posresidual.pdf");
  delete c1;
  delete fitter;

  // open event display
  display->setOptions("P");
  display->open();

}


