/// \file
/// \ingroup tutorial_eve
/// Demonstrates usage of class TEvePointSet.
///
/// \image html eve_pointset.png
/// \macro_code
///
/// \author Matevz Tadel

#include <TEveManager.h>
#include <TEvePointSet.h>
#include <TEveRGBAPalette.h>
#include <TColor.h>
#include <TRandom.h>
#include <TMath.h>

TEvePointSet *pointset(Int_t npoints = 512, TEveElement *parent = nullptr)
{
   TEveManager::Create();

   if (!gRandom)
      gRandom = new TRandom(0);
   TRandom &r = *gRandom;

   Float_t s = 100;

   auto ps = new TEvePointSet();
   ps->SetOwnIds(kTRUE);

   for (Int_t i = 0; i < npoints; i++) {
      ps->SetNextPoint(r.Uniform(-s, s), r.Uniform(-s, s), r.Uniform(-s, s));
      ps->SetPointId(new TNamed(Form("Point %d", i), ""));
   }

   ps->SetMarkerColor(TMath::Nint(r.Uniform(2, 9)));
   ps->SetMarkerSize(r.Uniform(1, 2));
   ps->SetMarkerStyle(4);

   if (parent) {
      parent->AddElement(ps);
   } else {
      gEve->AddElement(ps);
      gEve->Redraw3D();
   }

   return ps;
}

TEvePointSet *
pointset_hierarchy(Int_t level = 3, Int_t nps = 1, Int_t fac = 2, Int_t npoints = 512, TEveElement *parent = nullptr)
{
   // This only works in compiled mode!

   TEvePointSet *ps = nullptr;
   --level;
   for (Int_t i = 0; i < nps; ++i) {
      printf("level=%d nps=%d i=%d\n", level, nps, i);
      ps = pointset(npoints, parent);
      if (level) {
         pointset_hierarchy(level, nps * fac, fac, npoints / fac, ps);
      }
   }
   return ps;
}

TEvePointSetArray *pointsetarray()
{
   TEveManager::Create();

   TRandom r(0);

   auto l = new TEvePointSetArray("TPC hits - Charge Slices", "");
   l->SetSourceCS(TEvePointSelectorConsumer::kTVT_RPhiZ);
   l->SetMarkerColor(3);
   l->SetMarkerStyle(4); // antialiased circle
   l->SetMarkerSize(0.8);

   gEve->AddElement(l);
   l->InitBins("Charge", 9, 10, 100);

   TColor::SetPalette(1, nullptr); // Spectrum palette
   const Int_t nCol = TColor::GetNumberOfColors();
   for (Int_t i = 1; i <= 9; ++i)
      l->GetBin(i)->SetMainColor(TColor::GetColorPalette(i * nCol / 10));

   l->GetBin(0)->SetMainColor(kGray);
   l->GetBin(10)->SetMainColor(kWhite);

   Double_t rad, phi, z;
   for (Int_t i = 0; i < 10000; ++i) {
      rad = r.Uniform(60, 180);
      phi = r.Uniform(0, TMath::TwoPi());
      z = r.Uniform(-250, 250);
      l->Fill(rad * TMath::Cos(phi), rad * TMath::Sin(phi), z, r.Uniform(0, 110));
   }

   l->CloseBins();

   gEve->Redraw3D();

   return l;
}
