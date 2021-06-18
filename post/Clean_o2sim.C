//#include <TDatabasePDG.h>
//#include <TTree.h>
//#include <TNtuple.h>
//#include <TMCParticle.h>
//#include <AnalysisCore/MC.h>
//#include <SimulationDataFormat/MCTrack.h>
//#include <SimulationDataFormat/MCEventHeader.h>
//#include "src/CleanQCHistos.h"

using namespace o2;

class CleanQCHistos {

    public:

        TH1D *fh_pt;       // track pt
        TH1D *fh_ptAll;    // track pt for all particles
        TH1D *fh_phi;      // track phi
        TH1D *fh_phiAll;   // track phi for all particles
        TH1D *fh_eta;      // track eta
        TH1D *fh_etaAll;   // track eta for all particles

        void CreateQCHistos() {
            fh_pt     = new TH1D("h_pt",     "h_pt",     100, 0,  100);
            fh_ptAll  = new TH1D("h_ptAll",  "h_ptAll",  100, 0,  100);
            fh_phi    = new TH1D("h_phi",    "h_phi",    30,  0,  2*TMath::Pi());
            fh_phiAll = new TH1D("h_phiAll", "h_phiAll", 30,  0,  2*TMath::Pi());
            fh_eta    = new TH1D("h_eta",    "h_eta",    30,  -5, 5);
            fh_etaAll = new TH1D("h_etaAll", "h_etaAll", 30,  -5, 5);
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
    nEntries=10;
    std::cout << "MC events : " << nEntries << std::endl;


    // The order of fOut, ntuple, fOutQC, and histos is important!
    TFile *fOut = new TFile(sOutFileName, "RECREATE");
    TNtuple *ntuple = new TNtuple("amptEvents", "detector simulated ampt data", "eventId:particleId:px:py:pz:E:charge:isHadron");

    TString sQCname = sOutFileName.Data();
    sQCname.Insert(sQCname.Last('.'), "_QChisto");
    TFile *fOutQC = new TFile(sQCname,"RECREATE");

    CleanQCHistos *h = new CleanQCHistos();
    cout << h << endl;
    h->CreateQCHistos();


    int particleid;
    double px, py, pz, E;//, x, y, z, t;
    double charge;
    bool ishadron;
    int kS, parent, firstchild, lastchild;

    int itrackcount=0;
    int iprimarycount=0;

    //TDatabasePDG *pdg = new TDatabasePDG();
    //TString sPdgTable = gSystem->GetFromPipe("echo $ROOTSYS/etc/pdg_table.txt");
    //pdg->ReadPDGtable(sPdgTable.Data());
    TParticlePDG *pdgParticle;
    TParticle tparticle;
    TMCParticle *tmcpart;

    for (UInt_t ient = 0; ient < nEntries; ient++) {
        std::cout << "Entry " << ient << std::endl;
        mcbr->GetEntry(ient);
        for (auto &track : *mctrack) {
            itrackcount++;
            //std::cout << "Pdw:       " << track.GetPdgCode() << std::endl;
            //std::cout << "isPrimary: " << track.isPrimary() << std::endl;
            //std::cout << "Charge:    " << pdg->GetParticle(particleid)->Charge() << std::endl;

            //parent = track.getMotherTrackId();
            //firstchild = track.getFirstDaughterTrackId();
            //lastchild = track.getLastDaughterTrackId();

            particleid = track.GetPdgCode();
            ishadron = isHadron(TMath::Abs(particleid));
            h->fh_ptAll->Fill(track.GetPt());
            h->fh_etaAll->Fill(track.GetEta());
            h->fh_phiAll->Fill(track.GetPhi());

            //if(track.isPrimary()) { // This one will select only incoming p and n for AMPT.
            //if(isPhysicalPrimary(*mctrack, track)) { // Will be implemented for Kine
            //if(MC::isPhysicalPrimary((TMCParticles&)0x0, (TMCParticle)0x0) {
            //if((ishadron || particleid==22) && parent==-1 && firstchild==-1 && lastchild==-1) {
            if((ishadron || particleid==22) && track.isPrimary()) {
            //if(true) {
                px = track.Px(); // mStartVertexMomentumX
                py = track.Py(); // mStartVertexMomentumY
                pz = track.Pz(); // mStartVertexMomentumZ
                E = track.GetEnergy(); 
                h->fh_pt->Fill(track.GetPt());
                h->fh_eta->Fill(track.GetEta());
                h->fh_phi->Fill(track.GetPhi());
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
                charge=0;
                pdgParticle = TDatabasePDG::Instance()->GetParticle(particleid);
                if(pdgParticle==0x0) {
                    std::cout << "Error, no pdg info found for particle id " << particleid 
                              << " in evt " << ient << std::endl;
                    return;
                }
                charge = TDatabasePDG::Instance()->GetParticle(particleid)->Charge();
                ntuple->Fill(ient, particleid, px, py, pz, E, charge, ishadron);
            }
        }
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
bool isHadron(int particleid) {
    if (particleid <= 100 || (particleid >= 1000000 && particleid <= 9000000) || particleid >= 9900000)
        return false;
    if (particleid == 130 || particleid == 310)
        return true;
    if (particleid%10 == 0 || (particleid/10)%10 == 0 || (particleid/100)%10 == 0)
        return false;
    return true;

}




// List of unkonwn info:
// particle statusCode?   //Answer=TAmpt.cxx sets status always to 1.
// paritlce producedByGenerator?
// particle mother0=first mother =? getMotherTrackId?
//               ====  Mothers and daughters are set to -1 always in AMPT!
// Ported from AliRoot AliStack::IsPhysicalPrimary
// Particle.index() ?
/*
template <typename TMCParticle, typename TMCParticles>
bool isPhysicalPrimary(std::vector<o2::MCTrack>& mcParticles, o2::MCTrack const& particle)
{
  // Test if a particle is a physical primary according to the following definition:
  // Particles produced in the collision including products of strong and
  // electromagnetic decay and excluding feed-down from weak decays of strange
  // particles.

  //LOGF(debug, "isPhysicalPrimary for %d", particle.index());

  const int ist = 1; //particle.statusCode();
  const int pdg = std::abs(particle.GetPdgCode());
  const int mother0 = particle.getMotherTrackId(); //Was: particle.mother0()
  const int daughter0 = particle.getFirstDaughterTrackId(); //Was: particle.daughter0()
  // AliGenAmpt.h says: "if has no mothers then it was created by AMPT"
  const bool producedByGenerator = mother0==-1; //Was: particle.producedByGenerator()

  // Initial state particle
  // Solution for K0L decayed by Pythia6
  // ->
  if ((ist > 1) && (pdg != 130) && producedByGenerator) {
    return false;
  }
  if ((ist > 1) && !producedByGenerator) {
    return false;
  }
  // <-

  if (!isStable(pdg)) {
    return false;
  }

  if (producedByGenerator) {
    // Solution for K0L decayed by Pythia6
    // ->
    if (mother0 != -1) {
      auto mother = mcParticles.at(mother0); //Was:iteratorAt(mother0)
      if (std::abs(mother.GetPdgCode()) == 130) {
        return false;
      }
    }
    // <-
    // check for direct photon in parton shower
    // ->
    if (pdg == 22 && daughter0 != -1) {
      LOGF(debug, "D %d", daughter0);
      auto daughter = mcParticles.at(particle.getFirstDaughterTrackId());
      if (daughter.GetPdgCode() == 22) {
        return false;
      }
    }
    // <-
    return true;
  }

  // Particle produced during transport

  LOGF(debug, "M0 %d %d", producedByGenerator, mother0);
  auto mother = mcParticles.at(mother0);
  int mpdg = std::abs(mother.GetPdgCode());
  const int motherMother0 = mother.getMotherTrackId();
  const bool motherProducedByGenerator = motherMother0==-1;

  // Check for Sigma0
  if ((mpdg == 3212) && motherProducedByGenerator) {
    return true;
  }

  // Check if it comes from a pi0 decay
  if ((mpdg == kPi0) && motherProducedByGenerator) {
    return true;
  }

  // Check if this is a heavy flavor decay product
  int mfl = int(mpdg / std::pow(10, int(std::log10(mpdg))));

  // Light hadron
  if (mfl < 4) {
    return false;
  }

  // Heavy flavor hadron produced by generator
  if (motherProducedByGenerator) {
    return true;
  }

  // To be sure that heavy flavor has not been produced in a secondary interaction
  // Loop back to the generated mother
  LOGF(debug, "M0 %d %d", motherProducedByGenerator, motherMother0);
  while (motherMother0 != -1 && !motherProducedByGenerator) {
    mother = mcParticles.at(motherMother0);
    LOGF(debug, "M+ %d %d", motherProducedByGenerator, motherMother0);
    mpdg = std::abs(mother.GetPdgCode());
    mfl = int(mpdg / std::pow(10, int(std::log10(mpdg))));
  }

  if (mfl < 4) {
    return false;
  } else {
    return true;
  }
}


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

*/
