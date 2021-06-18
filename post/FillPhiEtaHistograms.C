void FillPhiEtaHistograms(TString sOutput="inclusive.root", TString name = "5p5TeV_midcent", bool bOnlyPrimaries = 0, bool bOnlySecondaries = 0, TString sDirName="/mnt/o2-data-5500GeV-v3")
{
    TStopwatch timer;
    timer.Start();

    TFile *fOut = TFile::Open(sOutput, "RECREATE");
   
    TH1D *hCounter = new TH1D("hCounter", "hCounter", 5, 0.5, 5.5);

    TH1D *hPhiFV0 = new TH1D("hPhiFV0", "hPhiFV0", 401, -TMath::Pi(), TMath::Pi());
    hPhiFV0->Sumw2();
    TH1D *hPhiFT0A = new TH1D("hPhiFT0A", "hPhiFT0A", 401, -TMath::Pi(), TMath::Pi());
    hPhiFT0A->Sumw2();
    TH1D *hPhiFT0C = new TH1D("hPhiFT0C", "hPhiFT0C", 401, -TMath::Pi(), TMath::Pi());
    hPhiFT0C->Sumw2();
    
    TH1D *hEtaFV0 = new TH1D("hEtaFV0", "hEtaFV0", 401, 1.5, 5.5);
    hEtaFV0->Sumw2();
    TH1D *hEtaFT0A = new TH1D("hEtaFT0A", "hEtaFT0A", 401, 3.3, 5.5);
    hEtaFT0A->Sumw2();
    TH1D *hEtaFT0C = new TH1D("hEtaFT0C", "hEtaFT0C", 401, -3.5, -1.9);
    hEtaFT0C->Sumw2();

    TH1D *hVrtxFV0 = new TH1D("hVrtxFV0", "hVrtxFV0", 401, -1000.0, 1000.0);
    hVrtxFV0->Sumw2();
    TH1D *hVrtxFT0A = new TH1D("hVrtxFT0A", "hVrtxFT0A", 401, -1000.0, 1000.0);
    hVrtxFT0A->Sumw2();
    TH1D *hVrtxFT0C = new TH1D("hVrtxFT0C", "hVrtxFT0C", 401, -1000.0, 1000.0);
    hVrtxFT0C->Sumw2();

    //TString cmd(Form("ls %s/cent%s*/run_cent%s*/extrafiles/o2sim_Kine_nockov.root", sDirName.Data(), cent.Data(), cent.Data()));
    TString cmd(Form("ls %s/sim/run_job*/o2sim_Kine_nockov.root", sDirName.Data()));

    //alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -x -l -q "/projappl/project_2003583/simO2/post_analysis/FillPhiEtaHistograms.C(\"/scratch/project_2003583/simO2_outputs/PbPb_o2ver-21-06-10_MBPythia8hi/phiEtaInclusiveHistos.root\", \"\",0,0,\"/scratch/project_2003583/simO2_outputs/PbPb_o2ver-21-06-10_MBPythia8hi\")";

    std::vector<TString> fileNames;

    TString tok;
    Ssiz_t from = 0;
    TString output = gSystem->GetFromPipe(cmd);
    while (output.Tokenize(tok, from, "\n")) {
        fileNames.push_back(tok);
    }

    int nFiles = fileNames.size();
    std::cout << "\nAnalyse files : " << std::endl;
    for (int iFile = 0; iFile < nFiles; iFile++) {

        std::cout << "\r\tProcessing file " << iFile+1 << " / " << nFiles << std::flush;
        
        TFile *fIn = TFile::Open(fileNames[iFile]);
        TTree *kineTree = (TTree*)fIn->Get("o2sim");

        std::vector<o2::MCTrack>* mctrack = nullptr;
        auto mcbr = kineTree->GetBranch("MCTrack");
        mcbr->SetAddress(&mctrack);
        //int counter = 0;
        UInt_t nEntries = kineTree->GetEntries();
        for (UInt_t iev = 0; iev < nEntries; iev++) {
            
            mcbr->GetEntry(iev);

            hCounter->Fill(1);

            for (auto &t : *mctrack) {

	        hCounter->Fill(2);

                double phi = TMath::ATan2(t.Vy() + t.Py(), t.Vx() + t.Px());
                double eta = t.GetEta();
                double z = t.Vz();

                if (bOnlyPrimaries && t.isSecondary()) continue;
                if (bOnlySecondaries && !t.isSecondary()) continue;

	        hCounter->Fill(3);

                if (t.leftTrace(o2::detectors::DetID::FV0)) {
		    hCounter->Fill(4);
                    hPhiFV0->Fill(phi);
                    hVrtxFV0->Fill(z);
                    hEtaFV0->Fill(eta);
                }

                if (t.leftTrace(o2::detectors::DetID::FT0)) {
		    hCounter->Fill(5);
                    if (eta > 0.0) {
                        hPhiFT0A->Fill(phi);
                        hVrtxFT0A->Fill(z);
                        hEtaFT0A->Fill(eta);

                        /**if (t.Vz()>10.0) {
                        std::cout << "\n{x, y, z} = "<< "{" << t.Vx() << ", " << t.Vy() << ", " << t.Vz() << "}" << std::endl;
                        std::cout << "prev = "<< "{" << t.Px() << ", " << t.Py() << ", " << t.Pz() << "}" << std::endl;
                        std::cout << "transformed = "<< "{" << t.Vx() + t.Px() << ", " << t.Vy() + t.Py() << ", " << t.Vz() + t.Pz() << "}" << std::endl;
                        std::cout << "angle = " << TMath::ATan2(t.Py(), t.Px()) << " --> " << TMath::ATan2(t.Vy() + t.Py(), t.Vx() + t.Px()) << std::endl;
                        counter++;
                        if (counter==10) return;
                        }**/
                    }

                    if (eta < 0.0) {
                        hPhiFT0C->Fill(phi);    
                        hVrtxFT0C->Fill(z);
                        hEtaFT0C->Fill(eta);
                    }
                }

            }
        }

        fIn->Close();
    }

    fOut->cd();
    fOut->Write("", TObject::kOverwrite);
    fOut->Close();

    std::cout << "\n";
    timer.Print();
}
