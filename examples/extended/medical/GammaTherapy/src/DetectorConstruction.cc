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
// $Id: DetectorConstruction.cc 101905 2016-12-07 11:34:39Z gunter $
//
/// \file medical/GammaTherapy/src/DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class
//
//
// -------------------------------------------------------------
//      GEANT4 ibrem test
//
// Authors: V.Grichine, V.Ivanchenko
//
// Modified:
//
// 18-02-03 V.Ivanchenko create
//
// -------------------------------------------------------------

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
#include "PhantomSD.hh"
#include "TargetSD.hh"
#include "CheckVolumeSD.hh"
#include "Histo.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Material.hh"
#include "G4SDManager.hh"
#include "PhantomSD.hh"
#include "G4NistManager.hh"

#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4RunManager.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "globals.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
{
  fLogicTarget1 = 0;
  fLogicTarget2 = 0;

  fMessenger = new DetectorMessenger(this);

  fCheckSD = new CheckVolumeSD("checkSD");
  (G4SDManager::GetSDMpointer())->AddNewDetector( fCheckSD );
  fPhantomSD = new PhantomSD("phantomSD");
  (G4SDManager::GetSDMpointer())->AddNewDetector( fPhantomSD );
  fTargetSD = new TargetSD("targetSD");
  (G4SDManager::GetSDMpointer())->AddNewDetector( fTargetSD );

  fDistanceVacuumTarget = 30.*mm,

  fDelta                = 0.001*mm;

  fTargetRadius         = 100.*mm;
  fTarget1Z             = 9.*mm;
  fTarget2Z             = 6.*mm;

  fGasVolumeRadius      = 210.*mm;
  fGasVolumeZ           = 690.*mm;
  fMylarVolumeZ         = 0.02*mm;

  fCheckVolumeZ         = 0.1*mm;
  fCheckShiftZ          = 200.*mm;

  fAbsorberRadius       = 200.*mm;
  fPhantomRadius        = 300.*mm;
  fPhantomZ             = 300.*mm;

  fAirZ                 = 210.*mm;
  fAbsorberShiftZ       = 70.*mm;
  fWindowZ              = 0.05*mm;

  G4NistManager* man = G4NistManager::Instance();
  //man->SetVerbose(1);

  fTarget1Material = man->FindOrBuildMaterial("G4_Be");
  fWindowMaterial  = fTarget1Material;
  fTarget2Material = man->FindOrBuildMaterial("G4_W");
  fLightMaterial   = man->FindOrBuildMaterial("G4_He");
  fAbsorberMaterial= man->FindOrBuildMaterial("G4_WATER");
  fWorldMaterial   = man->FindOrBuildMaterial("G4_AIR");
  fMylar           = man->FindOrBuildMaterial("G4_MYLAR");

  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::InitialiseGeometryParameters()
{
  // Volumee sizes

  G4double factor = 1.2;

  fWorldXY       = factor*std::max(fPhantomRadius,fGasVolumeRadius);
  G4double nz    = (G4int)((Histo::GetPointer())->GetNumberDivZ());
  fAbsorberZ     = fPhantomZ/nz;
  fGasVolumeZ    = 1000.*mm - fAbsorberShiftZ - fAirZ - fTarget1Z - fTarget2Z;

  G4double ztot  = fGasVolumeZ + fAirZ + fPhantomZ + fDistanceVacuumTarget;
  fTargetVolumeZ = fDistanceVacuumTarget + fTarget2Z + fTarget1Z + fDelta;
  fWorldZ  = factor*ztot*0.5;

  if(fCheckShiftZ < fDelta) { fCheckShiftZ = fDelta; }
  if(fCheckShiftZ > fAirZ - fCheckVolumeZ -fDelta) {
    fCheckShiftZ = fAirZ - fCheckVolumeZ -fDelta;
  }

  // Z position of volumes from upstream to downstream

  fWindowPosZ      =  -(ztot + fWindowZ)*0.5;
  fGeneratorPosZ   =  fWindowPosZ - 0.5*fWindowZ - fDelta;

  fTargetVolumePosZ=  -0.5*(ztot - fTargetVolumeZ);
  fTarget1PosZ     =  -0.5*(fTargetVolumeZ - fTarget1Z) + fDistanceVacuumTarget;
  fTarget2PosZ     =  fTarget1PosZ + 0.5*(fTarget2Z + fTarget1Z);

  fGasVolumePosZ   =  fTargetVolumePosZ + 0.5*(fTargetVolumeZ + fGasVolumeZ);
  fCheckVolumePosZ =  fGasVolumePosZ + 0.5*(fGasVolumeZ + fCheckVolumeZ)
                                +  fCheckShiftZ;
  fMylarPosZ =  fGasVolumePosZ + 0.5*(fGasVolumeZ + fMylarVolumeZ) + fDelta;

  fPhantomPosZ     =  fGasVolumePosZ + 0.5*(fGasVolumeZ + fPhantomZ) + fAirZ;
  fAbsorberPosZ    =  fAbsorberShiftZ - 0.5*(fPhantomZ - fAbsorberZ);
  (Histo::GetPointer())->SetAbsorberZ(fPhantomZ);
  (Histo::GetPointer())->SetAbsorberR(fAbsorberRadius);
  (Histo::GetPointer())->SetScoreZ(fAbsorberShiftZ);
  G4double shiftZPh = fPhantomPosZ-0.5*fPhantomZ;
  fPhantomSD->SetShiftZ(shiftZPh);

  G4cout << "===================================================" << G4endl;
  G4cout << "#           GammaTherapy Geometry                 #" << G4endl;
  G4cout << "===================================================" << G4endl;
  G4cout << "  World   width= " << fWorldZ/mm << " mm " << G4endl;
  G4cout << "  Window  width= " << fWindowZ/mm << " mm    position = "
                                << fWindowPosZ/mm << " mm:" << G4endl;
  G4cout << "  TargetV width= " << fTargetVolumeZ/mm << " mm  position = "
                                << fTargetVolumePosZ/mm << " mm:" << G4endl;
  G4cout << "  Target1 width= " << fTarget1Z/mm << " mm       position = "
                                << fTarget1PosZ/mm << " mm:" << G4endl;
  G4cout << "  Target2 width= " << fTarget2Z/mm << " mm       position = "
                                << fTarget2PosZ/mm << " mm:" << G4endl;
  G4cout << "  Gas     width= " << fGasVolumeZ/mm << " mm     position = "
                                << fGasVolumePosZ/mm << " mm:" << G4endl;
  G4cout << "  Mylar   width= " << fMylarVolumeZ/mm << " mm    position = "
                                << fMylarPosZ/mm << " mm:" << G4endl;
  G4cout << "  Check   width= " << fCheckVolumeZ/mm << " mm     position = "
                                << fCheckVolumePosZ/mm << " mm:" << G4endl;
  G4cout << "  Air     width= " << fAirZ/mm << " mm " << G4endl;
  G4cout << "  Phantom width= " << fPhantomZ/mm << " mm     position = "
                                << fPhantomPosZ/mm << " mm:" << G4endl;
  G4cout << "  Absorb  width= " << fAbsorberZ/mm << " mm       position = "
                                << fAbsorberPosZ/mm << " mm:" << G4endl;
  G4cout << "  Absorb  shift= " << shiftZPh/mm << " mm " << G4endl;
  G4cout << "  Target1        " << fTarget1Material->GetName() << G4endl;
  G4cout << "  Target2        " << fTarget2Material->GetName() << G4endl;
  G4cout << "  Phantom        " << fAbsorberMaterial->GetName() << G4endl;
  G4cout << "===================================================" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  InitialiseGeometryParameters();

  //
  // World
  //
  G4VPhysicalVolume* pv;

  G4Box* solidWorld = new G4Box("World",fWorldXY,fWorldXY,fWorldZ);
  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld,
                                                    fWorldMaterial,"World");
  G4VPhysicalVolume* physWorld = new G4PVPlacement(0,G4ThreeVector(),"World",
                                                   logicWorld,0,false,0);

  // Be Vacuum window
  G4Tubs* solidWin = new G4Tubs("Window",0.,fTargetRadius*0.25,0.5*fWindowZ,
                                0.,twopi);
  G4LogicalVolume* logicWin = new G4LogicalVolume(solidWin,
                                                  fWindowMaterial,"Window");
  pv = new G4PVPlacement(0,G4ThreeVector(0.,0.,fWindowPosZ),"Window",logicWin,
                         physWorld,false,0);

  // Target Volume
  G4Tubs* solidTGVolume = new G4Tubs("TargetVolume",0.,fTargetRadius,
                                     0.5*fTargetVolumeZ,0.,twopi);
  G4LogicalVolume* logicTGVolume = new G4LogicalVolume(solidTGVolume,
                                                       fLightMaterial,
                                                       "TargetVolume");
  pv = new G4PVPlacement(0,G4ThreeVector(0.,0.,fTargetVolumePosZ),
                         logicTGVolume,"TargetVolume",
                         logicWorld,false,0);

  // Target 1
  G4Tubs* solidTarget1 = new G4Tubs("Target1",0.,fTargetRadius*0.5,
                                    0.5*fTarget1Z,0.,twopi);
  fLogicTarget1 = new G4LogicalVolume(solidTarget1,fTarget1Material,"Target1");
  pv = new G4PVPlacement(0,G4ThreeVector(0.,0.,fTarget1PosZ),
                         fLogicTarget1,"Target1",
                         logicTGVolume,false,0);
  (Histo::GetPointer())->SetTarget1(pv);
  fLogicTarget1->SetSensitiveDetector(fTargetSD);

  // Target 2 (for combined targets)
  G4Tubs* solidTarget2 = new G4Tubs("Target2",0.,fTargetRadius*0.5,
                                    0.5*fTarget2Z,0.,twopi);
  fLogicTarget2 = new G4LogicalVolume(solidTarget2,fTarget2Material,"Target2");
  pv = new G4PVPlacement(0,G4ThreeVector(0.,0.,fTarget2PosZ),
                         fLogicTarget2,"Target2",
                         logicTGVolume,false,0);

  (Histo::GetPointer())->SetTarget2(pv);
  fLogicTarget2->SetSensitiveDetector(fTargetSD);

  // Gas Volume
  G4Tubs* solidGasVolume = new G4Tubs("GasVolume",0.,fGasVolumeRadius,
                                      0.5*fGasVolumeZ,0.,twopi);
  G4LogicalVolume* logicGasVolume = new G4LogicalVolume(solidGasVolume,
                                                        fLightMaterial,
                                                        "GasVolume");
  pv = new G4PVPlacement(0,G4ThreeVector(0.,0.,fGasVolumePosZ),
                         "GasVolume",logicGasVolume,
                         physWorld,false,0);
  (Histo::GetPointer())->SetGasVolume(pv);

  // Mylar window
  G4Tubs* sMylarVolume = new G4Tubs("Mylar",0.,fGasVolumeRadius,
                                    0.5*fMylarVolumeZ,0.,twopi);
  G4LogicalVolume* lMylarVolume = new G4LogicalVolume(sMylarVolume,
                                                      fMylar,"Mylar");
  pv = new G4PVPlacement(0,G4ThreeVector(0.,0.,fMylarPosZ),"Mylar",lMylarVolume,
                         physWorld,false,0);

  // Check Volume
  G4Tubs* solidCheckVolume = new G4Tubs("CheckVolume",0.,fGasVolumeRadius,
                                        0.5*fCheckVolumeZ,0.,twopi);
  G4LogicalVolume* logicCheckVolume = new G4LogicalVolume(solidCheckVolume,
                                                          fWorldMaterial,
                                                          "CheckVolume");
  pv = new G4PVPlacement(0,G4ThreeVector(0.,0.,fCheckVolumePosZ),
                         "CheckVolume",logicCheckVolume,
                         physWorld,false,0);
  (Histo::GetPointer())->SetCheckVolume(pv);
  logicCheckVolume->SetSensitiveDetector(fCheckSD);

  // Phantom
  G4Box* solidPhantom = new G4Box("Phantom",fPhantomRadius,fPhantomRadius,
                                  0.5*fPhantomZ);
  G4LogicalVolume* logicPhantom = new G4LogicalVolume(solidPhantom,
                                                      fAbsorberMaterial,
                                                      "Phantom");
  G4VPhysicalVolume* physPhantom = 
    new G4PVPlacement(0, G4ThreeVector(0.,0.,fPhantomPosZ),
                      "Phantom",logicPhantom,
                      physWorld,false,0);

  G4Tubs* solidPh = new G4Tubs("PhantomSD",0.,fAbsorberRadius,
                               0.5*fPhantomZ,0.,twopi);
  G4LogicalVolume* logicPh = new G4LogicalVolume(solidPh,
                                                 fAbsorberMaterial,"PhantomSD");
  G4VPhysicalVolume* physPh = new G4PVPlacement(0,G4ThreeVector(0.,0.,0.),
                                                "Phantom",logicPh,
                                                physPhantom,false,0);
  G4cout << "Phantom R= " << fAbsorberRadius << " dz= " << 0.5*fPhantomZ 
         << G4endl;

  // Sensitive Absorber
  G4double absWidth = 0.5*fAbsorberZ;
  G4Tubs* solidAbsorber = new G4Tubs("Absorber",0.,fAbsorberRadius,absWidth,
                                     0.,twopi);
  G4LogicalVolume* logicAbsorber = new G4LogicalVolume(solidAbsorber,
                                                       fAbsorberMaterial,
                                                       "Absorber");
  G4cout << "Absorber R= " << fAbsorberRadius << " dz= " << absWidth 
         << " posZ= " << fAbsorberPosZ<< G4endl;

  pv = new G4PVPlacement(0,G4ThreeVector(0.,0.,fAbsorberPosZ),"Absorber",
                         logicAbsorber,physPh,false,0);
  (Histo::GetPointer())->SetPhantom(physPh);
  G4int numR = (Histo::GetPointer())->GetNumberDivR();
  G4double stepR = fAbsorberRadius/(G4double)numR;

  G4double r1 = 0.0;
  G4double r2 = 0.0;
  G4Tubs* solidRing;
  G4LogicalVolume* logicRing;

  for(G4int k=0; k<numR; k++) {
    r2 = r1 + stepR;
    if(k == numR-1) r2 = fAbsorberRadius;
    //    G4cout << "New ring r1= " << r1 << " r2= " << r2 
    //  << " dz= " << absWidth << G4endl;
    solidRing = new G4Tubs("Ring",r1,r2,absWidth,0.,twopi);
    logicRing = new G4LogicalVolume(solidRing,fAbsorberMaterial,"Ring");
    logicRing->SetSensitiveDetector(fPhantomSD);
    logicRing->SetVisAttributes(G4VisAttributes::GetInvisible());
    pv = new G4PVPlacement(0,G4ThreeVector(0.,0.,0.),logicRing,"Ring",
                           logicAbsorber,false,k);
    r1 = r2;
  }
  //
  // Sensitive Detectors: Absorber
  //
  logicPh->SetSensitiveDetector(fPhantomSD);
  logicAbsorber->SetSensitiveDetector(fPhantomSD);
  //
  // Visualization attributes
  //
  G4VisAttributes* VisAtt = 0;
  VisAtt = new G4VisAttributes(G4Colour(1.0,1.0,1.0));
  VisAtt->SetVisibility(true);
  logicAbsorber->SetVisAttributes(VisAtt);

  VisAtt= new G4VisAttributes(G4Colour(1.0,1.0,2.0));
  VisAtt->SetVisibility(true);
  logicPhantom->SetVisAttributes(VisAtt);

  VisAtt= new G4VisAttributes(G4Colour(1.0,0.0,2.0));
  VisAtt->SetVisibility(true);
  logicPh->SetVisAttributes(VisAtt);

  VisAtt= new G4VisAttributes(G4Colour(1.0,1.0,0.0));
  VisAtt->SetVisibility(true);
  logicAbsorber->SetVisAttributes(VisAtt);

  VisAtt= new G4VisAttributes(G4Colour(0.1,1.0,2.0));
  VisAtt->SetVisibility(true);
  logicWorld->SetVisAttributes(VisAtt);

  VisAtt= new G4VisAttributes(G4Colour(1.0,1.0,0.0));
  VisAtt->SetVisibility(true);
  logicGasVolume->SetVisAttributes(VisAtt);

  VisAtt= new G4VisAttributes(G4Colour(0.0,0.5,1.0));
  VisAtt->SetVisibility(true);
  fLogicTarget1->SetVisAttributes(VisAtt);
  fLogicTarget2->SetVisAttributes(VisAtt);
  logicTGVolume->SetVisAttributes(VisAtt);

  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetTarget1Material(const G4String& mat)
{
  // search the material by its name
  G4Material* pttoMaterial = 
    G4NistManager::Instance()->FindOrBuildMaterial(mat);
  if(!pttoMaterial) {
    G4cout << "Material " << mat << " is not found out!" << G4endl;
  } else if (pttoMaterial != fTarget1Material) {
    G4cout << "New target1 material " << mat << G4endl;
    if(fLogicTarget1) { fLogicTarget1->SetMaterial(fTarget1Material); }
    G4RunManager::GetRunManager()->PhysicsHasBeenModified();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetTarget2Material(const G4String& mat)
{
  // search the material by its name
  G4Material* pttoMaterial = 
    G4NistManager::Instance()->FindOrBuildMaterial(mat);

  if(!pttoMaterial) {
    G4cout << "Material " << mat << " is not found out!" << G4endl;
  } else if (pttoMaterial != fTarget2Material) {
    fTarget2Material = pttoMaterial;
    G4cout << "New target2 material " << mat << G4endl;
    if(fLogicTarget2) { fLogicTarget2->SetMaterial(fTarget2Material); }
    G4RunManager::GetRunManager()->PhysicsHasBeenModified();
  } 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
