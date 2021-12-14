//#include <TDatabasePDG.h>
//#include <TTree.h>
//#include <TNtuple.h>
//#include <TMCParticle.h>
//#include <AnalysisCore/MC.h>
//#include <SimulationDataFormat/MCTrack.h>
//#include <SimulationDataFormat/MCEventHeader.h>
#include "Framework/Logger.h"
#include "TPDGCode.h"
#include "TDatabasePDG.h"

using namespace o2;

class PPQCHistos {

    public:

	enum DETECTOR {
	    D_ALL,
	    D_TPC,
	    D_TPC_A,
	    D_TPC_B,
	    D_FT0_A,
	    D_FT0_C,
	    D_FV0,
	    DET_N
	};
	const double cov[DET_N][2] = {
	    {-DBL_MAX, DBL_MAX},
	    {-0.8, 0.8},
	    {-0.8, -0.1},
	    {0.1, 0.8},
	    {3.5, 4.9},
	    {-3.3, -2.1},
	    {2.2, 5.0},
	};

	const TString detName[DET_N] = {
	    "",
	    "TPC",
	    "TPC_A",
	    "TPC_B",
	    "FT0_A",
	    "FT0_C",
	    "FV0",
	};

    int multiCounter[DET_N] = {0};

	TDirectoryFile *fd_baseDir;

	TH1D *fh_pt[DET_N];                // track pt
	TDirectoryFile *fd_pt;
	TH1D *fh_phi[DET_N];               // track phi
	TDirectoryFile *fd_phi;
	TH1D *fh_eta[DET_N];               // track eta
	TDirectoryFile *fd_eta;
	TH1D *fh_pz[DET_N];                // track pz
	TDirectoryFile *fd_pz;
	TH1D *fh_PID[DET_N];
	TDirectoryFile *fd_PID;
	TH1D *fh_Multi[DET_N];
	TDirectoryFile *fd_Multi;

	TH1D *fh_eventInfo;               // info

	void CreatePPQCHistos() {
	    fd_baseDir = new TDirectoryFile("d_baseDir","d_baseDir");
	    fd_baseDir->cd();

	    fd_pt = new TDirectoryFile("d_pt","d_pt");
	    fd_phi = new TDirectoryFile("d_phi","d_phi");
	    fd_eta = new TDirectoryFile("d_eta","d_eta");
	    fd_pz = new TDirectoryFile("d_pz","d_pz");
	    fd_PID = new TDirectoryFile("d_PID","d_PID");
	    fd_Multi = new TDirectoryFile("d_Multi","d_Multi");

	    for (int i=0; i<DET_N; i++) {
		fd_pt->cd();
		fh_pt[i]              = new TH1D(Form("h_pt%s",detName[i].Data()),     Form("h_pt%s",detName[i].Data()),     100, 0,  100);
		fh_pt[i]->Sumw2();

		fd_phi->cd();
		fh_phi[i]             = new TH1D(Form("h_phi%s",detName[i].Data()),    Form("h_phi%s",detName[i].Data()),    30,  0,  2*TMath::Pi());
		fh_phi[i]->Sumw2();

		fd_eta->cd();
		fh_eta[i]             = new TH1D(Form("h_eta%s",detName[i].Data()),    Form("h_eta%s",detName[i].Data()),    45,  -7, 7);
		fh_eta[i]->Sumw2();

		fd_pz->cd();
		fh_pz[i]              = new TH1D(Form("h_pz%s",detName[i].Data()),     Form("h_pz%s",detName[i].Data()),     300, 0,  14000);
		fh_pz[i]->Sumw2();

		fd_PID->cd();
		fh_PID[i]              = new TH1D(Form("h_PID%s",detName[i].Data()),     Form("h_PID%s",detName[i].Data()),     10000000, 0,  10000000);
		//fh_PID[i]->Sumw2();

		fd_Multi->cd();
		fh_Multi[i]              = new TH1D(Form("h_Multi%s",detName[i].Data()),     Form("h_Multi%s",detName[i].Data()),     10000, 0,  100000);
		fh_Multi[i]->Sumw2();

	    }
	    fd_baseDir->cd();
	    fh_eventInfo  = new TH1D(Form("h_eventInfo"),  Form("h_eventInfo"),  5, 0,  5);
	    //fh_eventInfo->Sumw2();

	}

	void resetMultiCounters() {
        for (int i=0; i<DET_N; i++) {
            multiCounter[i]=0;
        }
    }

	void countMultiplisity(double eta) {
        for (int i=0; i<DET_N; i++) {
            if ( i==0 || (cov[i][0]<eta && cov[i][1]>eta) )
                multiCounter[i]++;
        }
	    return;
    }

	void fillForDetectors(TH1D** h, double eta, double fill, double weight=1.0) {
        for (int i=0; i<DET_N; i++) {
            if ( i==0 || (cov[i][0]<eta && cov[i][1]>eta) )
                h[i]->Fill(fill,weight);
        }
	    //cout << "PPQCHistos WARNING: No detector found" << endl;
	    return;
	}

	void fillMulti() {
        for (int i=0; i<DET_N; i++) {
                fh_Multi[i]->Fill(multiCounter[i]);
        }
	    //cout << "PPQCHistos WARNING: No detector found" << endl;
	    return;
	}

};

void Multi(TString sFolder) {
    //DEBUG=0: no debug, 1: end debug, 2: all particles debug
    int DEBUG = 1;
    TString inFileName(Form("%s/o2sim_Kine_PPOnly.root", sFolder.Data()));
    TFile *fIn = TFile::Open(inFileName);
    TTree* kineTree = (TTree*)fIn->Get("o2sim");
    kineTree->SetBranchStatus("*", 1);
    std::vector<o2::MCTrack>* mctrack = nullptr;
    auto mcbr = kineTree->GetBranch("MCTrack");
    mcbr->SetAddress(&mctrack);

    UInt_t nEntries = kineTree->GetEntries();
    //nEntries=10;
    std::cout << "MC events : " << nEntries << std::endl;

    // The order of fOut, ntuple, fOutQC, and histos is important!
    TFile *fOut = new TFile(Form("%s/o2sim_Kine_PPOnly_MultiplisityQC.root", sFolder.Data()), "RECREATE");

    PPQCHistos *h = new PPQCHistos();
    h->CreatePPQCHistos();

    double px, py, pz, E, eta;//, x, y, z, t;
    //bool ishadron;
    //int kS, parent, firstchild, lastchild;
    //bool isPP;

    //int pionCounter=0;
    //int kaonCounter=0;
    //int protCounter=0;
    //int pionIsPPCounter=0;
    //int kaonIsPPCounter=0;
    //int protIsPPCounter=0;

    //int itrackcount=0;
    int iprimarycount=0;

    for (UInt_t ient = 0; ient < nEntries; ient++) {
        std::cout << "Entry " << ient << std::endl;
        h->fh_eventInfo->Fill("events",1.0);
        mcbr->GetEntry(ient);
        h->resetMultiCounters();
        for (auto &track : *mctrack) {
            h->fh_eventInfo->Fill("all tracks",1.0);
            //itrackcount++;
            px = track.Px(); // mStartVertexMomentumX
            py = track.Py(); // mStartVertexMomentumY
            pz = track.Pz(); // mStartVertexMomentumZ
            E = track.GetEnergy(); 
            eta = track.GetEta();
            h->countMultiplisity(eta);
            h->fillForDetectors(h->fh_pt,  eta, track.GetPt());
            h->fillForDetectors(h->fh_eta, eta, track.GetEta());
            h->fillForDetectors(h->fh_phi, eta, track.GetPhi());
            h->fillForDetectors(h->fh_pz,  eta, TMath::Abs(track.Pz()));
            h->fillForDetectors(h->fh_PID, eta, TMath::Abs(track.GetPdgCode()));
            iprimarycount++;
            /*
            if(DEBUG>1) {
                cout << "ParticleID: " << particleid
                    //<< ", UniqueID: " << track.GetUniqueID()
                    << ", FirstdaugID: " << track.getFirstDaughterTrackId()
                    << ", LastdaugID: " << track.getLastDaughterTrackId()
                    << ", MotherTrackID: " << track.getMotherTrackId()
                    << ", SecondMotherTrackID: " << track.getSecondMotherTrackId()
                    << ", getProcess: " << track.getProcess()
                    << ", hasHits: " << track.hasHits()
                    << endl;
            }
            */
        }
        h->fillMulti(); //Remember to use resetMultiCounters in the beginning of the loop.
    }
    fOut->Write();
    fOut->Close();

    return;
}

