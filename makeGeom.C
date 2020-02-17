void makeGeom()
{
  //--- Definition of a simple geometry
  //   gSystem->Load("libGeom");
   new TGeoManager("genfitGeom", "GENFIT geometry");


   unsigned int medInd(0);
   Double_t mPar[10];
   //TAG sollte wieder 0 werden sens flag
   mPar[0]=0.;//sensitive volume flag
   mPar[1]=1.;//magnetic field flag
   mPar[2]=40.;//max fiel in kGauss
   mPar[3]=0.1;//maximal angular dev. due to field
   mPar[4]=0.01;//max step allowed (in cm)
   mPar[5]=1.e-5;//max fractional energy loss
   mPar[6]=1.e-3;//boundary crossing accuracy
   mPar[7]=1.e-5;//minimum step
   mPar[8]=0.;//not defined
   mPar[9]=0.;//not defined
   
   TGeoMaterial *_ironMat = new TGeoMaterial("ironMat",55.845,26,7.874);
   _ironMat->SetRadLen(13.84);//calc automatically, need this for elemental mats.
   TGeoMedium *_iron = new TGeoMedium("iron",medInd++,_ironMat,mPar);

   TGeoMixture *polyMat = new TGeoMixture("polypropylene",2);
   polyMat->AddElement(12.01,6.,.34);
   polyMat->AddElement(1.01,1.,.66);
   polyMat->SetDensity(0.905);
   polyMat->SetRadLen(44.77);
   TGeoMedium *_poly = new TGeoMedium("poly",medInd++,polyMat,mPar);

   TGeoMixture *_airMat = new TGeoMixture("airMat",3);
   _airMat->AddElement(14.01,7.,.78);
   _airMat->AddElement(16.00,8.,.21);
   _airMat->AddElement(39.95,18.,.01);
   _airMat->SetDensity(1.2e-3);
   _airMat->SetRadLen(3.039e+4);
   TGeoMedium *_air = new TGeoMedium("air",medInd++,_airMat,mPar);

   TGeoMixture *_vacuumMat = new TGeoMixture("vacuumMat",3);
   _vacuumMat->AddElement(14.01,7.,.78);
   _vacuumMat->AddElement(16.00,8.,.21);
   _vacuumMat->AddElement(39.95,18.,.01);
   _vacuumMat->SetDensity(1.2e-15);
   TGeoMedium *_vacuum = new TGeoMedium("vacuum",medInd++,_vacuumMat,mPar);




   TGeoMedium *vacuum = gGeoManager->GetMedium("vacuum");
   assert(vacuum!=NULL);
   TGeoMedium *air = gGeoManager->GetMedium("air");
   assert(air!=NULL);
   TGeoMedium *poly = gGeoManager->GetMedium("poly");
   assert(sil!=NULL);
   TGeoMedium *iron = gGeoManager->GetMedium("iron");
   assert(sil!=NULL);

   TGeoVolume *top = gGeoManager->MakeBox("TOPPER", air, 10000., 10000., 10000.);
   gGeoManager->SetTopVolume(top); // mandatory !
   top->SetTransparency(50);

   double thickness(1);
   double distance = 24;

   for (int i=0; i<20; ++i){
     TGeoVolume *redBullCan = gGeoManager->MakeBox("redBullCan", poly, thickness, 100, 100);//, 90., 270.);
     redBullCan->SetLineColor(kRed);
     top->AddNode(redBullCan, i, new TGeoTranslation(-239+(distance*i),0,0));
   }
   for (int i=0; i<20; ++i){
     TGeoVolume *blueBullCan = gGeoManager->MakeBox("blueBullCan", iron, thickness, 100, 100);//, 90., 270.);
     blueBullCan->SetLineColor(kBlue);
     top->AddNode(blueBullCan, i, new TGeoTranslation(-217+(distance*i),0,0));
   }


   //--- close the geometry
   gGeoManager->CloseGeometry();

   //--- draw the ROOT box
   gGeoManager->SetVisLevel(10);
   top->Draw("ogl");
   TFile *outfile = TFile::Open("genfitGeom.root","RECREATE");
   gGeoManager->Write();
   outfile->Close();
}
