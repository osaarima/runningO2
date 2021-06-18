double GetPhi(double x, double y);

void FillFV0Hits(TString sOutName = "fv0-map.root", TString sInName = "o2sim_HitsFV0.root")
{

    TFile *fOut = TFile::Open(sOutName, "RECREATE");
    TH2D *hHitMap = new TH2D("hHitMap", "hHitMap", 250, -80.0, 80.0, 250, -80.0, 80.0);

    TFile *fIn = TFile::Open(sInName);
    TTree *hitsTree = (TTree*)fIn->Get("o2sim");

    std::vector<o2::fv0::Hit> fv0hits, *fv0hitsPtr = &fv0hits;
    hitsTree->SetBranchAddress("FV0Hit", &fv0hitsPtr);

    UInt_t nEntries = hitsTree->GetEntries();
    for (UInt_t ient = 0; ient < nEntries; ient++) {
        hitsTree->GetEntry(ient);

        int nhits = fv0hits.size();
        for (int ihit = 0; ihit < nhits; ihit++) {
            o2::fv0::Hit* hit = &(fv0hits.at(ihit));

            int idet = hit->GetDetectorID();
            double x = hit->GetStartX();
            double y = hit->GetStartY();
            hHitMap->Fill(x, y);
        }
    }

    fIn->Close();

    fOut->cd();
    fOut->Write("", TObject::kOverwrite);
    fOut->Close();
}

double GetPhi(double x, double y)
{
    return TMath::ATan2(y,x);
}

