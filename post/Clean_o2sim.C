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
    //cout << "ispr1" << endl;
    const int ist = -99;
    // !!!
    const int pdg = std::abs(particle.GetPdgCode());

    // Initial state particle
    // Solution for K0L decayed by Pythia6
    // ->
    //cout << "ispr2" << endl;
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

    //cout << "ispr3" << endl;
    if (particle.isPrimary()) {
	// Solution for K0L decayed by Pythia6
	// ->
	if (particle.getMotherTrackId() != -1) {
	    //cout << "mother: " << particle.getMotherTrackId() << endl;
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
	    //cout << "firstdaughter: " << particle.getFirstDaughterTrackId() << endl;
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
    if(particle.getMotherTrackId()<0) return false; //Note: added here for the AMPT as some particles have mother id -1
    //cout << "mother: " << particle.getMotherTrackId() << endl;
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
	//cout << "mother: " << particle.getMotherTrackId() << endl;
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
	    D_TPC_A,
	    D_TPC_B,
	    D_FT0_A,
	    D_FT0_C,
	    D_FV0,
	    D_FDD_A,
	    D_FDD_C,
	    DET_N
	};
	const double cov[DET_N][2] = {
	    {-DBL_MAX, DBL_MAX},
	    {-0.8, 0.8},
	    {-0.8, -0.1},
	    {0.1, 0.8},
	    {3.8, 5.0},
	    {-3.4, -2.3},
	    {2.2, 5.0},
	    {4.7, 6.3},
	    {-6.9, -4.9}
	};

	const TString detName[DET_N] = {
	    "",
	    "TPC",
	    "TPC_A",
	    "TPC_B",
	    "FT0_A",
	    "FT0_C",
	    "FV0",
	    "FDD_A",
	    "FDD_C"
	};

	TDirectoryFile *fd_baseDir;

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
	TH1D *fh_pz[DET_N];                // track pz
	TDirectoryFile *fd_pz;
	TH1D *fh_pzAllParticles[DET_N];
	TDirectoryFile *fd_pzAllParticles;
	TH1D *fh_PID[DET_N];
	TDirectoryFile *fd_PID;
	TH1D *fh_PIDAllParticles[DET_N];
	TDirectoryFile *fd_PIDAllParticles;

	TH1D *fh_PIDisPPFraction;
	TH1D *fh_PIDisPrimaryFraction;
	TH1D *fh_PIDisPrimaryAndPtFraction;
	TH1D *fh_eventInfo;               // info

	void CreateQCHistos() {
	    fd_baseDir = new TDirectoryFile("d_baseDir","d_baseDir");
	    fd_baseDir->cd();

	    fd_pt = new TDirectoryFile("d_pt","d_pt");
	    fd_ptAllParticles = new TDirectoryFile("d_ptAllParticles","d_ptAllParticles");
	    fd_phi = new TDirectoryFile("d_phi","d_phi");
	    fd_phiAllParticles = new TDirectoryFile("d_phiAllParticles","d_phiAllParticles");
	    fd_eta = new TDirectoryFile("d_eta","d_eta");
	    fd_etaAllParticles = new TDirectoryFile("d_etaAllParticles","d_etaAllParticles");
	    fd_pz = new TDirectoryFile("d_pz","d_pz");
	    fd_pzAllParticles = new TDirectoryFile("d_pzAllParticles","d_pzAllParticles");
	    fd_PID = new TDirectoryFile("d_PID","d_PID");
	    fd_PIDAllParticles = new TDirectoryFile("d_PIDAllParticles","d_PIDAllParticles");

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

		fd_pz->cd();
		fh_pz[i]              = new TH1D(Form("h_pz%s",detName[i].Data()),     Form("h_pz%s",detName[i].Data()),     300, 0,  14000);
		fh_pz[i]->Sumw2();

		fd_pzAllParticles->cd();
		fh_pzAllParticles[i]  = new TH1D(Form("h_pzAllParticles%s",detName[i].Data()),  Form("h_pzAllParticles%s",detName[i].Data()),  300, 0,  14000);
		fh_pzAllParticles[i]->Sumw2();

		fd_PID->cd();
		fh_PID[i]              = new TH1D(Form("h_PID%s",detName[i].Data()),     Form("h_PID%s",detName[i].Data()),     10000000, 0,  10000000);
		//fh_PID[i]->Sumw2();

		fd_PIDAllParticles->cd();
		fh_PIDAllParticles[i]  = new TH1D(Form("h_PIDAllParticles%s",detName[i].Data()),  Form("h_PIDAllParticles%s",detName[i].Data()),  10000000, 0,  10000000);
		//fh_PIDAllParticles[i]->Sumw2();

	    }
	    fd_baseDir->cd();
	    fh_PIDisPPFraction  = new TH1D(Form("h_PIDFraction"),  Form("h_PIDFraction"),  10000000, 0,  10000000);
	    //fh_PIDisPPFraction->Sumw2();
	    fh_PIDisPrimaryFraction  = new TH1D(Form("h_PIDisPrimaryFraction"),  Form("h_PIDisPrimaryFraction"),  10000000, 0,  10000000);
	    //fh_PIDisPrimaryFraction->Sumw2();
	    fh_PIDisPrimaryAndPtFraction  = new TH1D(Form("h_PIDisPrimaryAndPtFraction"),  Form("h_PIDisPrimaryAndPtFraction"),  10000000, 0,  10000000);
	    //fh_PIDisPrimaryAndPtFraction->Sumw2();
	    fh_eventInfo  = new TH1D(Form("h_eventInfo"),  Form("h_eventInfo"),  5, 0,  5);
	    //fh_eventInfo->Sumw2();

	}

	void fillForDetectors(TH1D** h, double rap, double fill, double weight=1.0) {
	    for (int i=0; i<DET_N; i++) {
		if ( i==0 || (cov[i][0]<rap && cov[i][1]>rap) )
		    h[i]->Fill(fill,weight);
	    }
	    //cout << "CleanQCHistos WARNING: No detector found" << endl;
	    return;
	}

};

bool isHadron(int particleid);

void Clean_o2sim(TString sFolder, TString sOutFileName, TString options) {

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

    int pionCounter=0;
    int kaonCounter=0;
    int protCounter=0;
    int pionIsPPCounter=0;
    int kaonIsPPCounter=0;
    int protIsPPCounter=0;

    int itrackcount=0;
    int iprimarycount=0;

    //TDatabasePDG *pdg = new TDatabasePDG();
    //TString sPdgTable = gSystem->GetFromPipe("echo $ROOTSYS/etc/pdg_table.txt");
    //pdg->ReadPDGtable(sPdgTable.Data());
    TParticlePDG *pdgParticle;
    TParticle tparticle;
    TMCParticle *tmcpart;

    int test = 0;
    //int cops = 0;
    for (UInt_t ient = 0; ient < nEntries; ient++) {
	std::cout << "Entry " << ient << std::endl;
	h->fh_eventInfo->Fill("events",1.0);
	mcbr->GetEntry(ient);
	headerbr->GetEntry(ient);
	mcnewbr->GetEntry(ient);
	test=0;
	//o2::MCTrack track;
	for (auto &track : *mctrack) {
	//for (int i=0; i<mctrack->size(); i++) {
	    //track=mctrack->at(i);
	    h->fh_eventInfo->Fill("all tracks",1.0);
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
	    h->fillForDetectors(h->fh_pzAllParticles, rap, TMath::Abs(track.Pz()));
	    h->fillForDetectors(h->fh_PIDAllParticles, rap, TMath::Abs(track.GetPdgCode()));

	    //cout << "tset0" << endl;

	    //if(track.isPrimary()) { // This one will select only incoming p and n for AMPT.
	    //if(isPhysicalPrimary(*mctrack, track)) { // Will be implemented for Kine
	    //if(MC::isPhysicalPrimary((TMCParticles&)0x0, (TMCParticle)0x0) {
	    //if((ishadron || particleid==22) && parent==-1 && firstchild==-1 && lastchild==-1) {
	    // Charged hadrons, photons=22, e=11, mu=13, tau=15
	    //cout << "tset1" << endl;

	    //https://pdg.lbl.gov/2007/reviews/montecarlorpp.pdf
	    //e-   11, g     21, pi0  111, p 2212, K0L 130
	    //ve   12, gamma 22, pi+  211, n 2112, K0S 310
	    //mu-  13, Z     23, eta  221          K0  311 
	    //vmu  14, W+    24, rho  113,         K+  321
	    //tau- 15, H     25, rho+ 213
	    //vtau 16,
	    // Photons only 27% primaries, should we include them still?
	    int pidPos = TMath::Abs(particleid);
	    if(options.Contains("ampt",TString::kIgnoreCase)) {
		//cout << "getpt" << endl;
		if(track.GetPt()!=0) {
		    if(/*pidPos==15      || pidPos==21      || pidPos==113     || pidPos==213     || pidPos==221     ||
		       pidPos==223     || pidPos==311     || pidPos==313     || pidPos==315     || pidPos==323     ||
		       pidPos==325     || pidPos==331     || pidPos==333     || pidPos==411     || pidPos==413     || 
		       pidPos==421     || pidPos==423     || pidPos==431     || pidPos==433     || pidPos==435     ||
		       pidPos==441     || pidPos==443     || pidPos==445     || pidPos==511     || pidPos==513     ||
		       pidPos==521     || pidPos==523     || pidPos==531     || pidPos==533     || pidPos==553     ||
		       pidPos==555     || pidPos==990     || pidPos==1103    || pidPos==1114    || pidPos==2101    ||
		       pidPos==2103    || pidPos==2114    || pidPos==2203    || pidPos==2214    || pidPos==2224    ||
		       pidPos==3101    || pidPos==3103    || pidPos==3114    || pidPos==3201    || pidPos==3203    ||
		       pidPos==3214    || pidPos==3224    || pidPos==3312    || pidPos==3314    || pidPos==3322    ||
		       pidPos==3324    || pidPos==3334    || pidPos==4112    || pidPos==4114    || pidPos==4122    ||
		       pidPos==4132    || pidPos==4212    || pidPos==4214    || pidPos==4222    || pidPos==4224    ||
		       pidPos==4232    || pidPos==4312    || pidPos==4314    || pidPos==4324    || pidPos==5122    ||
		       pidPos==5132    || pidPos==5214    || pidPos==5222    || pidPos==5224    || pidPos==5232    ||
		       pidPos==5303    || pidPos==10221   || pidPos==10311   || pidPos==10313   || pidPos==10321   ||
		       pidPos==10323   || pidPos==10423   || pidPos==10433   || pidPos==10441   || pidPos==10551   ||
		       pidPos==20213   || pidPos==20313   || pidPos==20413   || pidPos==20423   || pidPos==20433   ||
		       pidPos==20443   || pidPos==20553   || pidPos==100443  || pidPos==9000211 || pidPos==9010221 ||
		       pidPos==9902210 || pidPos==9940003 || pidPos==9941103 || pidPos==9942003 || pidPos==9942103 ||
		       pidPos==9950003 || pidPos==111     || pidPos==211     || pidPos==2212    || pidPos==321     ||
		       pidPos==2112    || pidPos==11      || pidPos==13      || pidPos==22      || pidPos==130     ||
		       pidPos==310*/
			isHadron(pidPos) || pidPos==22 || pidPos==11 || pidPos==13
		      ){
			//cout << "tset101" << TMCProcess::kPPrimary << endl;
			track.setProcess(TMCProcess::kPPrimary);
			//cout << "tset102" << endl;
		    } else{
			//cout << "tset111 " << TMCProcess::kPNoProcess << endl;
			//track.setProcess(0);
			track.setProcess(TMCProcess::kPNoProcess);
			//cout << "tset112 " << endl;
		    }
		} else {
		    //track.setProcess(0);
		    track.setProcess(TMCProcess::kPNoProcess);
		}
	    }
	    
	    //cout << "tset2" << endl;
	    if(pidPos==111 || pidPos==211) pionCounter++;
	    if(pidPos==311 || pidPos==321) kaonCounter++;
	    if(pidPos==2212) protCounter++;
	    isPP = isPhysicalPrimary<o2::MCTrack, std::vector<o2::MCTrackT<float> >>(*mctrack, track);
	    if(isPP) {
		if(pidPos==111 || pidPos==211) pionIsPPCounter++;
		if(pidPos==311 || pidPos==321) kaonIsPPCounter++;
		if(pidPos==2212) protIsPPCounter++;
	    }
	    
	    if(track.isPrimary()) {
		h->fh_PIDisPrimaryFraction->Fill(TMath::Abs(track.GetPdgCode()));
		h->fh_eventInfo->Fill("isPrimary tracks",1.0);
		if(track.Px()!=0.0 || track.Py()!=0.0) {
		    //if(cops==0) track.Paint();
		    //cops++;
		    h->fh_PIDisPrimaryAndPtFraction->Fill(TMath::Abs(track.GetPdgCode()));
		}
	    }
	    if(isPP) {
		h->fh_eventInfo->Fill("isPP tracks",1.0);
		h->fh_PIDisPPFraction->Fill(TMath::Abs(track.GetPdgCode()));
	    }
	    
	    if(((ishadron && charge!=0) || particleidAbs==22 || particleidAbs==11 || particleidAbs==13 || particleidAbs==15) && isPP/*track.isPrimary()*/) {
		h->fh_eventInfo->Fill("clean tracks",1.0);
		px = track.Px(); // mStartVertexMomentumX
		py = track.Py(); // mStartVertexMomentumY
		pz = track.Pz(); // mStartVertexMomentumZ
		E = track.GetEnergy(); 
		h->fillForDetectors(h->fh_pt, rap, track.GetPt());
		h->fillForDetectors(h->fh_eta, rap, track.GetEta());
		h->fillForDetectors(h->fh_phi, rap, track.GetPhi());
		h->fillForDetectors(h->fh_pz, rap, TMath::Abs(track.Pz()));
		h->fillForDetectors(h->fh_PID, rap, TMath::Abs(track.GetPdgCode()));
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
    for (int i=0; i<h->fh_PIDisPPFraction->GetNbinsX(); i++) {
	if(h->fh_PIDAllParticles[0]->GetBinContent(i)!=0 /*&& h->fh_PIDisPrimaryFraction->GetBinContent(i)/h->fh_PIDAllParticles[0]->GetBinContent(i)>0.9*/) {
	    int pid=TMath::FloorNint(h->fh_PIDisPPFraction->GetBinCenter(i));
	    TString stable, lifetime;
	    if(TDatabasePDG::Instance()->GetParticle(pid)==0){
		stable="?";
		lifetime=Form("%6s","?");
	    } else {
		stable=Form("%d",TDatabasePDG::Instance()->GetParticle(pid)->Stable());
		lifetime=Form("%6.3f",1000000000*TDatabasePDG::Instance()->GetParticle(pid)->Lifetime()); //ns
	    }
	    cout << Form("PID: %7.d (s:%s,l:%s), isPP: %7.f (%1.3f), isPrimary: %7.f (%1.3f), isP & pt!=0: %7.f (%1.3f), total: %7.f",
		    pid,
		    stable.Data(),
		    lifetime.Data(),
		    h->fh_PIDisPPFraction->GetBinContent(i),
		    h->fh_PIDisPPFraction->GetBinContent(i)/h->fh_PIDAllParticles[0]->GetBinContent(i),
		    h->fh_PIDisPrimaryFraction->GetBinContent(i),
		    h->fh_PIDisPrimaryFraction->GetBinContent(i)/h->fh_PIDAllParticles[0]->GetBinContent(i),
		    h->fh_PIDisPrimaryAndPtFraction->GetBinContent(i),
		    h->fh_PIDisPrimaryAndPtFraction->GetBinContent(i)/h->fh_PIDAllParticles[0]->GetBinContent(i),
		    h->fh_PIDAllParticles[0]->GetBinContent(i)) << endl;
	}

    }
    h->fh_PIDisPrimaryFraction->Divide(h->fh_PIDAllParticles[0]);
    h->fh_PIDisPPFraction->Divide(h->fh_PIDAllParticles[0]);
    std::cout << "Tracks: " << itrackcount << std::endl;
    std::cout << "Primaries: " << iprimarycount << std::endl;
    std::cout << "pions: " << pionCounter << ", pion isPP: " << pionIsPPCounter << std::endl;
    std::cout << "Kaons: " << kaonCounter << ", Kaon isPP: " << kaonIsPPCounter << std::endl;
    std::cout << "prots: " << protCounter << ", prot isPP: " << protIsPPCounter << std::endl;
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

