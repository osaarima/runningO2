//#include <TDatabasePDG.h>
//#include <TTree.h>
//#include <TNtuple.h>
//#include <TMCParticle.h>
//#include <AnalysisCore/MC.h>
//#include <SimulationDataFormat/MCTrack.h>
//#include <SimulationDataFormat/MCEventHeader.h>
//#include "src/CleanQCHistos.h"
#include "Framework/Logger.h"
#include "TPDGCode.h"
#include "TDatabasePDG.h"

using namespace o2;


// Courtesy of Sebastian Bysiak, original from O2/AliRoot
bool isStable(int pdg)
{
    // Decide whether particle (pdg) is stable

    // All ions/nucleons are considered as stable
    // Nuclear code is 10LZZZAAAI
    if (pdg > 1000000000) {
	return true;
    }

    constexpr int kNstable = 18;
    int pdgStable[kNstable] = {
	kGamma,      // Photon
	kElectron,   // Electron
	kMuonPlus,   // Muon
	kPiPlus,     // Pion
	kKPlus,      // Kaon
	kK0Short,    // K0s
	kK0Long,     // K0l
	kProton,     // Proton
	kNeutron,    // Neutron
	kLambda0,    // Lambda_0
	kSigmaMinus, // Sigma Minus
	kSigmaPlus,  // Sigma Plus
	3312,        // Xsi Minus
	3322,        // Xsi
	3334,        // Omega
	kNuE,        // Electron Neutrino
	kNuMu,       // Muon Neutrino
	kNuTau       // Tau Neutrino
    };

    for (int i = 0; i < kNstable; i++) {
	if (pdg == std::abs(pdgStable[i])) {
	    return true;
	}
    }

    return false;
}

// Courtesy of Sebastian Bysiak, original from O2/AliRoot
// Note: This function needs to be on top of a macro in order to work.
    template <typename TMCParticle, typename TMCParticles>
bool isPhysicalPrimary(TMCParticles& mcParticles, TMCParticle const& particle)
{
    // Test if a particle is a physical primary according to the following definition:
    // Particles produced in the collision including products of strong and
    // electromagnetic decay and excluding feed-down from weak decays of strange
    // particles.

    //LOGF(debug, "isPhysicalPrimary for %d", particle.index());

    // !!! WARNING: here we assume status code always < 1
    // const int ist = particle.statusCode();
    const int ist = -99;
    // !!!
    const int pdg = std::abs(particle.GetPdgCode());

    // Initial state particle
    // Solution for K0L decayed by Pythia6
    // ->
    if ((ist > 1) && (pdg != 130) && particle.isPrimary()) {
	return false;
    }
    if ((ist > 1) && !particle.isPrimary()) {
	return false;
    }
    // <-

    if (!isStable(pdg)) {
	return false;
    }

    if (particle.isPrimary()) {
	// Solution for K0L decayed by Pythia6
	// ->
	if (particle.getMotherTrackId() != -1) {
	    auto mother = mcParticles.at(particle.getMotherTrackId());
	    if (std::abs(mother.GetPdgCode()) == 130) {
		return false;
	    }
	}
	// <-
	// check for direct photon in parton shower
	// ->
	if (pdg == 22 && particle.getFirstDaughterTrackId() != -1) {
	    LOGF(debug, "D %d", particle.getFirstDaughterTrackId());
	    auto daughter = mcParticles.at(particle.getFirstDaughterTrackId());
	    if (daughter.GetPdgCode() == 22) {
		return false;
	    }
	}
	// <-
	return true;
    }

    // Particle produced during transport

    LOGF(debug, "M0 %d %d", particle.isPrimary(), particle.getMotherTrackId());
    auto mother = mcParticles.at(particle.getMotherTrackId());
    int mpdg = std::abs(mother.GetPdgCode());

    // Check for Sigma0
    if ((mpdg == 3212) && mother.isPrimary()) {
	return true;
    }

    // Check if it comes from a pi0 decay
    if ((mpdg == kPi0) && mother.isPrimary()) {
	return true;
    }

    // Check if this is a heavy flavor decay product
    int mfl = int(mpdg / std::pow(10, int(std::log10(mpdg))));

    // Light hadron
    if (mfl < 4) {
	return false;
    }

    // Heavy flavor hadron produced by generator
    if (mother.isPrimary()) {
	return true;
    }

    // To be sure that heavy flavor has not been produced in a secondary interaction
    // Loop back to the generated mother
    LOGF(debug, "M0 %d %d", mother.isPrimary(), mother.getMotherTrackId());
    while (mother.getMotherTrackId() != -1 && !mother.isPrimary()) {
	mother = mcParticles.at(mother.getMotherTrackId());
	LOGF(debug, "M+ %d %d", mother.isPrimary(), mother.getMotherTrackId());
	mpdg = std::abs(mother.GetPdgCode());
	mfl = int(mpdg / std::pow(10, int(std::log10(mpdg))));
    }

    if (mfl < 4) {
	return false;
    } else {
	return true;
    }
}



class CleanQCHistos {

    public:

	enum DETECTOR {
	    D_ALL,
	    D_TPC,
	    D_T0_A,
	    D_T0_C,
	    D_V0_A,
	    D_V0_C,
	    DET_N
	};
	const double cov[DET_N][2] = {
	    {-DBL_MAX, DBL_MAX},
	    {-0.8, 0.8},
	    {4.5, 5.0},
	    {-3.3, -2.9},
	    {2.8, 5.1},
	    {-3.7, -1.7}
	};

	const TString detName[DET_N] = {
	    "",
	    "TPC",
	    "T0_A",
	    "T0_C",
	    "V0_A",
	    "V0_C"
	};

	TH1D *fh_pt[DET_N];                // track pt
	TDirectoryFile *fd_pt;
	TH1D *fh_ptAllParticles[DET_N];    // track pt for all particles
	TDirectoryFile *fd_ptAllParticles;
	TH1D *fh_phi[DET_N];               // track phi
	TDirectoryFile *fd_phi;
	TH1D *fh_phiAllParticles[DET_N];   // track phi for all particles
	TDirectoryFile *fd_phiAllParticles;
	TH1D *fh_eta[DET_N];               // track eta
	TDirectoryFile *fd_eta;
	TH1D *fh_etaAllParticles[DET_N];   // track eta for all particles
	TDirectoryFile *fd_etaAllParticles;

	void CreateQCHistos() {
	    fd_pt = new TDirectoryFile("d_pt","d_pt");
	    fd_ptAllParticles = new TDirectoryFile("d_ptAllParticles","d_ptAllParticles");
	    fd_phi = new TDirectoryFile("d_phi","d_phi");
	    fd_phiAllParticles = new TDirectoryFile("d_phiAllParticles","d_phiAllParticles");
	    fd_eta = new TDirectoryFile("d_eta","d_eta");
	    fd_etaAllParticles = new TDirectoryFile("d_etaAllParticles","d_etaAllParticles");
	    for (int i=0; i<DET_N; i++) {
		fd_pt->cd();
		fh_pt[i]              = new TH1D(Form("h_pt%s",detName[i].Data()),     Form("h_pt%s",detName[i].Data()),     100, 0,  100);
		fh_pt[i]->Sumw2();

		fd_ptAllParticles->cd();
		fh_ptAllParticles[i]  = new TH1D(Form("h_ptAllParticles%s",detName[i].Data()),  Form("h_ptAllParticles%s",detName[i].Data()),  100, 0,  100);
		fh_ptAllParticles[i]->Sumw2();

		fd_phi->cd();
		fh_phi[i]             = new TH1D(Form("h_phi%s",detName[i].Data()),    Form("h_phi%s",detName[i].Data()),    30,  0,  2*TMath::Pi());
		fh_phi[i]->Sumw2();

		fd_phiAllParticles->cd();
		fh_phiAllParticles[i] = new TH1D(Form("h_phiAllParticles%s",detName[i].Data()), Form("h_phiAllParticles%s",detName[i].Data()), 30,  0,  2*TMath::Pi());
		fh_phiAllParticles[i]->Sumw2();

		fd_eta->cd();
		fh_eta[i]             = new TH1D(Form("h_eta%s",detName[i].Data()),    Form("h_eta%s",detName[i].Data()),    30,  -5, 5);
		fh_eta[i]->Sumw2();

		fd_etaAllParticles->cd();
		fh_etaAllParticles[i] = new TH1D(Form("h_etaAllParticles%s",detName[i].Data()), Form("h_etaAllParticles%s",detName[i].Data()), 30,  -5, 5);
		fh_etaAllParticles[i]->Sumw2();
	    }
	}

	void fillForDetectors(TH1D** h, double rap, double fill) {
	    for (int i=0; i<DET_N; i++) {
		if ( cov[i][0]<rap && cov[i][1]>rap )
		    h[i]->Fill(fill);
	    }
	    //cout << "CleanQCHistos WARNING: No detector found" << endl;
	    return;
	}

};

bool isHadron(int particleid);
//bool isPhysicalPrimary(std::vector<o2::MCTrack>& mcParticles, o2::MCTrack const& particle);
//bool isStable(int pdg);

void Clean_o2sim(TString sFolder, TString sOutFileName) {

    bool DEBUG = false;
    TString inFileName(Form("%s/o2sim_Kine.root", sFolder.Data()));
    TFile *fIn = TFile::Open(inFileName);
    TTree* kineTree = (TTree*)fIn->Get("o2sim");
    kineTree->SetBranchStatus("*", 1);
    std::vector<o2::MCTrack>* mctrack = nullptr;
    auto mcbr = kineTree->GetBranch("MCTrack");
    mcbr->SetAddress(&mctrack);

    o2::dataformats::MCEventHeader* mcheader = nullptr;
    auto headerbr = kineTree->GetBranch("MCEventHeader.");
    headerbr->SetAddress(&mcheader);

    UInt_t nEntries = kineTree->GetEntries();
    //nEntries=10;
    std::cout << "MC events : " << nEntries << std::endl;


    // The order of fOut, ntuple, fOutQC, and histos is important!
    TFile *fOut = new TFile(sOutFileName, "RECREATE");
    //TNtuple *ntuple = new TNtuple("amptEvents", "detector simulated ampt data", "eventId:particleId:px:py:pz:E:charge:isHadron");
    auto newTree = kineTree->CloneTree(0);
    std::vector<o2::MCTrack>* mctracknew = nullptr;
    auto mcnewbr = newTree->GetBranch("MCTrack");
    mcnewbr->SetAddress(&mctracknew);

    TString sQCname = sOutFileName.Data();
    sQCname.Insert(sQCname.Last('.'), "_QChisto");
    TFile *fOutQC = new TFile(sQCname,"RECREATE");

    CleanQCHistos *h = new CleanQCHistos();
    cout << h << endl;
    h->CreateQCHistos();


    int particleid, particleidAbs;
    double px, py, pz, E, rap;//, x, y, z, t;
    double charge;
    bool ishadron;
    int kS, parent, firstchild, lastchild;
    bool isPP;

    int itrackcount=0;
    int iprimarycount=0;

    //TDatabasePDG *pdg = new TDatabasePDG();
    //TString sPdgTable = gSystem->GetFromPipe("echo $ROOTSYS/etc/pdg_table.txt");
    //pdg->ReadPDGtable(sPdgTable.Data());
    TParticlePDG *pdgParticle;
    TParticle tparticle;
    TMCParticle *tmcpart;

    int test = 0;
    for (UInt_t ient = 0; ient < nEntries; ient++) {
	std::cout << "Entry " << ient << std::endl;
	mcbr->GetEntry(ient);
	headerbr->GetEntry(ient);
	mcnewbr->GetEntry(ient);
	test=0;
	for (auto &track : *mctrack) {
	    itrackcount++;
	    //std::cout << "Pdw:       " << track.GetPdgCode() << std::endl;
	    //std::cout << "isPrimary: " << track.isPrimary() << std::endl;
	    //std::cout << "Charge:    " << pdg->GetParticle(particleid)->Charge() << std::endl;

	    //parent = track.getMotherTrackId();
	    //firstchild = track.getFirstDaughterTrackId();
	    //lastchild = track.getLastDaughterTrackId();

	    particleid = track.GetPdgCode();
	    particleidAbs = TMath::Abs(track.GetPdgCode());
	    ishadron = isHadron(particleidAbs);
	    charge=0;
	    pdgParticle = TDatabasePDG::Instance()->GetParticle(particleid);
	    //if(pdgParticle==0x0) {
	    //    std::cout << "Error, no pdg info found for particle id " << particleid 
	    //        << " in evt " << ient << std::endl;
	    //    return;
	    //}
	    if(TDatabasePDG::Instance()->GetParticle(particleid)==0) {
		charge=0;
	    } else {
		charge = TDatabasePDG::Instance()->GetParticle(particleid)->Charge();
	    }
	    rap = track.GetRapidity();
	    h->fillForDetectors(h->fh_ptAllParticles, rap, track.GetPt());
	    h->fillForDetectors(h->fh_etaAllParticles, rap, track.GetEta());
	    h->fillForDetectors(h->fh_phiAllParticles, rap, track.GetPhi());

	    //if(track.isPrimary()) { // This one will select only incoming p and n for AMPT.
	    //if(isPhysicalPrimary(*mctrack, track)) { // Will be implemented for Kine
	    //if(MC::isPhysicalPrimary((TMCParticles&)0x0, (TMCParticle)0x0) {
	    //if((ishadron || particleid==22) && parent==-1 && firstchild==-1 && lastchild==-1) {
	    // Charged hadrons, photons=22, e=11, mu=13, tau=15
	    isPP = isPhysicalPrimary<o2::MCTrack, std::vector<o2::MCTrackT<float> >>(*mctrack, track);
	    if(((ishadron && charge!=0) || particleidAbs==22 || particleidAbs==11 || particleidAbs==13 || particleidAbs==15) && isPP){//track.isPrimary()) {
		//if(true) {
		px = track.Px(); // mStartVertexMomentumX
		py = track.Py(); // mStartVertexMomentumY
		pz = track.Pz(); // mStartVertexMomentumZ
		E = track.GetEnergy(); 
		h->fillForDetectors(h->fh_pt, rap, track.GetPt());
		h->fillForDetectors(h->fh_eta, rap, track.GetEta());
		h->fillForDetectors(h->fh_phi, rap, track.GetPhi());
		mctracknew->push_back(track);
		test++;
		iprimarycount++;
		//if(!((px==0)&&(py==0))){ //Definition of primary particle in TAmpt.cxx
		if(DEBUG) {
		    //cout << "I have non-zero transverse momentum!" << endl;
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
		//} else {
		//ntuple->Fill(ient, particleid, px, py, pz, E, charge, ishadron);
		//}
	    }
	}
	newTree->Fill();
	mctracknew->clear();
    }
    std::cout << "Tracks: " << itrackcount << std::endl;
    std::cout << "Primaries: " << iprimarycount << std::endl;
    fOut->Write();
    fOut->Close();
    fOutQC->Write();
    fOutQC->Close();

    return;
}

// Function from pythia code
// Needs positive particle id
bool isHadron(int particleid) {
    if (particleid <= 100 || (particleid >= 1000000 && particleid <= 9000000) || particleid >= 9900000)
	    return false;
	if (particleid == 130 || particleid == 310)
	    return true;
	if (particleid%10 == 0 || (particleid/10)%10 == 0 || (particleid/100)%10 == 0)
	    return false;
	return true;

}

