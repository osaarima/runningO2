void RemoveCkov(TString sKineFile = "o2sim_Kine.root")
{
    TFile *fIn = TFile::Open(sKineFile);
    TTree* kineTree = (TTree*)fIn->Get("o2sim");

    kineTree->SetBranchStatus("*", 1);

    std::vector<o2::MCTrack>* mctrack = nullptr;
    auto mcbr = kineTree->GetBranch("MCTrack");
    mcbr->SetAddress(&mctrack);

    o2::dataformats::MCEventHeader* mcheader = nullptr;
    auto headerbr = kineTree->GetBranch("MCEventHeader.");
    headerbr->SetAddress(&mcheader);

    std::vector<o2::TrackReference>* trackrefs = nullptr;
    auto refbr = kineTree->GetBranch("TrackRefs");
    refbr->SetAddress(&trackrefs);

    //o2::dataformats::MCTruthContainer<o2::TrackReference>* indexedtrackrefs = nullptr;
    //auto irefbr = kineTree->GetBranch("IndexedTrackRefs");
    //irefbr->SetAddress(&indexedtrackrefs);

    UInt_t nEntries = kineTree->GetEntries();
    std::cout << "MC events : " << nEntries << std::endl;

    TFile *fOut = TFile::Open("o2sim_Kine_nockov.root", "RECREATE");
    fOut->cd();
    auto newTree = kineTree->CloneTree(0);

    std::vector<o2::MCTrack>* mctracknew = nullptr;
    auto mcnewbr = newTree->GetBranch("MCTrack");
    mcnewbr->SetAddress(&mctracknew);

    int sum = 0;
    for (UInt_t ient = 0; ient < nEntries; ient++) {
        int notCkov = 0;

        std::cout << "\tEntry " << ient;

        mcbr->GetEntry(ient);
        mcnewbr->GetEntry(ient);

        headerbr->GetEntry(ient);
        refbr->GetEntry(ient);
        //irefbr->GetEntry(ient);

        int itrack = 0;
        UInt_t nTracks = mctrack->size();
        std::cout << ",\tMCtracks : " << nTracks;
        for (auto &track : *mctrack) {
            int pdg = track.GetPdgCode();
            if (pdg!=50000050) {
                mctracknew->push_back(track);
                notCkov++;
            }
            itrack++;
        }

        newTree->Fill();
        mctracknew->clear();

        std::cout << ",\tNon-cerenkovs : " << notCkov << std::endl;
        sum += notCkov;
    }

    std::cout << "\nSum of saved tracks : " << sum << std::endl;

    fOut->Write("",TObject::kOverwrite);
    fOut->Close();
    fIn->Close();
}
